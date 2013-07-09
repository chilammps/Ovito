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
#include "ParticleTypeProperty.h"
#include "ParticleDisplay.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ParticlePropertyObject, SceneObject)

/******************************************************************************
* Default constructor.
******************************************************************************/
ParticlePropertyObject::ParticlePropertyObject(ParticleProperty* storage)
	: _storage(storage != nullptr ? storage : new ParticleProperty())
{
}

/******************************************************************************
* Factory function that creates a user-defined property object.
******************************************************************************/
OORef<ParticlePropertyObject> ParticlePropertyObject::create(size_t particleCount, int dataType, size_t dataTypeSize, size_t componentCount)
{
	return create(new ParticleProperty(particleCount, dataType, dataTypeSize, componentCount));
}

/******************************************************************************
* Factory function that creates a standard property object.
******************************************************************************/
OORef<ParticlePropertyObject> ParticlePropertyObject::create(size_t particleCount, ParticleProperty::Type which, size_t componentCount)
{
	return create(new ParticleProperty(particleCount, which, componentCount));
}

/******************************************************************************
* Factory function that creates a property object based on an existing storage.
******************************************************************************/
OORef<ParticlePropertyObject> ParticlePropertyObject::create(ParticleProperty* storage)
{
	OORef<ParticlePropertyObject> propertyObj;

	switch(storage->type()) {
	case ParticleProperty::ParticleTypeProperty:
		propertyObj = new ParticleTypeProperty(storage);
		break;
	default:
		propertyObj = new ParticlePropertyObject(storage);
	}

	if(storage->type() == ParticleProperty::PositionProperty)
		propertyObj->setDisplayObject(new ParticleDisplay());

	return propertyObj;
}

/******************************************************************************
* Replaces the internal storage object with the given one.
******************************************************************************/
void ParticlePropertyObject::replaceStorage(ParticleProperty* storage)
{
	OVITO_CHECK_POINTER(storage);
	_storage = storage;
	notifyDependents(ReferenceEvent::TargetChanged);
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
	_storage.constData()->saveToStream(stream, !saveWithScene());
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void ParticlePropertyObject::loadFromStream(ObjectLoadStream& stream)
{
	SceneObject::loadFromStream(stream);

	stream.expectChunk(0x01);
	_storage.data()->loadFromStream(stream);
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> ParticlePropertyObject::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<ParticlePropertyObject> clone = static_object_cast<ParticlePropertyObject>(SceneObject::clone(deepCopy, cloneHelper));

	// Shallow copy storage.
	clone->_storage = this->_storage;

	return clone;
}

};	// End of namespace
