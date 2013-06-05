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
#include <core/viewport/Viewport.h>
#include <core/gui/undo/UndoManager.h>

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(ObjectNode, SceneNode)
DEFINE_REFERENCE_FIELD(ObjectNode, _sceneObject, "SceneObject", SceneObject)
DEFINE_PROPERTY_FIELD(ObjectNode, _objectTransform, "ObjectTransform")
SET_PROPERTY_FIELD_LABEL(ObjectNode, _sceneObject, "Object")
SET_PROPERTY_FIELD_LABEL(ObjectNode, _objectTransform, "Object transformation")

/******************************************************************************
* Constructor.
******************************************************************************/
ObjectNode::ObjectNode(SceneObject* object) : _objectTransform(AffineTransformation::Identity())
{
	INIT_PROPERTY_FIELD(ObjectNode::_sceneObject);
	INIT_PROPERTY_FIELD(ObjectNode::_objectTransform);
	setSceneObject(object);
}

/******************************************************************************
* Evaluates the geometry pipeline of this scene node at the given time.
******************************************************************************/
const PipelineFlowState& ObjectNode::evalPipeline(TimePoint time)
{
	// Do not record any object creation operation during pipeline evaluation.
	UndoSuspender noUndo;

	// Check if the cache is filled.
	if(_pipelineCache.result() == NULL || !_pipelineCache.stateValidity().contains(time)) {
		if(sceneObject()) {
			// Evaluate object and save result in local cache.
			_pipelineCache = sceneObject()->evalObject(time);
		}
		else {
			// Clear cache if this node is empty.
			_pipelineCache = PipelineFlowState();
		}
	}
	return _pipelineCache;
}

/******************************************************************************
* This method is called when a referenced object has changed.
******************************************************************************/
bool ObjectNode::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::TargetChanged && source == sceneObject()) {
		// Object has changed -> rebuild pipeline cache.
		invalidatePipelineCache();
	}
	else if(event->type() == ReferenceEvent::TargetDeleted && source == sceneObject()) {
		// Object has been deleted -> delete node too.
		if(!UndoManager::instance().isUndoingOrRedoing())
			deleteNode();
	}
	return SceneNode::referenceEvent(source, event);
}

/******************************************************************************
* Gets called when the scene object of the node has been replaced.
******************************************************************************/
void ObjectNode::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(ObjectNode::_sceneObject))
		invalidatePipelineCache();

	SceneNode::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* Returns the bounding box of the object node in local coordinates.
* The ObjectTransform is already applied to the returned box.
******************************************************************************/
Box3 ObjectNode::localBoundingBox(TimePoint time)
{
	const PipelineFlowState& state = evalPipeline(time);
	if(state.result() == NULL) return Box3();

	// Compute bounding box of scene object.
	Box3 bb = state.result()->boundingBox(time, this);

	// Apply internal object transformation.
	return bb.transformed(objectTransform());
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

#if 0
/******************************************************************************
* Performs a hit test on this node.
* Returns distance of the
* hit from the viewer or HIT_TEST_NONE if no hit was found.
******************************************************************************/
FloatType ObjectNode::hitTest(TimeTicks time, Viewport* vp, const PickRegion& pickRegion)
{
	CHECK_POINTER(vp);

	const PipelineFlowState& flowState = evalPipeline(time);
	if(flowState.result() == NULL) return HIT_TEST_NONE;
    CHECK_OBJECT_POINTER(flowState.result());

	// Setup transformation.
	TimeInterval iv;
	const AffineTransformation& nodeTM = getWorldTransform(time, iv);
	vp->setWorldMatrix(objectTransform() * nodeTM);

	// Hit test object.
	return flowState.result()->hitTest(time, vp, this, pickRegion);
}
#endif

#if 0
/******************************************************************************
* Applies the given modifier to the object node.
* The modifier is put on top of the modifier stack.
******************************************************************************/
void ObjectNode::applyModifier(Modifier* modifier)
{
	CHECK_OBJECT_POINTER(sceneObject());
	if(sceneObject() == NULL)
		throw Exception("Cannot apply modifier to an empty object node.");

	ModifiedObject* modObj = dynamic_object_cast<ModifiedObject>(sceneObject());
	if(modObj == NULL) {
		ModifiedObject::SmartPtr mo = new ModifiedObject();
		mo->setInputObject(sceneObject());
		setSceneObject(mo);
		modObj = mo.get();
	}
	modObj->insertModifier(modifier, modObj->modifierApplications().size());
}
#endif

};
