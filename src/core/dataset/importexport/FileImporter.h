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
class FileImporter : public RefTarget
{
protected:
	
	/// \brief The default constructor.
	FileImporter() : _shouldReplaceScene(true) {}

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
	/// \param suppressDialogs Controls the suppression of any dialog boxes. This is used when
	///                        the function is invoked from a script.
	/// \return \c true if the file has been successfully imported.
	//	        \c false if the import has been aborted by the user.
	/// \throw Exception when the import has failed.
	virtual bool importFile(const QUrl& sourceUrl, DataSet* scene, bool suppressDialogs = false)  = 0;

	/// \brief Checks if the given file has format that can be read by this importer.
	/// \param input The I/O device that contains the file data to check.
	/// \return \c true if the data can be parsed.
	//	        \c false if the data has some unknown format.
	/// \throw Exception when the check has failed.
	virtual bool checkFileFormat(QIODevice& input) { return false; }

	/// \brief Returns whether the importer will reset the scene before importing the new data by calling DataSet::clearScene().
	/// \return \c true if the old scene should be completely by the new data; \c false if the imported
	///         data should be added to the current scene.
	bool shouldReplaceScene() const { return _shouldReplaceScene; }

	/// \brief Sets whether the importer should reset the scene before importing the new data by calling DataSet::clearScene().
	/// \param replace \c true to have the old scene completely replaced by the new data; \c false if the imported
	///         data should be added to the existing scene.
	void setShouldReplaceScene(bool replace) { _shouldReplaceScene = replace; }

private:

	/// \brief Controls whether the importer should reset the scene before importing the new data.
	bool _shouldReplaceScene;

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief This descriptor contains information about an installed FileImorpter service.
 */
class FileImporterDescription
{
public:

	/// \brief Default constructor (required by QVector container).
	FileImporterDescription() {}

	/// \brief Initializes this descriptor.
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
	/// \return The descriptor of the FileImporterderived class.
	const OvitoObjectType* pluginClass() const { return _pluginClass; }

private:

	QString _fileFilter;
	QString _fileFilterDescription;
	const OvitoObjectType* _pluginClass;
};

};

#endif // __OVITO_FILE_IMPORTER_H
