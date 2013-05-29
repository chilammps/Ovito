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

/** 
 * \file ObjectLoadStream.h 
 * \brief Contains definition of the Ovito::ObjectLoadStream class.
 */

#ifndef __OVITO_OBJECT_LOADSTREAM_H
#define __OVITO_OBJECT_LOADSTREAM_H

#include <core/Core.h>
#include <base/io/LoadStream.h>
#include <core/object/OvitoObject.h>
#include <core/object/OvitoObjectReference.h>

namespace Ovito {

/**
 * \brief An input stream that is used to parse a graph of objects from a file.
 * 
 * \sa ObjectSaveStream
 * \author Alexander Stukowski
 */
class ObjectLoadStream : public LoadStream
{
	Q_OBJECT

public:

	/// \brief Opens the stream for reading.
	/// \param source The data stream from which the binary data is read. This must be 
	///               stream that supports random access.
	/// \throw Exception when the given data stream \a source does only support sequential access. 	
	ObjectLoadStream(QDataStream& source);

	/// \brief The destructor closes the stream.
	/// \sa close()
	virtual ~ObjectLoadStream() { close(); }

	/// \brief Closes the stream.
	/// \note The underlying data stream is not closed by this method.
	virtual void close();

	/// Loads an object with runtime type information from the stream.
	/// The method returns a pointer to the object but this object will be
	/// in an uninitialized state until it is loaded at a later time.
	template<class T>
	OORef<T> loadObject() {
		OORef<OvitoObject> ptr(loadObject());
		OVITO_ASSERT(!ptr || ptr->getOOType().isDerivedFrom(T::OOType));
		if(ptr && !ptr->getOOType().isDerivedFrom(T::OOType))
			throw Exception(tr("Class hierarchy mismatch in file. The object class '%1' is not derived from class '%2' as expected.").arg(ptr->getOOType().name()).arg(T::OOType.name()));
		return static_object_cast<T>(ptr);
	}

private:

	/// Loads an object with runtime type information from the stream.
	/// The method returns a pointer to the object but this object will be
	/// in an uninitialized state until it is loaded at a later time.
	OORef<OvitoObject> loadObject();
	
	struct PropertyFieldEntry {
		QByteArray identifier;
		OvitoObjectType* definingClass;
		int flags;
		bool isReferenceField;
		OvitoObjectType* targetClass;
		const PropertyFieldDescriptor* field;
	};

	struct ClassEntry {
		OvitoObjectType* descriptor;
		QVector<PropertyFieldEntry> propertyFields;
	};
	
	struct ObjectEntry {
		OORef<OvitoObject> object;
		ClassEntry* pluginClass;
		qint64 fileOffset;
	};

	/// The plugin classes used in the current scene file.
	QVector<ClassEntry> _classes;
	
	/// List all the objects of the current scene file.
	QVector<ObjectEntry> _objects;

	/// Indices of those objects that need to be loaded.
	QVector<quint32> _objectsToLoad;
	
	/// This points to the current object while the objects are being loaded from the stream.
	ObjectEntry* _currentObject;
	
	friend class RefMaker;
};

};

#endif // __OVITO_OBJECT_LOADSTREAM_H
