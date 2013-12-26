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
#include <core/plugins/Plugin.h>
#include <core/plugins/PluginManager.h>

namespace Ovito {

/******************************************************************************
* Constructor for the Plugin class.
******************************************************************************/
Plugin::Plugin(const QString& manifestFile) :
	_manifestFilename(manifestFile), _isManifestParsed(false),
	_isLoaded(false)
{
	// Load plugin manifest file into DOM.
	QFile file(manifestFile);
	if(!file.open(QIODevice::ReadOnly))
		throw Exception(tr("Failed to open plugin manifest file %1").arg(manifestFile));
	QString errorMsg;
	int errorLine, errorColumn;
	if(!_manifest.setContent(&file, true, &errorMsg, &errorLine, &errorColumn))
		throw Exception(tr("Failed to load plugin manifest file.\nXML File: %1\nError Message: %2\nLine %3, Column %4").arg(manifestFile, errorMsg).arg(errorLine).arg(errorColumn));

	// Extract the plugin identifier from the manifest.
	_pluginId = _manifest.documentElement().attribute("Plugin-Id");
	_pluginVendor = _manifest.documentElement().attribute("Plugin-Vendor");
	_pluginVersion = _manifest.documentElement().attribute("Plugin-Version");
}

/******************************************************************************
* Destructor
******************************************************************************/
Plugin::~Plugin()
{
	// Unload resource files.
	for(const QString& path : _resourceFiles)
		QResource::unregisterResource(path);
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
* Parses the plugin's XML manifest.
******************************************************************************/
void Plugin::parseManifest()
{
	OVITO_ASSERT(!_manifest.isNull());
	OVITO_ASSERT(pluginId().isEmpty() == false);

	if(_isManifestParsed) return;	// Is already parsed?
	_isManifestParsed = true;		// Prevent re-entrance.

	for(QDomElement rootLevelNode = _manifest.documentElement().firstChildElement(); !rootLevelNode.isNull(); rootLevelNode = rootLevelNode.nextSiblingElement()) {
		if(rootLevelNode.localName() == "Plugin-Dependencies") {
			parsePluginDependencies(rootLevelNode);
		}
		else if(rootLevelNode.localName() == "Resource-File") {
			parseResourceFileReference(rootLevelNode);
		}
		else parseToplevelManifestElement(rootLevelNode);
	}
}

/******************************************************************************
* Parses the <Plugin-Dependencies> element.
******************************************************************************/
void Plugin::parsePluginDependencies(const QDomElement& parentNode)
{
	for(QDomElement depNode = parentNode.firstChildElement(); !depNode.isNull(); depNode = depNode.nextSiblingElement()) {
		if(depNode.localName() == "Plugin-Dependency") {
			// Parse plugin name.
			QString depPluginName = depNode.attribute("Plugin-Id");
			if(depPluginName.isEmpty())
				throw Exception(tr("Invalid plugin dependency attribute in manifest."));

			// Skip disabled elements.
			if(depNode.attribute("Enabled").compare("false", Qt::CaseInsensitive) == 0 ||
				depNode.attribute("Enabled").compare("off", Qt::CaseInsensitive) == 0 ||
				depNode.attribute("Enabled").compare("no", Qt::CaseInsensitive) == 0)
				continue;

			_dependencies.push_back(depPluginName);
		}
		else throw Exception(QString("Unknown element tag in XML file: <%1>").arg(depNode.localName()));
	}
}

/******************************************************************************
* Parses a resource file reference in the manifest file.
******************************************************************************/
void Plugin::parseResourceFileReference(const QDomElement& element)
{
	QString path = element.attribute("Path");
	if(path.isEmpty())
		throw Exception(QString("Element <Resource-File> has no Path attribute in manifest file %1.").arg(manifestFile()));

	// Resolve path.
	QDir baseDir = QFileInfo(manifestFile()).dir();
	QString fullPath = baseDir.absoluteFilePath(path);

	// Load resource file into memory.
	if(!QResource::registerResource(fullPath))
		throw Exception(QString("Could not load plugin resource file %1").arg(fullPath));

	_resourceFiles.push_back(fullPath);
}

/******************************************************************************
* Finds the plugin class with the given name defined by the plugin.
******************************************************************************/
OvitoObjectType* Plugin::findClass(const QString& name) const
{
	for(OvitoObjectType* descriptor : classes()) {
		if(descriptor->name() == name)
			return descriptor;
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

};

