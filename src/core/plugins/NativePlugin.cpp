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
#include <core/plugins/NativePlugin.h>
#include <core/object/NativeOvitoObjectType.h>

namespace Ovito { namespace PluginSystem { namespace Internal {

/******************************************************************************
* Constructor for the NativePlugin class.
******************************************************************************/
NativePlugin::NativePlugin(const QString& manifestFile) :
	Plugin(manifestFile), _library(nullptr)
{
}

/******************************************************************************
* Parses a custom top-level element from the manifest that is specific to the plugin type.
******************************************************************************/
bool NativePlugin::parseToplevelManifestElement(const QDomElement& element)
{
	// Process the <NativePlugin> element that describes the native plugin's properties.
	if(element.localName() == "native-library") {

		// Get the name of the shared library file.
		QString libBasename = element.text();

		// Resolve the filename by adding the platform specific suffix/extension
		// and make the path absolute.
		QDir baseDir;
		if(isCore())	// The core library is not in the plugins directory.
			baseDir = QCoreApplication::applicationDirPath();
		else
			baseDir = QFileInfo(manifestFile()).dir();
#if defined(Q_OS_WIN)
		QFileInfo libFile(baseDir.absoluteFilePath(libBasename + ".dll"));
#else
		QFileInfo libFile(baseDir.absoluteFilePath(libBasename + ".so"));
#endif
		_libraryFilename = QDir::cleanPath(libFile.absoluteFilePath());

		return true;
	}
	return false;
}

/******************************************************************************
* Loads a native plugin's library.
******************************************************************************/
void NativePlugin::loadPluginImpl()
{
	NativeOvitoObjectType* linkedListBefore = nullptr;
#ifndef OVITO_MONOLITHIC_BUILD
	if(isCore() == false) {
		linkedListBefore = NativeOvitoObjectType::_firstInfo;

		// Load dynamic library.
		if(_library == nullptr || _library->isLoaded() == false) {
			if(libraryFilename().isEmpty())
				throw Exception(QString("The manifest file of the native plugin %1 does not specify the library name.").arg(pluginId()));
			_library = new QLibrary(libraryFilename(), this);
			_library->setLoadHints(QLibrary::ExportExternalSymbolsHint);
			if(!_library->load()) {
				throw Exception(QString("Failed to load native plugin library.\nLibrary file: %1\nError: %2").arg(libraryFilename(), _library->errorString()));
			}
		}
	}
#endif
	NativeOvitoObjectType* linkedListAfter = NativeOvitoObjectType::_firstInfo;

	// Connect all newly loaded class descriptors with this plugin.
	for(NativeOvitoObjectType* clazz = linkedListAfter; clazz != linkedListBefore; clazz = clazz->_next) {
#ifdef OVITO_MONOLITHIC_BUILD
		if(clazz->pluginId() != pluginId())
			continue;
#else
		if(clazz->pluginId() != pluginId())
			throw Exception(QString("Plugin ID %1 assigned to class %2 does not match plugin %3 that contains the class.").arg(clazz->pluginId()).arg(clazz->name()).arg(pluginId()));
#endif
		OVITO_ASSERT(clazz->plugin() == nullptr);
		clazz->_plugin = this;
		registerClass(clazz);
	}
}

}}}	// End of namespace
