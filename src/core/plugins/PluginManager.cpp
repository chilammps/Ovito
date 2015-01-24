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

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(PluginSystem)

/// The singleton instance of this class.
PluginManager* PluginManager::_instance = nullptr;

/******************************************************************************
* Initializes the plugin manager.
******************************************************************************/
PluginManager::PluginManager() : _corePlugin(nullptr)
{
	OVITO_ASSERT_MSG(!_instance, "PluginManager constructor", "Multiple instances of this singleton class have been created.");
}

/******************************************************************************
* Unloads all plugins.
******************************************************************************/
PluginManager::~PluginManager()
{
	// Unload plugins in reverse order.
	for(int i = plugins().size() - 1; i >= 0; --i) {
		delete plugins()[i];
	}
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

	// In Ovito 2.1, the "Viz" plugin has been renamed to "Particles".
	// To support loading of old scene files in newer versions of Ovito,
	// use "Viz" as an alias for the Particles plugin.
	if(pluginId == QStringLiteral("Viz"))
		return plugin(QStringLiteral("Particles"));

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
		throw Exception(tr("Non-unique plugin identifier detected: %1").arg(id));
	}

	_plugins.push_back(plugin);
}

/******************************************************************************
* Returns the list of directories containing the Ovito plugins.
******************************************************************************/
QList<QDir> PluginManager::pluginDirs()
{
	QDir prefixDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
	return { QDir(prefixDir.absolutePath() + QStringLiteral("/plugins")) };
#else
	prefixDir.cdUp();
	return { QDir(prefixDir.absolutePath() + QStringLiteral("/lib/ovito/plugins")) };
#endif
}

/******************************************************************************
* Searches the plugin directories for installed plugins and
* loads their XML manifests.
******************************************************************************/
void PluginManager::registerPlugins()
{
	// Register the built-in classes of the core.
	_corePlugin = new NativePlugin(QStringLiteral(":/core/Core.json"));
	registerPlugin(_corePlugin);

	// Scan the plugin directories for installed plugins.
	for(QDir pluginDir : pluginDirs()) {
		if(!pluginDir.exists())
			throw Exception(tr("Failed to scan the plugin directory. Path %1 does not exist.").arg(pluginDir.path()));

		// List all manifest files.
		pluginDir.setNameFilters(QStringList("*.json"));
		pluginDir.setFilter(QDir::Files);

		// Load each manifest file in the plugin directory.
		for(const QString& file : pluginDir.entryList()) {
			QString filePath = pluginDir.absoluteFilePath(file);
			try {
				Plugin* plugin = new NativePlugin(filePath);
				registerPlugin(plugin);
			}
			catch(Exception& ex) {
				ex.prependGeneralMessage(tr("Failed to load plugin manifest:\n\n%1").arg(filePath));
				ex.showError();
			}
		}
	}

	// Load the core plugin.
	corePlugin()->loadPlugin();

	// Load all other plugins too.
	for(Plugin* plugin : plugins()) {
		plugin->loadPlugin();
	}
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

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
