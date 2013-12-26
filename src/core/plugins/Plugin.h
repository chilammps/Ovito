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
 * \file Plugin.h
 * \brief Contains the definition of the Ovito::Plugin class.
 */

#ifndef __OVITO_PLUGIN_H
#define __OVITO_PLUGIN_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>

namespace Ovito {

/**
 * \brief Represents a plugin that is loaded at runtime.
 */
class OVITO_CORE_EXPORT Plugin : public QObject
{
	Q_OBJECT

public:

	/// \brief Destructor
	virtual ~Plugin();

	/// \brief Gets the path of the plugin's manifest file.
	/// \return The full path to the plugin's manifest file.
	/// \sa libraryFilename()
	const QString& manifestFile() const { return _manifestFilename; }

	/// \brief Returns the plugin's XML manifest document that contains all descriptive
	///        information about it.
	/// \return The manifest in XML representation.
	const QDomDocument& manifest() const { return _manifest; }

	/// \brief Gets the unique identifier of the plugin.
	/// \return A string identifier that identifies this plugin.
	const QString& pluginId() const { return _pluginId; }

	/// \brief Gets the vendor of the plugin.
	/// \return A string that identifies this plugin's vendor. This string is read
	///         from the plugin's manifest file.
	const QString& pluginVendor() const { return _pluginVendor; }

	/// \brief Gets the version of the plugin.
	/// \return A string that identifies this plugin's version. This string is read
	///         from the plugin's manifest file.
	const QString& pluginVersion() const { return _pluginVersion; }

	/// \brief Finds the plugin class with the given name defined by the plugin.
	/// \param name The class name.
	/// \return The descriptor for the plugin class with the given name or \c NULL
	///         if no such class is defined by the plugin.
	/// \sa classes()
	OvitoObjectType* findClass(const QString& name) const;

	/// \brief Returns whether the plugin dynamic library has been loaded.
	/// \return \c true if the dynamic link library of this plugin has been loaded
	///         into the address space of the application. The plugin's classes
	///         can only be used after the plugin has been loaded.
	/// \sa loadPlugin()
	bool isLoaded() const { return _isLoaded; }

	/// \brief Loads the plugin's dynamic link library.
	/// \throw Exception on error.
	/// \note Normally it is not required to call loadPlugin() because
	///       the plugin is automatically loaded as soon as one of its
	///       classes is instantiated.
	/// \sa isLoaded()
	void loadPlugin();

	/// \brief Returns all classes provided by the plugin.
	/// \return A list of descriptors for all classes defined by the plugin.
	/// \sa findClass()
	const QVector<OvitoObjectType*>& classes() const { return _classes; }

	/// \brief Returns whether this is the built-in core plugin.
	/// \return \c true if this Plugin instance is the application core; \c false
	///         if this is an ordinary plugin.
	bool isCore() const { return pluginId() == "Core"; }

	/// \brief Returns all plugins this plugin directly depends on.
	/// \return The list of plugin this plugiun directly depends on.
	QSet<Plugin*> dependencies() const;

protected:

	/// \brief Constructor that loads the given manifest file.
	/// \param manifestFile Full path to the plugin's manifest file.
	/// \throw Exception on parsing error.
	Plugin(const QString& manifestFile);

	/// \brief Parses a custom top-level element from the manifest that is specific to the plugin type.
	/// \return \c true if the element was processed by the method; \c false if the element was not known
	///         to the implementation of the method.
	/// \throw Exception on error.
	virtual bool parseToplevelManifestElement(const QDomElement& element) { return false; }

	/// \brief Performs the type specific work of loading the plugin.
	/// \throw Exception on loading error.
	virtual void loadPluginImpl() = 0;

	/// \brief Adds a class to the list of plugin classes.
	void registerClass(OvitoObjectType* clazz) { _classes.push_back(clazz); }

private:

	/// \brief Parse the XML document containing the manifest.
	///        Loads all class definitions from the manifest file.
	/// \throw Exception on parsing error.
	void parseManifest();

	/// \brief Parses the plugin-dependencies element in the manifest file.
	/// \param parentNode The XML parent element that contains the dependency elements.
	void parsePluginDependencies(const QDomElement& parentNode);

	/// \brief Parses a resource file reference in the manifest file.
	/// \param element The XML element to parse.
	void parseResourceFileReference(const QDomElement& element);

private:

	/// The file path of the plugin's manifest.
	QString _manifestFilename;

	/// The unique identifier of the plugin.
	QString _pluginId;

	/// The vendor of the plugin.
	QString _pluginVendor;

	/// The version string of the plugin.
	QString _pluginVersion;

	/// The classes provided by the plugin.
	QVector<OvitoObjectType*> _classes;

	/// Contains all class declarations in the manifest document.
	QMap<QString, QDomElement> _classDeclarations;

	/// The plugins this plugin explicitly depends on.
	QVector<QString> _dependencies;

	/// The plugins this plugin implicitly depends on.
	QSet<Plugin*> _implicitDependencies;

	/// The XML manifest document.
	QDomDocument _manifest;

	/// Indicates whether the plugin dynamic library has been loaded.
	bool _isLoaded;

	/// Indicates that the manifest has been completely parsed.
	/// This is need to avoid re-entrance into parseManifest().
	bool _isManifestParsed;

	/// List of external resource files loaded by the plugin.
	QStringList _resourceFiles;

	friend class PluginManager;
};

};

#endif // __OVITO_PLUGIN_H
