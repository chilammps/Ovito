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
 * \file GroupNode.h 
 * \brief Contains the definition of the Ovito::GroupNode class.
 */
 
#ifndef __OVITO_GROUP_NODE_H
#define __OVITO_GROUP_NODE_H

#include <core/Core.h>
#include "SceneNode.h"

namespace Ovito {

/**
 * \brief Scene node that groups all child nodes together.
 */
class OVITO_CORE_EXPORT GroupNode : public SceneNode
{
public:

	/// \brief Constructs an empty group node that is in the closed state.
	Q_INVOKABLE GroupNode();

	/// \brief Returns whether this is an instance of the GroupNode class.
	/// \return Returns always \c true.
	virtual bool isGroupNode() const override { return true; }

	/// \brief Returns whether this group is currently open.
	/// \return \c true if the group is in the open state.
	/// 
	/// When the group is open then child nodes the group can be edited individually.
	bool isGroupOpen() const { return _isGroupOpen; }

	/// \brief Opens the group for editing or closes it.
	/// \param open Controls whether the group should be opened or closed.
	/// \undoable
	void setGroupOpen(bool open) { _isGroupOpen = open; }

	/// \brief Returns the bounding box of the group.
	/// \param time The time at which the bounding box should be computed.
	/// \return An axis-aligned box in the node's local coordinate system that contains
	///         the bounding boxes of all child nodes.
	virtual Box3 localBoundingBox(TimePoint time) override;
		
public:

	Q_PROPERTY(bool isGroupOpen READ isGroupOpen WRITE setGroupOpen)

protected:

	/// Is called when a reference target has been removed from a list reference field of this RefMaker.
	virtual void referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex) override;

private:

	/// Indicates if this group of nodes is currently open.
	PropertyField<bool, bool, ReferenceEvent::GroupNodeOpenedOrClosed> _isGroupOpen;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_isGroupOpen)
};

};

#endif // __OVITO_GROUP_NODE_H
