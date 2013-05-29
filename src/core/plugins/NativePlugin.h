///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2008) Alexander Stukowski
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
 * \file NativePlugin.h
 * \brief Contains the definition of the Ovito::NativePlugin class.
 */

#ifndef __OVITO_NATIVE_PLUGIN_H
#define __OVITO_NATIVE_PLUGIN_H

#include <core/Core.h>
#include "Plugin.h"

namespace Ovito {

/**
 * \brief A plugin that is implemented as a native shared library.
 *
 * \author Alexander Stukowski
 */
class NativePlugin : public Plugin
{
	Q_OBJECT

protected:

	/// \brief Constructor that loads the given manifest file.
	/// \param manifestFile Full path to the plugin's manifest file.
	/// \throw Exception on parsing error.
	NativePlugin(const QString& manifestFile);

	/// \brief Parses a custom top-level element from the manifest that is specific to the plugin type.
	/// \return \c true if the element was processed by the method; \c false if the element was not known
	///         to the implementation of the method.
	/// \throw Exception on error.
	virtual bool parseToplevelManifestElement(const QDomElement& element);

	/// \brief Loads the plugin's dynamic link library.
	/// \throw Exception on error.
	virtual void loadPluginImpl();

private:

	/// A pointer into the linked list of plugin classes.
	NativeOvitoObjectType* _infoBefore;

	/// A pointer into the linked list of plugin classes.
	NativeOvitoObjectType* _infoAfter;

	friend class PluginManager;
};

};

#endif // __OVITO_NATIVE_PLUGIN_H
