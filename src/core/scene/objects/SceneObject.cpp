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

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, SceneObject, RefTarget)
DEFINE_FLAGS_REFERENCE_FIELD(SceneObject, _displayObject, "DisplayObject", DisplayObject, PROPERTY_FIELD_NO_CHANGE_MESSAGE)
SET_PROPERTY_FIELD_LABEL(SceneObject, _displayObject, "Display")

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, AbstractCameraObject, SceneObject)

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
	// Automatically increment revision counter each time the object changes.
	if(event.type() == ReferenceEvent::TargetChanged)
		_revisionNumber++;

	RefTarget::notifyDependents(event);
}

/******************************************************************************
* Handles reference events sent by reference targets of this object.
******************************************************************************/
bool SceneObject::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	// Intercept messages from the display object since they don't represent a change of
	// the scene object.
	if(source == displayObject()) {
		return false;
	}
	return RefTarget::referenceEvent(source, event);
}


};
