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
#include <core/utilities/io/ObjectSaveStream.h>
#include <core/object/OvitoObject.h>
#include <core/reference/PropertyFieldDescriptor.h>
#include <core/dataset/DataSet.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(IO)

using namespace std;

/******************************************************************************
* The destructor closes the stream.
******************************************************************************/
ObjectSaveStream::~ObjectSaveStream()
{
	try {
		close();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Saves an object with runtime type information to the stream.
******************************************************************************/
void ObjectSaveStream::saveObject(OvitoObject* object)
{
	if(object == nullptr) *this << (quint32)0;
	else {
		OVITO_CHECK_OBJECT_POINTER(object);
		OVITO_ASSERT(_objects.size() == _objectMap.size());
		quint32& id = _objectMap[object];
		if(id == 0) {
			_objects.push_back(object);
			id = (quint32)_objects.size();

			if(object->getOOType() == DataSet::OOType)
				_dataset = static_object_cast<DataSet>(object);

			OVITO_ASSERT(!object->getOOType().isDerivedFrom(RefTarget::OOType) || static_object_cast<RefTarget>(object)->dataset() == _dataset);
		}
		*this << id;
	}
}

/******************************************************************************
* Closes the stream.
******************************************************************************/
void ObjectSaveStream::close()
{
	if(!isOpen())
		return;

	try {
		QVector<qint64> objectOffsets;

		// Save all objects.
		beginChunk(0x100);
		for(int i = 0; i < _objects.size(); i++) {
			OvitoObject* obj = _objects[i];
			OVITO_CHECK_OBJECT_POINTER(obj);
			objectOffsets.push_back(filePosition());
			obj->saveToStream(*this);
		}
		endChunk();

		// Save RTTI.
		map<const OvitoObjectType*, quint32> classes;
		qint64 beginOfRTTI = filePosition();
		beginChunk(0x200);
		Q_FOREACH(OvitoObject* obj, _objects) {
			const OvitoObjectType* descriptor = &obj->getOOType();
			if(classes.find(descriptor) == classes.end()) {
				classes.insert(make_pair(descriptor, (quint32)classes.size()));
				// Write the runtime type information to the stream.
				if(descriptor->isSerializable() == false)
					throw Exception(tr("Failed to save class %1 because it is marked as non-serializable.").arg(descriptor->name()));
				beginChunk(0x201);
				OvitoObjectType::serializeRTTI(*this, descriptor);
				endChunk();
				// Write the property fields to the stream.
				beginChunk(0x202);
				for(const OvitoObjectType* clazz = descriptor; clazz; clazz = clazz->superClass()) {
					for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field; field = field->next()) {
						beginChunk(0x01);
						*this << QByteArray::fromRawData(field->identifier(), qstrlen(field->identifier()));
						OVITO_ASSERT(field->definingClass() == clazz);
						OvitoObjectType::serializeRTTI(*this, field->definingClass());
						*this << field->flags();
						*this << field->isReferenceField();
						if(field->isReferenceField()) {
							OvitoObjectType::serializeRTTI(*this, field->targetClass());
						}
						endChunk();
					}
				}
				// Write list terminator.
				beginChunk(0x00000000);
				endChunk();

				endChunk();
			}
		}
		endChunk();

		// Save object table.
		qint64 beginOfObjTable = filePosition();
		beginChunk(0x300);
		auto offsetIterator = objectOffsets.constBegin();
		Q_FOREACH(OvitoObject* obj, _objects) {
			*this << classes[&obj->getOOType()];
			*this << *offsetIterator++;
		}
		endChunk();

		// Write index.
		*this << beginOfRTTI;
		*this << (quint32)classes.size();
		*this << beginOfObjTable;
		*this << (quint32)_objects.size();
	}
	catch(...) {
		SaveStream::close();
		throw;
	}
	SaveStream::close();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
