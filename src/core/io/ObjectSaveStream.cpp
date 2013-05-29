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
#include <core/io/ObjectSaveStream.h>
#include <core/object/OvitoObject.h>

namespace Ovito {

using namespace std;

/******************************************************************************
* Saves an object with runtime type information to the stream.
******************************************************************************/
void ObjectSaveStream::saveObject(OvitoObject* object)
{
	if(object == NULL) *this << (quint32)0;
	else {
		OVITO_CHECK_OBJECT_POINTER(object);
		OVITO_ASSERT(_objects.size() == _objectMap.size());
		quint32& id = _objectMap[object];
		if(id == 0) {
			_objects.push_back(object);
			id = (quint32)_objects.size();
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
	map<OvitoObjectType*, quint32> classes;
	qint64 beginOfRTTI = filePosition();
	beginChunk(0x200);
	Q_FOREACH(OvitoObject* obj, _objects) {
		OvitoObjectType* descriptor = obj->getOOType();
		if(classes.find(descriptor) == classes.end()) {
			classes.insert(make_pair(descriptor, (quint32)classes.size()));
			// Write the runtime type information to the stream.
			OvitoObjectType::serializeRTTI(*this, descriptor);
			// Write the property fields to the stream.
			beginChunk(0x202);
			for(const OvitoObjectType* clazz = descriptor; clazz; clazz = clazz->superClass()) {
#if 0
				for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field; field = field->next()) {
					beginChunk(0x00000001);
					*this << QByteArray(field->identifier());
					OVITO_ASSERT(field->definingClass() == clazz);
					PluginClassDescriptor::saveRTTI(*this, field->definingClass());
					this->writeEnum(field->flags());
					*this << field->isReferenceField();
					if(field->isReferenceField()) {
						PluginClassDescriptor::saveRTTI(*this, field->targetClass());
					}
					endChunk();
				}
#endif
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
		*this << classes[obj->getOOType()];
		*this << *offsetIterator++;
	}
	endChunk();

	// Write index.
	*this << beginOfRTTI;
	*this << (quint32)classes.size();
	*this << beginOfObjTable;
	*this << (quint32)_objects.size();

	SaveStream::close();
}

};
