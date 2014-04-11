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
 * \brief An input stream that is loads OVITO objects from a file that has been create by an ObjectSaveStream.
 */
class OVITO_CORE_EXPORT ObjectLoadStream : public LoadStream
{
	Q_OBJECT

public:

	/// Data structure describing a property or reference field of a RefMaker-derived
	/// class, which has been stored in the file.
	struct SerializedPropertyField {

		/// The identifier of the property field.
		QByteArray identifier;

		/// The RefMaker-derived class that owns the property field.
		OvitoObjectType* definingClass;

		/// The stored flags of the property field (see PropertyFieldFlag).
		int flags;

		/// Indicates whether this is a reference field or a property field.
		bool isReferenceField;

		/// If this is a reference field, this is its RefTarget-derived class.
		OvitoObjectType* targetClass;

		/// The property field of the defining class that matches the
		/// stored field. Can be NULL if the property field no longer exists.
		const PropertyFieldDescriptor* field;
	};

	/// \brief Opens a stream for reading.
	/// \param source The data stream from which the data is read. This must be a supporting random access.
	/// \throw Exception when the source stream does not support random access.
	ObjectLoadStream(QDataStream& source);

	/// \brief The destructor calls close().
	virtual ~ObjectLoadStream() { close(); }

	/// \brief Closes the stream.
	/// \note The underlying input stream is not closed by this method.
	virtual void close();

	/// Loads an object from the stream.
	/// Note that the returned object is still uninitialized when the function returns.
	/// The actual object data is loaded on a call to close().
	template<class T>
	OORef<T> loadObject() {
		OORef<OvitoObject> ptr = loadObjectInternal();
		OVITO_ASSERT(!ptr || ptr->getOOType().isDerivedFrom(T::OOType));
		if(ptr && !ptr->getOOType().isDerivedFrom(T::OOType))
			throw Exception(tr("Class hierarchy mismatch in file. The object class '%1' is not derived from class '%2' as expected.").arg(ptr->getOOType().name()).arg(T::OOType.name()));
		return static_object_cast<T>(ptr);
	}

private:

	/// Loads an object with runtime type information from the stream.
	OORef<OvitoObject> loadObjectInternal();

	/// Data structure describing a class of objects stored in the file.
	struct ClassEntry {

		/// The corresponding runtime class.
		OvitoObjectType* descriptor;

		/// The list of reference and property fields stored for each instance of the class
		/// in the file.
		QVector<SerializedPropertyField> propertyFields;
	};
	
	/// Data structure describing an object instance loaded from the file.
	struct ObjectEntry {
		/// The runtime instance.
		OORef<OvitoObject> object;
		/// The class information.
		ClassEntry* pluginClass;
		/// The position at which the object data is stored in the file.
		qint64 fileOffset;
	};

	/// The list of classes stored in the file.
	QVector<ClassEntry> _classes;
	
	/// List all the object instances stored in the file.
	QVector<ObjectEntry> _objects;

	/// Objects that need to be loaded.
	QVector<quint32> _objectsToLoad;
	
	/// The object currently being loaded from the stream.
	ObjectEntry* _currentObject;
	
	/// The current dataset being loaded.
	DataSet* _dataset;

	friend class RefMaker;
};

};

#endif // __OVITO_OBJECT_LOADSTREAM_H
