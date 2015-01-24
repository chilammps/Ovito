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

#ifndef __OVITO_PROPERTY_FIELD_DESCRIPTOR_H
#define __OVITO_PROPERTY_FIELD_DESCRIPTOR_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

class SingleReferenceFieldBase;		// defined in PropertyField.h
class VectorReferenceFieldBase;		// defined in PropertyField.h

/// Bit-flags controlling the behavior of a property field.
enum PropertyFieldFlag
{
	/// Selects the default behavior.
	PROPERTY_FIELD_NO_FLAGS				= 0,
	/// Indicates that a reference field is a vector of references.
	PROPERTY_FIELD_VECTOR				= (1<<1),
	/// Do not create automatic undo records when the value of the property or reference field changes.
	PROPERTY_FIELD_NO_UNDO				= (1<<2),
	/// Create a weak reference to the reference target.
	PROPERTY_FIELD_WEAK_REF				= (1<<3),
	/// Controls whether or not a ReferenceField::TargetChanged event should
	/// be generated each time the property value changes.
	PROPERTY_FIELD_NO_CHANGE_MESSAGE	= (1<<4),
	/// The target of the reference field is never cloned when the owning object is cloned.
	PROPERTY_FIELD_NEVER_CLONE_TARGET	= (1<<5),
	/// The target of the reference field is shallow/deep copied depending on the mode when the owning object is cloned.
	PROPERTY_FIELD_ALWAYS_CLONE         = (1<<6),
	/// The target of the reference field is always deep-copied completely when the owning object is cloned.
	PROPERTY_FIELD_ALWAYS_DEEP_COPY		= (1<<7),
	/// Save the last value of the property in the application's settings store and use it to initialize
	/// the property when a new object instance is created.
	PROPERTY_FIELD_MEMORIZE				= (1<<8),
	/// Indicates that the reference field is NOT an animatable parameter owned by the RefMaker object.
	PROPERTY_FIELD_NO_SUB_ANIM			= (1<<9)
};
Q_DECLARE_FLAGS(PropertyFieldFlags, PropertyFieldFlag);
Q_DECLARE_OPERATORS_FOR_FLAGS(PropertyFieldFlags);

/**
 * \brief This class describes one member field of a RefMaker that stores a property of the object.
 */
class OVITO_CORE_EXPORT PropertyFieldDescriptor
{
public:

	/// Constructor	for a property field that stores a non-animatable property.
	PropertyFieldDescriptor(const NativeOvitoObjectType* definingClass, const char* identifier, PropertyFieldFlags flags,
			QVariant (*_propertyStorageReadFunc)(RefMaker*), void (*_propertyStorageWriteFunc)(RefMaker*, const QVariant&),
			void (*_propertyStorageSaveFunc)(RefMaker*, SaveStream&), void (*_propertyStorageLoadFunc)(RefMaker*, LoadStream&))
		: _definingClassDescriptor(definingClass), _targetClassDescriptor(nullptr), _identifier(identifier), _flags(flags), singleStorageAccessFunc(nullptr), vectorStorageAccessFunc(nullptr),
			propertyStorageReadFunc(_propertyStorageReadFunc), propertyStorageWriteFunc(_propertyStorageWriteFunc),
			propertyStorageSaveFunc(_propertyStorageSaveFunc), propertyStorageLoadFunc(_propertyStorageLoadFunc) {
		OVITO_ASSERT(_identifier != nullptr);
		OVITO_ASSERT(!_flags.testFlag(PROPERTY_FIELD_VECTOR));
		OVITO_ASSERT(definingClass != nullptr);
		// Make sure that there is no other reference field with the same identifier in the defining class.
		OVITO_ASSERT_MSG(definingClass->findPropertyField(identifier) == nullptr, "PropertyFieldDescriptor", "Property field identifier is not unique.");
		// Insert into linked list of reference fields stored in the defining class' descriptor.
		this->_next = definingClass->_firstPropertyField;
		const_cast<NativeOvitoObjectType*>(definingClass)->_firstPropertyField = this;
	}

	/// Constructor	for a property field that stores a single reference to a RefTarget.
	PropertyFieldDescriptor(const NativeOvitoObjectType* definingClass, const OvitoObjectType* targetClass, const char* identifier, PropertyFieldFlags flags, SingleReferenceFieldBase& (*_storageAccessFunc)(RefMaker*))
		: _definingClassDescriptor(definingClass), _targetClassDescriptor(targetClass), _identifier(identifier), _flags(flags), singleStorageAccessFunc(_storageAccessFunc), vectorStorageAccessFunc(nullptr),
			propertyStorageReadFunc(nullptr), propertyStorageWriteFunc(nullptr), propertyStorageSaveFunc(nullptr), propertyStorageLoadFunc(nullptr) {
		OVITO_ASSERT(_identifier != nullptr);
		OVITO_ASSERT(singleStorageAccessFunc != nullptr);
		OVITO_ASSERT(!_flags.testFlag(PROPERTY_FIELD_VECTOR));
		OVITO_ASSERT(definingClass != nullptr);
		OVITO_ASSERT(targetClass != nullptr);
		// Make sure that there is no other reference field with the same identifier in the defining class.
		OVITO_ASSERT_MSG(definingClass->findPropertyField(identifier) == nullptr, "PropertyFieldDescriptor", "Property field identifier is not unique.");
		// Insert into linked list of reference fields stored in the defining class' descriptor.
		this->_next = definingClass->_firstPropertyField;
		const_cast<NativeOvitoObjectType*>(definingClass)->_firstPropertyField = this;
	}

	/// Constructor	for a property field that stores a vector of references to RefTarget objects.
	PropertyFieldDescriptor(const NativeOvitoObjectType* definingClass, const OvitoObjectType* targetClass, const char* identifier, PropertyFieldFlags flags, VectorReferenceFieldBase& (*_storageAccessFunc)(RefMaker*))
		: _definingClassDescriptor(definingClass), _targetClassDescriptor(targetClass), _identifier(identifier), _flags(flags), singleStorageAccessFunc(nullptr), vectorStorageAccessFunc(_storageAccessFunc),
			propertyStorageReadFunc(nullptr), propertyStorageWriteFunc(nullptr), propertyStorageSaveFunc(nullptr), propertyStorageLoadFunc(nullptr) {
		OVITO_ASSERT(_identifier != nullptr);
		OVITO_ASSERT(vectorStorageAccessFunc != nullptr);
		OVITO_ASSERT(_flags.testFlag(PROPERTY_FIELD_VECTOR));
		OVITO_ASSERT(definingClass != NULL);
		OVITO_ASSERT(targetClass != NULL);
		// Make sure that there is no other reference field with the same identifier in the defining class.
		OVITO_ASSERT_MSG(definingClass->findPropertyField(identifier) == nullptr, "PropertyFieldDescriptor", "Property field identifier is not unique.");
		// Insert into linked list of reference fields stored in the defining class' descriptor.
		this->_next = definingClass->_firstPropertyField;
		const_cast<NativeOvitoObjectType*>(definingClass)->_firstPropertyField = this;
	}

	/// Returns the unique identifier of the reference field.
	const char* identifier() const { return _identifier; }

	/// Returns the RefMaker derived class that owns the reference.
	const OvitoObjectType* definingClass() const { return _definingClassDescriptor; }

	/// Returns the base type of the objects stored in this property field if it is a reference field; otherwise returns NULL.
	const OvitoObjectType* targetClass() const { return _targetClassDescriptor; }

	/// Returns whether this is a reference field that stores a pointer to a RefTarget derived class.
	bool isReferenceField() const { return _targetClassDescriptor != nullptr; }

	/// Returns whether this reference field stores weak references.
	bool isWeakReference() const { return _flags.testFlag(PROPERTY_FIELD_WEAK_REF); }

	/// Returns true if this reference field stores a vector of objects.
	bool isVector() const { return _flags.testFlag(PROPERTY_FIELD_VECTOR); }

	/// Indicates that automatic undo-handling for this property field is enabled.
	/// This is the default.
	bool automaticUndo() const { return !_flags.testFlag(PROPERTY_FIELD_NO_UNDO); }

	/// Returns true if a TargetChanged event should be generated each time the property's value changes.
	bool shouldGenerateChangeEvent() const { return !_flags.testFlag(PROPERTY_FIELD_NO_CHANGE_MESSAGE); }

    /// Returns the human readable and localized name of the property field.
	/// It will be used as label text in the user interface.
	QString displayName() const;

	/// Returns the next property field in the linked list (of the RefMaker derived class defining this property field).
	const PropertyFieldDescriptor* next() const { return _next; }

	/// Returns the ParameterUnit-derived class assigned to a numerical property or controller field.
	const QMetaObject* parameterUnitType() const { return _parameterUnitType; }

	/// Returns the flags that control the behavior of the property field.
	PropertyFieldFlags flags() const { return _flags; }

	/// Compares two property fields.
	bool operator==(const PropertyFieldDescriptor& other) const { return (this == &other); }

	/// Compares two property fields.
	bool operator!=(const PropertyFieldDescriptor& other) const { return (this != &other); }

	/// Saves the current value of a property field in the application's settings store.
	void memorizeDefaultValue(RefMaker* object) const;

	/// Loads the default value of a property field from the application's settings store.
	bool loadDefaultValue(RefMaker* object) const;

protected:

	/// The unique identifier of the reference field. This must be unique within
	/// a RefMaker derived class.
	const char* _identifier;

	/// The base type of the objects stored in this field if this is a reference field.
	const OvitoObjectType* _targetClassDescriptor;

	/// The RefMaker derived class that owns the property.
	const OvitoObjectType* _definingClassDescriptor;

	/// The next property field in the linked list (of the RefMaker derived class defining this property field).
	const PropertyFieldDescriptor* _next;

	/// The flags that control the behavior of the property field.
	PropertyFieldFlags _flags;

	/// Stores a pointer to the function that can be used to read the property field's
	/// value for a certain RefMaker instance.
	QVariant (*propertyStorageReadFunc)(RefMaker*);

	/// Stores a pointer to the function that can be used to write the property field's
	/// value for a certain RefMaker instance.
	void (*propertyStorageWriteFunc)(RefMaker*, const QVariant&);

	/// Stores a pointer to the function that can be used to save the property field's
	/// value to a stream.
	void (*propertyStorageSaveFunc)(RefMaker*, SaveStream&);

	/// Stores a pointer to the function that can be used to load the property field's
	/// value from a stream.
	void (*propertyStorageLoadFunc)(RefMaker*, LoadStream&);

	/// Stores a pointer to the function that can be used to access the single reference field's
	/// value for a certain RefMaker instance.
	SingleReferenceFieldBase& (*singleStorageAccessFunc)(RefMaker*);

	/// Stores a pointer to the function that can be used to access the vector reference field's
	/// values for a certain RefMaker instance.
	VectorReferenceFieldBase& (*vectorStorageAccessFunc)(RefMaker*);

	/// The human-readable name of this property field. It will be used
	/// as label text in the user interface.
	QString _displayName;

	/// A ParameterUnit-derived class which is assigned to a numerical property or controller.
	const QMetaObject* _parameterUnitType;

	friend class RefMaker;
	friend class RefTarget;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_PROPERTY_FIELD_DESCRIPTOR_H
