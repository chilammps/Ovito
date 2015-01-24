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

#ifndef __OVITO_NATIVE_OVITO_OBJECT_TYPE_H
#define __OVITO_NATIVE_OVITO_OBJECT_TYPE_H

#include <core/Core.h>
#include "OvitoObjectType.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief Every C++ class derived from OvitoObject is described by an instance of this class.
 *
 * This is the runtime part of the plugin class system. Every C++ plugin class derived from
 * OvitoObject comes with an instance of NativeOvitoObjectType as a static member. It is used to identify
 * the class at runtime.
 *
 * It can be accessed through the the static \c pluginClassDescriptor() method of each OvitoObject-derived class.
 */
class OVITO_CORE_EXPORT NativeOvitoObjectType : public OvitoObjectType
{
public:

	/// \brief Constructs the plugin class descriptor object.
	/// \note This is an internal constructor that is not for public use.
	NativeOvitoObjectType(const QString& name, const char* pluginId, const NativeOvitoObjectType* superClass, const QMetaObject* qtClassInfo, bool isSerializable);

	/// \brief Returns the name of this class.
	/// \return A pointer to the class name string (without namespace qualifier).
	const char* className() const { return _pureClassName; }

	/// \brief Returns the identifier of the plugin this class belongs to.
	const char* pluginId() const { return _pluginId; }

	/// Returns the Qt runtime-type information associated with this object type.
	virtual const QMetaObject* qtMetaObject() const override { return _qtClassInfo; }

protected:

	/// \brief Creates an instance of the class described by this descriptor.
	/// \param dataset The dataset the newly created object will belong to.
	/// \return The new instance of the class. The pointer can safely be cast to the corresponding C++ class type.
	/// \throw Exception if the instance could not be created.
	virtual OvitoObject* createInstanceImpl(DataSet* dataset) const override;

	/// This is called by the NativePlugin that contains this class to initialize
	/// the properties of this class type.
	virtual void initializeClassDescriptor(Plugin* plugin) override;

private:

	/// The runtime-type information provided by Qt.
	const QMetaObject* _qtClassInfo;

	/// The name of the class.
	const char* _pureClassName;

	/// The identifier of the plugin this class belongs to.
	const char* _pluginId;

	/// All native object types are stored in a linked list.
	NativeOvitoObjectType* _next;

	/// The head of the linked list of all native object types.
	static NativeOvitoObjectType* _firstInfo;

	friend class NativePlugin;
	friend class PropertyFieldDescriptor;
};

///////////////////////////////////// Macros //////////////////////////////////////////

/// This macro must be included in the class definition of a OvitoObject-derived class.
#define OVITO_OBJECT													\
	public:																\
		static const Ovito::NativeOvitoObjectType OOType;				\
		virtual const Ovito::OvitoObjectType& getOOType() const { return OOType; }

/// This macro must be included in the .cpp file for a OvitoObject-derived class.
#define IMPLEMENT_OVITO_OBJECT(plugin, name, basename)							\
	const Ovito::NativeOvitoObjectType name::OOType(QStringLiteral(#name), #plugin, &basename::OOType, &name::staticMetaObject, false);

/// This macro must be included in the .cpp file for a OvitoObject-derived class.
#define IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(plugin, name, basename)				\
	const Ovito::NativeOvitoObjectType name::OOType(QStringLiteral(#name), #plugin, &basename::OOType, &name::staticMetaObject, true);

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_NATIVE_OVITO_OBJECT_TYPE_H
