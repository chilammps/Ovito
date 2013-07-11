///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#include <core/Core.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/gui/undo/UndoManager.h>

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, PipelineObject, SceneObject)
DEFINE_REFERENCE_FIELD(PipelineObject, _inputObject, "InputObject", SceneObject)
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(PipelineObject, _modApps, "ModifierApplications", ModifierApplication, PROPERTY_FIELD_ALWAYS_CLONE)
SET_PROPERTY_FIELD_LABEL(PipelineObject, _inputObject, "Input")
SET_PROPERTY_FIELD_LABEL(PipelineObject, _modApps, "Modifier Applications")

/******************************************************************************
* Default constructor.
******************************************************************************/
PipelineObject::PipelineObject() : _cacheIndex(-1)
{
	INIT_PROPERTY_FIELD(PipelineObject::_inputObject);
	INIT_PROPERTY_FIELD(PipelineObject::_modApps);
}

/******************************************************************************
* Asks the object for the result of the geometry pipeline at the given time
* up to a given point in the modifier stack.
* If upToHere is NULL then the complete modifier stack will be evaluated.
* Otherwise only the modifiers in the pipeline before the given point will be
* applied to the input object. The 'include' parameter specifies whether the
* last modifier given by 'upToHere' will be applied.
******************************************************************************/
PipelineFlowState PipelineObject::evaluatePipeline(TimePoint time, ModifierApplication* upToHere, bool including)
{
	UndoSuspender undoSuspender;	// Do not create undo records for any of this.

	if(!inputObject())
		return PipelineFlowState();	// Cannot evaluate pipeline if there is no input.

	// Determine the index up to which the pipeline should be evaluated.
	int upToHereIndex;
	if(upToHere != nullptr) {
		upToHereIndex = modifierApplications().indexOf(upToHere);
		OVITO_ASSERT(upToHereIndex != -1);
		if(including) upToHereIndex++;
	}
	else upToHereIndex = modifierApplications().size();

	// Evaluate the input object.
	PipelineFlowState inputState = inputObject()->evaluate(time);

	// The index from which on the stack should be evaluated.
	int fromHereIndex = 0;
	PipelineFlowState flowState = inputState;

	// Use the cached results if possible. First check if the cache is filled.
	if(_cacheIndex >= 0 && _cacheIndex <= upToHereIndex &&
			_cachedModifiedState.stateValidity().contains(time) &&
			_lastInput.stateValidity().contains(time) &&
			_lastInput.count() == inputState.count()) {

		// Check if there have been any changes in the input data
		// since the last time the pipeline was evaluated.
		// If any of the input objects has been replaced, removed, or newly added,
		// this change is considered significant and the cache becomes invalid.
		// If only the revision of some input objects has changed, the modifiers
		// are asked whether they depend on these input objects. If yes,
		// the cache is considered invalid and a full re-evaluation of the pipeline
		// is triggered. If not, however, the pipeline doesn't need to be re-evaluated
		// and we can directly update the cache with the changed input objects.

		// Need to re-evaluate pipeline if any of the input objects have been replaced.
		bool cacheIsValid = (_lastInput.objects() == inputState.objects());

		if(cacheIsValid) {
			for(int index = 0; index < inputState.count(); index++) {
				// If the revision number of an input object has changed, we ask the modifiers
				// if they depend on this input object.
				if(inputState.revisionNumber(index) != _lastInput.revisionNumber(index)) {
					for(int stackIndex = fromHereIndex; stackIndex < _cacheIndex; stackIndex++) {
						Modifier* modifier = modifierApplications()[stackIndex]->modifier();
						OVITO_CHECK_OBJECT_POINTER(modifier);
						if(modifier->isEnabled() == false)
							continue;
						if(modifier->dependsOnInput(inputState.objects()[index].get())) {
							cacheIsValid = false;
							break;
						}
					}
					if(!cacheIsValid)
						break;
				}
			}
		}

		if(cacheIsValid) {
			// Use cached state.
			fromHereIndex = _cacheIndex;
			flowState = _cachedModifiedState;
			flowState.intersectStateValidity(inputState.stateValidity());
		}
	}

	// Clear cache and then regenerate it below.
	_cachedModifiedState.clear();
	_cacheIndex = -1;

	// Save the input state as a reference for the next pipeline evaluation.
	_lastInput = inputState;

	bool isPending = (flowState.status().type() == ObjectStatus::Pending);

    // Apply the modifiers, one after another.
	int stackIndex;
	for(stackIndex = fromHereIndex; stackIndex < upToHereIndex; stackIndex++) {
    	ModifierApplication* app = modifierApplications()[stackIndex];
    	OVITO_CHECK_OBJECT_POINTER(app);

		Modifier* mod = app->modifier();
		OVITO_CHECK_OBJECT_POINTER(mod);

		// Skip disabled modifiers.
		if(mod->isEnabled() == false)
			continue;

		// Save current evaluation result in cache at this point of the pipeline
		// if the next modifier is changing frequently (because it is currently being edited).
		if(mod->modifierValidity(time).isEmpty()) {
			_cachedModifiedState = flowState;
			_cacheIndex = stackIndex;
		}

		if(flowState.isEmpty() == false) {
			// Apply modifier to current flow state.
			ObjectStatus modifierStatus = mod->modifyObject(time, app, flowState);
			isPending |= (modifierStatus.type() == ObjectStatus::Pending);
		}
	}

	flowState.updateRevisionNumbers();
	if(isPending)
		flowState.setStatus(ObjectStatus::Pending);

	// Cache the final results.
	if(_cacheIndex < 0 && flowState.isEmpty() == false) {
		_cachedModifiedState = flowState;
		_cacheIndex = stackIndex;
	}
	else _cachedModifiedState.updateRevisionNumbers();

	return flowState;
}

/******************************************************************************
* Inserts the given modifier into this object.
******************************************************************************/
ModifierApplication* PipelineObject::insertModifier(Modifier* modifier, int atIndex)
{
	OVITO_CHECK_OBJECT_POINTER(modifier);

	// Create a modifier application object.
	OORef<ModifierApplication> modApp(new ModifierApplication(modifier));
	insertModifierApplication(modApp.get(), atIndex);
	return modApp.get();
}

/******************************************************************************
* Inserts the given modifier into this object.
******************************************************************************/
void PipelineObject::insertModifierApplication(ModifierApplication* modApp, int atIndex)
{
	OVITO_ASSERT(atIndex >= 0);
	OVITO_CHECK_OBJECT_POINTER(modApp);
	atIndex = std::min(atIndex, modifierApplications().size());
	_modApps.insert(atIndex, modApp);

	if(modApp->modifier())
		modApp->modifier()->initializeModifier(this, modApp);
}

/******************************************************************************
* Removes the given modifier application.
******************************************************************************/
void PipelineObject::removeModifier(ModifierApplication* app)
{
	OVITO_CHECK_OBJECT_POINTER(app);
	OVITO_ASSERT(app->pipelineObject() == this);

	int index = _modApps.indexOf(app);
	OVITO_ASSERT(index >= 0);

	_modApps.remove(index);
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool PipelineObject::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == inputObject()) {
		if(event->type() == ReferenceEvent::TargetChanged ||
			event->type() == ReferenceEvent::PendingOperationSucceeded ||
			event->type() == ReferenceEvent::PendingOperationFailed) {
			modifierChanged(-1);
		}
	}
	else {
		if(event->type() == ReferenceEvent::TargetChanged ||
			event->type() == ReferenceEvent::TargetEnabledOrDisabled ||
			event->type() == ReferenceEvent::PendingOperationSucceeded ||
			event->type() == ReferenceEvent::PendingOperationFailed) {
			// If one of the modifiers has changed then all other modifiers
			// following it in the stack need to be informed.
			int index = _modApps.indexOf(source);
			if(index != -1) {
				modifierChanged(index);
				if(event->type() == ReferenceEvent::TargetEnabledOrDisabled)
					notifyDependents(ReferenceEvent::TargetChanged);
			}
		}
	}
	return SceneObject::referenceEvent(source, event);
}

/******************************************************************************
* Is called when a reference target has been added to a list reference field of this RefMaker.
******************************************************************************/
void PipelineObject::referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex)
{
	// If a new modifier has been inserted into the stack then all
	// modifiers following it in the stack need to be informed.
	if(field == PROPERTY_FIELD(PipelineObject::_modApps)) {

		// Inform modifier that its input has changed when it is inserted into the pipeline.
		ModifierApplication* app = static_object_cast<ModifierApplication>(newTarget);
		if(app && app->modifier())
			app->modifier()->modifierInputChanged(app);

		// Inform all subsequent modifiers that their input has changed.
		modifierChanged(listIndex);
	}
	SceneObject::referenceInserted(field, newTarget, listIndex);
}

/******************************************************************************
* Is called when a reference target has been removed from a list reference field of this RefMaker.
******************************************************************************/
void PipelineObject::referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex)
{
	// If a new modifier has been removed from the stack then all
	// modifiers following it in the stack need to be informed.
	if(field == PROPERTY_FIELD(PipelineObject::_modApps)) {
		modifierChanged(listIndex - 1);
	}
	SceneObject::referenceRemoved(field, oldTarget, listIndex);
}

/******************************************************************************
* Is called when the value of a reference field of this RefMaker changes.
******************************************************************************/
void PipelineObject::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(PipelineObject::_inputObject)) {
		// Invalidate cache if input object has been replaced.
		modifierChanged(-1);
	}
	SceneObject::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* Notifies all modifiers from the given index on that their input has changed.
******************************************************************************/
void PipelineObject::modifierChanged(int changedIndex)
{
	if(isBeingLoaded())
		return;	// Do not nothing while modifiers are being loaded.

	OVITO_ASSERT(changedIndex >= -1 && changedIndex < modifierApplications().size());

	// Invalidate the internal cache if it stores a state that was produced by the changing modifier.
	if(changedIndex < _cacheIndex) {
		_lastInput.clear();
		_cachedModifiedState.clear();
		_cacheIndex = -1;
	}

	// Call the modifierInputChanged() method for all affected modifiers.
	while(++changedIndex < modifierApplications().size()) {
		ModifierApplication* app = modifierApplications()[changedIndex];
		if(app && app->modifier()) {
			OVITO_CHECK_OBJECT_POINTER(app->modifier());
			app->modifier()->modifierInputChanged(app);
		}
	}
}

};
