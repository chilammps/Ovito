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
#include "ParticlePropertyObject.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ParticlePropertyObject, SceneObject)
DEFINE_PROPERTY_FIELD(ParticlePropertyObject, _objectTitle, "ObjectTitle")
SET_PROPERTY_FIELD_LABEL(ParticlePropertyObject, _objectTitle, "Object title")

/******************************************************************************
* Default constructor.
******************************************************************************/
ParticlePropertyObject::ParticlePropertyObject()
	: _storage(new ParticleProperty())
{
	INIT_PROPERTY_FIELD(ParticlePropertyObject::_objectTitle);
}

/******************************************************************************
* Constructor.
******************************************************************************/
ParticlePropertyObject::ParticlePropertyObject(int dataType, size_t dataTypeSize, size_t componentCount)
	: _storage(new ParticleProperty(dataType, dataTypeSize, componentCount))
{
	INIT_PROPERTY_FIELD(ParticlePropertyObject::_objectTitle);
}

/******************************************************************************
* Constructor that creates a standard property storage.
******************************************************************************/
ParticlePropertyObject::ParticlePropertyObject(ParticleProperty::Type which, size_t componentCount)
	: _storage(new ParticleProperty(which, componentCount))
{
	INIT_PROPERTY_FIELD(ParticlePropertyObject::_objectTitle);
}

/******************************************************************************
* Sets the property's name.
******************************************************************************/
void ParticlePropertyObject::setName(const QString& newName)
{
	if(newName == name())
		return;

	// Make the property change undoable.
	if(UndoManager::instance().isRecording())
		UndoManager::instance().push(new SimplePropertyChangeOperation(this, "name"));

	_storage->setName(newName);
	notifyDependents(ReferenceEvent::TargetChanged);
	notifyDependents(ReferenceEvent::TitleChanged);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void ParticlePropertyObject::saveToStream(ObjectSaveStream& stream)
{
	SceneObject::saveToStream(stream);

	stream.beginChunk(0x01);
	stream << *_storage.constData();
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void ParticlePropertyObject::loadFromStream(ObjectLoadStream& stream)
{
	SceneObject::loadFromStream(stream);

	stream.expectChunk(0x01);
	stream >> *_storage.data();
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> ParticlePropertyObject::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<ParticlePropertyObject> clone = static_object_cast<ParticlePropertyObject>(RefTarget::clone(deepCopy, cloneHelper));

	// Shallow copy storage.
	clone->_storage = this->_storage;

	return clone;
}

};	// End of namespace
