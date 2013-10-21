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
#include <core/gui/undo/UndoManager.h>
#include <core/animation/AnimManager.h>
#include <core/utilities/io/ObjectLoadStream.h>
#include <core/utilities/io/ObjectSaveStream.h>
#include <core/utilities/io/FileManager.h>
#include <core/utilities/concurrent/Task.h>
#include <core/utilities/concurrent/ProgressManager.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/scene/ObjectNode.h>
#include <core/gui/dialogs/ImportFileDialog.h>
#include <core/gui/dialogs/ImportRemoteFileDialog.h>
#include <core/dataset/importexport/ImportExportManager.h>
#include <core/dataset/DataSetManager.h>
#include <core/gui/actions/ActionManager.h>
#include "LinkedFileObject.h"
#include "LinkedFileObjectEditor.h"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LinkedFileObject, SceneObject)
SET_OVITO_OBJECT_EDITOR(LinkedFileObject, LinkedFileObjectEditor)
DEFINE_FLAGS_REFERENCE_FIELD(LinkedFileObject, _importer, "Importer", LinkedFileImporter, PROPERTY_FIELD_ALWAYS_DEEP_COPY|PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(LinkedFileObject, _sceneObjects, "SceneObjects", SceneObject, PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_PROPERTY_FIELD(LinkedFileObject, _adjustAnimationIntervalEnabled, "AdjustAnimationIntervalEnabled")
DEFINE_FLAGS_PROPERTY_FIELD(LinkedFileObject, _sourceUrl, "SourceUrl", PROPERTY_FIELD_NO_UNDO)
DEFINE_PROPERTY_FIELD(LinkedFileObject, _saveDataWithScene, "SaveDataWithScene")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _importer, "File Importer")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _sceneObjects, "Objects")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _adjustAnimationIntervalEnabled, "Adjust animation interval")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _sourceUrl, "Source location")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _saveDataWithScene, "Save data with scene")

/******************************************************************************
* Constructs the object.
******************************************************************************/
LinkedFileObject::LinkedFileObject() :
	_adjustAnimationIntervalEnabled(true), _loadedFrame(-1), _frameBeingLoaded(-1),
	_saveDataWithScene(false)
{
	INIT_PROPERTY_FIELD(LinkedFileObject::_importer);
	INIT_PROPERTY_FIELD(LinkedFileObject::_sceneObjects);
	INIT_PROPERTY_FIELD(LinkedFileObject::_adjustAnimationIntervalEnabled);
	INIT_PROPERTY_FIELD(LinkedFileObject::_sourceUrl);
	INIT_PROPERTY_FIELD(LinkedFileObject::_saveDataWithScene);

	connect(&_loadFrameOperationWatcher, &FutureWatcher::finished, this, &LinkedFileObject::loadOperationFinished);
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
		Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(newSourceUrl);
		if(!ProgressManager::instance().waitForTask(fetchFileFuture))
			return false;

		// Detect file format.
		fileimporter = ImportExportManager::instance().autodetectFileFormat(fetchFileFuture.result(), newSourceUrl.path());
		if(!fileimporter)
			throw Exception(tr("Could not detect the format of the file to be imported. The format might not be supported."));
	}
	else {
		fileimporter = importerType->createService();
		if(!fileimporter)
			return false;
	}
	OORef<LinkedFileImporter> newImporter = dynamic_object_cast<LinkedFileImporter>(fileimporter);
	if(!newImporter)
		throw Exception(tr("You did not select a compatible file."));

	ViewportSuspender noVPUpdate;

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

	// If URL is not already a wildcard pattern, generate a default pattern by
	// replacing last sequence of numbers in the filename with a wildcard character.
	QFileInfo fileInfo(sourceUrl.path());
	QString originalFilename = fileInfo.fileName();
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

	// Make the import process reversible.
	UndoableTransaction transaction(tr("Set input file"));

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
	if(UndoManager::instance().isRecording())
		UndoManager::instance().push(new SetSourceOperation(this));

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
			DataSetManager::instance().runWhenSceneIsReady([]() {
				ActionManager::instance().getAction(ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS_ALL)->trigger();
			});
		}

		// Cancel any old load operation in progress.
		cancelLoadOperation();

		transaction.commit();
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
	if(!importer())
		return false;

	Future<QVector<LinkedFileImporter::FrameSourceInformation>> framesFuture = importer()->findFrames(sourceUrl());
	if(!ProgressManager::instance().waitForTask(framesFuture))
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
* Asks the object for the result of the geometry pipeline at the given time.
******************************************************************************/
PipelineFlowState LinkedFileObject::evaluate(TimePoint time)
{
	if(!importer())
		return PipelineFlowState();

	int frame = AnimManager::instance().timeToFrame(time);

	bool oldTaskCanceled = false;
	if(_frameBeingLoaded != -1) {
		if(_frameBeingLoaded == frame) {
			// The requested frame is already being loaded at the moment. Indicate to the caller that the result is pending.
			return PipelineFlowState(ObjectStatus::Pending, _sceneObjects.targets(), TimeInterval(time));
		}
		else {
			// Another frame than the requested one is already being loaded. Cancel loading operation now.
			try {
				// This will suppress any pending notification events.
				_loadFrameOperationWatcher.unsetFuture();
				_loadFrameOperation.cancel();
				_loadFrameOperation.waitForFinished();
			} catch(...) {}
			_frameBeingLoaded = -1;
			// Inform previous caller that the existing loading operation has been canceled.
			oldTaskCanceled = true;
		}
	}
	if(_loadedFrame == frame) {
		if(oldTaskCanceled)
			notifyDependents(ReferenceEvent::PendingStateChanged);

		// The requested frame has already been loaded and is available immediately.
		setStatus(ObjectStatus::Success);
		return PipelineFlowState(status(), _sceneObjects.targets(), TimeInterval(time));
	}
	else {
		// The requested frame needs to be loaded first. Start background loading task.
		OVITO_CHECK_OBJECT_POINTER(importer());
		if(frame < 0 || frame >= numberOfFrames()) {
			if(oldTaskCanceled)
				notifyDependents(ReferenceEvent::PendingStateChanged);
			if(numberOfFrames() > 0)
				setStatus(ObjectStatus(ObjectStatus::Error, tr("The requested animation frame (%1) is out of range.").arg(frame)));
			else
				setStatus(ObjectStatus(ObjectStatus::Error, tr("The source location is empty (no files found).")));
			return PipelineFlowState(status(), _sceneObjects.targets(), TimeInterval(time));
		}
		_frameBeingLoaded = frame;
		_loadFrameOperation = importer()->load(_frames[frame]);
		_loadFrameOperationWatcher.setFuture(_loadFrameOperation);
		setStatus(ObjectStatus::Pending);
		if(oldTaskCanceled)
			notifyDependents(ReferenceEvent::PendingStateChanged);
		// Indicate to the caller that the result is pending.
		return PipelineFlowState(ObjectStatus::Pending, _sceneObjects.targets(), TimeInterval(time));
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
			if(importedData) {
				importedData->insertIntoScene(this);
				newStatus = importedData->status();
			}
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

	// Notify dependents that the evaluation request was satisfied or not satisfied.
	notifyDependents(ReferenceEvent::PendingStateChanged);
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

	QSet<RefMaker*> datasets = findDependents(DataSet::OOType);
	if(datasets.empty())
		return;

	DataSet* dataset = static_object_cast<DataSet>(*datasets.cbegin());
	AnimationSettings* animSettings = dataset->animationSettings();

	animSettings->clearNamedFrames();
	for(int frameIndex = 0; frameIndex < _frames.size(); frameIndex++) {
		if(!_frames[frameIndex].label.isEmpty())
			animSettings->assignFrameName(frameIndex, _frames[frameIndex].label);
	}

	if(numberOfFrames() > 1) {
		TimeInterval interval(0, (numberOfFrames()-1) * animSettings->ticksPerFrame());
		animSettings->setAnimationInterval(interval);
		if(gotoFrameIndex >= 0 && gotoFrameIndex < numberOfFrames()) {
			animSettings->setTime(gotoFrameIndex * animSettings->ticksPerFrame());
		}
		else if(animSettings->time() > interval.end())
			animSettings->setTime(interval.end());
	}
	else {
		animSettings->setAnimationInterval(0);
		animSettings->setTime(0);
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
	if(saveDataWithScene())
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
