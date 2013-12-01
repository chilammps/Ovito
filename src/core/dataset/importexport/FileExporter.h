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
 * \file FileExporter.h
 * \brief Contains the definition of the Ovito::FileExporter class.
 */

#ifndef __OVITO_FILE_EXPORTER_H
#define __OVITO_FILE_EXPORTER_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>
#include <core/dataset/DataSet.h>

namespace Ovito {

/**
 * \brief Abstract base class for file export services.
 */
class OVITO_CORE_EXPORT FileExporter : public RefTarget
{
protected:
	
	/// \brief The constructor.
	FileExporter(DataSet* dataset) : RefTarget(dataset) {}

public:

	/// \brief Returns the file filter that specifies the files that can be exported by this service.
	/// \return A wild-card pattern that specifies the file types that can be handled by this export class.
	virtual QString fileFilter() = 0;

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	/// \return A string that describes the file format.
	virtual QString fileFilterDescription() = 0;

	/// \brief Exports the scene to a file.
	/// \param filePath The path of the file to export to.
	/// \param scene The scene that should be exported.
	/// \return \c true if the file has been successfully written.
	///         \c false if the export operation has been aborted by the user.
	/// \throw Exception when the export has failed or an error has occurred.
	virtual bool exportToFile(const QString& filePath, DataSet* scene) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief This descriptor contains information about an installed FileExporter service.
 */
class OVITO_CORE_EXPORT FileExporterDescription
{
public:

	/// \brief Default constructor (required by QVector container).
	FileExporterDescription() {}

	/// \brief Initializes this descriptor from a file exporter instance.
	FileExporterDescription(FileExporter* exporter) :
		_fileFilter(exporter->fileFilter()),
		_fileFilterDescription(exporter->fileFilterDescription()),
		_pluginClass(&exporter->getOOType()) {}

	/// \brief Returns the file filter that specifies the files that can be exported by the service.
	/// \return A wild-card pattern that specifies the file types that can be handled by the exporter class.
	const QString& fileFilter() const { return _fileFilter; }

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	/// \return A string that describes the file format.
	const QString& fileFilterDescription() const { return _fileFilterDescription; }

	/// \brief Creates an instance of the file exporter class.
	/// \param The dataset within which the exporter object is to be created.
	OORef<FileExporter> createService(DataSet* dataset) const {
		return static_object_cast<FileExporter>(pluginClass()->createInstance(dataset));
	}

	/// \brief Returns the class descriptor for the file exporter service.
	/// \return The descriptor of the FileExporter-derived class.
	const OvitoObjectType* pluginClass() const { return _pluginClass; }

private:

	QString _fileFilter;
	QString _fileFilterDescription;
	const OvitoObjectType* _pluginClass;
};

};

#endif // __OVITO_FILE_EXPORTER_H
