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
 * \file FileImporter.h
 * \brief Contains the definition of the Ovito::FileImporter class.
 */

#ifndef __OVITO_FILE_IMPORTER_H
#define __OVITO_FILE_IMPORTER_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>
#include <core/dataset/DataSet.h>

namespace Ovito {

/**
 * \brief Abstract base class for file import services.
 */
class OVITO_CORE_EXPORT FileImporter : public RefTarget
{
protected:
	
	/// \brief The default constructor.
	FileImporter() {}

public:

	/// \brief Returns the file filter that specifies the files that can be imported by this service.
	/// \return A wild-card pattern that specifies the file types that can be handled by this import class.
	virtual QString fileFilter() = 0;

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	/// \return A string that describes the file format.
	virtual QString fileFilterDescription() = 0;

	/// \brief Imports a file into the scene.
	/// \param sourceUrl The location of the file to import.
	/// \param scene The scene into which the file contents should be imported.
	/// \return \c true if the file has been successfully imported.
	//	        \c false if the import has been aborted by the user.
	/// \throw Exception when the import has failed.
	virtual bool importFile(const QUrl& sourceUrl, DataSet* scene)  = 0;

	/// \brief Checks if the given file has format that can be read by this importer.
	/// \param input The I/O device that contains the file data to check.
	/// \param sourceLocation The original source location of the file if it was loaded from a remote location.
	/// \return \c true if the data can be parsed.
	//	        \c false if the data has some unknown format.
	/// \throw Exception when the check has failed.
	virtual bool checkFileFormat(QIODevice& input, const QUrl& sourceLocation) { return false; }

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief This descriptor contains information about an installed FileImporter service.
 */
class OVITO_CORE_EXPORT FileImporterDescription
{
public:

	/// \brief Default constructor (required by QVector container).
	FileImporterDescription() {}

	/// \brief Initializes this descriptor from a file importer instance.
	FileImporterDescription(FileImporter* importer) :
		_fileFilter(importer->fileFilter()),
		_fileFilterDescription(importer->fileFilterDescription()),
		_pluginClass(&importer->getOOType()) {}

	/// \brief Returns the file filter that specifies the files that can be imported by the service.
	/// \return A wild-card pattern that specifies the file types that can be handled by the importer class.
	const QString& fileFilter() const { return _fileFilter; }

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	/// \return A string that describes the file format.
	const QString& fileFilterDescription() const { return _fileFilterDescription; }

	/// \brief Creates an instance of the file importer class.
	OORef<FileImporter> createService() const {
		return static_object_cast<FileImporter>(pluginClass()->createInstance());
	}

	/// \brief Returns the class descriptor for the file importer service.
	/// \return The descriptor of the FileImporter-derived class.
	const OvitoObjectType* pluginClass() const { return _pluginClass; }

private:

	QString _fileFilter;
	QString _fileFilterDescription;
	const OvitoObjectType* _pluginClass;
};

};

#endif // __OVITO_FILE_IMPORTER_H
