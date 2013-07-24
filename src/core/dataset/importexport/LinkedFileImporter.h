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

#ifndef __OVITO_LINKED_FILE_IMPORTER_H
#define __OVITO_LINKED_FILE_IMPORTER_H

#include <core/Core.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/utilities/ObjectStatus.h>
#include <core/utilities/concurrent/Future.h>

namespace Ovito {

class LinkedFileObject;		// defined in LinkedFileObject.h

/**
 * \brief Base class for file parsers that can reload a file that has been imported into the scene.
 */
class LinkedFileImporter : public FileImporter
{
public:

	/// \brief This structures stores source information about an imported animation frame.
	struct FrameSourceInformation {

		/// The source file that contains the data of the animation frame.
		QUrl sourceFile;

		/// The byte offset into the source file where the data is stored.
		qint64 byteOffset;

		/// The line number in the source file where the data is stored, if the file has a text-based format.
		int lineNumber;

		/// The last modification time of the source file.
		/// This is used to detect changes of the source file, which let the stored byte offset become invalid.
		QDateTime lastModificationTime;

		/// The name or label of the source frame.
		QString label;

		/// Compares two structures.
		bool operator!=(const FrameSourceInformation& other) const {
			return (sourceFile != other.sourceFile) ||
					(byteOffset != other.byteOffset) ||
					(lineNumber != other.lineNumber) ||
					(lastModificationTime != other.lastModificationTime);
		}
	};

	class ImportedData {
	public:

		/// Destructor of virtual class.
		virtual ~ImportedData() {}

		/// Lets the data container insert the data it holds into the scene by creating
		/// appropriate scene objects.
		virtual void insertIntoScene(LinkedFileObject* destination) = 0;

		/// Returns a status object that describes the outcome of the loading operation.
		ObjectStatus status() const { return ObjectStatus(ObjectStatus::Success, QString(), _infoText); }

		/// Sets the informational text.
		void setInfoText(const QString& text) { _infoText = text; }

	private:

		/// Contains information about the loaded file.
		QString _infoText;
	};

	typedef std::shared_ptr<ImportedData> ImportedDataPtr;

public:

	/// \brief Constructs a new instance of this class.
	LinkedFileImporter() {}

	///////////////////////////// from FileImporter /////////////////////////////

	/// \brief Imports the given file into the scene.
	/// \return \a true if the file has been imported; \a false if the import has been aborted by the user.
	/// \throws Exception when the import has failed.
	virtual bool importFile(const QUrl& sourceUrl, DataSet* scene) override;

	//////////////////////////// Specific methods ////////////////////////////////

	/// \brief Opens the settings dialog for this importer.
	/// \param parent The parent window for the dialog box.
	/// \param object The LinkedFileObject that owns this importer.
	/// \return \c true if the dialog has been approved by the user; \c false when the user has canceled the operation.
	///
	/// The default implementation of this method does not show any dialog and always returns \a true.
	/// If this method is overridden to show a dialog box then the method hasSettingsDialog() should
	/// be overridden too.
	virtual bool showSettingsDialog(QWidget* parent, LinkedFileObject* object) { return true; }

	/// \brief Returns whether this importer has a settings dialog box to let the user configure the import settings.
	/// \return \c true if a call to showSettingsDialog() will show a dialog box; \c false otherwise.
	///
	/// The default implementation returns \c false.
	virtual bool hasSettingsDialog() { return false; }

	/// \brief Reads the data from the input file(s).
	/// \param frame The record that specifies the frame to load.
	/// \return A future that will give access to the loaded data.
	virtual Future<ImportedDataPtr> load(FrameSourceInformation frame);

	/// \brief Scans the input source (which can be a directory or a single file) to discover all animation frames.
	///
	/// The default implementation of this method checks if the source URL contains a wild-card pattern.
	/// If yes, it scans the directory to find all matching files.
	virtual Future<QVector<FrameSourceInformation>> findFrames(const QUrl& sourceUrl);

protected:

	/// \brief Reads the data from the input file(s).
	virtual void loadImplementation(FutureInterface<ImportedDataPtr>& futureInterface, FrameSourceInformation frame) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

/// \brief Writes an animation frame information record to a binary output stream.
inline SaveStream& operator<<(SaveStream& stream, const LinkedFileImporter::FrameSourceInformation& frame)
{
	stream.beginChunk(0x01);
	stream << frame.sourceFile << frame.byteOffset << frame.lineNumber << frame.lastModificationTime;
	stream.endChunk();
	return stream;
}

/// \brief Reads a box from a binary input stream.
inline LoadStream& operator>>(LoadStream& stream, LinkedFileImporter::FrameSourceInformation& frame)
{
	stream.expectChunk(0x01);
	stream >> frame.sourceFile >> frame.byteOffset >> frame.lineNumber >> frame.lastModificationTime;
	stream.closeChunk();
	return stream;
}

};

#endif // __OVITO_LINKED_FILE_IMPORTER_H
