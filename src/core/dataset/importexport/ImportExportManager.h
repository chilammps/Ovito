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
 * \file ImportExportManager.h 
 * \brief Contains the definition of the Ovito::ImportExportManager class.
 */

#ifndef __OVITO_IMPORT_EXPORT_MANAGER_H
#define __OVITO_IMPORT_EXPORT_MANAGER_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>
#include "FileImporter.h"
#include "FileExporter.h"

namespace Ovito {

/**
 * \brief Manages the installed data import and export services.
 */
class OVITO_CORE_EXPORT ImportExportManager : public QObject
{
public:

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the DataSetManager singleton class.
	inline static ImportExportManager& instance() {
		OVITO_ASSERT_MSG(_instance != nullptr, "ImportExportManager::instance", "Singleton object is not initialized yet.");
		return *_instance;
	}

	/// Return the list of available import services.
	const QVector<FileImporterDescription*>& fileImporters(DataSet* dataset);

	/// Return the list of available export services.
	const QVector<FileExporterDescription*>& fileExporters(DataSet* dataset);

	/// \brief Tries to detect the format of the given file.
	/// \return The importer class that can handle the given file. If the file format could not be recognized then NULL is returned.
	/// \throw Exception if url is invalid or if operation has been canceled by the user.
	/// \note This is a blocking function, which downloads the file and can take a long time to return.
	OORef<FileImporter> autodetectFileFormat(DataSet* dataset, const QUrl& url);

	/// \brief Tries to detect the format of the given file.
	/// \return The importer class that can handle the given file. If the file format could not be recognized then NULL is returned.
	OORef<FileImporter> autodetectFileFormat(DataSet* dataset, const QString& localFile, const QUrl& sourceLocation = QUrl());

private:

	/// List of data import plugins.
	QVector<FileImporterDescription*> _fileImporters;

	/// List of data export plugins.
	QVector<FileExporterDescription*> _fileExporters;

private:

	/// Private constructor.
	/// This is a singleton class; no public instances are allowed.
	ImportExportManager();

	/// Create the singleton instance of this class.
	static void initialize() { _instance = new ImportExportManager(); }

	/// Deletes the singleton instance of this class.
	static void shutdown() { delete _instance; _instance = nullptr; }

	/// The singleton instance of this class.
	static ImportExportManager* _instance;

	Q_OBJECT

	friend class Application;
};

};

#endif // __OVITO_IMPORT_EXPORT_MANAGER_H
