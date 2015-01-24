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

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

IMPLEMENT_OVITO_OBJECT(Core, RefTargetListenerBase, RefMaker);
IMPLEMENT_OVITO_OBJECT(Core, VectorRefTargetListenerBase, RefMaker);
DEFINE_FLAGS_REFERENCE_FIELD(RefTargetListenerBase, _target, "Target", RefTarget, PROPERTY_FIELD_NEVER_CLONE_TARGET|PROPERTY_FIELD_NO_UNDO|PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_WEAK_REF);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(VectorRefTargetListenerBase, _targets, "Targets", RefTarget, PROPERTY_FIELD_NEVER_CLONE_TARGET|PROPERTY_FIELD_NO_UNDO|PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_WEAK_REF);

/******************************************************************************
* Is called when the RefTarget referenced by this listener has sent a message.
******************************************************************************/
bool RefTargetListenerBase::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	// Emit Qt signal.
	Q_EMIT notificationEvent(event);
	
	return RefMaker::referenceEvent(source, event);
}

/******************************************************************************
* Is called when the RefTarget referenced by this listener has sent a message.
******************************************************************************/
bool VectorRefTargetListenerBase::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	// Emit Qt signal.
	Q_EMIT notificationEvent(source, event);

	return RefMaker::referenceEvent(source, event);
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

