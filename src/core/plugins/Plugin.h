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

#ifndef __OVITO_PLUGIN_H
#define __OVITO_PLUGIN_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(PluginSystem)

/**
 * \brief Represents a plugin that is loaded at runtime.
 */
class OVITO_CORE_EXPORT Plugin : public QObject
{
	Q_OBJECT

public:

	/// \brief Destructor
	virtual ~Plugin();

	/// \brief Returns the plugin's metadata.
	const QJsonDocument& metadata() const { return _metadata; }

	/// \brief Returns the unique identifier of the plugin.
	const QString& pluginId() const { return _pluginId; }

	/// \brief Returns the plugin's vendor string.
	const QString& pluginVendor() const { return _pluginVendor; }

	/// \brief Returns the plugin's version string.
	const QString& pluginVersion() const { return _pluginVersion; }

	/// \brief Finds the plugin class with the given name defined by the plugin.
	/// \param name The class name.
	/// \return The descriptor for the plugin class with the given name or \c NULL
	///         if no such class is defined by the plugin.
	/// \sa classes()
	OvitoObjectType* findClass(const QString& name) const;

	/// \brief Returns whether the plugin's dynamic library has been loaded.
	/// \sa loadPlugin()
	bool isLoaded() const { return _isLoaded; }

	/// \brief Loads the plugin's dynamic link library into memory.
	/// \throw Exception if an error occurs.
	///
	/// This method may load other plugins first if this plugin
	/// depends on them.
	/// \sa isLoaded()
	void loadPlugin();

	/// \brief Returns all classes defined by the plugin.
	/// \sa findClass()
	const QVector<OvitoObjectType*>& classes() const { return _classes; }

	/// \brief Indicates whether this is the built-in pseudo plugin, which represents OVITO's core library.
	bool isCore() const { return pluginId() == QStringLiteral("Core"); }

	/// \brief Returns all other plugins this plugin (directly) depends on.
	QSet<Plugin*> dependencies() const;

protected:

	/// \brief Constructor that loads the given manifest file.
	/// \param manifestFile Path to the plugin's JSON manifest file.
	/// \throw Exception If a parsing error occurred.
	Plugin(const QString& manifestFile);

	/// \brief Implementation method that loads the plugin.
	/// \throw Exception if an error occurred.
	virtual void loadPluginImpl() = 0;

	/// \brief Adds a class to the list of plugin classes.
	void registerClass(OvitoObjectType* clazz) { _classes.push_back(clazz); }

private:

	/// The unique identifier of the plugin.
	QString _pluginId;

	/// The vendor of the plugin.
	QString _pluginVendor;

	/// The version string of the plugin.
	QString _pluginVersion;

	/// The classes provided by the plugin.
	QVector<OvitoObjectType*> _classes;

	/// The plugins this plugin explicitly depends on.
	QVector<QString> _dependencies;

	/// The plugins this plugin implicitly depends on.
	QSet<Plugin*> _implicitDependencies;

	/// The plugin's metadata.
	QJsonDocument _metadata;

	/// Indicates whether the plugin dynamic library has been loaded.
	bool _isLoaded;

	friend class PluginManager;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_PLUGIN_H
