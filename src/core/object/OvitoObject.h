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
 * \file OvitoObject.h
 * \brief Contains the definition of the Ovito::OvitoObject class.
 */

#ifndef __OVITO_OBJECT_H
#define __OVITO_OBJECT_H

#include <core/Core.h>
#include "OvitoObjectReference.h"
#include "NativeOvitoObjectType.h"

namespace Ovito {

class ObjectSaveStream;		// defined in ObjectSaveStream.h
class ObjectLoadStream;		// defined in ObjectLoadStream.h

#ifdef OVITO_DEBUG
	/// Checks whether a pointer to an OvitoObject is valid.
	#define OVITO_CHECK_OBJECT_POINTER(object) { OVITO_CHECK_POINTER(object); OVITO_ASSERT_MSG((object)->__isObjectAlive(), "OVITO_CHECK_OBJECT_POINTER", "OvitoObject pointer is invalid. Object has been deleted."); }
#else
	/// Do nothing for release builds.
	#define OVITO_CHECK_OBJECT_POINTER(object)
#endif

/**
 * \brief Universal base class for most core and plug-in classes.
 */
class OVITO_CORE_EXPORT OvitoObject : public QObject
{
public:

	/// \brief The default constructor. Sets the reference count to zero.
	OvitoObject() : _referenceCount(0) {
#ifdef OVITO_DEBUG
		_magicAliveCode = 0x87ABCDEF;
#endif
	}

	/// \brief The destructor.
	///
	/// The default implementation does nothing.
	virtual ~OvitoObject() {
#ifdef OVITO_DEBUG
		OVITO_CHECK_OBJECT_POINTER(this);
		_magicAliveCode = 0xFEDCBA87;
#endif
	}

	/// \brief Saves the class' contents to an output stream.
	/// \param stream The destination stream.
	///
	/// Derived classes can overwrite this virtual method to store their specific data
	/// in the output stream. The derived class \b must always call the base implementation of the saveToStream() method
	/// before it writes its own data to the stream.
	///
	/// The base implementation of OvitoObject::saveToStream() does nothing.
	///
	/// \sa loadFromStream()
	virtual void saveToStream(ObjectSaveStream& stream) {}

	/// \brief Loads the class' contents from an input stream.
	/// \param stream The source stream.
	/// \throw Exception when a parsing error has occurred.
	///
	/// Derived classes can overwrite this virtual method to read their specific data
	/// from the input stream. The derived class \b must always call the loadFromStream() method
	/// of the base class before reading its own data from the stream.
	///
	/// The base implementation of OvitoObject::loadFromStream() method does nothing.
	///
	/// \note An OvitoObject is not in a fully initialized state when its loadFromStream() method is called.
	///       In particular the developer cannot assume that all other objects stored in the data stream and
	///       referenced by this object have already been restored at the time loadFromStream() is called.
	///       The OvitoObject::loadFromStreamComplete() method will be called for every OvitoObject-derived object
	///       after all objects have been completely deserialized. If the object has to do some post-deserialization
	///       tasks that rely on other objects referenced by this object then this should be done in an implementation of
	///       the loadFromStreamComplete() method.
	///
	/// \sa saveToStream(), loadFromStreamComplete()
	virtual void loadFromStream(ObjectLoadStream& stream) {}

	/// This method is called once for this object after it has been
	/// loaded from the input stream and each other object in the stream
	/// has been loaded as well. This function can therefore safely access
	/// sub-objects stored in this class that have been loaded via ObjectLoadStream::loadObject().
	/// \sa loadFromStream()
	virtual void loadFromStreamComplete() {}

	/// Returns true if this object is currently being loaded from an ObjectLoadStream.
	bool isBeingLoaded() const;

	/// \brief Returns the current value of the object's reference counter.
	/// \return The reference count for this object.
	size_t objectReferenceCount() const { return _referenceCount; }

#ifdef OVITO_DEBUG
	/// \brief Returns whether this object has not been deleted yet.
	bool __isObjectAlive() const { return _magicAliveCode == 0x87ABCDEF; }
#endif

protected:

	/// \brief Calls "delete this" on this object.
	///
	/// This method is called when the reference counter of this object has reached zero.
	virtual void deleteThis() {
		OVITO_CHECK_OBJECT_POINTER(this);
		delete this;
	}

private:

	/// The number of references to this object.
	size_t _referenceCount;

private:

	/// \brief Increments the reference count by one.
	/// \sa decrementReferenceCount()
	void incrementReferenceCount() {
		OVITO_CHECK_OBJECT_POINTER(this);
		++_referenceCount;
	}

	/// \brief Decrements the reference count by one.
	///
	/// If the reference count becomes zero then the object is auto-deleted.
	/// \sa autoDeleteObject(), incrementReferenceCount()
	void decrementReferenceCount() {
		OVITO_CHECK_OBJECT_POINTER(this);
		OVITO_ASSERT_MSG(_referenceCount > 0, "OvitoObject::decrementReferenceCount()", "Reference count became negative.");
		if(--_referenceCount == 0) {
			deleteThis();
		}
	}

#ifdef OVITO_DEBUG
	/// This field is initialized with a special value in the constructor to indicate that
	/// the object is alive and has not been deleted. When the object is deleted, the
	/// destructor sets the field to another value indicate that the object is no longer alive.
	quint32 _magicAliveCode;
#endif

private:

	Q_OBJECT
	OVITO_OBJECT

	template<class T> friend class OORef;	// Give OORef smart pointer access to the inernal reference count.
	friend class VectorReferenceFieldBase;
};

/// \brief Dynamic casting function for OvitoObject derived classes.
///
/// Returns the given object cast to type \c T if the object is of type \c T
/// (or of a subclass); otherwise returns \c NULL.
template<class T, class U>
inline T* dynamic_object_cast(U* obj) {
	return qobject_cast<T*>(obj);
}

/// \brief Dynamic casting function for OvitoObject derived classes.
///
/// Returns the given object cast to type \c T if the object is of type \c T
/// (or of a subclass); otherwise returns \c NULL.
template<class T, class U>
inline const T* dynamic_object_cast(const U* obj) {
	return qobject_cast<const T*>(obj);
}

/// \brief Static casting function for OvitoObject derived classes.
///
/// Returns the given object cast to type \c T.
/// Performs a runtime check of the object type in debug build.
template<class T, class U>
inline T* static_object_cast(U* obj) {
	OVITO_ASSERT_MSG(!obj || obj->getOOType().isDerivedFrom(T::OOType), "static_object_cast", "Runtime type check failed. The source object is not an instance of the target class.");
	return static_cast<T*>(obj);
}

/// \brief Static casting function for OvitoObject derived object.
///
/// Returns the given object cast to type \c T.
/// Performs a runtime check of the object type in debug build.
template<class T, class U>
inline const T* static_object_cast(const U* obj) {
	OVITO_ASSERT_MSG(!obj || obj->getOOType().isDerivedFrom(T::OOType), "static_object_cast", "Runtime type check failed. The source object is not an instance of the target class.");
	return static_cast<const T*>(obj);
}


/// \brief Dynamic casting function for smart pointer to OvitoObject.
///
/// Returns the given object cast to type \c T if the object is of type \c T
/// (or of a subclass); otherwise returns \c NULL.
template<class T, class U>
inline OORef<T> dynamic_object_cast(const OORef<U>& obj) {
	return OORef<T>(qobject_cast<T*>(obj.get()));
}

/// \brief Static casting function for smart pointers to OvitoObject derived objects.
///
/// Returns the given object cast to type \c T.
/// Performs a runtime check of the object type in debug build.
template<class T, class U>
inline OORef<T> static_object_cast(const OORef<U>& obj) {
	OVITO_ASSERT_MSG(!obj || obj->getOOType().isDerivedFrom(T::OOType), "static_object_cast", "Runtime type check failed. The source object is not an instance of the target class.");
	return static_pointer_cast<T>(obj);
}

};	// End of namespace Ovito

Q_DECLARE_METATYPE(Ovito::OORef<Ovito::OvitoObject>);
Q_DECLARE_TYPEINFO(Ovito::OORef<Ovito::OvitoObject>, Q_MOVABLE_TYPE);

#include <core/utilities/io/ObjectSaveStream.h>
#include <core/utilities/io/ObjectLoadStream.h>

#endif // __OVITO_OBJECT_H
