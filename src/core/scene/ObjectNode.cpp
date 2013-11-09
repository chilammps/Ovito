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
#include <core/scene/ObjectNode.h>
#include <core/scene/objects/SceneObject.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/viewport/Viewport.h>
#include <core/gui/undo/UndoManager.h>

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ObjectNode, SceneNode)
DEFINE_REFERENCE_FIELD(ObjectNode, _sceneObject, "SceneObject", SceneObject)
DEFINE_VECTOR_REFERENCE_FIELD(ObjectNode, _displayObjects, "DisplayObjects", DisplayObject)
SET_PROPERTY_FIELD_LABEL(ObjectNode, _sceneObject, "Object")

/******************************************************************************
* Constructor.
******************************************************************************/
ObjectNode::ObjectNode(SceneObject* object)
{
	INIT_PROPERTY_FIELD(ObjectNode::_sceneObject);
	INIT_PROPERTY_FIELD(ObjectNode::_displayObjects);
	setSceneObject(object);
}

/******************************************************************************
* Evaluates the geometry pipeline of this scene node at the given time.
******************************************************************************/
const PipelineFlowState& ObjectNode::evalPipeline(TimePoint time)
{
	// Check if the cache needs to be updated.
	if(_pipelineCache.stateValidity().contains(time) == false) {
		if(sceneObject()) {

			// Do not record any object creation operations while evaluating the pipeline.
			UndoSuspender noUndo;

			// Evaluate object and save result in local cache.
			_pipelineCache = sceneObject()->evaluate(time);

			// Update list of display objects.

			// First unlink those display objects from the node which are no longer needed.
			for(int i = displayObjects().size() - 1; i >= 0; i--) {
				DisplayObject* displayObj = displayObjects()[i];
				// Check if display object is still being used by any of the scene objects that came out of the pipeline.
				bool isAlive = false;
				for(const auto& sceneObj : _pipelineCache.objects()) {
					if(sceneObj->displayObjects().contains(displayObj)) {
						isAlive = true;
						break;
					}
				}
				// Discard display object if no longer needed.
				if(!isAlive)
					_displayObjects.remove(i);
			}

			// Now add new display objects to this node.
			for(const auto& sceneObj : _pipelineCache.objects()) {
				for(DisplayObject* displayObj : sceneObj->displayObjects()) {
					OVITO_CHECK_OBJECT_POINTER(displayObj);
					if(displayObjects().contains(displayObj) == false)
						_displayObjects.push_back(displayObj);
				}
			}
		}
		else {
			// Clear cache if this node is empty.
			invalidatePipelineCache();
			// Discard display objects as well.
			_displayObjects.clear();
		}
	}
	return _pipelineCache;
}

/******************************************************************************
* This method invalidates the geometry pipeline cache of the object node.
******************************************************************************/
void ObjectNode::invalidatePipelineCache()
{
	_pipelineCache.clear();
}

/******************************************************************************
* Renders the node's scene objects.
******************************************************************************/
void ObjectNode::render(TimePoint time, SceneRenderer* renderer)
{
	const PipelineFlowState& state = evalPipeline(time);
	for(const auto& sceneObj : state.objects()) {
		for(DisplayObject* displayObj : sceneObj->displayObjects()) {
			if(displayObj && displayObj->isEnabled()) {
				displayObj->render(time, sceneObj.get(), state, renderer, this);
			}
		}
	}
}

/******************************************************************************
* This method is called when a referenced object has changed.
******************************************************************************/
bool ObjectNode::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == sceneObject()) {
		if(event->type() == ReferenceEvent::TargetChanged || event->type() == ReferenceEvent::PendingStateChanged) {
			invalidatePipelineCache();
			invalidateBoundingBox();
		}
		else if(event->type() == ReferenceEvent::TargetDeleted) {
			// Object has been deleted -> delete node too.
			if(!UndoManager::instance().isUndoingOrRedoing())
				deleteNode();
		}
		else if(event->type() == ReferenceEvent::TitleChanged) {
			notifyDependents(ReferenceEvent::TitleChanged);
		}
	}
	return SceneNode::referenceEvent(source, event);
}

/******************************************************************************
* Gets called when the scene object of the node has been replaced.
******************************************************************************/
void ObjectNode::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(ObjectNode::_sceneObject)) {
		invalidatePipelineCache();
		invalidateBoundingBox();

		// When the scene object is being replaced, the pending state of the node might change.
		// Even though we don't know for sure if the state has really changed, we send a notification event here.
		notifyDependents(ReferenceEvent::PendingStateChanged);
	}

	SceneNode::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* Returns the bounding box of the object node in local coordinates.
******************************************************************************/
Box3 ObjectNode::localBoundingBox(TimePoint time)
{
	Box3 bb;
	const PipelineFlowState& state = evalPipeline(time);

	// Compute bounding boxes of scene objects.
	for(const auto& sceneObj : state.objects()) {
		for(DisplayObject* displayObj : sceneObj->displayObjects()) {
			if(displayObj && displayObj->isEnabled())
				bb.addBox(displayObj->boundingBox(time, sceneObj.get(), this, state));
		}
	}

	return bb;
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void ObjectNode::saveToStream(ObjectSaveStream& stream)
{
	SceneNode::saveToStream(stream);
	stream.beginChunk(0x01);
	// For future use...
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void ObjectNode::loadFromStream(ObjectLoadStream& stream)
{
	SceneNode::loadFromStream(stream);
	stream.expectChunk(0x01);
	// For future use...
	stream.closeChunk();
}

/******************************************************************************
* Returns the title of this object.
******************************************************************************/
QString ObjectNode::objectTitle()
{
	// If a name has been assigned to this node, return it as display title.
	if(name().isEmpty() == false)
		return name();

	// Otherwise, use the display title of the node's source scene object.
	SceneObject* sceneObj = sceneObject();
	while(sceneObj) {
		if(sceneObj->inputObjectCount() == 0)
			return sceneObj->objectTitle();
		sceneObj = sceneObj->inputObject(0);
	}

	return SceneNode::objectTitle();
}


/******************************************************************************
* Applies the given modifier to the object node.
* The modifier is put on top of the modifier stack.
******************************************************************************/
void ObjectNode::applyModifier(Modifier* modifier)
{
	OVITO_CHECK_OBJECT_POINTER(sceneObject());
	if(!sceneObject())
		throw Exception("Cannot apply modifier to an empty object node.");

	PipelineObject* pipelineObj = dynamic_object_cast<PipelineObject>(sceneObject());
	if(!pipelineObj) {
		OORef<PipelineObject> p = new PipelineObject();
		p->setInputObject(sceneObject());
		setSceneObject(p);
		pipelineObj = p.get();
	}
	pipelineObj->insertModifier(modifier, pipelineObj->modifierApplications().size());
}

};
