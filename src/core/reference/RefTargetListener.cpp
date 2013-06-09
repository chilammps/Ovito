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
#include <core/reference/RefTargetListener.h>

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(RefTargetListener, RefMaker)
DEFINE_FLAGS_REFERENCE_FIELD(RefTargetListener, _target, "Target", RefTarget, PROPERTY_FIELD_NEVER_CLONE_TARGET|PROPERTY_FIELD_NO_UNDO|PROPERTY_FIELD_NO_CHANGE_MESSAGE)

/******************************************************************************
* Constructor.
******************************************************************************/
RefTargetListener::RefTargetListener()
{
	INIT_PROPERTY_FIELD(RefTargetListener::_target);
}

/******************************************************************************
* Destructor.
******************************************************************************/
RefTargetListener::~RefTargetListener()
{
	clearAllReferences();
}


/******************************************************************************
* Deletes this object when it is no longer needed.
******************************************************************************/
void RefTargetListener::autoDeleteObject()
{
	OVITO_ASSERT_MSG(false, "RefTargetListener::autoDeleteObject()", "Invalid use of this class. A RefTargetListener should not be used with smart pointers.");
}

/******************************************************************************
* Is called when the RefTarget referenced by this listener has sent a message.
******************************************************************************/
bool RefTargetListener::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	// Emit Qt signal.
	notificationEvent(event);
	
	return RefMaker::referenceEvent(source, event);
}

};
