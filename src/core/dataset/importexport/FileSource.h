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

#ifndef __OVITO_FILE_SOURCE_H
#define __OVITO_FILE_SOURCE_H

#include <core/Core.h>
#include <core/scene/objects/CompoundObject.h>
#include "FileSourceImporter.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(DataIO)

/**
 * \brief A place holder object that feeds data read from an external file into the scene.
 *
 * This class is used in conjunction with a FileSourceImporter class.
 */
class OVITO_CORE_EXPORT FileSource : public CompoundObject
{
public:

	/// Constructs an empty file source which is not referring to an external file.
	Q_INVOKABLE FileSource(DataSet* dataset);

	/// Returns the object that is responsible for loading data from the external file referenced by this FileSource.
	/// \return The importer owned by this FileSource; or \c nullptr if this FileSource is currently not referring to an external file.
	/// The importer can be replaced by calling setSource().
	FileSourceImporter* importer() const { return _importer; }

	/// \brief Sets the source location for importing data.
	/// \param sourceUrl The new source location.
	/// \param importerType The FileSourceImporter type that will be used to parse the input file (can be \c nullptr to request format auto-detection).
	/// \return false if the operation has been canceled by the user.
	bool setSource(const QUrl& sourceUrl, const OvitoObjectType* importerType = nullptr);

	/// \brief Sets the source location for importing data.
	/// \param sourceUrl The new source location.
	/// \param importer The importer object that will parse the input file.
	/// \return false if the operation has been canceled by the user.
	bool setSource(QUrl sourceUrl, FileSourceImporter* importer, bool useExactURL = false);

	/// \brief Returns the source location of the data.
	const QUrl& sourceUrl() const { return _sourceUrl; }

	/// \brief This reloads the input data from the external file.
	/// \param frameIndex The animation frame to reload from the external file.
	Q_INVOKABLE void refreshFromSource(int frameIndex = -1);

	/// \brief Returns the status returned by the file parser on its last invocation.
	virtual PipelineStatus status() const override { return _importStatus; }

	/// \brief Scans the input source for animation frames and updates the internal list of frames.
	Q_INVOKABLE bool updateFrames();

	/// \brief Returns the number of animation frames that can be loaded from the data source.
	int numberOfFrames() const { return _frames.size(); }

	/// \brief Returns the index of the animation frame loaded last from the input file.
	int loadedFrameIndex() const { return _loadedFrameIndex; }

	/// \brief Returns the list of animation frames in the input file(s).
	const QVector<FileSourceImporter::Frame>& frames() const { return _frames; }

	/// \brief Given an animation time, computes the input frame index to be shown at that time.
	Q_INVOKABLE int animationTimeToInputFrame(TimePoint time) const;

	/// \brief Given an input frame index, returns the animation time at which it is shown.
	Q_INVOKABLE TimePoint inputFrameToAnimationTime(int frame) const;

	/// \brief Returns whether the scene's animation interval is being adjusted to the number of frames reported by the file parser.
	bool adjustAnimationIntervalEnabled() const { return _adjustAnimationIntervalEnabled; }

	/// \brief Controls whether the scene's animation interval should be adjusted to the number of frames reported by the file parser.
	void setAdjustAnimationIntervalEnabled(bool enabled) { _adjustAnimationIntervalEnabled = enabled; }

	/// \brief Adjusts the animation interval of the current data set to the number of frames in the data source.
	void adjustAnimationInterval(int gotoFrameIndex = -1);

	/// \brief Requests a frame of the input file sequence.
	PipelineFlowState requestFrame(int frameIndex);

	/// \brief Asks the object for the result of the geometry pipeline at the given time.
	virtual PipelineFlowState evaluate(TimePoint time) override;

	/// Returns the title of this object.
	virtual QString objectTitle() override;

public Q_SLOTS:

	/// \brief Displays the file selection dialog and lets the user select a new input file.
	void showFileSelectionDialog(QWidget* parent = nullptr);

	/// \brief Displays the remote file selection dialog and lets the user select a new source URL.
	void showURLSelectionDialog(QWidget* parent = nullptr);

protected Q_SLOTS:

	/// \brief This is called when the background loading operation has finished.
	void loadOperationFinished();

protected:

	/// \brief Saves the status returned by the parser object and generates a ReferenceEvent::ObjectStatusChanged event.
	void setStatus(const PipelineStatus& status);

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// \brief Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// \brief Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// \brief Cancels the current load operation if there is any in progress.
	void cancelLoadOperation();

private:

	/// The associated importer object that is responsible for parsing the input file.
	ReferenceField<FileSourceImporter> _importer;

	/// Controls whether the scene's animation interval is adjusted to the number of frames found in the input file.
	PropertyField<bool> _adjustAnimationIntervalEnabled;

	/// The source file (may include a wild-card pattern).
	PropertyField<QUrl, QUrl, ReferenceEvent::TitleChanged> _sourceUrl;

	/// Controls the mapping of input file frames to animation frames (i.e. the numerator of the playback rate for the file sequence).
	PropertyField<int> _playbackSpeedNumerator;

	/// Controls the mapping of input file frames to animation frames (i.e. the denominator of the playback rate for the file sequence).
	PropertyField<int> _playbackSpeedDenominator;

	/// Specifies the starting animation frame to which the first frame of the file sequence is mapped.
	PropertyField<int> _playbackStartTime;

	/// Stores the list of animation frames in the input file(s).
	QVector<FileSourceImporter::Frame> _frames;

	/// The index of the animation frame loaded last from the input file.
	int _loadedFrameIndex;

	/// The index of the animation frame currently being loaded.
	int _frameBeingLoaded;

	/// The asynchronous file loading task started by requestFrame().
	std::shared_ptr<FileSourceImporter::FrameLoader> _activeFrameLoader;

	/// The watcher object that is used to monitor the background operation.
	FutureWatcher _frameLoaderWatcher;

	/// The status returned by the parser during its last call.
	PipelineStatus _importStatus;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("ClassNameAlias", "LinkedFileObject");	// This for backward compatibility with files written by Ovito 2.4 and older.

	DECLARE_REFERENCE_FIELD(_importer);
	DECLARE_PROPERTY_FIELD(_adjustAnimationIntervalEnabled);
	DECLARE_PROPERTY_FIELD(_sourceUrl);
	DECLARE_PROPERTY_FIELD(_playbackSpeedNumerator);
	DECLARE_PROPERTY_FIELD(_playbackSpeedDenominator);
	DECLARE_PROPERTY_FIELD(_playbackStartTime);
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_FILE_SOURCE_H
