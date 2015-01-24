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
#include <core/plugins/PluginManager.h>
#include <core/utilities/io/FileManager.h>
#include <core/dataset/DataSet.h>
#include <core/dataset/DataSetContainer.h>
#include "FileImporter.h"
#include "moc_FileExporter.cpp"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(DataIO)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, FileImporter, RefTarget);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, FileExporter, RefTarget);

/******************************************************************************
* Return the list of available import services.
******************************************************************************/
QVector<OvitoObjectType*> FileImporter::availableImporters()
{
	return PluginManager::instance().listClasses(FileImporter::OOType);
}

/******************************************************************************
* Return the list of available export services.
******************************************************************************/
QVector<OvitoObjectType*> FileExporter::availableExporters()
{
	return PluginManager::instance().listClasses(FileExporter::OOType);
}

/******************************************************************************
* Tries to detect the format of the given file.
******************************************************************************/
OORef<FileImporter> FileImporter::autodetectFileFormat(DataSet* dataset, const QUrl& url)
{
	if(!url.isValid())
		throw Exception(tr("Invalid path or URL."));

	// Download file so we can determine its format.
	DataSetContainer* container = dataset->container();
	Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(*container, url);
	if(!container->taskManager().waitForTask(fetchFileFuture))
		throw Exception(tr("Operation has been canceled by the user."));

	// Detect file format.
	return autodetectFileFormat(dataset, fetchFileFuture.result(), url.path());
}

/******************************************************************************
* Tries to detect the format of the given file.
******************************************************************************/
OORef<FileImporter> FileImporter::autodetectFileFormat(DataSet* dataset, const QString& localFile, const QUrl& sourceLocation)
{
	UndoSuspender noUnder(dataset);
	for(const OvitoObjectType* importerType : availableImporters()) {
		try {
			OORef<FileImporter> importer = static_object_cast<FileImporter>(importerType->createInstance(dataset));
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

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
