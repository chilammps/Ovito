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
#include <core/dataset/CurrentSelectionProxy.h>
#include <core/dataset/DataSetManager.h>

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, CurrentSelectionProxy, SelectionSet)
DEFINE_FLAGS_REFERENCE_FIELD(CurrentSelectionProxy, _selectionSet, "SelectionSet", SelectionSet, PROPERTY_FIELD_NO_UNDO)

/******************************************************************************
* Default constructor.
******************************************************************************/
CurrentSelectionProxy::CurrentSelectionProxy() : _changeEventInQueue(false)
{
	INIT_PROPERTY_FIELD(CurrentSelectionProxy::_selectionSet);

	connect(this, SIGNAL(internalSelectionChanged()), this, SLOT(onInternalSelectionChanged()), Qt::QueuedConnection);
}

/******************************************************************************
* Returns the number of scene nodes in the selection set.
******************************************************************************/
int CurrentSelectionProxy::count() const
{
	if(!currentSelectionSet()) return 0;
	return currentSelectionSet()->count();
}

/******************************************************************************
* Returns a scene node from the selection set.
******************************************************************************/
SceneNode* CurrentSelectionProxy::node(int index) const
{
	return currentSelectionSet()->node(index);
}

/******************************************************************************
* Returns true if the given scene node is part of the selection set.
******************************************************************************/
bool CurrentSelectionProxy::contains(SceneNode* node) const
{
	if(!currentSelectionSet()) return false;
	return currentSelectionSet()->contains(node);
}

/******************************************************************************
* Adds a scene node to this selection set. 
******************************************************************************/
void CurrentSelectionProxy::add(SceneNode* node)
{
	if(currentSelectionSet())
		currentSelectionSet()->add(node);
}

/******************************************************************************
* Adds multiple scene nodes to this selection set. 
******************************************************************************/
void CurrentSelectionProxy::addAll(const QVector<SceneNode*>& nodes)
{
	if(currentSelectionSet())
		currentSelectionSet()->addAll(nodes);
}

/******************************************************************************
* Sets the contents of the selection set. 
******************************************************************************/
void CurrentSelectionProxy::setNodes(const QVector<SceneNode*>& nodes)
{
	if(currentSelectionSet())
		currentSelectionSet()->setNodes(nodes);
}

/******************************************************************************
* Resets the selection set to contain only a single node.  
******************************************************************************/
void CurrentSelectionProxy::setNode(SceneNode* node)
{
	if(currentSelectionSet())
		currentSelectionSet()->setNode(node);
}

/******************************************************************************
* Removes a scene node from this selection set. 
******************************************************************************/
void CurrentSelectionProxy::remove(SceneNode* node)
{
	if(currentSelectionSet())
		currentSelectionSet()->remove(node);
}

/******************************************************************************
* Clears the selection.
******************************************************************************/
void CurrentSelectionProxy::clear()
{
	if(currentSelectionSet())
		currentSelectionSet()->clear();
}

/******************************************************************************
* Returns the bounding box that includes all selected nodes.
******************************************************************************/
Box3 CurrentSelectionProxy::boundingBox(TimePoint time)
{
	if(!currentSelectionSet()) return Box3();
	return currentSelectionSet()->boundingBox(time);
}

/******************************************************************************
* Returns all nodes that are selected.
******************************************************************************/
const QVector<SceneNode*>& CurrentSelectionProxy::nodes() const
{
	if(currentSelectionSet())
		return currentSelectionSet()->nodes();
	else {
		static QVector<SceneNode*> emptyList;
		return emptyList;
	}
}

/******************************************************************************
* This method is called when a reference target changes. 
******************************************************************************/
bool CurrentSelectionProxy::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::TargetChanged) {
        onChanged();        
	}
	else if(event->type() == ReferenceEvent::TitleChanged && source == currentSelectionSet()) {
		return false;
	}
	return true;
}

/******************************************************************************
* Is called when the selection set has changed.
* Raises the selectionChanged event for the DataSetManager.
******************************************************************************/
void CurrentSelectionProxy::onChanged()
{
	// Raise event.
	DataSetManager::instance().emitSelectionChanged(this);

	// Raise delayed event.
	if(!_changeEventInQueue) {
		_changeEventInQueue = true;
		internalSelectionChanged(); 
	}	
}

/******************************************************************************
* Is called after the selection set has changed multiple times.
* Raises the selectionChangeComplete event for the DataSetManager.
******************************************************************************/
void CurrentSelectionProxy::onInternalSelectionChanged() 
{
	_changeEventInQueue = false;
	DataSetManager::instance().emitSelectionChangeComplete(this);
}

};
