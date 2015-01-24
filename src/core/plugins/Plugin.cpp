///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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
#include <core/plugins/Plugin.h>
#include <core/plugins/PluginManager.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(PluginSystem)

/******************************************************************************
* Constructor for the Plugin class.
******************************************************************************/
Plugin::Plugin(const QString& manifestFile) : _isLoaded(false)
{
	// Load plugin manifest.
	QFile file(manifestFile);
	if(!file.open(QIODevice::ReadOnly))
		throw Exception(tr("Failed to open plugin manifest file %1").arg(manifestFile));
	QJsonParseError parserError;
	_metadata = QJsonDocument::fromJson(file.readAll(), &parserError);
	if(parserError.error != QJsonParseError::NoError || _metadata.isNull() || !_metadata.isObject())
		throw Exception(tr("Failed to load plugin manifest file %1:\n%2").arg(manifestFile, parserError.errorString()));

	// Extract the metadata fields.
	QJsonObject root = _metadata.object();
	_pluginId = root.value(QStringLiteral("plugin-id")).toString();
	_pluginVendor = root.value(QStringLiteral("plugin-vendor")).toString();
	_pluginVersion = root.value(QStringLiteral("plugin-version")).toString();

	// Parse dependency list.
	for(QJsonValue dep : root.value(QStringLiteral("dependencies")).toArray()) {
		QString depPluginName = dep.toString();
		if(depPluginName.isEmpty())
			throw Exception(tr("Invalid plugin dependency in plugin manifest %1.").arg(manifestFile));
		_dependencies.push_back(depPluginName);
	}
}

/******************************************************************************
* Destructor
******************************************************************************/
Plugin::~Plugin()
{
}

/******************************************************************************
* Loads the plugin into memory.
******************************************************************************/
void Plugin::loadPlugin()
{
    if(isLoaded()) return;	// Plugin is already loaded.

	// Load other plugins this plugin depends on explicitly.
	for(QVector<QString>::const_iterator depName = _dependencies.begin(); depName != _dependencies.end(); ++depName) {
		Plugin* depPlugin = PluginManager::instance().plugin(*depName);
		if(depPlugin == NULL)
			throw Exception(QString("Cannot load plugin %1 because it depends on the plugin %2, which is not installed.").arg(pluginId(), *depName));
		_isLoaded = true;
		try {
			depPlugin->loadPlugin();
			_isLoaded = false;
		}
		catch(...) { _isLoaded = false; throw; }
	}

	// Load other plugins this plugin depends on implicitly.
	for(QSet<Plugin*>::const_iterator dep = _implicitDependencies.begin(); dep != _implicitDependencies.end(); ++dep) {
		_isLoaded = true;
		try {
			(*dep)->loadPlugin();
			_isLoaded = false;
		}
		catch(Exception& ex) {
			_isLoaded = false;
			throw ex.prependGeneralMessage(tr("Failed to load plugin %1, which is required by plugin %2.").arg((*dep)->pluginId(), pluginId()));
		}
		catch(...) { _isLoaded = false; throw; }
	}

	// Do the plugin type specific work.
	loadPluginImpl();

	// Loading was successful.
	this->_isLoaded = true;
}

/******************************************************************************
* Finds the plugin class with the given name defined by the plugin.
******************************************************************************/
OvitoObjectType* Plugin::findClass(const QString& name) const
{
	for(OvitoObjectType* type : classes()) {
		if(type->name() == name || type->nameAlias() == name)
			return type;
	}
	return nullptr;
}

/******************************************************************************
* Returns all plugins this plugin directly depends on.
******************************************************************************/
QSet<Plugin*> Plugin::dependencies() const
{
	QSet<Plugin*> dep = _implicitDependencies;
	for(const QString& depName : _dependencies) {
		Plugin* depPlugin = PluginManager::instance().plugin(depName);
		if(depPlugin != NULL)
			dep.insert(depPlugin);
	}
	return dep;
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

