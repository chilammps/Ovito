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

/**
 * \file SceneNode.h
 * \brief Contains the definition of the Ovito::SceneNode class.
 */

#ifndef __OVITO_SCENE_NODE_H
#define __OVITO_SCENE_NODE_H

#include <core/Core.h>
#include <base/utilities/Color.h>
#include <core/reference/RefTarget.h>
#include <core/animation/TimeInterval.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/controller/TransformationController.h>

namespace Ovito {


/**
 * \brief Tree node in the scene hierarchy.
 *
 * A SceneNode is a node in the scene graph. Every object shown in the viewports
 * has an associated SceneNode.
 */
class OVITO_CORE_EXPORT SceneNode : public RefTarget
{
protected:

	/// \brief constructor.
	SceneNode(DataSet* dataset);

public:

	/// \brief Returns the controller that controls this node's local transformation matrix.
	/// \return The transformation controller.
	TransformationController* transformationController() const { return _transformation; }

	/// \brief Sets the controller that controls this node's local transformation.
	/// \param ctrl The new transformation controller.
	/// \undoable
	void setTransformationController(TransformationController* ctrl) { _transformation = ctrl; }

	/// \brief Sets the controller that controls this node's local transformation.
	/// \param ctrl The new transformation controller.
	/// \undoable
	void setTransformationController(const OORef<TransformationController>& ctrl) { setTransformationController(ctrl.get()); }

	/// \brief Returns this node's world transformation matrix.
	/// \param[in] time The animation for which the transformation matrix should be computed.
	/// \param[in,out] validityInterval The validity interval of the returned transformation matrix.
	///                                 The interval passed to the method is reduced to the time interval during which the transformation stays constant.
	/// \return The matrix that transforms from this node's local space to absolute world space.
	///         This matrix contains also the transformation of the parent node.
	const AffineTransformation& getWorldTransform(TimePoint time, TimeInterval& validityInterval);

	/// \brief Returns this node's local transformation matrix.
	/// \param[in] time The animation for which the transformation matrix should be computed.
	/// \param[in,out] validityInterval The validity interval of the returned transformation matrix.
	///                                 The interval passed to the method is reduced to the time interval during which the transformation stays constant.
	/// \return The matrix that transforms from this node's local space to the coordinate space of the parent node.
	///         This matrix does therefore not contain the transformation of the parent node.
	///
	/// The local transformation does not contain the object transform of this node and
	/// does not contain the transformation of the parent node.
	AffineTransformation getLocalTransform(TimePoint time, TimeInterval& validityInterval);

	/// \brief Gets the node's display name.
	/// \return A string given by the user to this node.
	const QString& name() const { return _nodeName; }

	/// \brief Sets the node's display name.
	/// \param name The new name to be given to the node.
	/// \undoable
	void setName(const QString& name) { _nodeName = name; }

	/// \brief Gets the display color of the node.
	/// \return This color is used to render the node in viewports.
	const Color& displayColor() const { return _displayColor; }

	/// \brief Sets the display color of the node.
	/// \param color The new color to be used to render the node in the viewports.
	/// \undoable
	void setDisplayColor(const Color& color) { _displayColor = color; }

	/// \brief Returns the parent node of this node in the scene tree graph.
	/// \return This node's parent node or \c NULL if this is the root node.
	SceneNode* parentNode() const { return _parentNode; }

	/// \brief Deletes this node from the scene.
	///
	/// This will also deletes all child nodes.
	///
	/// \undoable
	virtual void deleteNode();

	/// \brief Adds a child scene node to this node.
	/// \param newChild The node that becomes a child of this node. If \a newChild is already a child
	///                 of another parent node then it is first removed form that parent.
	///
	/// This method preserves the world transformation of the new child node by calling
	/// Transformation::changeParents() on the node's local transformation controller.
	/// Please note that is actually only preserves the node's world position at the current
	/// animation time.
	///
	/// \undoable
	void addChild(SceneNode* newChild);

	/// \brief Adds a child scene node to this node.
	/// \param newChild The node that becomes a child of this node. If \a newChild is already a child
	///                 of another parent node then it is first removed form that parent.
	///
	/// This is the same method as the one above but takes a smart pointer instead of a raw pointer.
	/// \undoable
	void addChild(const OORef<SceneNode>& newChild) { addChild(newChild.get()); }

	/// \brief Removes a child node from this parent node.
	/// \param child A child node of this parent node.
	///
	/// This method preserves the world transformation of the child node by calling
	/// Transformation::changeParents() on the node's local transformation controller.
	/// Please note that is actually only preserves the node's world position at the current
	/// animation time.
	///
	/// \undoable
	/// \sa addChild()
	void removeChild(SceneNode* child);

	/// \brief Returns the number of children of this node.
	/// \return The number of children this parent node has.
	/// \sa children()
	int childCount() const { return _children.size(); }

	/// \brief Returns the child of this node with the given index.
	/// \param index The index of the child node to return. This must be between 0 and childCount()-1.
	/// \return The requested child node.
	/// \sa children()
	SceneNode* childNode(int index) const;

	/// \brief Returns the array of child nodes.
	/// \return A vector that contains all children of this scene node.
	/// \sa childCount(), childNode(), addChild(), removeChild()
	const QVector<SceneNode*>& children() const { return _children; }

	/// \brief Recursively visits all nodes below this parent node
	///        and invokes the given visitor function for every node.
	///
	/// \param fn A function that takes a SceneNode pointer as argument and returns a boolean value.
	/// \return true if all child nodes have been visited; false if the loop has been
	///         terminated early because the visitor function has returned false.
	///
	/// The visitor function must return a boolean value to indicate whether
	/// it wants to continue visit more nodes. A return value of false
	/// leads to early termination and no further nodes are visited.
	template<class Function>
	bool visitChildren(Function fn) const {
		for(SceneNode* child : children()) {
			if(!fn(child) || !child->visitChildren(fn))
				return false;
		}
		return true;
	}

	/// \brief Recursively visits all object nodes below this parent node
	///        and invokes the given visitor function for every ObjectNode.
	///
	/// \param fn A function that takes an ObjectNode pointer as argument and returns a boolean value.
	/// \return true if all object nodes have been visited; false if the loop has been
	///         terminated early because the visitor function has returned false.
	///
	/// The visitor function must return a boolean value to indicate whether
	/// it wants to continue visit more nodes. A return value of false
	/// leads to early termination and no further nodes are visited.
	template<class Function>
	bool visitObjectNodes(Function fn) const {
		for(SceneNode* child : children()) {
			if(child->isObjectNode()) {
				if(!fn((ObjectNode*)child))
					return false;
			}
			else if(!child->visitObjectNodes(fn))
				return false;
		}
		return true;
	}

	/// \brief Binds this scene node to a target node and creates a LookAtController
	///        that lets this scene node look at the target.
	/// \param targetNode The target to look at or \c NULL to unbind the node from its old target.
	/// \return The newly created LookAtController assigned as rotation controller for this node.
	///
	/// The target will automatically be deleted if this SceneNode is deleted and vice versa.
	/// \undoable
	LookAtController* bindToTarget(SceneNode* targetNode);

	/// \brief Returns the target node this scene node is looking at.
	/// \return The target this node's rotation controller is bound to or \c NULL if this scene node
	///         has not been bound to a target via a call to bindToTarget().
	SceneNode* targetNode() const { return _targetNode; }

	/// \brief Returns the bounding box of the scene node in local coordinates.
	/// \param time The time at which the bounding box should be computed.
	/// \return An axis-aligned box in the node's local coordinate system that contains
	///         the whole node geometry.
	/// \note The returned box does not contains the bounding boxes of the child nodes.
	/// \sa worldBoundingBox()
	virtual Box3 localBoundingBox(TimePoint time) = 0;

	/// \brief Returns the bounding box of the scene node in world coordinates.
	/// \param time The time at which the bounding box should be computed.
	/// \return An axis-aligned box in the world local coordinate system that contains
	///         the whole node geometry including the bounding boxes of all child nodes.
	/// \note The returned box does also contain the bounding boxes of the child nodes.
	const Box3& worldBoundingBox(TimePoint time);

	/// \brief Returns whether this scene node is currently selected.
	/// \return \c true if this node is part of the current SelectionSet;
	///         \c false otherwise.
	///
	/// A node is considered selected if it is in the current SelectionSet of the scene
	/// or if its upper most closed group parent is in the selection set.
	bool isSelected() const;

	/// \brief Selects or unselects this node.
	/// \param selected Controls the selection state of this node.
	void setSelected(bool selected);

	/// \brief Returns whether this is an object node that represents an object in the scene.
	/// \return \c true if this is an ObjectNode instance; \c false otherwise.
	virtual bool isObjectNode() const { return false; }

	/// \brief Returns whether this is a group node that combines several child nodes.
	/// \return \c true if this is a GroupNode instance; \c false otherwise.
	/// \sa isObjectNode(), isRootNode()
	virtual bool isGroupNode() const { return false; }

	/// \brief Returns whether this is the root scene node.
	/// \return \c true if this is the root node of the scene, i.e. parentNode() returns \c NULL;
	///         \c false if this a node with a parent.
	///
	/// \sa DataSet::sceneRoot()
	/// \sa parentNode()
	bool isRootNode() const { return parentNode() == nullptr; }

	/// \brief Retrieves the upper most (closed) GroupNode this node is part of or
	///        \c NULL if this node is not part of a closed group node.
	/// \return The GroupNode that contains this scene node at is in the closed state at the same time.
	GroupNode* closedParentGroup() const;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return _nodeName; }

	/// \brief Returns whether the rendering of the node's motion trajectory in the
	///        viewports is enabled.
	/// \return \c true if the trajectory is being shown; \c false if not.
	bool showTrajectoryEnabled() const { return (_flags & SCENENODE_SHOW_TRAJECTORY) != 0; }

public:

	Q_PROPERTY(bool isSelected READ isSelected WRITE setSelected)
	Q_PROPERTY(SceneNode* targetNode READ targetNode WRITE bindToTarget)
	Q_PROPERTY(QString name READ name WRITE setName)
	Q_PROPERTY(Color displayColor READ displayColor WRITE setDisplayColor)

protected:

	/// From RefMaker.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// From RefMaker.
	virtual void referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex) override;

	/// From RefMaker.
	virtual void referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex) override;

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

	/// This method marks the world transformation cache as invalid,
	/// so it will be rebuilt during the next call to getWorldTransform().
	virtual void invalidateWorldTransformation();

	/// This method marks the cached world bounding box as invalid,
	/// so it will be rebuilt during the next call to worldBoundingBox().
	virtual void invalidateBoundingBox();

	/// This node's parent node in the hierarchy.
	SceneNode* _parentNode;

	/// Transformation matrix controller.
	ReferenceField<TransformationController> _transformation;

	/// This node's cached world transformation matrix.
	/// It contains the transformation of the parent node.
	AffineTransformation _worldTransform;

	/// This time interval indicates for which times the cached world transformation matrix
	/// has been computed.
	TimeInterval _worldTransformValidity;

	/// The name of this scene node.
	PropertyField<QString, QString, ReferenceEvent::TitleChanged> _nodeName;

	/// The display color of the node.
	PropertyField<Color, QColor> _displayColor;

	/// Stores the target node this scene node is bound to using a look
	/// at controller or null if this scene node is not bound to a target node.
	ReferenceField<SceneNode> _targetNode;

	/// Contains all child nodes.
	VectorReferenceField<SceneNode> _children;

	/// The cached world bounding box of this node.
	Box3 _worldBB;

	/// The time at which the cached bounding box is valid.
	TimePoint _worldBBTime;

	/// Flags that can be set for a scene node.
	enum SceneNodeFlag {
		SCENENODE_NOFLAGS = 0,				//< No flags
		SCENENODE_SHOW_TRAJECTORY = (1<<0),	//< Show the motion trajectory of the node in the viewports.
	};
	Q_DECLARE_FLAGS(SceneNodeFlags, SceneNodeFlag)

	/// The bit flags.
	SceneNodeFlags _flags;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_transformation);
	DECLARE_REFERENCE_FIELD(_targetNode);
	DECLARE_VECTOR_REFERENCE_FIELD(_children);
	DECLARE_PROPERTY_FIELD(_nodeName);
	DECLARE_PROPERTY_FIELD(_displayColor);

	friend class SceneRoot;
};

/**
 * \brief An iterator over all nodes in a scene.
 *
 * The constructors takes a root node as argument. The iterator then delivers all sub-nodes of the
 * specified root node.
 *
 * \note The scene tree may not be modified while iterating over the scene nodes using this iterator class.
 */
class SceneNodesIterator
{
public:
	/// \brief Constructor an iterator over all sub-nodes.
	/// \param rootNode The root node for which all sub-nodes should be visited recursively.
	///                 This can be the scene root, i.e. DataSet::sceneRoot(), or some other node below it.
	SceneNodesIterator(SceneNode* rootNode) {
		if(rootNode->childCount() != 0)
			_nodeStack.push_back(std::make_pair(rootNode, 0));
	}

	/// \brief Returns whether the end of the iteration has been reached.
	/// \return \c true if the end of the collection has been reached;
	///         \c false if next() can be called to step to the next node.
	bool finished() const { return _nodeStack.empty(); }

	/// \brief Advances the iterator to the next scene node.
	/// \return The next scene node or \c NULL if the end of the iteration has been reached.
	/// \note This may only be called as long as the end of the iteration has not been reached, i.e. finished() returns \c false.
	SceneNode* next() {
		OVITO_ASSERT(!finished());
		OVITO_ASSERT_MSG(_nodeStack.back().second < _nodeStack.back().first->childCount(), "SceneNodesIterator::next", "Node tree has been modified in the meantime.");
		SceneNode* child = _nodeStack.back().first->childNode(_nodeStack.back().second);
		if(child->childCount()) {
			_nodeStack.push_back(std::make_pair(child, 0));
		}
		else {
			while(!_nodeStack.empty() && ++_nodeStack.back().second >= _nodeStack.back().first->childCount())
				_nodeStack.pop_back();
		}
		if(finished()) return NULL;
		else return current();
	}

	/// \brief Returns the scene node at the current iteration position.
	/// \return The current scene node pointed to by the iterator.
	/// \note This may only be called as long as the end of the iteration has not been reached, i.e. finished() returns \c false.
 	/// \note The scene tree may not be modified while iterating over the scene nodes using this iterator class.
	SceneNode* current() const {
		OVITO_ASSERT(!finished());
		return _nodeStack.back().first->childNode(_nodeStack.back().second);
	}

private:

	QVector<std::pair<SceneNode*, int>> _nodeStack;
};

};

#endif // __OVITO_SCENE_NODE_H
