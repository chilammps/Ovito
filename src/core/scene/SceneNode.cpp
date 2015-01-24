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
#include <core/scene/SceneNode.h>
#include <core/animation/controller/LookAtController.h>
#include <core/animation/controller/PRSTransformationController.h>
#include <core/animation/AnimationSettings.h>
#include <core/dataset/UndoStack.h>
#include <core/dataset/DataSet.h>
#include <core/animation/TimeInterval.h>
#include <core/reference/CloneHelper.h>
#include <core/scene/SelectionSet.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, SceneNode, RefTarget);
DEFINE_FLAGS_REFERENCE_FIELD(SceneNode, _transformation, "Transform", Controller, PROPERTY_FIELD_ALWAYS_DEEP_COPY);
DEFINE_FLAGS_REFERENCE_FIELD(SceneNode, _lookatTargetNode, "TargetNode", SceneNode, PROPERTY_FIELD_ALWAYS_CLONE | PROPERTY_FIELD_NO_SUB_ANIM);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(SceneNode, _children, "Children", SceneNode, PROPERTY_FIELD_ALWAYS_CLONE | PROPERTY_FIELD_NO_SUB_ANIM);
DEFINE_PROPERTY_FIELD(SceneNode, _nodeName, "NodeName");
DEFINE_PROPERTY_FIELD(SceneNode, _displayColor, "DisplayColor");
SET_PROPERTY_FIELD_LABEL(SceneNode, _transformation, "Transformation");
SET_PROPERTY_FIELD_LABEL(SceneNode, _lookatTargetNode, "Target");
SET_PROPERTY_FIELD_LABEL(SceneNode, _children, "Children");
SET_PROPERTY_FIELD_LABEL(SceneNode, _nodeName, "Name");
SET_PROPERTY_FIELD_LABEL(SceneNode, _displayColor, "Display color");

/******************************************************************************
* Default constructor.
******************************************************************************/
SceneNode::SceneNode(DataSet* dataset) : RefTarget(dataset), _parentNode(nullptr), _worldTransform(AffineTransformation::Identity()),
	_worldTransformValidity(TimeInterval::empty()), _worldBBTime(TimeNegativeInfinity()), _displayColor(0,0,0)
{
	INIT_PROPERTY_FIELD(SceneNode::_transformation);
	INIT_PROPERTY_FIELD(SceneNode::_lookatTargetNode);
	INIT_PROPERTY_FIELD(SceneNode::_children);
	INIT_PROPERTY_FIELD(SceneNode::_nodeName);
	INIT_PROPERTY_FIELD(SceneNode::_displayColor);

	// Assign random color to node.
	static std::default_random_engine rng;
	_displayColor = Color::fromHSV(std::uniform_real_distribution<FloatType>()(rng), 1, 1);

	// Create a transformation controller for the node.
	_transformation = ControllerManager::instance().createTransformationController(dataset);
}

/******************************************************************************
* Returns this node's world transformation matrix.
* This matrix contains the transformation of the parent node.
******************************************************************************/
const AffineTransformation& SceneNode::getWorldTransform(TimePoint time, TimeInterval& validityInterval)
{
	if(!_worldTransformValidity.contains(time)) {
		_worldTransformValidity.setInfinite();
		_worldTransform.setIdentity();
		// Get parent node's tm.
		if(parentNode() && !parentNode()->isRootNode()) {
			_worldTransform = _worldTransform * parentNode()->getWorldTransform(time, _worldTransformValidity);
		}
		// Apply own tm.
		if(transformationController())
			transformationController()->applyTransformation(time, _worldTransform, _worldTransformValidity);
	}
	validityInterval.intersect(_worldTransformValidity);
	return _worldTransform;
}

/******************************************************************************
* Returns this node's local transformation matrix.
* This matrix  does not contain the ObjectTransform of this node and
* does not contain the transformation of the parent node.
******************************************************************************/
AffineTransformation SceneNode::getLocalTransform(TimePoint time, TimeInterval& validityInterval)
{
	AffineTransformation result = AffineTransformation::Identity();
	if(transformationController())
		transformationController()->applyTransformation(time, result, validityInterval);
	return result;
}

/******************************************************************************
* This method marks the world transformation cache as invalid,
* so it will be rebuilt during the next call to GetWorldTransform().
******************************************************************************/
void SceneNode::invalidateWorldTransformation()
{
	_worldTransformValidity.setEmpty();
	invalidateBoundingBox();
	for(SceneNode* child : children())
		child->invalidateWorldTransformation();
	notifyDependents(ReferenceEvent::TransformationChanged);
}

/******************************************************************************
* Deletes this node from the scene. This will also delete all child nodes.
******************************************************************************/
void SceneNode::deleteNode()
{
	// Delete target too.
	OORef<SceneNode> tn = lookatTargetNode();
	if(tn) {
		// Clear reference first to prevent infinite recursion.
		_lookatTargetNode = nullptr;
		tn->deleteNode();
	}

	// Delete all child nodes recursively.
	for(SceneNode* child : children())
		child->deleteNode();

	OVITO_ASSERT(children().empty());

	// Delete node itself.
	deleteReferenceObject();
}

/******************************************************************************
* Binds this scene node to a target node and creates a look at controller
* that lets this scene node look at the target. The target will automatically
* be deleted if this scene node is deleted and vice versa.
* Returns the newly created LookAtController assigned as rotation controller for this node.
******************************************************************************/
LookAtController* SceneNode::setLookatTargetNode(SceneNode* targetNode)
{
	_lookatTargetNode = targetNode;

	// Let this node look at the target.
	PRSTransformationController* prs = dynamic_object_cast<PRSTransformationController>(transformationController());
	if(prs) {
		if(targetNode) {
			OVITO_CHECK_OBJECT_POINTER(targetNode);

			// Create a look at controller.
			OORef<LookAtController> lookAtCtrl = dynamic_object_cast<LookAtController>(prs->rotationController());
			if(!lookAtCtrl)
				lookAtCtrl = new LookAtController(dataset());
			lookAtCtrl->setTargetNode(targetNode);

			// Assign it as rotation sub-controller.
			prs->setRotationController(lookAtCtrl);

			return dynamic_object_cast<LookAtController>(prs->rotationController());
		}
		else {
			// Save old rotation.
			TimePoint time = dataset()->animationSettings()->time();
			TimeInterval iv;
			Rotation rotation;
			prs->rotationController()->getRotationValue(time, rotation, iv);

			// Reset to default rotation controller.
			OORef<Controller> controller = ControllerManager::instance().createRotationController(dataset());
			controller->setRotationValue(time, rotation, true);
			prs->setRotationController(controller);
		}
	}

	return nullptr;
}

/******************************************************************************
* This method marks the cached world bounding box as invalid,
* so it will be rebuilt during the next call to worldBoundingBox().
******************************************************************************/
void SceneNode::invalidateBoundingBox()
{
	_worldBBTime = TimeNegativeInfinity();
	if(parentNode())
		parentNode()->invalidateBoundingBox();
}

/******************************************************************************
* From RefMaker.
******************************************************************************/
bool SceneNode::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::TargetChanged) {
		if(source == transformationController()) {
			// TM has changed -> rebuild world tm cache.
			invalidateWorldTransformation();
		}
		else {
			// The bounding box might have changed if the object has changed.
			invalidateBoundingBox();
		}
	}
	else if(event->type() == ReferenceEvent::TargetDeleted && source == lookatTargetNode()) {
		// Lookat target node has been deleted -> delete this node too.
		if(!dataset()->undoStack().isUndoingOrRedoing())
			deleteNode();
	}
	return RefTarget::referenceEvent(source, event);
}

/******************************************************************************
* From RefMaker.
******************************************************************************/
void SceneNode::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(SceneNode::_transformation)) {
		// TM controller has changed -> rebuild world tm cache.
		invalidateWorldTransformation();
	}
	RefTarget::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* From RefMaker.
******************************************************************************/
void SceneNode::referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex)
{
	if(field == PROPERTY_FIELD(SceneNode::_children)) {
		// A new child node has been added.
		SceneNode* child = static_object_cast<SceneNode>(newTarget);
		OVITO_CHECK_OBJECT_POINTER(child);
		OVITO_ASSERT(child->parentNode() == nullptr);
		child->_parentNode = this;

		// Invalidate cached world bounding box of this parent node.
		invalidateBoundingBox();
	}
	RefTarget::referenceInserted(field, newTarget, listIndex);
}

/******************************************************************************
* From RefMaker.
******************************************************************************/
void SceneNode::referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex)
{
	if(field == PROPERTY_FIELD(SceneNode::_children)) {
		// A child node has been removed.
		SceneNode* child = static_object_cast<SceneNode>(oldTarget);
		OVITO_ASSERT(child->parentNode() == this);
		child->_parentNode = nullptr;

		// Invalidate cached world bounding box of this parent node.
		invalidateBoundingBox();

		// Whenever a node has been removed from the scene, the pending state of the scene might change.
		// Even though we don't know for sure if the state has changed, we send a notification event here.
		notifyDependents(ReferenceEvent::PendingStateChanged);
	}
	RefTarget::referenceRemoved(field, oldTarget, listIndex);
}

/******************************************************************************
* Adds a child scene node to this node.
******************************************************************************/
void SceneNode::insertChild(int index, SceneNode* newChild)
{
	OVITO_CHECK_OBJECT_POINTER(newChild);

	// Check whether it is already a child of this parent.
	if(newChild->parentNode() == this) {
		OVITO_ASSERT(children().contains(newChild));
		return;
	}

	// Remove new child from old parent node first.
	if(newChild->parentNode())
		newChild->parentNode()->removeChild(newChild);
	OVITO_ASSERT(newChild->parentNode() == nullptr);

	// Insert into children array of this parent.
	_children.insert(index, newChild);
	// This parent should be automatically filled into the child's parent pointer.
	OVITO_ASSERT(newChild->parentNode() == this);

	// Adjust transformation to preserve world position.
	TimeInterval iv = TimeInterval::infinite();
	const AffineTransformation& newParentTM = getWorldTransform(dataset()->animationSettings()->time(), iv);
	if(newParentTM != AffineTransformation::Identity())
		newChild->transformationController()->changeParent(dataset()->animationSettings()->time(), AffineTransformation::Identity(), newParentTM, newChild);
	newChild->invalidateWorldTransformation();
}

/******************************************************************************
* Removes a child node from this parent node.
******************************************************************************/
void SceneNode::removeChild(SceneNode* child)
{
	OVITO_CHECK_OBJECT_POINTER(child);
	OVITO_ASSERT_MSG(child->parentNode() == this, "SceneNode::removeChild()", "The given node is not a child of this parent node.");

	int index = _children.indexOf(child);
	OVITO_ASSERT(index != -1);

	// Remove child node from array.
	_children.remove(index);
	OVITO_ASSERT(_children.contains(child) == false);
	OVITO_ASSERT(child->parentNode() == nullptr);

	// Update child node.
	TimeInterval iv = TimeInterval::infinite();
	AffineTransformation oldParentTM = getWorldTransform(dataset()->animationSettings()->time(), iv);
	if(oldParentTM != AffineTransformation::Identity())
		child->transformationController()->changeParent(dataset()->animationSettings()->time(), oldParentTM, AffineTransformation::Identity(), child);
	child->invalidateWorldTransformation();
}

/******************************************************************************
* Returns the bounding box of the scene node in world coordinates.
*    time - The time at which the bounding box should be returned.
******************************************************************************/
const Box3& SceneNode::worldBoundingBox(TimePoint time)
{
    if(_worldBBTime == time)
		return _worldBB;

	_worldBBTime = time;
	TimeInterval iv = TimeInterval::infinite();
	AffineTransformation tm = getWorldTransform(time, iv);
	_worldBB = localBoundingBox(time).transformed(tm);

	for(SceneNode* child : children())
		_worldBB.addBox(child->worldBoundingBox(time));

	return _worldBB;
}

/******************************************************************************
* Returns true if this node is currently selected.
******************************************************************************/
bool SceneNode::isSelected() const
{
	return dataset()->selection()->contains(const_cast<SceneNode*>(this));
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void SceneNode::saveToStream(ObjectSaveStream& stream)
{
	RefTarget::saveToStream(stream);

	stream.beginChunk(0x02);
	// This is for future use...
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void SceneNode::loadFromStream(ObjectLoadStream& stream)
{
	RefTarget::loadFromStream(stream);

	stream.expectChunkRange(0x01, 0x02);
	// This is for future use...
	stream.closeChunk();

	// Restore parent/child hierarchy.
	for(SceneNode* child : children())
		child->_parentNode = this;
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> SceneNode::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<SceneNode> clone = static_object_cast<SceneNode>(RefTarget::clone(deepCopy, cloneHelper));

	// Clone orientation target node too.
	if(clone->lookatTargetNode()) {
		OVITO_ASSERT(lookatTargetNode());

		// Insert the cloned target into the same scene as out target.
		if(lookatTargetNode()->parentNode() && !clone->lookatTargetNode()->parentNode()) {
			lookatTargetNode()->parentNode()->addChild(clone->lookatTargetNode());
		}

		// Set new target for look-at controller.
		clone->setLookatTargetNode(clone->lookatTargetNode());
	}

	return clone;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
