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

#ifndef __OVITO_FILE_SOURCE_IMPORTER_H
#define __OVITO_FILE_SOURCE_IMPORTER_H

#include <core/Core.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/scene/pipeline/PipelineStatus.h>
#include <core/utilities/concurrent/Future.h>

namespace Ovito { namespace DataIO {

/**
 * \brief Base class for file parsers that can reload a file that has been imported into the scene.
 */
class OVITO_CORE_EXPORT FileSourceImporter : public FileImporter
{
public:

	/// \brief This data structure stores source information about an imported animation frame.
	struct Frame {

		/// The source file that contains the data of the animation frame.
		QUrl sourceFile;

		/// The byte offset into the source file where the frame's data is stored.
		qint64 byteOffset;

		/// The line number in the source file where the frame data is stored, if the file has a text-based format.
		int lineNumber;

		/// The last modification time of the source file.
		/// This is used to detect changes of the source file, which let the stored byte offset become invalid.
		QDateTime lastModificationTime;

		/// The name or label of the source frame.
		QString label;

		/// Compares two data records.
		bool operator!=(const Frame& other) const {
			return (sourceFile != other.sourceFile) ||
					(byteOffset != other.byteOffset) ||
					(lineNumber != other.lineNumber) ||
					(lastModificationTime != other.lastModificationTime);
		}
	};

	/**
	 * Base class for background file loaders.
	 */
	class FrameLoader {
	public:

		/// Constructor.
		FrameLoader(const Frame& frame) : _frame(frame) {}

		// Virtual base destructor.
		virtual ~FrameLoader() = default;

		/// \brief Is called in the background thread to perform the actual loading.
		virtual void load(DataSetContainer& container, FutureInterfaceBase& futureInterface) = 0;

		/// \brief Lets the task object insert the data it holds into the scene by
		///        creating appropriate data objects.
		/// \return All data objects newly inserted into the destination FileSource
		///         or existing data objects modified by the importer. The FileSource
		///         will discard all existing data objects which are not in this set.
		virtual QSet<DataObject*> insertIntoScene(FileSource* destination) = 0;

		/// Returns the source file information.
		const Frame& frame() const { return _frame; }

		/// Returns the status of the import operation.
		PipelineStatus status() const { return PipelineStatus(PipelineStatus::Success, _infoText); }

		/// Sets the informational text.
		void setInfoText(const QString& text) { _infoText = text; }

		/// Returns the informational text.
		const QString& infoText() const { return _infoText; }

	private:

		/// The source file information.
		Frame _frame;

		/// Contains information about the loaded file set by the parser.
		QString _infoText;
	};

public:

	/// \brief Constructs a new instance of this class.
	FileSourceImporter(DataSet* dataset) : FileImporter(dataset) {}

	///////////////////////////// from FileImporter /////////////////////////////

	/// \brief Imports the given file into the scene.
	virtual bool importFile(const QUrl& sourceUrl, ImportMode importMode) override;

	//////////////////////////// Specific methods ////////////////////////////////

	/// \brief Reads the data from the input file(s).
	/// \param frame The record that specifies the frame to load.
	/// \return A future that will give access to the loaded data.
	virtual Future<std::shared_ptr<FrameLoader>> loadFrame(const Frame& frame);

	/// This method indicates whether a wildcard pattern should be automatically generated
	/// when the user picks a new input filename. The default implementation returns true.
	/// Subclasses can override this method to disable generation of wildcard patterns.
	virtual bool autoGenerateWildcardPattern() { return true; }

	/// \brief Scans the input source (which can be a directory or a single file) to discover all animation frames.
	///
	/// The default implementation of this method checks if the source URL contains a wild-card pattern.
	/// If yes, it scans the directory to find all matching files.
	virtual Future<QVector<Frame>> findFrames(const QUrl& sourceUrl) {
		return findWildcardMatches(sourceUrl, dataset()->container());
	}

	/// \brief Returns the list of files that match the given wildcard pattern.
	static Future<QVector<Frame>> findWildcardMatches(const QUrl& sourceUrl, DataSetContainer* datasetContainer);

	/// \brief Sends a request to the FileSource owning this importer to reload the input file.
	void requestReload(int frame = -1);

	/// \brief Sends a request to the FileSource owning this importer to refresh the animation frame sequence.
	void requestFramesUpdate();

	/// This method is called by the FileSource each time a new source
	/// file has been selected by the user. The importer class may inspect
	/// the new file at this point before it is actually loaded.
	/// Returns false if the operation has been canceled by the user.
	virtual bool inspectNewFile(FileSource* obj) { return true; }

protected:

	/// \brief Creates an import task object to read the given frame.
	virtual std::shared_ptr<FrameLoader> createImportTask(const Frame& frame) = 0;

	/// This method is called when the scene node for the FileSource is created.
	/// It can be overwritten by importer subclasses to customize the node, add modifiers, etc.
	/// The default implementation does nothing.
	virtual void prepareSceneNode(ObjectNode* node, FileSource* importObj) {}

	/// Checks if a filename matches to the given wildcard pattern.
	static bool matchesWildcardPattern(const QString& pattern, const QString& filename);

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("ClassNameAlias", "LinkedFileImporter");	// This for backward compatibility with files written by Ovito 2.4 and older.
};

/// \brief Writes an animation frame information record to a binary output stream.
inline SaveStream& operator<<(SaveStream& stream, const FileSourceImporter::Frame& frame)
{
	stream.beginChunk(0x02);
	stream << frame.sourceFile << frame.byteOffset << frame.lineNumber << frame.lastModificationTime << frame.label;
	stream.endChunk();
	return stream;
}

/// \brief Reads a box from a binary input stream.
inline LoadStream& operator>>(LoadStream& stream, FileSourceImporter::Frame& frame)
{
	int version = stream.expectChunkRange(0, 2);
	stream >> frame.sourceFile >> frame.byteOffset >> frame.lineNumber >> frame.lastModificationTime;
	if(version >= 2)
		stream >> frame.label;
	stream.closeChunk();
	return stream;
}

}}	// End of namespace

#endif // __OVITO_FILE_SOURCE_IMPORTER_H
