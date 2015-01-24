///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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
#include <core/utilities/io/ObjectLoadStream.h>
#include <core/reference/PropertyFieldDescriptor.h>
#include <core/plugins/Plugin.h>
#include <core/dataset/DataSet.h>
#include <core/object/OvitoObject.h>
#include <core/object/OvitoObjectReference.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(IO)

/******************************************************************************
* Opens the stream for reading.
******************************************************************************/
ObjectLoadStream::ObjectLoadStream(QDataStream& source) : LoadStream(source), _currentObject(nullptr), _dataset(nullptr)
{
	qint64 oldPos = filePosition();

	// Jump to index at the end of the file.
	setFilePosition(source.device()->size() - 2 * (qint64)(sizeof(qint64) + sizeof(quint32)));

	// Read index.
	qint64 beginOfRTTI, beginOfObjTable;
	quint32 classCount, objCount;
	*this >> beginOfRTTI;
	*this >> classCount;
	_classes.resize(classCount);
	*this >> beginOfObjTable;
	*this >> objCount;
	_objects.resize(objCount);

	// Jump to beginning of the class table, which is stored at the end of the file.
	setFilePosition(beginOfRTTI);
	expectChunk(0x200);
	for(ClassEntry& classEntry : _classes) {
		// Load the runtime type information from the stream.
		expectChunk(0x201);
		classEntry.descriptor = OvitoObjectType::deserializeRTTI(*this);
		if(classEntry.descriptor->isSerializable() == false)
			throw Exception(tr("Failed to load class %1, because it is flagged as non-serializable.").arg(classEntry.descriptor->name()));
		closeChunk();

		// Load the plugin containing the class now.
		classEntry.descriptor->plugin()->loadPlugin();

		// Read the stored property fields for this class from the stream.
		expectChunk(0x202);
		for(;;) {
			quint32 chunkId = openChunk();
			if(chunkId == 0x00000000) {
				closeChunk();
				break;	// end of list
			}
			if(chunkId != 0x00000001)
				throw Exception(tr("File format is invalid. Failed to load property fields of class %1.").arg(classEntry.descriptor->name()));

			SerializedPropertyField propFieldEntry;
			*this >> propFieldEntry.identifier;
			propFieldEntry.definingClass = OvitoObjectType::deserializeRTTI(*this);
			if(classEntry.descriptor->isDerivedFrom(*propFieldEntry.definingClass) == false) {
				qDebug() << "WARNING:" << classEntry.descriptor->name() << "is not derived from" << propFieldEntry.definingClass->name();
				throw Exception(tr("The class hierarchy stored in the file differs from the class hierarchy of the program."));
			}
			*this >> propFieldEntry.flags;
			*this >> propFieldEntry.isReferenceField;
			if(propFieldEntry.isReferenceField)
				propFieldEntry.targetClass = OvitoObjectType::deserializeRTTI(*this);
			else
				propFieldEntry.targetClass = nullptr;
			closeChunk();

			propFieldEntry.field = propFieldEntry.definingClass->findPropertyField(propFieldEntry.identifier.constData(), true);

			if(propFieldEntry.field) {
				if(propFieldEntry.field->isReferenceField() != propFieldEntry.isReferenceField ||
						propFieldEntry.field->isVector() != ((propFieldEntry.flags & PROPERTY_FIELD_VECTOR) != 0) ||
						(propFieldEntry.isReferenceField && propFieldEntry.targetClass->isDerivedFrom(*propFieldEntry.field->targetClass()) == false))
					throw Exception(tr("File format error: The type of the property field '%1' in class %2 has changed.").arg(propFieldEntry.identifier, propFieldEntry.definingClass->name()));
			}

			classEntry.propertyFields.push_back(propFieldEntry);
		}
		closeChunk();
	}
	closeChunk();

	// Jump to start of object table.
	setFilePosition(beginOfObjTable);
	expectChunk(0x300);
	for(ObjectEntry& entry : _objects) {
		entry.object = NULL;
		quint32 id;
		*this >> id;
		entry.pluginClass = &_classes[id];
		*this >> entry.fileOffset;
	}
	closeChunk();

	// Go back to previous position in file.
	setFilePosition(oldPos);
}

/******************************************************************************
* Loads an object with runtime type information from the stream.
* The method returns a pointer to the object but this object will be
* in an uninitialized state until it is loaded at a later time.
******************************************************************************/
OORef<OvitoObject> ObjectLoadStream::loadObjectInternal()
{
	quint32 id;
	*this >> id;
	if(id == 0) return nullptr;
	else {
		ObjectEntry& entry = _objects[id - 1];
		if(entry.object != nullptr) return entry.object;
		else {
			// Create an instance of the object class.
			entry.object = entry.pluginClass->descriptor->createInstance(_dataset);
			if(entry.pluginClass->descriptor == &DataSet::OOType)
				_dataset = static_object_cast<DataSet>(entry.object.get());
			else {
				OVITO_ASSERT(!entry.pluginClass->descriptor->isDerivedFrom(RefTarget::OOType) || _dataset != nullptr);
			}
			OVITO_ASSERT(!entry.pluginClass->descriptor->isDerivedFrom(RefTarget::OOType) || _dataset == static_object_cast<RefTarget>(entry.object)->dataset());

			_objectsToLoad.push_back(id - 1);
			return entry.object;
		}
	}
}

/******************************************************************************
* Closes the stream.
******************************************************************************/
void ObjectLoadStream::close()
{
	// This prevents re-entrance in case of an exception.
	if(!_currentObject) {

		for(int i = 0; i < _objectsToLoad.size(); i++) {
			quint32 index = _objectsToLoad[i];
			_currentObject = &_objects[index];
			OVITO_CHECK_OBJECT_POINTER(_currentObject->object);

			// Seek to object data.
			setFilePosition(_currentObject->fileOffset);

			// Load class contents.
			try {
				try {
					// Make the object being loaded a child of this stream object.
					// This is to let the OvitoObject::isBeingLoaded() function detect that
					// the object is being loaded from this stream.
					OVITO_ASSERT(_currentObject->object->parent() == nullptr);
					_currentObject->object->setParent(this);
					OVITO_ASSERT(_currentObject->object->isBeingLoaded());

					// Let the object load its data fields.
					_currentObject->object->loadFromStream(*this);

					OVITO_ASSERT(_currentObject->object->parent() == this);
					_currentObject->object->setParent(nullptr);
				}
				catch(...) {
					// Clean up.
					OVITO_CHECK_OBJECT_POINTER(_currentObject->object);
					_currentObject->object->setParent(nullptr);
					throw;
				}
			}
			catch(Exception& ex) {
				throw ex.appendDetailMessage(tr("Object of class type %1 failed to load.").arg(_currentObject->object->getOOType().name()));
			}
		}

		// Now that all references are in place call post-processing function on each loaded object.
		for(const ObjectEntry& entry : _objects)
			entry.object->loadFromStreamComplete();
	}
	LoadStream::close();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
