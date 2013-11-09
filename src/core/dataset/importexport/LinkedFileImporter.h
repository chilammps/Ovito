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
class SceneObject;			// defined in SceneObject.h

/**
 * \brief Base class for file parsers that can reload a file that has been imported into the scene.
 */
class OVITO_CORE_EXPORT LinkedFileImporter : public FileImporter
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

	/**
	 * Base class for background file loaders.
	 */
	class ImportTask {
	public:

		/// Constructor.
		ImportTask(const FrameSourceInformation& frame) : _frame(frame) {}

		/// Destructor of virtual class.
		virtual ~ImportTask() {}

		/// \brief Is called in the background thread to perform the actual loading.
		virtual void load(FutureInterfaceBase& futureInterface) = 0;

		/// \brief Lets the data container insert the data it holds into the scene by
		///        creating appropriate scene objects.
		/// \return All scene objects newly inserted into the destination LinkedFileObject
		///         or existing scene objects modified by the importer. The LinkedFileObject
		///         will discard all existing scene objects which are not in this set.
		virtual QSet<SceneObject*> insertIntoScene(LinkedFileObject* destination) = 0;

		/// Returns the source file information.
		const FrameSourceInformation& frame() const { return _frame; }

		/// Returns a status object that describes the outcome of the loading operation.
		ObjectStatus status() const { return ObjectStatus(ObjectStatus::Success, _infoText); }

		/// Sets the informational text.
		void setInfoText(const QString& text) { _infoText = text; }

	private:

		/// The source file information.
		FrameSourceInformation _frame;

		/// Contains information about the loaded file set by the parser.
		QString _infoText;
	};

	typedef std::shared_ptr<ImportTask> ImportTaskPtr;

public:

	/// \brief Constructs a new instance of this class.
	LinkedFileImporter() {}

	///////////////////////////// from FileImporter /////////////////////////////

	/// \brief Imports the given file into the scene.
	/// \return \a true if the file has been imported; \a false if the import has been aborted by the user.
	/// \throws Exception when the import has failed.
	virtual bool importFile(const QUrl& sourceUrl) override;

	//////////////////////////// Specific methods ////////////////////////////////

	/// \brief Reads the data from the input file(s).
	/// \param frame The record that specifies the frame to load.
	/// \return A future that will give access to the loaded data.
	virtual Future<ImportTaskPtr> load(const FrameSourceInformation& frame);

	/// This method indicates whether a wildcard pattern should be automatically generated
	/// when the user picks a new input filename. The default implementation returns true.
	/// Subclasses can override this method to disable generation of wildcard patterns.
	virtual bool autoGenerateWildcardPattern() { return true; }

	/// \brief Scans the input source (which can be a directory or a single file) to discover all animation frames.
	///
	/// The default implementation of this method checks if the source URL contains a wild-card pattern.
	/// If yes, it scans the directory to find all matching files.
	virtual Future<QVector<FrameSourceInformation>> findFrames(const QUrl& sourceUrl);

	/// \brief Sends a request to the LinkedFileObject owning this importer to reload the input file.
	void requestReload(int frame = -1);

	/// \brief Sends a request to the LinkedFileObject owning this importer to refresh the animation frame sequence.
	void requestFramesUpdate();

	/// This method is called by the LinkedFileObject each time a new source
	/// file has been selected by the user. The importer class may inspect
	/// the new file at this point before it is actually loaded.
	/// Returns false if the operation has been canceled by the user.
	virtual bool inspectNewFile(LinkedFileObject* obj) { return true; }

protected:

	/// \brief Creates an import task object to read the given frame.
	virtual ImportTaskPtr createImportTask(const FrameSourceInformation& frame) = 0;

	/// Checks if a filename matches to the given wildcard pattern.
	static bool matchesWildcardPattern(const QString& pattern, const QString& filename);

private:

	Q_OBJECT
	OVITO_OBJECT
};

/// \brief Writes an animation frame information record to a binary output stream.
inline SaveStream& operator<<(SaveStream& stream, const LinkedFileImporter::FrameSourceInformation& frame)
{
	stream.beginChunk(0x02);
	stream << frame.sourceFile << frame.byteOffset << frame.lineNumber << frame.lastModificationTime << frame.label;
	stream.endChunk();
	return stream;
}

/// \brief Reads a box from a binary input stream.
inline LoadStream& operator>>(LoadStream& stream, LinkedFileImporter::FrameSourceInformation& frame)
{
	int version = stream.expectChunkRange(0, 2);
	stream >> frame.sourceFile >> frame.byteOffset >> frame.lineNumber >> frame.lastModificationTime;
	if(version >= 2)
		stream >> frame.label;
	stream.closeChunk();
	return stream;
}

};

#endif // __OVITO_LINKED_FILE_IMPORTER_H
