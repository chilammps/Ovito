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

#include <core/Core.h>
#include <core/animation/AnimationSettings.h>
#include <core/utilities/io/ObjectLoadStream.h>
#include <core/utilities/io/ObjectSaveStream.h>
#include <core/utilities/io/FileManager.h>
#include <core/utilities/concurrent/Task.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/scene/ObjectNode.h>
#include <core/gui/dialogs/ImportFileDialog.h>
#include <core/gui/dialogs/ImportRemoteFileDialog.h>
#include <core/dataset/importexport/ImportExportManager.h>
#include <core/dataset/DatasetContainer.h>
#include <core/dataset/UndoStack.h>
#include "LinkedFileObject.h"
#include "LinkedFileObjectEditor.h"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LinkedFileObject, SceneObject)
SET_OVITO_OBJECT_EDITOR(LinkedFileObject, LinkedFileObjectEditor)
DEFINE_FLAGS_REFERENCE_FIELD(LinkedFileObject, _importer, "Importer", LinkedFileImporter, PROPERTY_FIELD_ALWAYS_DEEP_COPY|PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(LinkedFileObject, _sceneObjects, "SceneObjects", SceneObject, PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_PROPERTY_FIELD(LinkedFileObject, _adjustAnimationIntervalEnabled, "AdjustAnimationIntervalEnabled")
DEFINE_FLAGS_PROPERTY_FIELD(LinkedFileObject, _sourceUrl, "SourceUrl", PROPERTY_FIELD_NO_UNDO)
DEFINE_PROPERTY_FIELD(LinkedFileObject, _playbackSpeedNumerator, "PlaybackSpeedNumerator")
DEFINE_PROPERTY_FIELD(LinkedFileObject, _playbackSpeedDenominator, "PlaybackSpeedDenominator")
DEFINE_PROPERTY_FIELD(LinkedFileObject, _playbackStartTime, "PlaybackStartTime")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _importer, "File Importer")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _sceneObjects, "Objects")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _adjustAnimationIntervalEnabled, "Auto-adjust animation interval")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _sourceUrl, "Source location")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _playbackSpeedNumerator, "Playback speed numerator")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _playbackSpeedDenominator, "Playback speed denominator")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _playbackStartTime, "Playback start time")

/******************************************************************************
* Constructs the object.
******************************************************************************/
LinkedFileObject::LinkedFileObject(DataSet* dataset) : SceneObject(dataset),
	_adjustAnimationIntervalEnabled(true), _loadedFrame(-1), _frameBeingLoaded(-1),
	_playbackSpeedNumerator(1), _playbackSpeedDenominator(1), _playbackStartTime(0)
{
	INIT_PROPERTY_FIELD(LinkedFileObject::_importer);
	INIT_PROPERTY_FIELD(LinkedFileObject::_sceneObjects);
	INIT_PROPERTY_FIELD(LinkedFileObject::_adjustAnimationIntervalEnabled);
	INIT_PROPERTY_FIELD(LinkedFileObject::_sourceUrl);
	INIT_PROPERTY_FIELD(LinkedFileObject::_playbackSpeedNumerator);
	INIT_PROPERTY_FIELD(LinkedFileObject::_playbackSpeedDenominator);
	INIT_PROPERTY_FIELD(LinkedFileObject::_playbackStartTime);

	connect(&_loadFrameOperationWatcher, &FutureWatcher::finished, this, &LinkedFileObject::loadOperationFinished);

	// Do not save a copy of the linked external data in scene file by default.
	setSaveWithScene(false);
}

/******************************************************************************
* Sets the source location for importing data.
******************************************************************************/
bool LinkedFileObject::setSource(const QUrl& newSourceUrl, const FileImporterDescription* importerType)
{
	OORef<FileImporter> fileimporter;

	// Create file importer.
	if(!importerType) {

		// Download file so we can determine its format.
		Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(*dataSet()->container(), newSourceUrl);
		if(!dataSet()->container()->taskManager().waitForTask(fetchFileFuture))
			return false;

		// Detect file format.
		fileimporter = ImportExportManager::instance().autodetectFileFormat(dataSet(), fetchFileFuture.result(), newSourceUrl.path());
		if(!fileimporter)
			throw Exception(tr("Could not detect the format of the file to be imported. The format might not be supported."));
	}
	else {
		fileimporter = importerType->createService(dataSet());
		if(!fileimporter)
			return false;
	}
	OORef<LinkedFileImporter> newImporter = dynamic_object_cast<LinkedFileImporter>(fileimporter);
	if(!newImporter)
		throw Exception(tr("You did not select a compatible file."));

	ViewportSuspender noVPUpdate(dataSet()->viewportConfig());

	// Re-use the old importer if possible.
	if(importer() && importer()->getOOType() == newImporter->getOOType())
		newImporter = importer();

	// Set the new input location.
	return setSource(newSourceUrl, newImporter, true);
}

/******************************************************************************
* Sets the source location for importing data.
******************************************************************************/
bool LinkedFileObject::setSource(QUrl sourceUrl, const OORef<LinkedFileImporter>& importer, bool useExactURL)
{
	if(this->sourceUrl() == sourceUrl && this->importer() == importer)
		return true;

	QFileInfo fileInfo(sourceUrl.path());
	QString originalFilename = fileInfo.fileName();

	if(importer) {
		// If URL is not already a wildcard pattern, generate a default pattern by
		// replacing last sequence of numbers in the filename with a wildcard character.
		if(!useExactURL && importer->autoGenerateWildcardPattern() && !originalFilename.contains('*') && !originalFilename.contains('?')) {
			int startIndex, endIndex;
			for(endIndex = originalFilename.length() - 1; endIndex >= 0; endIndex--)
				if(originalFilename.at(endIndex).isNumber()) break;
			if(endIndex >= 0) {
				for(startIndex = endIndex-1; startIndex >= 0; startIndex--)
					if(!originalFilename.at(startIndex).isNumber()) break;
				QString wildcardPattern = originalFilename.left(startIndex+1) + '*' + originalFilename.mid(endIndex+1);
				fileInfo.setFile(fileInfo.dir(), wildcardPattern);
				sourceUrl.setPath(fileInfo.filePath());
				OVITO_ASSERT(sourceUrl.isValid());
			}
		}

		if(this->sourceUrl() == sourceUrl && this->importer() == importer)
			return true;
	}

	// Make the import process reversible.
	UndoableTransaction transaction(dataSet()->undoStack(), tr("Set input file"));

	// Make the call to setSource() undoable.
	class SetSourceOperation : public UndoableOperation {
	public:
		SetSourceOperation(LinkedFileObject* obj) : _obj(obj), _oldUrl(obj->sourceUrl()), _oldImporter(obj->importer()) {}
		virtual void undo() override {
			QUrl url = _obj->sourceUrl();
			OORef<LinkedFileImporter> importer = _obj->importer();
			_obj->setSource(_oldUrl, _oldImporter, true);
			_oldUrl = url;
			_oldImporter = importer;
		}
		virtual void redo() override { undo(); }
	private:
		QUrl _oldUrl;
		OORef<LinkedFileImporter> _oldImporter;
		OORef<LinkedFileObject> _obj;
	};
	if(dataSet()->undoStack().isRecording())
		dataSet()->undoStack().push(new SetSourceOperation(this));

	_sourceUrl = sourceUrl;
	_importer = importer;

	// Scan the input source for animation frames.
	if(updateFrames()) {

		// Jump to the right frame to show the originally selected file.
		int jumpToFrame = -1;
		for(int frameIndex = 0; frameIndex < _frames.size(); frameIndex++) {
			QFileInfo fileInfo(_frames[frameIndex].sourceFile.path());
			if(fileInfo.fileName() == originalFilename) {
				jumpToFrame = frameIndex;
				break;
			}
		}

		// Adjust the animation length number to match the number of frames in the input data source.
		adjustAnimationInterval(jumpToFrame);

		// Let the parser inspect the file. The user may still cancel the import
		// operation at this point.
		if(!_frames.empty() && importer->inspectNewFile(this) == false)
			return false;

		if(_adjustAnimationIntervalEnabled) {
			// Adjust views to completely show the new object.
			OORef<DataSet> ds(dataSet());
			ds->runWhenSceneIsReady([ds]() {
				ds->viewportConfig()->zoomToSelectionExtents();
			});
		}

		// Cancel any old load operation in progress.
		cancelLoadOperation();

		transaction.commit();
		notifyDependents(ReferenceEvent::TitleChanged);

		return true;
	}

	// Transaction has not been committed. We will revert to old state.
	return false;
}

/******************************************************************************
* Scans the input source for animation frames and updates the internal list of frames.
******************************************************************************/
bool LinkedFileObject::updateFrames()
{
	if(!importer()) {
		_frames.clear();
		_loadedFrame = -1;
		return false;
	}

	Future<QVector<LinkedFileImporter::FrameSourceInformation>> framesFuture = importer()->findFrames(sourceUrl());
	if(!dataSet()->container()->taskManager().waitForTask(framesFuture))
		return false;

	QVector<LinkedFileImporter::FrameSourceInformation> newFrames = framesFuture.result();

	// Reload current frame if file has changed.
	if(_loadedFrame >= 0) {
		if(_loadedFrame >= newFrames.size() || _loadedFrame >= _frames.size() || newFrames[_loadedFrame] != _frames[_loadedFrame])
			_loadedFrame = -1;
	}

	_frames = newFrames;
	notifyDependents(ReferenceEvent::TargetChanged);

	return true;
}

/******************************************************************************
* Cancels the current load operation if there is any in progress.
******************************************************************************/
void LinkedFileObject::cancelLoadOperation()
{
	if(_frameBeingLoaded != -1) {
		try {
			// This will suppress any pending notification events.
			_loadFrameOperationWatcher.unsetFuture();
			_loadFrameOperation.cancel();
			_loadFrameOperation.waitForFinished();
		} catch(...) {}
		_frameBeingLoaded = -1;
		notifyDependents(ReferenceEvent::PendingStateChanged);
	}
}

/******************************************************************************
* Given an animation time, computes the input frame index to be shown at that time.
******************************************************************************/
int LinkedFileObject::animationTimeToInputFrame(TimePoint time) const
{
	int animFrame = dataSet()->animationSettings()->timeToFrame(time);
	return (animFrame - _playbackStartTime) *
			std::max(1, (int)_playbackSpeedNumerator) /
			std::max(1, (int)_playbackSpeedDenominator);
}

/******************************************************************************
* Given an input frame index, returns the animation time at which it is shown.
******************************************************************************/
TimePoint LinkedFileObject::inputFrameToAnimationTime(int frame) const
{
	int animFrame = frame *
			std::max(1, (int)_playbackSpeedDenominator) /
			std::max(1, (int)_playbackSpeedNumerator) +
			_playbackStartTime;
	return dataSet()->animationSettings()->frameToTime(animFrame);
}

/******************************************************************************
* Asks the object for the result of the geometry pipeline at the given time.
******************************************************************************/
PipelineFlowState LinkedFileObject::evaluate(TimePoint time)
{
	return requestFrame(animationTimeToInputFrame(time));
}

/******************************************************************************
* Requests a frame of the input file sequence.
******************************************************************************/
PipelineFlowState LinkedFileObject::requestFrame(int frame)
{
	// Handle out-of-range cases.
	if(frame < 0) frame = 0;
	if(frame >= numberOfFrames()) frame = numberOfFrames() - 1;

	// Determine validity interval of the returned state.
	TimeInterval interval = TimeInterval::forever();
	if(frame > 0)
		interval.setStart(inputFrameToAnimationTime(frame));
	if(frame < numberOfFrames() - 1)
		interval.setEnd(inputFrameToAnimationTime(frame+1)-1);

	bool oldLoadingTaskWasCanceled = false;
	if(_frameBeingLoaded != -1) {
		if(_frameBeingLoaded == frame) {
			// The requested frame is already being loaded at the moment.
			// Indicate to the caller that the result is pending.
			return PipelineFlowState(ObjectStatus::Pending, _sceneObjects.targets(), interval,
					{{ QStringLiteral("Frame"), QVariant::fromValue(frame) }} );
		}
		else {
			// Another frame than the requested one is already being loaded.
			// Cancel pending loading operation first.
			try {
				// This will suppress any pending notification events.
				_loadFrameOperationWatcher.unsetFuture();
				_loadFrameOperation.cancel();
				_loadFrameOperation.waitForFinished();
			} catch(...) {}
			_frameBeingLoaded = -1;
			// Inform previous caller that the existing loading operation has been canceled.
			oldLoadingTaskWasCanceled = true;
		}
	}
	if(frame >= 0 && _loadedFrame == frame) {
		if(oldLoadingTaskWasCanceled)
			notifyDependents(ReferenceEvent::PendingStateChanged);

		// The requested frame has already been loaded and is available immediately.
		return PipelineFlowState(status(), _sceneObjects.targets(), interval,
				{{ QStringLiteral("Frame"), QVariant::fromValue(frame) }} );
	}
	else {
		// The requested frame needs to be loaded first. Start background loading task.
		if(frame < 0 || frame >= numberOfFrames() || !importer()) {
			if(oldLoadingTaskWasCanceled)
				notifyDependents(ReferenceEvent::PendingStateChanged);
			setStatus(ObjectStatus(ObjectStatus::Error, tr("The source location is empty (no files found).")));
			_loadedFrame = -1;
			return PipelineFlowState(status(), _sceneObjects.targets(), interval);
		}
		_frameBeingLoaded = frame;
		_loadFrameOperation = importer()->load(_frames[frame]);
		_loadFrameOperationWatcher.setFuture(_loadFrameOperation);
		setStatus(ObjectStatus::Pending);
		if(oldLoadingTaskWasCanceled)
			notifyDependents(ReferenceEvent::PendingStateChanged);
		// Indicate to the caller that the result is pending.
		return PipelineFlowState(ObjectStatus::Pending, _sceneObjects.targets(), interval,
				{{ QStringLiteral("Frame"), QVariant::fromValue(frame) }} );
	}
}

/******************************************************************************
* This is called when the background loading operation has finished.
******************************************************************************/
void LinkedFileObject::loadOperationFinished()
{
	OVITO_ASSERT(_frameBeingLoaded != -1);
	bool wasCanceled = _loadFrameOperation.isCanceled();
	_loadedFrame = _frameBeingLoaded;
	_frameBeingLoaded = -1;
	ObjectStatus newStatus = status();

	if(!wasCanceled) {
		try {
			// Adopt the data loaded by the importer.
			LinkedFileImporter::ImportTaskPtr importedData = _loadFrameOperation.result();
			QSet<SceneObject*> activeObjects;
			if(importedData) {
				activeObjects = importedData->insertIntoScene(this);
				newStatus = importedData->status();
			}
			removeInactiveObjects(activeObjects);
		}
		catch(Exception& ex) {
			// Transfer exception message to evaluation status.
			newStatus = ObjectStatus(ObjectStatus::Error, ex.messages().join(QChar('\n')));
			ex.showError();
		}
	}
	else {
		newStatus = ObjectStatus(ObjectStatus::Error, tr("Load operation has been canceled by the user."));
	}

	// Reset everything.
	_loadFrameOperationWatcher.unsetFuture();
	_loadFrameOperation.reset();

	// Set the new object status.
	setStatus(newStatus);

	// Notify dependents that the evaluation request was completed.
	notifyDependents(ReferenceEvent::PendingStateChanged);
	notifyDependents(ReferenceEvent::TitleChanged);
}

/******************************************************************************
* This will reload an animation frame.
******************************************************************************/
void LinkedFileObject::refreshFromSource(int frame)
{
	if(!importer())
		return;

	// Remove external file from local file cache so that it will be fetched from the
	// remote server again.
	if(frame >= 0 && frame < _frames.size())
		FileManager::instance().removeFromCache(_frames[frame].sourceFile);

	if(frame == loadedFrame() || frame == -1) {
		_loadedFrame = -1;
		notifyDependents(ReferenceEvent::TargetChanged);
	}
}

/******************************************************************************
* Saves the status returned by the parser object and generates a
* ReferenceEvent::ObjectStatusChanged event.
******************************************************************************/
void LinkedFileObject::setStatus(const ObjectStatus& status)
{
	if(status == _importStatus) return;
	_importStatus = status;
	notifyDependents(ReferenceEvent::ObjectStatusChanged);
}

/******************************************************************************
* Adjusts the animation interval of the current data set to the number of
* frames reported by the file parser.
******************************************************************************/
void LinkedFileObject::adjustAnimationInterval(int gotoFrameIndex)
{
	if(!_adjustAnimationIntervalEnabled)
		return;

	AnimationSettings* animSettings = dataSet()->animationSettings();

	int numFrames = std::max(1, numberOfFrames());
	TimeInterval interval(inputFrameToAnimationTime(0), inputFrameToAnimationTime(numberOfFrames()-1));
	animSettings->setAnimationInterval(interval);
	if(gotoFrameIndex >= 0 && gotoFrameIndex < numberOfFrames()) {
		animSettings->setTime(inputFrameToAnimationTime(gotoFrameIndex));
	}
	else if(animSettings->time() > interval.end())
		animSettings->setTime(interval.end());
	else if(animSettings->time() < interval.start())
		animSettings->setTime(interval.start());

	animSettings->clearNamedFrames();
	for(int animFrame = animSettings->timeToFrame(interval.start()); animFrame <= animSettings->timeToFrame(interval.end()); animFrame++) {
		int inputFrame = animationTimeToInputFrame(animSettings->frameToTime(animFrame));
		if(inputFrame >= 0 && inputFrame < _frames.size() && !_frames[inputFrame].label.isEmpty())
			animSettings->assignFrameName(animFrame, _frames[inputFrame].label);
	}
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void LinkedFileObject::saveToStream(ObjectSaveStream& stream)
{
	SceneObject::saveToStream(stream);
	stream.beginChunk(0x01);
	stream << _frames;
	if(saveWithScene())
		stream << _loadedFrame;
	else
		stream << -1;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void LinkedFileObject::loadFromStream(ObjectLoadStream& stream)
{
	SceneObject::loadFromStream(stream);
	stream.expectChunk(0x01);
	stream >> _frames;
	stream >> _loadedFrame;
	stream.closeChunk();
}

/******************************************************************************
* Returns the title of this object.
******************************************************************************/
QString LinkedFileObject::objectTitle()
{
	QString filename;
	int frameIndex = loadedFrame();
	if(frameIndex >= 0) {
		filename = QFileInfo(frames()[frameIndex].sourceFile.path()).fileName();
	}
	else if(!sourceUrl().isEmpty()) {
		filename = QFileInfo(sourceUrl().path()).fileName();
	}
	if(importer())
		return QString("%2 [%1]").arg(importer()->objectTitle()).arg(filename);
	return SceneObject::objectTitle();
}

/******************************************************************************
* Returns the number of sub-objects that should be displayed in the modifier stack.
******************************************************************************/
int LinkedFileObject::editableSubObjectCount()
{
	return sceneObjects().size();
}

/******************************************************************************
* Returns a sub-object that should be listed in the modifier stack.
******************************************************************************/
RefTarget* LinkedFileObject::editableSubObject(int index)
{
	return sceneObjects()[index];
}

/******************************************************************************
* Is called when a RefTarget has been added to a VectorReferenceField of this RefMaker.
******************************************************************************/
void LinkedFileObject::referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex)
{
	if(field == PROPERTY_FIELD(LinkedFileObject::_sceneObjects))
		notifyDependents(ReferenceEvent::SubobjectListChanged);

	SceneObject::referenceInserted(field, newTarget, listIndex);
}

/******************************************************************************
* Is called when a RefTarget has been added to a VectorReferenceField of this RefMaker.
******************************************************************************/
void LinkedFileObject::referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex)
{
	if(field == PROPERTY_FIELD(LinkedFileObject::_sceneObjects))
		notifyDependents(ReferenceEvent::SubobjectListChanged);

	SceneObject::referenceRemoved(field, newTarget, listIndex);
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void LinkedFileObject::propertyChanged(const PropertyFieldDescriptor& field)
{
	if(field == PROPERTY_FIELD(LinkedFileObject::_adjustAnimationIntervalEnabled) ||
			field == PROPERTY_FIELD(LinkedFileObject::_playbackSpeedNumerator) ||
			field == PROPERTY_FIELD(LinkedFileObject::_playbackSpeedDenominator) ||
			field == PROPERTY_FIELD(LinkedFileObject::_playbackStartTime)) {
		adjustAnimationInterval();
	}
	SceneObject::propertyChanged(field);
}

/******************************************************************************
* Displays the file selection dialog and lets the user select a new input file.
******************************************************************************/
void LinkedFileObject::showFileSelectionDialog(QWidget* parent)
{
	try {
		QUrl newSourceUrl;
		const FileImporterDescription* importerType;

		// Put code in a block: Need to release dialog before loading new input file.
		{
			// Let the user select a file.
			ImportFileDialog dialog(parent, tr("Pick input file"));
			if(sourceUrl().isLocalFile())
				dialog.selectFile(sourceUrl().toLocalFile());
			if(dialog.exec() != QDialog::Accepted)
				return;

			newSourceUrl = QUrl::fromLocalFile(dialog.fileToImport());
			importerType = dialog.selectedFileImporter();
		}

		// Set the new input location.
		setSource(newSourceUrl, importerType);
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Displays the file selection dialog and lets the user select a new input file.
******************************************************************************/
void LinkedFileObject::showURLSelectionDialog(QWidget* parent)
{
	try {
		QUrl newSourceUrl;
		const FileImporterDescription* importerType;

		// Put code in a block: Need to release dialog before loading new input file.
		{
			// Let the user select a new URL.
			ImportRemoteFileDialog dialog(parent, tr("Pick source"));
			dialog.selectFile(sourceUrl());
			if(dialog.exec() != QDialog::Accepted)
				return;

			newSourceUrl = dialog.fileToImport();
			importerType = dialog.selectedFileImporter();
		}

		// Set the new input location.
		setSource(newSourceUrl, importerType);
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

};
