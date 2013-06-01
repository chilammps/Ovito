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
 * \file OvitoObjectType.h
 * \brief Contains the definition of the Ovito::OvitoObjectType class.
 */

#ifndef __OVITO_OBJECT_TYPE_H
#define __OVITO_OBJECT_TYPE_H

#include <core/Core.h>
#include "OvitoObjectReference.h"

namespace Ovito {

class OvitoObjectType;				// defined below
class OvitoObject;					// defined in OvitoObject.h
class PropertyFieldDescriptor;		// defined in PropertyFieldDescriptor.h
class ObjectSaveStream;				// defined in ObjectSaveStream.h
class ObjectLoadStream;				// defined in ObjectLoadStream.h
class Plugin;						// defined in Plugin.h

/**
 * \brief Stores meta-information about a class in Ovito's object system.
 */
class OvitoObjectType
{
public:

	/// \brief Returns the name of the OvitoObject-derived class.
	/// \return The name of the class (without namespace qualifier).
	const QString& name() const { return _name; }

	/// \brief Returns the descriptor of the super class.
	/// \return The descriptor of the base class or \c NULL if this is the descriptor of the root OvitoObject class.
	const OvitoObjectType* superClass() const { return _superClass; }

	/// \brief Returns the plugin that contains the class.
	/// \return The plugin that defines this class.
	Plugin* plugin() const { return _plugin; }

	/// \brief Indicates whether this is an abstract class.
	/// \return \c true if this is an abstract class that cannot be instantiated using createInstance();
	///         \c false otherwise.
	/// \sa createInstance()
	bool isAbstract() const { return _isAbstract; }

	/// \brief Returns whether this class can be written to file.
	/// \return \c true if instances of this class can be serialized to a file;
	///         \c false otherwise.
	/// \note A class is only serializable if all its base classes are also serializable.
	bool isSerializable() const {
		OVITO_ASSERT_MSG(superClass() == NULL || !(_isSerializable && !superClass()->isSerializable()), "OvitoObjectType::isSerializable", "Class derived from a non-serializable class has to be non-serializable too.");
		return _isSerializable;
	}

	/// \brief Returns whether this class is directly or indirectly derived from some other class.
	/// \return \c true if this class is a kind of (inherits from) the given class;
	///         \c false otherwise.
	/// \note This method returns \c true if the given class \a other is the class itself.
	bool isDerivedFrom(const OvitoObjectType& other) const {
		for(const OvitoObjectType* c = this; c != NULL; c = c->superClass())
			if(c == &other) return true;
		return false;
	}

	/// \brief Creates an instance of the OvitoObject-derived class.
	/// \return The new instance of the class. The pointer can safely be cast to the appropriate C++ class type.
	/// \throw Exception if a plugin failed to load or the instantiation failed for some other reason.
	OORef<OvitoObject> createInstance() const;

	/// \brief Returns the first element of the linked list of reference fields defined for this class if it is a RefMaker derived class.
	const PropertyFieldDescriptor* firstPropertyField() const { return _firstPropertyField; }

	/// If this is the descriptor of a RefMaker-derived class then this method will return
	/// the reference field with the given identifier that has been defined in the RefMaker-derived
	/// class. If no such field is defined by that class then NULL is returned.
	/// Note that this method will NOT return reference fields that have been defined in
	/// super-classes.
	const PropertyFieldDescriptor* findPropertyField(const char* identifier) const;

	/// Compares two types.
	bool operator==(const OvitoObjectType& other) const { return (this == &other); }

	/// Compares two types.
	bool operator!=(const OvitoObjectType& other) const { return (this != &other); }

	/// \brief Writes a type descriptor to the stream.
	/// \note This method is for internal use only.
	static void serializeRTTI(ObjectSaveStream& stream, const OvitoObjectType* type);

	/// \brief Loads a type descriptor from the stream.
	/// \throw Exception if the class is not defined or the required plugin is not installed.
	/// \note This method is for internal use only.
	static OvitoObjectType* deserializeRTTI(ObjectLoadStream& stream);

protected:

	/// \brief Constructor.
	OvitoObjectType(const QString& name, const OvitoObjectType* superClass, bool isAbstract, bool serializable);

	/// \brief Creates an instance of the class described by this meta object.
	/// \return The new instance of the class. The pointer can safely be cast to the C++ class type.
	/// \throw Exception if the instance could not be created.
	virtual OORef<OvitoObject> createInstanceImpl() const = 0;

protected:

	/// The class name.
	QString _name;

	/// The plugin that defined the class.
	Plugin*	_plugin;

	/// The base class descriptor (or NULL if this is the descriptor for the root OvitoObject class).
	const OvitoObjectType* _superClass;

	/// Indicates whether the class is abstract.
	bool _isAbstract;

	/// Indicates whether the objects of the class can be serialized.
	bool _isSerializable;

	/// The linked list of property fields if the class is derived from RefMaker.
	const PropertyFieldDescriptor* _firstPropertyField;
};

};

#endif // __OVITO_OBJECT_TYPE_H
