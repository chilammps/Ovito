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

#ifndef __OVITO_FILE_IMPORTER_H
#define __OVITO_FILE_IMPORTER_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>
#include <core/dataset/DataSet.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(DataIO)

/**
 * \brief Abstract base class for file import services.
 */
class OVITO_CORE_EXPORT FileImporter : public RefTarget
{
protected:

	/// \brief The constructor.
	FileImporter(DataSet* dataset) : RefTarget(dataset) {}

public:

	/// Import modes that control the behavior of the importFile() method.
	enum ImportMode {
		AskUser,				///< Let the user decide how to insert the imported data into the scene.
		AddToScene,				///< Add the imported data as a new object to the scene.
		ReplaceSelected,		///< Replace existing dataset with newly imported data if possible. Add to scene otherwise.
								///  In any case, keep all other scene objects as they are.
		ResetScene				///< Clear the contents of the current scene first before importing the data.
	};
	Q_ENUMS(ImportMode);

	/// \brief Returns the file filter that specifies the files that can be imported by this service.
	/// \return A wild-card pattern that specifies the file types that can be handled by this import class.
	virtual QString fileFilter() = 0;

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	/// \return A string that describes the file format.
	virtual QString fileFilterDescription() = 0;

	/// \brief Imports a file into the scene.
	/// \param sourceUrl The location of the file to import.
	/// \param importMode Controls how the imported data is inserted into the scene.
	/// \return \c true if the file has been successfully imported.
	//	        \c false if the operation has been canceled by the user.
	/// \throw Exception when the import operation has failed.
	virtual bool importFile(const QUrl& sourceUrl, ImportMode importMode = AskUser) = 0;

	/// \brief Checks if the given file has format that can be read by this importer.
	/// \param input The file that contains the data to check.
	/// \param sourceLocation The original source location of the file if it was loaded from a remote location.
	/// \return \c true if the data can be parsed.
	//	        \c false if the data has some unknown format.
	/// \throw Exception when the check has failed.
	virtual bool checkFileFormat(QFileDevice& input, const QUrl& sourceLocation) { return false; }

	/// Returns a list of all available importer types.
	static QVector<OvitoObjectType*> availableImporters();

	/// \brief Tries to detect the format of the given file.
	/// \return The importer class that can handle the given file. If the file format could not be recognized then NULL is returned.
	/// \throw Exception if url is invalid or if operation has been canceled by the user.
	/// \note This is a blocking function, which downloads the file and can take a long time to return.
	static OORef<FileImporter> autodetectFileFormat(DataSet* dataset, const QUrl& url);

	/// \brief Tries to detect the format of the given file.
	/// \return The importer class that can handle the given file. If the file format could not be recognized then NULL is returned.
	static OORef<FileImporter> autodetectFileFormat(DataSet* dataset, const QString& localFile, const QUrl& sourceLocation = QUrl());

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::FileImporter::ImportMode);
Q_DECLARE_TYPEINFO(Ovito::FileImporter::ImportMode, Q_PRIMITIVE_TYPE);

#endif // __OVITO_FILE_IMPORTER_H
