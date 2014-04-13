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
#include "moc_FileExporter.cpp"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, FileImporter, RefTarget);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, FileExporter, RefTarget);

/// The singleton instance of the class.
ImportExportManager* ImportExportManager::_instance = nullptr;

/******************************************************************************
* Initializes the manager.
******************************************************************************/
ImportExportManager::ImportExportManager()
{
	OVITO_ASSERT_MSG(!_instance, "ImportExportManager constructor", "Multiple instances of this singleton class have been created.");
}

/******************************************************************************
* Return the list of available import services.
******************************************************************************/
const QVector<FileImporterDescription*>& ImportExportManager::fileImporters(DataSet* dataset)
{
	if(_fileImporters.empty()) {
		UndoSuspender noUnder(dataset);

		// Scan the class list for file import services.
		Q_FOREACH(const OvitoObjectType* clazz, PluginManager::instance().listClasses(FileImporter::OOType)) {
			try {
				// Create a temporary instance to get the supported file formats.
				OORef<FileImporter> obj = static_object_cast<FileImporter>(clazz->createInstance(dataset));
				if(obj)
					_fileImporters.push_back(new FileImporterDescription(this, obj));
			}
			catch(const Exception& ex) {
				ex.showError();
			}
		}
	}
	return _fileImporters;
}

/******************************************************************************
* Return the list of available export services.
******************************************************************************/
const QVector<FileExporterDescription*>& ImportExportManager::fileExporters(DataSet* dataset)
{
	if(_fileExporters.empty()) {
		UndoSuspender noUnder(dataset);

		// Scan the class list for file export services.
		Q_FOREACH(const OvitoObjectType* clazz, PluginManager::instance().listClasses(FileExporter::OOType)) {
			try {
				// Create a temporary instance to get the supported file formats.
				OORef<FileExporter> obj = static_object_cast<FileExporter>(clazz->createInstance(dataset));
				if(obj)
					_fileExporters.push_back(new FileExporterDescription(this, obj));
			}
			catch(const Exception& ex) {
				ex.showError();
			}
		}
	}
	return _fileExporters;
}

/******************************************************************************
* Tries to detect the format of the given file.
******************************************************************************/
OORef<FileImporter> ImportExportManager::autodetectFileFormat(DataSet* dataset, const QString& localFile, const QUrl& sourceLocation)
{
	UndoSuspender noUnder(dataset);
	for(FileImporterDescription* importerType : fileImporters(dataset)) {
		try {
			OORef<FileImporter> importer = importerType->createService(dataset);
			QFile file(localFile);
			if(importer && importer->checkFileFormat(file, sourceLocation)) {
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
