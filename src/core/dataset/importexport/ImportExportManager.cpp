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
#include "ImportExportManager.h"
#include "moc_FileImporter.cpp"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, FileImporter, RefTarget)

/// The singleton instance of the class.
ImportExportManager* ImportExportManager::_instance = nullptr;

/******************************************************************************
* Initializes the manager.
******************************************************************************/
ImportExportManager::ImportExportManager()
{
	OVITO_ASSERT_MSG(!_instance, "ImportExportManager constructor", "Multiple instances of this singleton class have been created.");

	// Scan the class list for file import services.
	Q_FOREACH(const OvitoObjectType* clazz, PluginManager::instance().listClasses(FileImporter::OOType)) {
		try {
			// Create a temporary instance to get the supported file formats.
			OORef<FileImporter> obj = static_object_cast<FileImporter>(clazz->createInstance());
			if(obj)
				_fileImporters.push_back(FileImporterDescription(obj.get()));
		}
		catch(const Exception& ex) {
			ex.showError();
		}
	}
}

/******************************************************************************
* Tries to detect the format of the given file.
******************************************************************************/
OORef<FileImporter> ImportExportManager::autodetectFileFormat(const QString& filename)
{
	for(const FileImporterDescription& importerType : fileImporters()) {
		try {
			OORef<FileImporter> importer = importerType.createService();
			QFile file(filename);
			if(importer && importer->checkFileFormat(file)) {
				return importer;
			}
		}
		catch(const Exception& ex) {
			ex.showError();
		}
	}
	return nullptr;
}

};
