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
PipelineObject::PipelineObject() : _pipelineCacheIndex(-1)
{
	INIT_PROPERTY_FIELD(PipelineObject::_inputObject);
	INIT_PROPERTY_FIELD(PipelineObject::_modApps);
}

/******************************************************************************
* Asks the object for its validity interval at the given time.
******************************************************************************/
TimeInterval PipelineObject::objectValidity(TimePoint time)
{
	return TimeInterval::forever();
}

#if 0
/******************************************************************************
* Render the object into the viewport.
******************************************************************************/
void PipelineObject::renderObject(TimePoint time, ObjectNode* contextNode, Viewport* vp)
{
	// A ModifiedObject should never be the result of the geometry pipeline.
	OVITO_ASSERT_MSG(false, "ModifiedObject::renderObject()", "A ModifiedObject should not be rendered in the viewports.");
}
#endif

/******************************************************************************
* Returns the bounding box of the object in local object coordinates.
******************************************************************************/
Box3 PipelineObject::boundingBox(TimePoint time, ObjectNode* contextNode)
{
	// A PipelineObject should never be the result of the geometry pipeline.
	OVITO_ASSERT_MSG(false, "PipelineObject::boundingBox()", "A PipelineObject should not be rendered in the viewports.");
	return Box3();
}

/******************************************************************************
* Asks the object for the result of the geometry pipeline at the given time
* up to a given point in the modifier stack.
* If upToHere is NULL then the complete modifier stack will be evaluated.
* Otherwise only the modifiers in the pipeline before the given point will be applied to the
* result. The 'include' specifies whether the last modifier given by 'upToHere'
* will be applied.
******************************************************************************/
PipelineFlowState PipelineObject::evalObject(TimePoint time, ModifierApplication* upToHere, bool including)
{
	UndoSuspender undoSuspender;	// Do not create undo records for this.
	PipelineFlowState state;

	// The index up to which to evaluate the stack.
	int upToHereIndex = modifierApplications().size();
	if(upToHere != NULL) {
		upToHereIndex = modifierApplications().indexOf(upToHere);
		if(including) upToHereIndex++;
	}

	// The index from which on the stack should be evaluated.
	int fromHereIndex = 0;
	// Use the cached result if possible.
	if(_pipelineCacheIndex >= 0 && _pipelineCacheIndex <= upToHereIndex && _pipelineCache.stateValidity().contains(time)) {
		fromHereIndex = _pipelineCacheIndex;
		state = _pipelineCache;
	}
	else {
		// Evaluate the geometry pipeline of the input object.
		if(!inputObject())
			return PipelineFlowState();	// No input object -> Cannot evaluate pipeline.
		state = inputObject()->evalObject(time);
	}

	// Clear cache and then regenerate it below.
	invalidatePipelineCache();

    // Apply the modifiers, one after another.
	int stackIndex;
	for(stackIndex = fromHereIndex; stackIndex < upToHereIndex; stackIndex++) {
    	ModifierApplication* app = modifierApplications()[stackIndex];
    	OVITO_CHECK_OBJECT_POINTER(app);

		// Skip disabled modifiers.
		if(app->isEnabled() == false) {
	    	// Reset evaluation status.
	    	app->setStatus(EvaluationStatus());
			continue;
		}

		Modifier* mod = app->modifier();
		OVITO_CHECK_OBJECT_POINTER(mod);

		// Put evaluation result into cache if the next modifier is changing frequently (because it is being edited).
		if(mod->modifierValidity(time).isEmpty()) {
			_pipelineCache = state;
			_pipelineCacheIndex = stackIndex;
		}

		// Apply modifier.
		if(state.result() != NULL) {
			app->setStatus(mod->modifyObject(time, app, state));
		}
		else {
			app->setStatus(EvaluationStatus(EvaluationStatus::EVALUATION_ERROR, tr("Modifier did not receive any input object.")));
		}
	}

	// Cache the final result.
	if(_pipelineCacheIndex < 0 && state.result() != NULL) {
		_pipelineCache = state;
		_pipelineCacheIndex = stackIndex;
	}

	return state;
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

	if(modApp->modifier()) {
		modApp->modifier()->initializeModifier(this, modApp);
	}
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
		if(event->type() == ReferenceEvent::TargetChanged) {
			// If the input object has changed then the whole modifier stack needs
			// to be informed of this.
			notifyModifiersInputChanged(-1);
		}
	}
	else {
		if(event->type() == ReferenceEvent::TargetChanged || event->type() == ReferenceEvent::TargetEnabledOrDisabled) {
			// If one of the modifiers has changed then all other modifiers
			// following it in the stack need to be informed.
			int index = _modApps.indexOf(source);
			if(index != -1) {
				notifyModifiersInputChanged(index);
				if(event->type() == ReferenceEvent::TargetEnabledOrDisabled)
					notifyDependents(ReferenceEvent::TargetChanged);
			}
		}
	}
	return SceneObject::referenceEvent(source, event);
}

/******************************************************************************
* Is called when the value of a reference field of this RefMaker changes.
******************************************************************************/
void PipelineObject::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	// If the input object has been replaced then the whole modifier stack needs
	// to be informed.
	if(field == PROPERTY_FIELD(PipelineObject::_inputObject)) {
		notifyModifiersInputChanged(-1);
	}
	SceneObject::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* Is called when a reference target has been added to a list reference field of this RefMaker.
******************************************************************************/
void PipelineObject::referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex)
{
	// If a new modifier has been inserted into the stack then all
	// modifiers following it in the stack need to be informed.
	if(field == PROPERTY_FIELD(PipelineObject::_modApps)) {
		notifyModifiersInputChanged(listIndex);
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
		notifyModifiersInputChanged(listIndex - 1);
	}
	SceneObject::referenceRemoved(field, oldTarget, listIndex);
}

/******************************************************************************
* Notifies all modifiers from the given index on that their input has changed.
******************************************************************************/
void PipelineObject::notifyModifiersInputChanged(int changedIndex)
{
	if(isBeingLoaded())
		return;	// Do not send notification messages while modifiers are being loaded.

	OVITO_ASSERT(changedIndex >= -1 && changedIndex < modifierApplications().size());

	// Invalidate the internal cache if it contains a state behind the changed modifier.
	if(changedIndex < _pipelineCacheIndex)
		invalidatePipelineCache();

	// Call the onInputChanged() method for all affected modifiers.
	while(++changedIndex < modifierApplications().size()) {
		ModifierApplication* app = modifierApplications()[changedIndex];
		if(app && app->modifier()) {
			OVITO_CHECK_OBJECT_POINTER(app->modifier());
			app->modifier()->inputChanged(app);
		}
	}
}

};
