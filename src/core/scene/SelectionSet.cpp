///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2014) Alexander Stukowski
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
#include <core/scene/SelectionSet.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, SelectionSet, RefTarget);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(SelectionSet, _selection, "SelectedNodes", SceneNode, PROPERTY_FIELD_NEVER_CLONE_TARGET);
SET_PROPERTY_FIELD_LABEL(SelectionSet, _selection, "Nodes");

/******************************************************************************
* Default constructor.
******************************************************************************/
SelectionSet::SelectionSet(DataSet* dataset) : RefTarget(dataset), _selectionChangeInProgress(false)
{
	INIT_PROPERTY_FIELD(SelectionSet::_selection);
}

/******************************************************************************
* Adds a scene node to this selection set. 
******************************************************************************/
void SelectionSet::push_back(SceneNode* node)
{
	OVITO_CHECK_OBJECT_POINTER(node);
	if(contains(node)) return;
			
	// Insert into children array.
	_selection.push_back(node);
}

/******************************************************************************
* Completely replaces the contents of the selection set.  
******************************************************************************/
void SelectionSet::setNodes(const QVector<SceneNode*>& nodes)
{
	// Remove all nodes from the selection set that are not in the new list of selected nodes.
	for(int i = _selection.size(); i--; ) {
		if(!nodes.contains(_selection[i]))
			_selection.remove(i);
	}
	for(SceneNode* node : nodes)
		push_back(node);
}

/******************************************************************************
* Resets the selection set to contain only a single node.  
******************************************************************************/
void SelectionSet::setNode(SceneNode* node)
{
	OVITO_CHECK_POINTER(node);
	if(!_selection.contains(node)) {
		clear();
		push_back(node);
	}
	else {
		// Remove all other nodes from the selection set.
		for(int i = _selection.size(); i--; ) {
			if(node != _selection[i])
				_selection.remove(i);
		}
	}
}

/******************************************************************************
* Removes a scene node from this selection set. 
******************************************************************************/
void SelectionSet::remove(SceneNode* node)
{
	int index = _selection.indexOf(node);
	if(index == -1) return;	
	_selection.remove(index);
	OVITO_ASSERT(!contains(node));
}

/******************************************************************************
* Is called when a RefTarget referenced by this object has generated an event.
******************************************************************************/
bool SelectionSet::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	// Do not propagate events from selected nodes.
	return false;
}

/******************************************************************************
* Is called when a RefTarget has been added to a VectorReferenceField of this RefMaker.
******************************************************************************/
void SelectionSet::referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex)
{
	if(field == PROPERTY_FIELD(SelectionSet::_selection)) {
		Q_EMIT selectionChanged(this);
		if(!_selectionChangeInProgress) {
			_selectionChangeInProgress = true;
			QMetaObject::invokeMethod(this, "onSelectionChangeCompleted", Qt::QueuedConnection);
		}
	}
	RefTarget::referenceInserted(field, newTarget, listIndex);
}

/******************************************************************************
* Is called when a RefTarget has been removed from a VectorReferenceField of this RefMaker.
******************************************************************************/
void SelectionSet::referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex)
{
	if(field == PROPERTY_FIELD(SelectionSet::_selection)) {
		Q_EMIT selectionChanged(this);
		if(!_selectionChangeInProgress) {
			_selectionChangeInProgress = true;
			QMetaObject::invokeMethod(this, "onSelectionChangeCompleted", Qt::QueuedConnection);
		}
	}
	RefTarget::referenceRemoved(field, oldTarget, listIndex);
}

/******************************************************************************
* This method is invoked after the change of the selection set is complete.
* It emits the selectionChangeComplete() signal.
******************************************************************************/
void SelectionSet::onSelectionChangeCompleted()
{
	OVITO_ASSERT(_selectionChangeInProgress);
	_selectionChangeInProgress = false;
	Q_EMIT selectionChangeComplete(this);
}

/******************************************************************************
* Returns the bounding box that includes all selected nodes.
******************************************************************************/
Box3 SelectionSet::boundingBox(TimePoint time) const
{
	Box3 bb;
	for(SceneNode* node : nodes()) {
		// Get node's world bounding box
		// and add it to global box.
		bb.addBox(node->worldBoundingBox(time));
	}
	return bb;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
