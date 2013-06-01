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
#include <core/plugins/NativePlugin.h>
#include <core/object/NativeOvitoObjectType.h>

namespace Ovito {

/******************************************************************************
* Constructor for the NativePlugin class.
******************************************************************************/
NativePlugin::NativePlugin(const QString& manifestFile) :
	Plugin(manifestFile)
{
}

/******************************************************************************
* Parses a custom top-level element from the manifest that is specific to the plugin type.
******************************************************************************/
bool NativePlugin::parseToplevelManifestElement(const QDomElement& element)
{
	return true;
}

/******************************************************************************
* Loads a native plugin's library.
******************************************************************************/
void NativePlugin::loadPluginImpl()
{
	// Connect all newly loaded class descriptors with this plugin.
	for(NativeOvitoObjectType* clazz = NativeOvitoObjectType::_firstInfo; clazz != NULL; clazz = clazz->_next) {
		OVITO_ASSERT(clazz->plugin() == NULL);
		clazz->_plugin = this;
		registerClass(clazz);
	}
}

};
