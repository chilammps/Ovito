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
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, AbstractCameraObject, SceneObject)
DEFINE_PROPERTY_FIELD(SceneObject, _saveWithScene, "SaveWithScene")
SET_PROPERTY_FIELD_LABEL(SceneObject, _saveWithScene, "Save data with scene")

/******************************************************************************
* Constructor.
******************************************************************************/
SceneObject::SceneObject() : _revisionNumber(0), _saveWithScene(true)
{
	INIT_PROPERTY_FIELD(SceneObject::_saveWithScene);
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
	// Automatically increment revision counter each time a sub-object of this object changes.
	if(event->type() == ReferenceEvent::TargetChanged) {
		_revisionNumber++;
	}

	return RefTarget::referenceEvent(source, event);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void SceneObject::saveToStream(ObjectSaveStream& stream)
{
	RefTarget::saveToStream(stream);
	stream.beginChunk(0x01);
	stream.saveObject(_displayObject.get());
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void SceneObject::loadFromStream(ObjectLoadStream& stream)
{
	RefTarget::loadFromStream(stream);
	stream.expectChunk(0x01);
	_displayObject = stream.loadObject<DisplayObject>();
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> SceneObject::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<SceneObject> clone = static_object_cast<SceneObject>(RefTarget::clone(deepCopy, cloneHelper));

	// Copy the reference to the associated display object.
	clone->_displayObject = this->_displayObject;

	return clone;
}

};
