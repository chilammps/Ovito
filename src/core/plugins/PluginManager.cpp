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
#include <core/plugins/PluginManager.h>
#include <core/plugins/Plugin.h>
#include <core/plugins/NativePlugin.h>

namespace Ovito {

/// The singleton instance of this class.
PluginManager* PluginManager::_instance = nullptr;

/******************************************************************************
* Initializes the plugin manager.
******************************************************************************/
PluginManager::PluginManager() : _corePlugin(NULL)
{
	OVITO_ASSERT_MSG(!_instance, "PluginManager constructor", "Multiple instances of this singleton class have been created.");
	registerPlugins();
}

/******************************************************************************
* Unloads all plugins.
******************************************************************************/
PluginManager::~PluginManager()
{
	// Unload plugins in reverse order.
	for(int i = plugins().size() - 1; i >= 0; --i)
		delete plugins()[i];
}

/******************************************************************************
* Returns the plugin with the given identifier.
* Returns NULL when no such plugin is installed.
******************************************************************************/
Plugin* PluginManager::plugin(const QString& pluginId)
{
	for(Plugin* plugin : plugins()) {
		if(plugin->pluginId() == pluginId)
			return plugin;
	}
	return nullptr;
}

/******************************************************************************
* Registers a new plugin with the manager.
******************************************************************************/
void PluginManager::registerPlugin(Plugin* plugin)
{
	OVITO_CHECK_POINTER(plugin);

	// Make sure the plugin's ID is unique.
	if(this->plugin(plugin->pluginId())) {
		QString id = plugin->pluginId();
		delete plugin;
		throw Exception(QString("Non-unique plugin identifier detected: %1.").arg(id));
	}

	_plugins.push_back(plugin);
}


/******************************************************************************
* Searches the plugin directory for installed plugins and
* loads their XML manifests.
******************************************************************************/
void PluginManager::registerPlugins()
{
	// Register the built-in classes of the core.
	_corePlugin = loadPluginManifest(":/core/Core.manifest.xml");

	// Parse the manifest of each plugin.
	for(Plugin* plugin : plugins()) {
		try {
			plugin->parseManifest();
		}
		catch(Exception& ex) {
			ex.prependGeneralMessage(tr("Failed to load plugin manifest:\n\n%1").arg(plugin->manifestFile()));
			_plugins.remove(_plugins.indexOf(plugin));
			if(plugin->isCore()) {
				delete plugin;
				throw ex;	// This is a fatal error.
			}
			else {
				delete plugin;
				ex.showError();
			}
		}
	}

	// Load the core plugin by default.
	corePlugin()->loadPlugin();
}

/******************************************************************************
* Loads the given plugin manifest file.
******************************************************************************/
Plugin* PluginManager::loadPluginManifest(const QString& file)
{
	// Check if the same manifest has already been loaded.
	for(Plugin* p : plugins())
		if(p->manifestFile() == file) return p;

	// Create Plugin object and load XML file into DOM.
	Plugin* plugin = new NativePlugin(file);

	// Add it to the list of plugins.
	registerPlugin(plugin);

	return plugin;
}

/******************************************************************************
* Returns all installed plugin classes derived from the given type.
******************************************************************************/
QVector<OvitoObjectType*> PluginManager::listClasses(const OvitoObjectType& superClass, bool skipAbstract)
{
	QVector<OvitoObjectType*> result;

	for(Plugin* plugin : plugins()) {
		for(OvitoObjectType* clazz : plugin->classes()) {
			if(!skipAbstract || !clazz->isAbstract()) {
				if(clazz->isDerivedFrom(superClass))
					result.push_back(clazz);
			}
		}
	}

	return result;
}

};
