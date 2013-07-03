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
#include <core/utilities/concurrent/Task.h>
#include <core/utilities/concurrent/ProgressManager.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/scene/ObjectNode.h>
#include <core/gui/dialogs/ImportFileDialog.h>
#include <core/dataset/importexport/ImportExportManager.h>
#include "LinkedFileObject.h"
#include "LinkedFileObjectEditor.h"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LinkedFileObject, SceneObject)
SET_OVITO_OBJECT_EDITOR(LinkedFileObject, LinkedFileObjectEditor)
DEFINE_FLAGS_REFERENCE_FIELD(LinkedFileObject, _importer, "Importer", LinkedFileImporter, PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(LinkedFileObject, _sceneObjects, "SceneObjects", SceneObject, PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_PROPERTY_FIELD(LinkedFileObject, _adjustAnimationIntervalEnabled, "AdjustAnimationIntervalEnabled")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _importer, "File Importer")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _sceneObjects, "Objects")
SET_PROPERTY_FIELD_LABEL(LinkedFileObject, _adjustAnimationIntervalEnabled, "Adjust animation interval")

/******************************************************************************
* Constructs the object.
******************************************************************************/
LinkedFileObject::LinkedFileObject() : _adjustAnimationIntervalEnabled(true), _loadedFrame(-1), _frameBeingLoaded(-1)
{
	INIT_PROPERTY_FIELD(LinkedFileObject::_importer);
	INIT_PROPERTY_FIELD(LinkedFileObject::_sceneObjects);
	INIT_PROPERTY_FIELD(LinkedFileObject::_adjustAnimationIntervalEnabled);

	connect(&_loadFrameOperationWatcher, &FutureWatcher::finished, this, &LinkedFileObject::loadOperationFinished);
}

/******************************************************************************
* Scans the input source for animation frames and updates the internal list of frames.
******************************************************************************/
bool LinkedFileObject::updateFrames()
{
	if(!importer())
		return false;

	Future<QVector<LinkedFileImporter::FrameSourceInformation>> framesFuture = importer()->findFrames();
	if(!ProgressManager::instance().waitForTask(framesFuture))
		return false;

	_frames = framesFuture.result();

	return true;
}

/******************************************************************************
* Asks the object for the result of the geometry pipeline at the given time.
******************************************************************************/
PipelineFlowState LinkedFileObject::evaluate(TimePoint time)
{
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
				_loadFrameOperation.cancel();
				_loadFrameOperation.waitForFinished();
			} catch(...) {}
			// This will suppress any pending notification events.
			_loadFrameOperationWatcher.unsetFuture();
			_frameBeingLoaded = -1;
			// Inform previous caller that the existing loading operation has been canceled.
			oldTaskCanceled = true;
		}
	}
	if(_loadedFrame == frame) {
		if(oldTaskCanceled)
			notifyDependents(ReferenceEvent::PendingOperationFailed);

		// The requested frame has already been loaded and is available immediately.
		return PipelineFlowState(status(), _sceneObjects.targets(), TimeInterval(time));
	}
	else {
		// The requested frame needs to be loaded first. Start background loading task.
		OVITO_CHECK_OBJECT_POINTER(importer());
		if(frame < 0 || frame >= numberOfFrames()) {
			if(oldTaskCanceled) {
				notifyDependents(ReferenceEvent::PendingOperationFailed);
			}
			setStatus(ObjectStatus(ObjectStatus::Error, tr("The requested animation frame %1 is out of range for source location %2").arg(frame).arg(importer()->sourceUrl().toString())));
			return PipelineFlowState(status(), _sceneObjects.targets(), TimeInterval(time));
		}
		_frameBeingLoaded = frame;
		_loadFrameOperation = importer()->load(_frames[frame]);
		_loadFrameOperationWatcher.setFuture(_loadFrameOperation);
		if(oldTaskCanceled) {
			notifyDependents(ReferenceEvent::PendingOperationFailed);
		}
		setStatus(ObjectStatus::Pending);
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
	ReferenceEvent::Type notificationType = ReferenceEvent::PendingOperationFailed;
	bool wasCanceled = _loadFrameOperation.isCanceled();
	_loadedFrame = _frameBeingLoaded;
	_frameBeingLoaded = -1;
	ObjectStatus newStatus = status();

	if(!wasCanceled) {
		try {
			// Adopt the data loaded by the importer.
			LinkedFileImporter::ImportedDataPtr importedData = _loadFrameOperation.result();
			if(importedData)
				importedData->insertIntoScene(this);

			newStatus = ObjectStatus(ObjectStatus::Success);

			// Notify dependents that the loading operation has succeeded and the new data is available.
			notificationType = ReferenceEvent::PendingOperationSucceeded;
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
	_loadFrameOperation = Future<LinkedFileImporter::ImportedDataPtr>();
	_loadFrameOperationWatcher.unsetFuture();

	// Set the new object status.
	setStatus(newStatus);

	// Notify dependents that the evaluation request was satisfied or not satisfied.
	notifyDependents(notificationType);
}

#if 0
/******************************************************************************
* Asks the object for the result of the geometry pipeline at the given time.
******************************************************************************/
PipelineFlowState LinkedFileObject::evalObject(TimePoint time)
{
	TimeInterval interval = TimeForever;
	if(!atomsObject() || !parser() || parser()->numberOfMovieFrames() <= 0) return PipelineFlowState(NULL, interval);

	int frame = ANIM_MANAGER.timeToFrame(time);
	int snapshot = frame / framesPerSnapshot();

	if(snapshot < 0) snapshot = 0;
	else if(snapshot >= parser()->numberOfMovieFrames()) snapshot = parser()->numberOfMovieFrames() - 1;
	frame = snapshot * framesPerSnapshot();

	if(snapshot != _loadedMovieFrame) {
		try {
			// Do not record this operation.
			UndoSuspender undoSuspender;
			// Do not create any animation keys.
			AnimationSuspender animSuspender;

			// Call the format specific parser.
			_loadedMovieFrame = snapshot;
			setStatus(parser()->loadAtomsFile(atomsObject(), snapshot, true));
		}
		catch(Exception& ex) {
			// Transfer exception message to evaluation status.
			QString msg = ex.message();
			for(int i=1; i<ex.messages().size(); i++) {
				msg += "\n";
				msg += ex.messages()[i];
			}
			setStatus(EvaluationStatus(EvaluationStatus::EVALUATION_ERROR, msg));

			ex.prependGeneralMessage(tr("Failed to load snapshot %1 of sequence.").arg(snapshot));
			ex.logError();
		}
	}

	// Calculate the validity interval of the current simulation snapshot.
	interval.intersect(atomsObject()->objectValidity(time));

	if(snapshot > 0) interval.setStart(max(interval.start(), ANIM_MANAGER.frameToTime(frame)));
	if(snapshot < parser()->numberOfMovieFrames() - 1) interval.setEnd(min(interval.end(), ANIM_MANAGER.frameToTime(frame+1)-1));

	return PipelineFlowState(atomsObject(), interval);
}

/******************************************************************************
* Sets the parser used by this object.
******************************************************************************/
void AtomsImportObject::setParser(AtomsFileParser* parser)
{
	this->_parser = parser;
}
#endif

/******************************************************************************
* This will reload an animation frame.
******************************************************************************/
void LinkedFileObject::refreshFromSource(int frame)
{
	if(!importer())
		return;

	if(frame == loadedFrame() || frame == -1) {
		_loadedFrame = -1;
		notifyDependents(ReferenceEvent::TargetChanged);
	}
}

/******************************************************************************
* Saves the status returned by the parser object and generates a
* ReferenceEvent::StatusChanged event.
******************************************************************************/
void LinkedFileObject::setStatus(const ObjectStatus& status)
{
	if(status == _importStatus) return;
	_importStatus = status;
	notifyDependents(ReferenceEvent::StatusChanged);
}

/******************************************************************************
* Adjusts the animation interval of the current data set to the number of
* frames reported by the file parser.
******************************************************************************/
void LinkedFileObject::adjustAnimationInterval()
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
		QString filename;
		if(_frames[frameIndex].sourceFile.isLocalFile()) {
			QFileInfo fileInfo(_frames[frameIndex].sourceFile.toLocalFile());
			filename = fileInfo.fileName();
		}
		else {
			QFileInfo fileInfo(_frames[frameIndex].sourceFile.fragment());
			filename = fileInfo.fileName();
		}
		animSettings->assignFrameName(frameIndex, filename);
	}

	if(numberOfFrames() > 1) {
		TimeInterval interval(0, (numberOfFrames()-1) * animSettings->ticksPerFrame());
		animSettings->setAnimationInterval(interval);
	}
	else {
		animSettings->setAnimationInterval(0);
		animSettings->setTime(0);
	}
}


#if 0

/******************************************************************************
* Asks the object for its validity interval at the given time.
******************************************************************************/
TimeInterval AtomsImportObject::objectValidity(TimeTicks time)
{
	return TimeForever;
}

#endif

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void LinkedFileObject::saveToStream(ObjectSaveStream& stream)
{
	SceneObject::saveToStream(stream);
	stream.beginChunk(0x01);
	stream << _frames;
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
	stream.closeChunk();
}

#if 0

/******************************************************************************
* This method is called once for this object after it has been loaded from the input stream
******************************************************************************/
void AtomsImportObject::loadFromStreamComplete()
{
	SceneObject::loadFromStreamComplete();

	CHECK_POINTER(atomsObject());
	if(!storeAtomsWithScene() && atomsObject() && parser()) {
		// Load atomic data from external file.
		try {
			reloadInputFile();
		}
		catch(Exception& ex) {
			ex.prependGeneralMessage(tr("Failed to restore atom data from external file. Sorry, your atoms are gone. Non-existing external data file: %1").arg(inputFile()));
			ex.showError();
		}
	}
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
RefTarget::SmartPtr AtomsImportObject::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	AtomsImportObject::SmartPtr clone = static_object_cast<AtomsImportObject>(SceneObject::clone(deepCopy, cloneHelper));

	// Copy internal data.
	clone->_loadedMovieFrame = this->_loadedMovieFrame;

	return clone;
}

/******************************************************************************
* From RefMaker.
* This method is called when an object referenced by this object
* sends a notification message.
******************************************************************************/
bool AtomsImportObject::onRefTargetMessage(RefTarget* source, RefTargetMessage* msg)
{
	// Generate SUBOBJECT_LIST_CHANGED message if a data channel is added or removed from our AtomsObject
	// since we replicate its list of sub-objects.
	if((msg->type() == REFERENCE_FIELD_ADDED || msg->type() == REFERENCE_FIELD_REMOVED || msg->type() == REFERENCE_FIELD_CHANGED) &&
			msg->sender() == atomsObject()) {
		notifyDependents(SUBOBJECT_LIST_CHANGED);
	}
	return SceneObject::onRefTargetMessage(source, msg);
}

#endif

/******************************************************************************
* Returns the title of this object.
******************************************************************************/
QString LinkedFileObject::objectTitle()
{
	if(!importer()) return SceneObject::objectTitle();
	return importer()->objectTitle();
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


#if 0

/******************************************************************************
* Displays the file selection dialog and lets the user select a new input file.
******************************************************************************/
void AtomsImportObject::showSelectionDialog(QWidget* parent)
{
	try {
		ImporterExporter::SmartPtr importer;
		QString importFile;

		// Put code in a block: Need to release dialog before loading new input file.
		{
			// Let the user select a file.
			ImportFileDialog dialog(parent, tr("Import"));
			if(!dialog.exec())
				return;

			// Create a parser object based on the selected filename filter.
			importFile = dialog.fileToImport();
			importer = dialog.createParser();
			if(!importer) return;
		}
		AtomsFileParser::SmartPtr newParser = dynamic_object_cast<AtomsFileParser>(importer);
		if(!newParser)
			throw Exception(tr("You did not select a file that contains an atomistic dataset."));

		// Try to re-use the existing parser.
		if(parser() && parser()->pluginClassDescriptor() == newParser->pluginClassDescriptor())
			newParser = parser();
		CHECK_OBJECT_POINTER(newParser.get());

		ViewportSuspender noVPUpdate;

		// Scan the input file.
		if(!newParser->setInputFile(importFile))
			return;

		// Show settings dialog.
		if(!newParser->showSettingsDialog(parent))
			return;

		setParser(newParser.get());
		reloadInputFile();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}
#endif

};
