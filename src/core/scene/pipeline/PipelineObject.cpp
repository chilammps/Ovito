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
#include <core/dataset/UndoStack.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, PipelineObject, DataObject);
DEFINE_REFERENCE_FIELD(PipelineObject, _sourceObject, "InputObject", DataObject);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(PipelineObject, _modApps, "ModifierApplications", ModifierApplication, PROPERTY_FIELD_ALWAYS_CLONE);
SET_PROPERTY_FIELD_LABEL(PipelineObject, _sourceObject, "Input");
SET_PROPERTY_FIELD_LABEL(PipelineObject, _modApps, "Modifier Applications");

/******************************************************************************
* Default constructor.
******************************************************************************/
PipelineObject::PipelineObject(DataSet* dataset) : DataObject(dataset), _cachedIndex(-1)
{
	INIT_PROPERTY_FIELD(PipelineObject::_sourceObject);
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
	// Prevent the recoding of transient operations while evaluating the pipeline.
	UndoSuspender undoSuspender(dataset()->undoStack());

	// Cannot evaluate a pipeline that doesn't have an input.
	if(!sourceObject())
		return PipelineFlowState();

	// Determine the position in the pipeline up to which it should be evaluated.
	int upToHereIndex;
	if(upToHere != nullptr) {
		upToHereIndex = modifierApplications().indexOf(upToHere);
		OVITO_ASSERT(upToHereIndex != -1);
		if(including) upToHereIndex++;
	}
	else upToHereIndex = modifierApplications().size();

	// Receive the input data from the source object.
	PipelineFlowState inputState = sourceObject()->evaluate(time);

	// Determine the modifier from which on to evaluate the pipeline.
	int fromHereIndex = 0;
	PipelineFlowState flowState = inputState;

	// Use the cached results if possible.
	// First check if the cache is filled.
	if(_cachedIndex >= 0 && _cachedIndex <= upToHereIndex &&
			_cachedState.stateValidity().contains(time) &&
			_lastInput.stateValidity().contains(time)) {

		// Check if there have been any changes in the input data
		// since the cache has been filled.
		// If any of the input objects has been replaced, removed, newly added,
		// or changed, then the cache is considered invalid.

		// Can use cached state only if none of the input objects have been replaced or modified.
		if(_lastInput.objects() == inputState.objects()) {
			// Also check if the auxiliary attributes have not changed.
			if(_lastInput.attributes() == inputState.attributes()) {
				// Use cached state.
				fromHereIndex = _cachedIndex;
				flowState = _cachedState;
				flowState.intersectStateValidity(inputState.stateValidity());
			}
		}
	}

	// Reset cache, then regenerate it below.
	_cachedState.clear();
	_cachedIndex = -1;

	// Store the input state, so we can detect changes in the input next time the
	// pipeline is evaluated.
	_lastInput = inputState;

	// Flag that indicates whether the output of the pipeline is considered incomplete.
	bool isPending = (flowState.status().type() == PipelineStatus::Pending);

    // Apply the modifiers one by one.
	for(int stackIndex = fromHereIndex; stackIndex < upToHereIndex; stackIndex++) {

		// Skip further processing steps if flow state became empty.
		if(flowState.isEmpty())
			break;

		ModifierApplication* app = modifierApplications()[stackIndex];
    	OVITO_CHECK_OBJECT_POINTER(app);

		Modifier* mod = app->modifier();
		OVITO_CHECK_OBJECT_POINTER(mod);

		// Skip disabled modifiers.
		if(mod->isEnabled() == false)
			continue;

		// Save current flow state in cache at this point of the pipeline
		// if the next modifier is changing frequently (because it is currently being edited).
		if(mod->modifierValidity(time).isEmpty()) {
			_cachedState = flowState;
			_cachedState.updateRevisionNumbers();
			_cachedIndex = stackIndex;
		}

		// Apply modifier.
		PipelineStatus modifierStatus = mod->modifyObject(time, app, flowState);
		if(modifierStatus.type() == PipelineStatus::Pending)
			isPending = true;
		else if(isPending)
			modifierStatus = PipelineStatus::Pending;

		// Give precedence to error status.
		if(flowState.status().type() != PipelineStatus::Error || isPending)
			flowState.setStatus(modifierStatus);
	}

	// Make sure the revision information in the output is up to date.
	flowState.updateRevisionNumbers();

	// Cache the pipeline output (if not already cached an intermediate state of the pipeline).
	if(_cachedIndex < 0 && flowState.isEmpty() == false) {
		_cachedState = flowState;
		_cachedIndex = upToHereIndex;
	}

	return flowState;
}

/******************************************************************************
* Inserts the given modifier into this object.
******************************************************************************/
ModifierApplication* PipelineObject::insertModifier(Modifier* modifier, int atIndex)
{
	OVITO_CHECK_OBJECT_POINTER(modifier);
	OVITO_ASSERT(modifier->dataset() == this->dataset());

	// Create a modifier application object.
	OORef<ModifierApplication> modApp(new ModifierApplication(dataset(), modifier));
	insertModifierApplication(modApp, atIndex);
	return modApp;
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
	if(source == sourceObject()) {
		if(event->type() == ReferenceEvent::TargetChanged || event->type() == ReferenceEvent::PendingStateChanged) {
			// If the source object changed, all modifiers need to be informed that
			// their input has changed.
			modifierChanged(-1);
		}
		else if(event->type() == ReferenceEvent::TitleChanged) {
			// Propagate title changed events from the source object on to the ObjectNode.
			notifyDependents(ReferenceEvent::TitleChanged);
		}
	}
	else {
		if(event->type() == ReferenceEvent::TargetChanged ||
			event->type() == ReferenceEvent::PendingStateChanged) {
			// If one of the modifiers has changed, then all subsequent
			// modifiers in the pipeline need to be informed (unless it's from a disabled modifier).
			int index = _modApps.indexOf(source);
			if(index != -1) {
				Modifier* mod = modifierApplications()[index]->modifier();
				if(mod && mod->isEnabled())
					modifierChanged(index);
			}
		}
		else if(event->type() == ReferenceEvent::TargetEnabledOrDisabled) {
			// If one of the modifiers gets enabled/disabled, then all subsequent
			// modifiers in the pipeline need to be informed.
			int index = _modApps.indexOf(source);
			if(index != -1) {
				modifierChanged(index);
				// We also consider this a change of the modification pipeline itself.
				notifyDependents(ReferenceEvent::TargetChanged);
			}
		}
	}
	return DataObject::referenceEvent(source, event);
}

/******************************************************************************
* Is called when a reference target has been added to a list reference field of
* this RefMaker.
******************************************************************************/
void PipelineObject::referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex)
{
	// If a new modifier has been inserted into the pipeline, then all
	// following modifiers need to be informed.
	if(field == PROPERTY_FIELD(PipelineObject::_modApps)) {

		// Also inform the new modifier itself that its input has changed
		// because it is being inserted into a pipeline.
		ModifierApplication* app = static_object_cast<ModifierApplication>(newTarget);
		if(app && app->modifier())
			app->modifier()->upstreamPipelineChanged(app);

		// Inform all subsequent modifiers that their input has changed.
		modifierChanged(listIndex);
	}
	DataObject::referenceInserted(field, newTarget, listIndex);
}

/******************************************************************************
* Is called when a reference target has been removed from a list reference
* field of this RefMaker.
******************************************************************************/
void PipelineObject::referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex)
{
	if(field == PROPERTY_FIELD(PipelineObject::_modApps)) {
		// If a modifier is being removed from the pipeline, then all
		// modifiers following it need to be informed.
		modifierChanged(listIndex - 1);
	}
	DataObject::referenceRemoved(field, oldTarget, listIndex);
}

/******************************************************************************
* Is called when the value of a reference field of this RefMaker changes.
******************************************************************************/
void PipelineObject::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(PipelineObject::_sourceObject)) {
		// Invalidate cache if input object has been replaced.
		modifierChanged(-1);
	}
	DataObject::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* This function is called when a part of the pipeline (or its source) have changed.
* Notifies all modifiers starting at the given index that their input has changed.
******************************************************************************/
void PipelineObject::modifierChanged(int changedIndex)
{
	OVITO_ASSERT(changedIndex >= -1 && changedIndex < modifierApplications().size());

	// Ignore signal while modifiers are being loaded.
	if(isBeingLoaded())
		return;

	// Invalidate the data cache if it contains a state that
	// is affected by the changing modifier.
	if(changedIndex < _cachedIndex) {
		_lastInput.clear();
		_cachedState.clear();
		_cachedIndex = -1;
	}

	// Inform modifiers following the changing modifier in the
	// modification pipeline that their input has changed.
	while(++changedIndex < modifierApplications().size()) {
		ModifierApplication* app = modifierApplications()[changedIndex];
		if(app && app->modifier())
			app->modifier()->upstreamPipelineChanged(app);
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
