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

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* This structure describes one member field of a RefMaker object that stores
* a property of that object.
******************************************************************************/
class OVITO_CORE_EXPORT NativePropertyFieldDescriptor : public PropertyFieldDescriptor
{
public:

	/// Constructor	for a property field that stores a non-animatable.
	NativePropertyFieldDescriptor(const NativeOvitoObjectType* definingClass, const char* identifier, PropertyFieldFlags flags,
			QVariant (*_propertyStorageReadFunc)(RefMaker*), void (*_propertyStorageWriteFunc)(RefMaker*, const QVariant&),
			void (*_propertyStorageSaveFunc)(RefMaker*, SaveStream&), void (*_propertyStorageLoadFunc)(RefMaker*, LoadStream&))
		: PropertyFieldDescriptor(definingClass, identifier, flags, _propertyStorageReadFunc, _propertyStorageWriteFunc,
				_propertyStorageSaveFunc, _propertyStorageLoadFunc) {}

	/// Constructor	for a property field that stores a single reference to a RefTarget.
	NativePropertyFieldDescriptor(const NativeOvitoObjectType* definingClass, const OvitoObjectType* targetClass, const char* identifier, PropertyFieldFlags flags, SingleReferenceFieldBase& (*_storageAccessFunc)(RefMaker*))
		: PropertyFieldDescriptor(definingClass, targetClass, identifier, flags, _storageAccessFunc) {}

	/// Constructor	for a property field that stores a vector of references to RefTarget objects.
	NativePropertyFieldDescriptor(const NativeOvitoObjectType* definingClass, const OvitoObjectType* targetClass, const char* identifier, PropertyFieldFlags flags, VectorReferenceFieldBase& (*_storageAccessFunc)(RefMaker*))
		: PropertyFieldDescriptor(definingClass, targetClass, identifier, flags, _storageAccessFunc) {}

public:

	// Internal helper class that is used to specify the units for a controller
	// property field. Do not use this class directly but use the
	// SET_PROPERTY_FIELD_UNITS macro instead.
	struct PropertyFieldUnitsSetter {
		PropertyFieldUnitsSetter(NativePropertyFieldDescriptor& propfield, const QMetaObject* parameterUnitType) {
			OVITO_ASSERT(parameterUnitType != nullptr);
			OVITO_ASSERT(propfield._parameterUnitType == nullptr);
			propfield._parameterUnitType = parameterUnitType;
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

#define PROPERTY_FIELD(RefMakerClassPlusStorageFieldName) \
		RefMakerClassPlusStorageFieldName##__propFieldInstance

#define DECLARE_REFERENCE_FIELD(storageFieldName)																	\
	public: 																										\
		static Ovito::SingleReferenceFieldBase& storageFieldName##__access_reffield(RefMaker* obj);					\
		static Ovito::NativePropertyFieldDescriptor storageFieldName##__propFieldInstance;							\
	private:

#define DEFINE_FLAGS_REFERENCE_FIELD(RefMakerClass, storageFieldName, UniqueFieldIdentifier, TargetClass, Flags)	\
	Ovito::SingleReferenceFieldBase& RefMakerClass::storageFieldName##__access_reffield(RefMaker* obj) {			\
		return static_cast<RefMakerClass*>(obj)->storageFieldName;													\
	}																												\
	Ovito::NativePropertyFieldDescriptor RefMakerClass::storageFieldName##__propFieldInstance(						\
		&RefMakerClass::OOType, &TargetClass::OOType, 																\
		UniqueFieldIdentifier, Flags, RefMakerClass::storageFieldName##__access_reffield		 					\
	);

#define DEFINE_REFERENCE_FIELD(RefMakerClass, storageFieldName, UniqueFieldIdentifier, TargetClass)	\
	DEFINE_FLAGS_REFERENCE_FIELD(RefMakerClass, storageFieldName, UniqueFieldIdentifier, TargetClass, PROPERTY_FIELD_NO_FLAGS)

#define DECLARE_VECTOR_REFERENCE_FIELD(storageFieldName)\
	public: 											\
		static Ovito::VectorReferenceFieldBase& storageFieldName##__access_reffield(RefMaker* obj);					\
		static Ovito::NativePropertyFieldDescriptor storageFieldName##__propFieldInstance;							\
	private:

#define DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(RefMakerClass, storageFieldName, UniqueFieldIdentifier, TargetClass, Flags)			\
	Ovito::VectorReferenceFieldBase& RefMakerClass::storageFieldName##__access_reffield(RefMaker* obj) {			\
		return static_cast<RefMakerClass*>(obj)->storageFieldName;													\
	}																												\
	Ovito::NativePropertyFieldDescriptor RefMakerClass::storageFieldName##__propFieldInstance(			\
		&RefMakerClass::OOType, &TargetClass::OOType, 																\
		UniqueFieldIdentifier, Flags | PROPERTY_FIELD_VECTOR, RefMakerClass::storageFieldName##__access_reffield 	\
	);

#define DEFINE_VECTOR_REFERENCE_FIELD(RefMakerClass, storageFieldName, UniqueFieldIdentifier, TargetClass)	\
	DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(RefMakerClass, storageFieldName, UniqueFieldIdentifier, TargetClass, PROPERTY_FIELD_VECTOR)

#define INIT_PROPERTY_FIELD(RefMakerClassPlusStorageFieldName)														\
	RefMakerClassPlusStorageFieldName.init(this, &PROPERTY_FIELD(RefMakerClassPlusStorageFieldName));

#define SET_PROPERTY_FIELD_UNITS(RefMakerClass, storageFieldName, ParameterUnitClass)								\
	static Ovito::NativePropertyFieldDescriptor::PropertyFieldUnitsSetter __unitsSetter##RefMakerClass##storageFieldName(RefMakerClass::storageFieldName##__propFieldInstance, &ParameterUnitClass::staticMetaObject);

#define SET_PROPERTY_FIELD_LABEL(RefMakerClass, storageFieldName, labelText)										\
	static Ovito::NativePropertyFieldDescriptor::PropertyFieldDisplayNameSetter __displayNameSetter##RefMakerClass##storageFieldName(RefMakerClass::storageFieldName##__propFieldInstance, labelText);

#define DECLARE_PROPERTY_FIELD(storageFieldName)																	\
	public: 																										\
		static QVariant __read_propfield_##storageFieldName(Ovito::RefMaker* obj);									\
		static void __write_propfield_##storageFieldName(Ovito::RefMaker* obj, const QVariant& newValue);			\
		static void __save_propfield_##storageFieldName(Ovito::RefMaker* obj, Ovito::SaveStream& stream);			\
		static void __load_propfield_##storageFieldName(Ovito::RefMaker* obj, Ovito::LoadStream& stream);			\
		static Ovito::NativePropertyFieldDescriptor storageFieldName##__propFieldInstance;	\
	private:

#define DEFINE_FLAGS_PROPERTY_FIELD(RefMakerClass, storageFieldName, UniqueFieldIdentifier, Flags)			\
	QVariant RefMakerClass::__read_propfield_##storageFieldName(RefMaker* obj) {							\
		return QVariant::fromValue(static_cast<RefMakerClass*>(obj)->storageFieldName.value());				\
	}																										\
	void RefMakerClass::__write_propfield_##storageFieldName(RefMaker* obj, const QVariant& newValue) {		\
		static_cast<RefMakerClass*>(obj)->storageFieldName = newValue;										\
	}																										\
	void RefMakerClass::__save_propfield_##storageFieldName(RefMaker* obj, SaveStream& stream) {			\
		static_cast<RefMakerClass*>(obj)->storageFieldName.saveToStream(stream);							\
	}																										\
	void RefMakerClass::__load_propfield_##storageFieldName(RefMaker* obj, LoadStream& stream) {			\
		static_cast<RefMakerClass*>(obj)->storageFieldName.loadFromStream(stream);							\
	}																										\
	Ovito::NativePropertyFieldDescriptor RefMakerClass::storageFieldName##__propFieldInstance(				\
		&RefMakerClass::OOType, 									 										\
		UniqueFieldIdentifier, Flags, __read_propfield_##storageFieldName, 	__write_propfield_##storageFieldName,	\
		__save_propfield_##storageFieldName, __load_propfield_##storageFieldName 							\
	);

#define DEFINE_PROPERTY_FIELD(RefMakerClass, storageFieldName, UniqueFieldIdentifier)						\
	DEFINE_FLAGS_PROPERTY_FIELD(RefMakerClass, storageFieldName, UniqueFieldIdentifier, PROPERTY_FIELD_NO_FLAGS)

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

// The RefTarget header must be present because
// we are using OORef<RefTarget> here.
#include "RefMaker.h"
#include "RefTarget.h"

#endif // __OVITO_NATIVE_PROPERTY_FIELD_DESCRIPTOR_H
