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

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(PipelineObject, SceneObject)
DEFINE_REFERENCE_FIELD(PipelineObject, _inputObject, "InputObject", SceneObject)
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(PipelineObject, _modApps, "ModifierApplications", ModifierApplication, PROPERTY_FIELD_ALWAYS_CLONE)
SET_PROPERTY_FIELD_LABEL(PipelineObject, _inputObject, "Input")
SET_PROPERTY_FIELD_LABEL(PipelineObject, _modApps, "Modifier Applications")

/******************************************************************************
* Default constructor.
******************************************************************************/
PipelineObject::PipelineObject()
{
	INIT_PROPERTY_FIELD(PipelineObject::_inputObject);
	INIT_PROPERTY_FIELD(PipelineObject::_modApps);

#if 0
	_cacheIndex = -1;
#endif
}

/******************************************************************************
* Asks the object for the result of the geometry pipeline at the given time
* up to a given point in the modifier stack.
* If upToHere is NULL then the complete modifier stack will be evaluated.
* Otherwise only the modifiers in the pipeline before the given point will be
* applied to the input object. The 'include' parameter specifies whether the
* last modifier given by 'upToHere' will be applied.
******************************************************************************/
PipelineFlowState PipelineObject::evalObject(TimePoint time, ModifierApplication* upToHere, bool including)
{
	UndoSuspender undoSuspender;	// Do not create undo records for any of this.

	if(!inputObject())
		return PipelineFlowState();	// Cannot evaluate pipeline if there is no input.

#if	0
	// Determine the index up to which the stack should be evaluated.
	int upToHereIndex = modifierApplications().size();
	if(upToHere != NULL) {
		upToHereIndex = modifierApplications().indexOf(upToHere);
		OVITO_ASSERT(upToHereIndex != -1);
		if(including) upToHereIndex++;
	}

	// Evaluate the input object.
	PipelineFlowState inputState = inputObject()->evaluate(time);

	// The index from which on the stack should be evaluated.
	int fromHereIndex = 0;
	PipelineFlowState flowState = inputState;

	// Use the cached results if possible. First check if the cache is filled.
	if(_cacheIndex >= 0 && _cacheIndex <= upToHereIndex &&
			_cachedModifiedState.stateValidity().contains(time) && !_cachedModifiedState.isEmpty() &&
			_lastInputState.stateValidity().contains(time) && _lastInputState.objects().size() == inputState.objects().size()) {

		// Check if there have been any changes in the input data
		// since the last time the pipeline was evaluated.
		// If any of the input objects has been replaced, removed, or newly added,
		// this change is considered significant and the cache is invalid.
		// If only the revision of some input objects has changed, the modifiers
		// are asked whether they depend on these input objects. If yes,
		// the cache is considered invalid and a full re-evaluation of the pipeline
		// is triggered. If no, however, the pipeline doesn't need to be re-evaluated
		// and we can directly update the cache with the changed input objects.
		bool cacheIsValid = true;
		auto eold = _lastInputState.objects().cbegin();
		auto enew = inputState.objects().cbegin();
		for(; eold != _lastInputState.objects().cend(); ++eold, ++enew) {
			// Need to re-evaluate pipeline if any of the input objects has been replaced.
			if(enew->first != eold->first) {
				cacheIsValid = false;
				break;
			}
			// If the revision number of an input has changed, we ask the modifiers
			// if they depend on this input object.
			if(enew->second != eold->second) {
				for(int stackIndex = fromHereIndex; stackIndex < _cacheIndex; stackIndex++) {
			    	ModifierApplication* app = modifierApplications()[stackIndex];
					if(app->isEnabled() == false)
						continue;
					Modifier* mod = app->modifier();
					OVITO_CHECK_OBJECT_POINTER(mod);
					if(mod->dependsOnInput(enew->first.get())) {
						cacheIsValid = false;
						break;
					}
				}
				if(cacheIsValid)
					break;

				// Update cache with the changed input object as no modifiers depend on it.
				_cachedModifiedState.setRevisionNumber(enew->first.get(), enew->second);
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
	invalidatePipelineCache();

	// Save the input state as a reference for the next pipeline evaluation.
	_lastInputState = inputState;

    // Apply the modifiers, one after another.
	int stackIndex;
	for(stackIndex = fromHereIndex; stackIndex < upToHereIndex; stackIndex++) {
    	ModifierApplication* app = modifierApplications()[stackIndex];
    	OVITO_CHECK_OBJECT_POINTER(app);

		// Skip disabled modifiers.
		if(app->isEnabled() == false) {
	    	// Reset evaluation status.
	    	app->setStatus(ObjectStatus());
			continue;
		}

		Modifier* mod = app->modifier();
		OVITO_CHECK_OBJECT_POINTER(mod);

		// Put evaluation result into cache if the next modifier is changing frequently (because it is being edited).
		if(mod->modifierValidity(time).isEmpty()) {
			_cachedModifiedState = flowState;
			_cacheIndex = stackIndex;
		}

		if(flowState.isEmpty() == false) {
			// Apply modifier to current flow state.
			app->setStatus(mod->modifyObject(time, app, flowState));
		}
		else {
			app->setStatus(ObjectStatus(ObjectStatus::Error, tr("Modifier has no input.")));
		}
	}

	// Cache the final results.
	if(_cacheIndex < 0 && flowState.isEmpty() == false) {
		_cachedModifiedState = flowState;
		_cacheIndex = stackIndex;
	}

	return flowState;
#endif

	return PipelineFlowState();
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
	if(source != inputObject()) {
		if(event->type() == ReferenceEvent::TargetChanged || event->type() == ReferenceEvent::TargetEnabledOrDisabled) {
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
* Notifies all modifiers from the given index on that their input has changed.
******************************************************************************/
void PipelineObject::modifierChanged(int changedIndex)
{
	if(isBeingLoaded())
		return;	// Do not nothing while modifiers are being loaded.

	OVITO_ASSERT(changedIndex >= -1 && changedIndex < modifierApplications().size());

#if 0
	// Invalidate the internal cache if it contains a state behind the changed modifier.
	if(changedIndex < _cacheIndex)
		invalidatePipelineCache();
#endif
}

};
