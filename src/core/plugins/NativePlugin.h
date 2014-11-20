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

#ifndef __OVITO_NATIVE_PLUGIN_H
#define __OVITO_NATIVE_PLUGIN_H

#include <core/Core.h>
#include "Plugin.h"

namespace Ovito { namespace PluginSystem { namespace Internal {

/**
 * \brief A plugin that is implemented as a native shared library.
 */
class OVITO_CORE_EXPORT NativePlugin : public Plugin
{
	Q_OBJECT

public:

	/// \brief Returns the file path of the plugin's dynamic library.
	/// \return The full path to the link library that contains the plugin's code.
	const QString& libraryFilename() const { return _libraryFilename; }

	/// \brief Returns the plugin library after it has been loaded.
	/// \return The runtime library or \c NULL if the plugin has not been loaded.
	QLibrary* library() const { return _library; }

protected:

	/// \brief Constructor that loads the given manifest file.
	/// \param manifestFile Full path to the plugin's manifest file.
	/// \throw Exception on parsing error.
	NativePlugin(const QString& manifestFile);

	/// \brief Parses a custom top-level element from the manifest that is specific to the plugin type.
	/// \return \c true if the element was processed by the method; \c false if the element was not known
	///         to the implementation of the method.
	/// \throw Exception on error.
	virtual bool parseToplevelManifestElement(const QDomElement& element) override;

	/// \brief Loads the plugin's dynamic link library.
	/// \throw Exception on error.
	virtual void loadPluginImpl() override;

private:

	/// The file path of the dynamic library.
	QString _libraryFilename;

	/// The plugin library after it has been loaded.
	QLibrary* _library;

	friend class Ovito::PluginSystem::PluginManager;
};

}}}	// End of namespace

#endif // __OVITO_NATIVE_PLUGIN_H
