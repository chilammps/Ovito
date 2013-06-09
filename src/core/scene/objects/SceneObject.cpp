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
#include <core/scene/objects/SceneObject.h>
#include <core/scene/objects/AbstractCameraObject.h>
#include <core/scene/display/DisplayObject.h>
#include "moc_AbstractCameraObject.cpp"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(SceneObject, RefTarget)
DEFINE_FLAGS_REFERENCE_FIELD(SceneObject, _displayObject, "DisplayObject", DisplayObject, PROPERTY_FIELD_NO_CHANGE_MESSAGE)
SET_PROPERTY_FIELD_LABEL(SceneObject, _displayObject, "Display")

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(AbstractCameraObject, SceneObject)

/******************************************************************************
* Constructor.
******************************************************************************/
SceneObject::SceneObject() : _revisionNumber(0)
{
	INIT_PROPERTY_FIELD(SceneObject::_displayObject);
}

/******************************************************************************
* Sends an event to all dependents of this RefTarget.
******************************************************************************/
void SceneObject::notifyDependents(ReferenceEvent& event)
{
	// Increment revision counter if the object changed.
	if(event.type() == ReferenceEvent::TargetChanged)
		_revisionNumber++;

	RefTarget::notifyDependents(event);
}

};
