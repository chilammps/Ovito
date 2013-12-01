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
 * \file SelectionSet.h
 * \brief Contains the definition of the Ovito::SelectionSet class.
 */

#ifndef __OVITO_SELECTION_SET_H
#define __OVITO_SELECTION_SET_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include "SceneNode.h"

namespace Ovito {

/**
 * \brief Stores a selection of scene nodes.
 *
 * This selection set class holds a reference list to all SceneNode objects
 * that are selected.
 *
 * The current selection set can be accessed via the DataSetManager::currentSelection() method.
 */
class OVITO_CORE_EXPORT SelectionSet : public RefTarget
{
public:

	/// \brief Creates an empty selection set.
	Q_INVOKABLE SelectionSet(DataSet* dataset);

	/// \brief Returns the number of scene nodes in the selection set.
	/// \return The number of selected objects.
	virtual int count() const { return _selection.size(); }

	/// \brief Returns whether this selection set is empty.
	/// \return \c true if the number of nodes in the set is zero.
	bool empty() const { return count() == 0; }

	/// \brief Returns a scene node from the selection set with the given index.
	/// \param index The index into the internal array of selected object. This must
	///              be between zero and count() - 1.
	/// \return The scene node that is selected.
	virtual SceneNode* node(int index) const { return _selection[index]; }

	/// \brief Returns the first scene node from the selection set.
	/// \return The first node from the set or \c NULL if the set is empty.
	/// \sa node(), empty()
	SceneNode* firstNode() const { return empty() ? nullptr : node(0); }

	/// \brief Returns whether a scene node is selected.
	/// \param node The node that should be checked.
	/// \return \c true if the given scene node is part of the selection set;
	///         \c false if the scene node is not selected.
	///
	/// An alternative way is to call SceneNode::isSelected() to check whether
	/// a scene node is part of the current selection set.
	virtual bool contains(SceneNode* node) const { return _selection.contains(node); }

	/// \brief Adds a scene node to this selection set.
	/// \param node The node to be added.
	/// \undoable
	virtual void add(SceneNode* node);

	/// \brief Adds a scene node to this selection set.
	/// \param node The node to be added.
	/// \undoable
	void add(const OORef<SceneNode>& node) { add(node.get()); }

	/// \brief Adds multiple scene nodes to this selection set.
	/// \param nodes The scene nodes to be added to the set.
	/// \undoable
	virtual void addAll(const QVector<SceneNode*>& nodes);

	/// \brief Removes a scene node from this selection set.
	/// \param node The node to be unselected.
	/// \undoable
	/// \sa clear()
	virtual void remove(SceneNode* node);

	/// \brief Clears the selection.
	///
	/// All nodes are removed from the selection set.
	/// \undoable
	virtual void clear();

	/// \brief Returns the bounding box that includes all selected nodes.
	/// \param time The animation for which the bounding box should be computed.
	/// \return The bounding box that contains the bounding boxes of all
	///         scene nodes in the selection set.
	virtual Box3 boundingBox(TimePoint time);

	/// \brief Returns all nodes that are selected.
	/// \return The list of nodes included in this selection set.
	virtual const QVector<SceneNode*>& nodes() const { return _selection; }

	/// \brief Sets the contents of the selection set.
	/// \param nodes The set of nodes to be selected.
	///
	/// The selection set is cleared before it is filled with the new nodes.
	virtual void setNodes(const QVector<SceneNode*>& nodes);

	/// \brief Resets the selection set to contain only a single node.
	/// \param node The node to be selected.
	///
	/// The selection set is cleared before the single node is added to it.
	virtual void setNode(SceneNode* node);

protected:

	/// From RefMaker.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private:

	/// Holds the references to the selected scene nodes.
	VectorReferenceField<SceneNode> _selection;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_selection)
};

/**
 * \brief This event is generated by a SelectionSet when it has received a ReferenceEvent::TargetChanged
 *        from one of the nodes in the selection set.
 *
 * This event type is ReferenceEvent::NodeInSelectionSetChanged.
 */
class NodeInSelectionSetChangedEvent : public ReferenceEvent
{
public:
	/// Constructor.
	NodeInSelectionSetChangedEvent(SelectionSet* sender, SceneNode* node, ReferenceEvent* originalEvent) :
		ReferenceEvent(ReferenceEvent::NodeInSelectionSetChanged, sender), _sceneNode(node), _originalEvent(originalEvent) {}

	/// Returns the scene node from the selection set that has changed in some way.
	SceneNode* node() const { return _sceneNode; }

	/// Returns the original event sent by the node.
	ReferenceEvent* originalEvent() const { return _originalEvent; }

private:

	SceneNode* _sceneNode;
	ReferenceEvent* _originalEvent;
};

};

#endif // __OVITO_SELECTION_SET_H
