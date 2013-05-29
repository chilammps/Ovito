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

#ifndef __OVITO_NATIVE_PROPERTY_FIELD_DESCRIPTOR_H
#define __OVITO_NATIVE_PROPERTY_FIELD_DESCRIPTOR_H

#include <core/Core.h>
#include "PropertyFieldDescriptor.h"

namespace Ovito {

/******************************************************************************
* This structure describes one member field of a RefMaker object that stores
* a property of that object.
******************************************************************************/
class NativePropertyFieldDescriptor : public PropertyFieldDescriptor
{
public:

	/// Constructor	for a property field that stores a non-animatable.
	NativePropertyFieldDescriptor(NativeOvitoObjectType* definingClass, const char* identifier, PropertyFieldFlags flags,
			QVariant (*_propertyStorageReadFunc)(RefMaker*), void (*_propertyStorageWriteFunc)(RefMaker*, const QVariant&),
			void (*_propertyStorageSaveFunc)(RefMaker*, SaveStream&), void (*_propertyStorageLoadFunc)(RefMaker*, LoadStream&))
		: PropertyFieldDescriptor(identifier, flags, _propertyStorageReadFunc, _propertyStorageWriteFunc,
				_propertyStorageSaveFunc, _propertyStorageLoadFunc), _definingClassInfo(definingClass), _targetClassInfo(NULL) {
		OVITO_ASSERT(_definingClassInfo != NULL);
		OVITO_ASSERT(_identifier != NULL);
		// Make sure that there is no other reference field with the same identifier in the defining class.
		OVITO_ASSERT_MSG(_definingClassInfo->findNativePropertyField(_identifier) == NULL, "NativePropertyFieldDescriptor", "Property field identifier is not unique.");
		// Insert into linked list of refence fields stored in the defining class' descriptor.
		this->_next = _definingClassInfo->_firstNativePropertyField;
		_definingClassInfo->_firstNativePropertyField = this;
	}

	/// Constructor	for a property field that stores a single reference to a RefTarget.
	NativePropertyFieldDescriptor(NativeOvitoObjectType* definingClass, OvitoObjectType* targetClass, const char* identifier, PropertyFieldFlags flags, SingleReferenceFieldBase& (*_storageAccessFunc)(RefMaker*))
		: PropertyFieldDescriptor(identifier, flags, _storageAccessFunc), _definingClassInfo(definingClass), _targetClassInfo(targetClass) {
		OVITO_ASSERT(_definingClassInfo != NULL);
		OVITO_ASSERT(_targetClassInfo != NULL);
		// Make sure that there is no other reference field with the same identifier in the defining class.
		OVITO_ASSERT_MSG(_definingClassInfo->findNativePropertyField(_identifier) == NULL, "NativePropertyFieldDescriptor", "Property field identifier is not unique.");
		// Insert into linked list of refence fields stored in the defining class' descriptor.
		this->_next = _definingClassInfo->_firstNativePropertyField;
		_definingClassInfo->_firstNativePropertyField = this;
	}

	/// Constructor	for a property field that stores a vector of references to RefTarget objects.
	NativePropertyFieldDescriptor(NativeOvitoObjectType* definingClass, OvitoObjectType* targetClass, const char* identifier, PropertyFieldFlags flags, VectorReferenceFieldBase& (*_storageAccessFunc)(RefMaker*))
		: PropertyFieldDescriptor(identifier, flags, _storageAccessFunc), _definingClassInfo(definingClass), _targetClassInfo(targetClass) {
		OVITO_ASSERT(_definingClassInfo != NULL);
		OVITO_ASSERT(_targetClassInfo != NULL);
		// Make sure that there is no other reference field with the same identifier in the defining class.
		OVITO_ASSERT_MSG(_definingClassInfo->findNativePropertyField(_identifier) == NULL, "NativePropertyFieldDescriptor", "Property field identifier is not unique.");
		// Insert into linked list of refence fields stored in the defining class' descriptor.
		this->_next = _definingClassInfo->_firstNativePropertyField;
		_definingClassInfo->_firstNativePropertyField = this;
	}

public:

	// Internal helper class that is used to specify the units for a controller
	// property field. Do not use this class directly but use the
	// SET_PROPERTY_FIELD_UNITS macro instead.
	struct PropertyFieldUnitsSetter {
		PropertyFieldUnitsSetter(NativePropertyFieldDescriptor& propfield, OvitoObjectType* parameterUnitClass) {
			OVITO_ASSERT(parameterUnitClass != NULL);
			OVITO_ASSERT(propfield._parameterUnitClassInfo == NULL);
			propfield._parameterUnitClassInfo = parameterUnitClass;
		}
	};

	// Internal helper class that is used to specify the label text for a
	// property field. Do not use this class directly but use the
	// SET_PROPERTY_FIELD_LABEL macro instead.
	struct PropertyFieldDisplayNameSetter {
		PropertyFieldDisplayNameSetter(NativePropertyFieldDescriptor& propfield, const QString& label) {
			OVITO_ASSERT(propfield._displayName.isEmpty());
			propfield._displayName = label;
		}
	};
};

/***************** Macros to define reference fields in RefMaker-derived classes. ********************/

#define DECLARE_REFERENCE_FIELD(storageFieldName)		\
	public: 											\
		static SingleReferenceFieldBase& __access_reffield_##storageFieldName(RefMaker* obj);	\
		static NativePropertyFieldDescriptor __propFieldInstance##storageFieldName;					\
	private:

#define DEFINE_FLAGS_REFERENCE_FIELD(RefMakerClass, TargetClass, UniqueFieldIdentifier, Flags, storageFieldName)	\
	SingleReferenceFieldBase& RefMakerClass::__access_reffield_##storageFieldName(RefMaker* obj) {			\
		return static_cast<RefMakerClass*>(obj)->storageFieldName;											\
	}																										\
	NativePropertyFieldDescriptor RefMakerClass::__propFieldInstance##storageFieldName(							\
		&RefMakerClass::OOType, &TargetClass::OOType, 								\
		UniqueFieldIdentifier, Flags, RefMakerClass::__access_reffield_##storageFieldName 					\
	);

#define DEFINE_REFERENCE_FIELD(RefMakerClass, TargetClass, UniqueFieldIdentifier, storageFieldName)	\
	DEFINE_FLAGS_REFERENCE_FIELD(RefMakerClass, TargetClass, UniqueFieldIdentifier, PROPERTY_FIELD_NO_FLAGS, storageFieldName)

#define DECLARE_VECTOR_REFERENCE_FIELD(storageFieldName)\
	public: 											\
		static VectorReferenceFieldBase& __access_reffield_##storageFieldName(RefMaker* obj);	\
		static NativePropertyFieldDescriptor __propFieldInstance##storageFieldName;					\
	private:

#define DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(RefMakerClass, TargetClass, UniqueFieldIdentifier, Flags, storageFieldName)	\
	VectorReferenceFieldBase& RefMakerClass::__access_reffield_##storageFieldName(RefMaker* obj) {			\
		return static_cast<RefMakerClass*>(obj)->storageFieldName;											\
	}																										\
	NativePropertyFieldDescriptor RefMakerClass::__propFieldInstance##storageFieldName(							\
		&RefMakerClass::OOType, &TargetClass::OOType, 										\
		UniqueFieldIdentifier, Flags | PROPERTY_FIELD_VECTOR, RefMakerClass::__access_reffield_##storageFieldName 	\
	);

#define DEFINE_VECTOR_REFERENCE_FIELD(RefMakerClass, TargetClass, UniqueFieldIdentifier, storageFieldName)	\
	DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(RefMakerClass, TargetClass, UniqueFieldIdentifier, PROPERTY_FIELD_VECTOR, storageFieldName)

#define PROPERTY_FIELD_DESCRIPTOR(RefMakerClass, storageFieldName) \
	RefMakerClass::__propFieldInstance##storageFieldName

#define INIT_PROPERTY_FIELD(RefMakerClass, storageFieldName)		\
	storageFieldName.init(this, &PROPERTY_FIELD_DESCRIPTOR(RefMakerClass, storageFieldName));

#define SET_PROPERTY_FIELD_UNITS(RefMakerClass, storageFieldName, ParameterUnitClass)		\
	static NativePropertyFieldDescriptor::PropertyFieldUnitsSetter __unitsSetter##RefMakerClass##storageFieldName(PROPERTY_FIELD_DESCRIPTOR(RefMakerClass, storageFieldName), &ParameterUnitClass::__pluginClassInfo);

#define SET_PROPERTY_FIELD_LABEL(RefMakerClass, storageFieldName, labelText)		\
	static NativePropertyFieldDescriptor::PropertyFieldDisplayNameSetter __displayNameSetter##RefMakerClass##storageFieldName(PROPERTY_FIELD_DESCRIPTOR(RefMakerClass, storageFieldName), labelText);

#define DECLARE_PROPERTY_FIELD(storageFieldName)		\
	public: 											\
		static QVariant __read_propfield_##storageFieldName(RefMaker* obj);					\
		static void __write_propfield_##storageFieldName(RefMaker* obj, const QVariant& newValue);	\
		static void __save_propfield_##storageFieldName(RefMaker* obj, SaveStream& stream);	\
		static void __load_propfield_##storageFieldName(RefMaker* obj, LoadStream& stream);	\
		static NativePropertyFieldDescriptor __propFieldInstance##storageFieldName;					\
	private:

#define DEFINE_FLAGS_PROPERTY_FIELD(RefMakerClass, UniqueFieldIdentifier, Flags, storageFieldName)	\
	QVariant RefMakerClass::__read_propfield_##storageFieldName(RefMaker* obj) {			\
		return static_cast<RefMakerClass*>(obj)->storageFieldName;	\
	}																										\
	void RefMakerClass::__write_propfield_##storageFieldName(RefMaker* obj, const QVariant& newValue) {			\
		static_cast<RefMakerClass*>(obj)->storageFieldName = newValue;	\
	}																										\
	void RefMakerClass::__save_propfield_##storageFieldName(RefMaker* obj, SaveStream& stream) {			\
		static_cast<RefMakerClass*>(obj)->storageFieldName.saveToStream(stream);	\
	}																										\
	void RefMakerClass::__load_propfield_##storageFieldName(RefMaker* obj, LoadStream& stream) {			\
		static_cast<RefMakerClass*>(obj)->storageFieldName.loadFromStream(stream);	\
	}																										\
	NativePropertyFieldDescriptor RefMakerClass::__propFieldInstance##storageFieldName(							\
		&RefMakerClass::OOType, 									 								\
		UniqueFieldIdentifier, Flags, __read_propfield_##storageFieldName, 	__write_propfield_##storageFieldName,	\
		__save_propfield_##storageFieldName, __load_propfield_##storageFieldName \
	);

#define DEFINE_PROPERTY_FIELD(RefMakerClass, UniqueFieldIdentifier, storageFieldName)	\
	DEFINE_FLAGS_PROPERTY_FIELD(RefMakerClass, UniqueFieldIdentifier, PROPERTY_FIELD_NO_FLAGS, storageFieldName)

};

// The RefTarget header must be present because
// we are using OORef<RefTarget> here.
#include "RefMaker.h"
#include "RefTarget.h"

#endif // __OVITO_NATIVE_PROPERTY_FIELD_DESCRIPTOR_H
