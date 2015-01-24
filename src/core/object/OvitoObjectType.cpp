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
#include "OvitoObjectType.h"
#include "NativeOvitoObjectType.h"
#include "OvitoObject.h"

#include <core/plugins/Plugin.h>
#include <core/plugins/PluginManager.h>
#include <core/utilities/io/ObjectSaveStream.h>
#include <core/utilities/io/ObjectLoadStream.h>
#include <core/reference/PropertyFieldDescriptor.h>
#include <core/reference/RefTarget.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

/******************************************************************************
* Constructor of the object.
******************************************************************************/
OvitoObjectType::OvitoObjectType(const QString& name, const OvitoObjectType* superClass, bool isSerializable) :
	_name(name), _displayName(name), _plugin(nullptr), _superClass(superClass),
	_isSerializable(isSerializable), _firstPropertyField(NULL), _editorClass(nullptr), _isAbstract(false)
{
	OVITO_ASSERT(superClass != NULL || name == QStringLiteral("OvitoObject"));
}

/******************************************************************************
* This is called after the class has been loaded to initialize its properties.
******************************************************************************/
void OvitoObjectType::initializeClassDescriptor(Plugin* plugin)
{
	_plugin = plugin;
}

/******************************************************************************
* Creates an instance of this object class.
* Throws an exception if the containing plugin failed to load.
******************************************************************************/
OORef<OvitoObject> OvitoObjectType::createInstance(DataSet* dataset) const
{
	if(plugin()) {
		OVITO_CHECK_POINTER(plugin());
		if(!plugin()->isLoaded()) {
			// Load plugin first.
			try {
				plugin()->loadPlugin();
			}
			catch(Exception& ex) {
				throw ex.prependGeneralMessage(Plugin::tr("Could not create instance of class %1. Failed to load plugin '%2'").arg(name()).arg(plugin()->pluginId()));
			}
		}
	}
	if(isAbstract())
		throw Exception(Plugin::tr("Cannot instantiate abstract class '%1'.").arg(name()));

	OVITO_ASSERT_MSG(!isDerivedFrom(RefTarget::OOType) || dataset != nullptr || *this == DataSet::OOType, "OvitoObjectType::createInstance()", "Tried to create instance of RefTarget derived class without passing a DatSet.");
	OVITO_ASSERT_MSG(isDerivedFrom(RefTarget::OOType) || dataset == nullptr, "OvitoObjectType::createInstance()", "Passed a DatSet to the constructor of a class that is not derived from RefTarget.");

	return createInstanceImpl(dataset);
}

/******************************************************************************
* Writes a class descriptor to the stream. This is for internal use of the core only.
******************************************************************************/
void OvitoObjectType::serializeRTTI(ObjectSaveStream& stream, const OvitoObjectType* type)
{
	OVITO_CHECK_POINTER(type);

	stream.beginChunk(0x10000000);
	stream << type->plugin()->pluginId();
	stream << type->name();
	stream.endChunk();
}

/******************************************************************************
* Loads a class descriptor from the stream. This is for internal use of the core only.
* Throws an exception if the class is not defined or the required plugin is not installed.
******************************************************************************/
OvitoObjectType* OvitoObjectType::deserializeRTTI(ObjectLoadStream& stream)
{
	QString pluginId, className;
	stream.expectChunk(0x10000000);
	stream >> pluginId;
	stream >> className;
	stream.closeChunk();

	try {

		// Lookup class descriptor.
		Plugin* plugin = PluginManager::instance().plugin(pluginId);
		if(!plugin)
			throw Exception(Plugin::tr("A required plugin is not installed: %1").arg(pluginId));
		OVITO_CHECK_POINTER(plugin);

		OvitoObjectType* type = plugin->findClass(className);
		if(!type) {

			// Handle legacy classes that no longer exist.
			if(className == QStringLiteral("VectorController")
					|| className == QStringLiteral("FloatController")
					|| className == QStringLiteral("IntegerController")
					|| className == QStringLiteral("RotationController")
					|| className == QStringLiteral("ScalingController")
					|| className == QStringLiteral("PositionController")
					|| className == QStringLiteral("TransformationController"))
				type = plugin->findClass(QStringLiteral("Controller"));

			if(!type)
				throw Exception(Plugin::tr("Required class %1 not found in plugin %2.").arg(className, pluginId));
		}

		return type;
	}
	catch(Exception& ex) {
		ex.prependGeneralMessage(Plugin::tr("File cannot be loaded, because it contains object types that are not (or no longer) available in this program version."));
		throw ex;
	}
}

/******************************************************************************
* Encodes the plugin ID and the class name in a string.
******************************************************************************/
QString OvitoObjectType::encodeAsString(const OvitoObjectType* type)
{
	OVITO_CHECK_POINTER(type);
	return type->plugin()->pluginId() + QStringLiteral("::") + type->name();
}

/******************************************************************************
* Decodes a class descriptor from a string, which has been generated by encodeAsString().
******************************************************************************/
OvitoObjectType* OvitoObjectType::decodeFromString(const QString& str)
{
	QStringList tokens = str.split(QStringLiteral("::"));
	if(tokens.size() != 2)
		throw Exception(Plugin::tr("Invalid type or encoding: %1").arg(str));

	// Lookup class descriptor.
	Plugin* plugin = PluginManager::instance().plugin(tokens[0]);
	if(!plugin)
		throw Exception(Plugin::tr("A required plugin is not installed: %1").arg(tokens[0]));
	OVITO_CHECK_POINTER(plugin);

	OvitoObjectType* type = plugin->findClass(tokens[1]);
	if(!type)
		throw Exception(Plugin::tr("Required class %1 not found in plugin %2.").arg(tokens[1], tokens[0]));

	return type;
}

/******************************************************************************
* Searches for a property field defined in this class or one of its super classes.
******************************************************************************/
const PropertyFieldDescriptor* OvitoObjectType::findPropertyField(const char* identifier, bool searchSuperClasses) const
{
	for(const OvitoObjectType* clazz = this; clazz != nullptr; clazz = searchSuperClasses ? clazz->superClass() : nullptr) {
		for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field; field = field->next())
			if(qstrcmp(field->identifier(), identifier) == 0) return field;
	}

	return nullptr;
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
