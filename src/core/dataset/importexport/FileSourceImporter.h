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
#include <core/utilities/concurrent/Task.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(DataIO)

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
	class FrameLoader : public AsynchronousTask {
	public:

		/// Constructor.
		FrameLoader(DataSetContainer* container, const Frame& frame) : _datasetContainer(*container), _frame(frame) {}

		/// Inserts the data loaded by perform() into the provided container object.
		/// This function is called by the system from the main thread after the asynchronous loading task
		/// has finished.
		/// \note The provided container may contain old data. It is the method's responsibility
		///       to remove unneeded data objects from the container so that it contains only
		///       the newly loaded data when the function returns. Existing data objects may be
		///       re-used to preserve certain setting if appropriate.
		virtual void handOver(CompoundObject* container) = 0;

		/// Returns the source file information.
		const Frame& frame() const { return _frame; }

		/// Returns the status of the load operation.
		const PipelineStatus& status() const { return _status; }

		/// Sets the status of the load operation.
		void setStatus(const QString& statusText) { _status.setText(statusText); }

		/// Returns the dataset container.
		DataSetContainer& datasetContainer() const { return _datasetContainer; }

	private:

		/// The dataset container.
		DataSetContainer& _datasetContainer;

		/// The source file information.
		Frame _frame;

		/// Stores additional status information about the load operation.
		PipelineStatus _status;
	};

public:

	/// \brief Constructs a new instance of this class.
	FileSourceImporter(DataSet* dataset) : FileImporter(dataset) {}

	///////////////////////////// from FileImporter /////////////////////////////

	/// \brief Imports the given file into the scene.
	virtual bool importFile(const QUrl& sourceUrl, ImportMode importMode) override;

	//////////////////////////// Specific methods ////////////////////////////////

	/// This method indicates whether a wildcard pattern should be automatically generated
	/// when the user picks a new input filename. The default implementation returns true.
	/// Subclasses can override this method to disable generation of wildcard patterns.
	virtual bool autoGenerateWildcardPattern() { return true; }

	/// Scans the given external path (which may be a directory and a wild-card pattern,
	/// or a single file containing multiple frames) to find all available animation frames.
	///
	/// \param sourceUrl The source file or wild-card pattern to scan for animation frames.
	/// \return A Future that will yield the list of discovered animation frames.
	///
	/// The default implementation of this method checks if the given URL contains a wild-card pattern.
	/// If yes, it scans the directory to find all matching files.
	/// Subclasses can override this method to support file formats which store multiple data frames per file.
	virtual Future<QVector<Frame>> discoverFrames(const QUrl& sourceUrl) {
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

	/// Creates an asynchronous loader object that loads the data for the given frame from the external file.
	virtual std::shared_ptr<FrameLoader> createFrameLoader(const Frame& frame) = 0;

protected:

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
/// \relates FileSourceImporter::Frame
inline SaveStream& operator<<(SaveStream& stream, const FileSourceImporter::Frame& frame)
{
	stream.beginChunk(0x02);
	stream << frame.sourceFile << frame.byteOffset << frame.lineNumber << frame.lastModificationTime << frame.label;
	stream.endChunk();
	return stream;
}

/// \brief Reads a box from a binary input stream.
/// \relates FileSourceImporter::Frame
inline LoadStream& operator>>(LoadStream& stream, FileSourceImporter::Frame& frame)
{
	int version = stream.expectChunkRange(0, 2);
	stream >> frame.sourceFile >> frame.byteOffset >> frame.lineNumber >> frame.lastModificationTime;
	if(version >= 2)
		stream >> frame.label;
	stream.closeChunk();
	return stream;
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_FILE_SOURCE_IMPORTER_H
