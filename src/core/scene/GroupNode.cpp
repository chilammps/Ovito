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
#include <core/scene/GroupNode.h>
#include <core/gui/undo/UndoManager.h>

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(GroupNode, SceneNode)
DEFINE_PROPERTY_FIELD(GroupNode, _isGroupOpen, "IsGroupOpen")
SET_PROPERTY_FIELD_LABEL(GroupNode, _isGroupOpen, "Open group")

/******************************************************************************
* Default constructor.
******************************************************************************/
GroupNode::GroupNode() : _isGroupOpen(false)
{
	INIT_PROPERTY_FIELD(GroupNode::_isGroupOpen);
}

/******************************************************************************
* Returns the bounding box of the group.
******************************************************************************/
Box3 GroupNode::localBoundingBox(TimePoint time)
{
	Box3 myBox;
	TimeInterval iv = TimeInterval::forever();
	for(SceneNode* child : children()) {
		// Get local child bounding box.
		Box3 childBox = child->localBoundingBox(time);
		// Transform to parent coord sys.
		AffineTransformation childTM = child->getLocalTransform(time, iv);
		// Add to parent box.
		myBox.addBox(childBox.transformed(childTM));
	}
	return myBox;
}

/******************************************************************************
* Is called when a reference target has been removed from a list reference field of this RefMaker.
******************************************************************************/
void GroupNode::referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex)
{
	SceneNode::referenceRemoved(field, oldTarget, listIndex);
	// Delete this group node if all child nodes have been removed from it.
	if(children().empty() && !UndoManager::instance().isUndoingOrRedoing()) {
		deleteNode();
	}
}

};
