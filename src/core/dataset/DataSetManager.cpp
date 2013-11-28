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
#include <core/dataset/DataSetManager.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/dataset/importexport/ImportExportManager.h>
#include <core/viewport/ViewportManager.h>
#include <core/gui/undo/UndoManager.h>
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/animation/AnimManager.h>
#include <core/utilities/io/ObjectSaveStream.h>
#include <core/utilities/io/ObjectLoadStream.h>
#include <core/utilities/io/FileManager.h>
#include <core/utilities/concurrent/ProgressManager.h>

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, DataSetManager, RefMaker)
DEFINE_FLAGS_REFERENCE_FIELD(DataSetManager, _currentSet, "CurrentSet", DataSet, PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_REFERENCE_FIELD(DataSetManager, _selectionSetProxy, "SelectionSetProxy", CurrentSelectionProxy, PROPERTY_FIELD_NO_UNDO)

/// The singleton instance of the class.
DataSetManager* DataSetManager::_instance = nullptr;

/******************************************************************************
* Initializes the dataset manager.
******************************************************************************/
DataSetManager::DataSetManager()
{
	OVITO_ASSERT_MSG(!_instance, "DataSetManager constructor", "Multiple instances of this singleton class have been created.");
	INIT_PROPERTY_FIELD(DataSetManager::_currentSet);
	INIT_PROPERTY_FIELD(DataSetManager::_selectionSetProxy);

	// Create internal selection proxy object.
	_selectionSetProxy = new CurrentSelectionProxy();

	// Reset the undo stack when a new scene has been loaded.
	connect(this, SIGNAL(dataSetReset(DataSet*)), &UndoManager::instance(), SLOT(clear()));
}

/******************************************************************************
* Sets the current data set being edited by the user.
******************************************************************************/
void DataSetManager::setCurrentSet(const OORef<DataSet>& set)
{
	OVITO_ASSERT_MSG(!UndoManager::instance().isRecording(), "DataSetManager::setCurrentSet", "The replacement of the current dataset cannot be undone.");

	// Do not record any operations while resetting the application.
	UndoSuspender noUndo;

	_currentSet = set;

	// Reset selection set
	_selectionSetProxy->setCurrentSelectionSet(set ? set->selection() : nullptr);

	// Inform listeners.
	dataSetReset(currentSet());

	// Update viewports to show the new scene.
	ViewportManager::instance().updateViewports();
}

/******************************************************************************
* Returns a viewport configuration that should be used as template for new scene files.
******************************************************************************/
OORef<ViewportConfiguration> DataSetManager::defaultViewportConfiguration()
{
	// Make sure the default configuration is initialized.
	if(!_defaultViewportConfig) {
		_defaultViewportConfig = new ViewportConfiguration();

		OORef<Viewport> topView = new Viewport();
		topView->setViewType(Viewport::VIEW_TOP);
		_defaultViewportConfig->addViewport(topView);

		OORef<Viewport> frontView = new Viewport();
		frontView->setViewType(Viewport::VIEW_FRONT);
		_defaultViewportConfig->addViewport(frontView);

		OORef<Viewport> leftView = new Viewport();
		leftView->setViewType(Viewport::VIEW_LEFT);
		_defaultViewportConfig->addViewport(leftView);

		OORef<Viewport> perspectiveView = new Viewport();
		perspectiveView->setViewType(Viewport::VIEW_PERSPECTIVE);
		perspectiveView->setCameraTransformation(ViewportSettings::getSettings().coordinateSystemOrientation() * AffineTransformation::lookAlong({90, -120, 100}, {-90, 120, -100}, {0,0,1}).inverse());
		_defaultViewportConfig->addViewport(perspectiveView);

		_defaultViewportConfig->setActiveViewport(topView.get());
		_defaultViewportConfig->setMaximizedViewport(NULL);
	}

	return _defaultViewportConfig;
}

/******************************************************************************
* Replaces the current data set with a new one and resets the
* application to its initial state.
******************************************************************************/
void DataSetManager::fileReset()
{
	setCurrentSet(new DataSet());
}

/******************************************************************************
* This is the implementation of the "Save" action.
* Returns true, if the scene has been saved.
******************************************************************************/
bool DataSetManager::fileSave()
{
	if(currentSet() == NULL)
		return false;

	// Ask the user for a filename if there is no one set.
	if(currentSet()->filePath().isEmpty())
		return fileSaveAs();

	// Save dataset to file.
	try {
		QFile fileStream(currentSet()->filePath());
	    if(!fileStream.open(QIODevice::WriteOnly))
			throw Exception(tr("Failed to open output file '%1' for writing.").arg(currentSet()->filePath()));

		QDataStream dataStream(&fileStream);
		ObjectSaveStream stream(dataStream);
		stream.saveObject(currentSet());
		stream.close();

		if(fileStream.error() != QFile::NoError)
			throw Exception(tr("Failed to write output file '%1'.").arg(currentSet()->filePath()));
		fileStream.close();
	}
	catch(const Exception& ex) {
		ex.showError();
		return false;
	}

	UndoManager::instance().setClean();

	return true;
}

/******************************************************************************
* This is the implementation of the "Save As" action.
* Returns true, if the scene has been saved.
******************************************************************************/
bool DataSetManager::fileSaveAs(const QString& filename)
{
	if(currentSet() == nullptr)
		return false;

	if(filename.isEmpty()) {
		if(Application::instance().guiMode() == false)
			throw Exception(tr("Cannot save scene. No filename has been set."));

		QFileDialog dialog(&MainWindow::instance(), tr("Save Scene As"));
		dialog.setNameFilter(tr("Scene Files (*.ovito);;All Files (*)"));
		dialog.setAcceptMode(QFileDialog::AcceptSave);
		dialog.setFileMode(QFileDialog::AnyFile);
		dialog.setConfirmOverwrite(true);
		dialog.setDefaultSuffix("ovito");

		QSettings settings;
		settings.beginGroup("file/scene");

		if(currentSet()->filePath().isEmpty()) {
			QString defaultPath = settings.value("last_directory").toString();
			if(!defaultPath.isEmpty())
				dialog.setDirectory(defaultPath);
		}
		else
			dialog.selectFile(currentSet()->filePath());

		if(!dialog.exec())
			return false;

		QStringList files = dialog.selectedFiles();
		if(files.isEmpty())
			return false;
		QString newFilename = files.front();

		// Remember directory for the next time...
		settings.setValue("last_directory", dialog.directory().absolutePath());

        currentSet()->setFilePath(newFilename);
	}
	else {
		currentSet()->setFilePath(filename);
	}
	return fileSave();
}

/******************************************************************************
* If the scene has been changed this will ask the user if he wants
* to save the changes.
* Returns false if the operation has been canceled by the user.
******************************************************************************/
bool DataSetManager::askForSaveChanges()
{
	if(!currentSet() || UndoManager::instance().isClean() || Application::instance().consoleMode())
		return true;

	QMessageBox::StandardButton result = QMessageBox::question(&MainWindow::instance(), tr("Save changes"),
		tr("The current scene has been modified. Do you want to save the changes?"),
		QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Cancel);
	if(result == QMessageBox::Cancel)
		return false; // Operation canceled by user
	else if(result == QMessageBox::No)
		return true; // Continue without saving scene first.
	else {
		// Save scene first.
        return fileSave();
	}
}

/******************************************************************************
* Loads the given scene file.
* Returns true if the file has been successfully loaded.
******************************************************************************/
bool DataSetManager::fileLoad(const QString& filename)
{
	// Load dataset from file.
	OORef<DataSet> dataSet;
	{
		QFile fileStream(filename);
		if(!fileStream.open(QIODevice::ReadOnly))
			throw Exception(tr("Failed to open file '%1' for reading.").arg(filename));

		QDataStream dataStream(&fileStream);
		ObjectLoadStream stream(dataStream);

		// Issue a warning when the floating-point precision of the input file does not match
		// the precision used in this build.
		if(stream.floatingPointPrecision() > sizeof(FloatType)) {
			if(Application::instance().guiMode()) {
				QString msg = tr("The scene file has been written with a version of this application that uses %1-bit floating-point precision. "
					   "The version of this application that you are using at this moment only supports %2-bit precision numbers. "
					   "The precision of all numbers stored in the input file will be truncated during loading.").arg(stream.floatingPointPrecision()*8).arg(sizeof(FloatType)*8);
				QMessageBox::warning(NULL, tr("Floating-point precision mismatch"), msg);
			}
		}

		dataSet = stream.loadObject<DataSet>();
		stream.close();
	}
	OVITO_CHECK_OBJECT_POINTER(dataSet);
	dataSet->setFilePath(filename);
	setCurrentSet(dataSet);
	return true;
}

/******************************************************************************
* Imports a given file into the scene.
******************************************************************************/
bool DataSetManager::importFile(const QUrl& url, const FileImporterDescription* importerType, FileImporter::ImportMode importMode)
{
	if(!url.isValid())
		throw Exception(tr("Failed to import file. URL is not valid: %1").arg(url.toString()));

	OORef<FileImporter> importer;
	if(!importerType) {

		// Download file so we can determine its format.
		Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(url);
		if(!ProgressManager::instance().waitForTask(fetchFileFuture))
			return false;

		// Detect file format.
		importer = ImportExportManager::instance().autodetectFileFormat(fetchFileFuture.result(), url.path());
		if(!importer)
			throw Exception(tr("Could not detect the format of the file to be imported. The format might not be supported."));
	}
	else {
		importer = importerType->createService();
		if(!importer)
			throw Exception(tr("Failed to import file. Could not initialize import service."));
	}

	return importer->importFile(url, importMode);
}

/******************************************************************************
* Checks all scene nodes if their geometry pipeline is fully evaluated at the
* given animation time.
******************************************************************************/
bool DataSetManager::isSceneReady(TimePoint time) const
{
	OVITO_ASSERT_MSG(QThread::currentThread() == QApplication::instance()->thread(), "DataSetManager::isSceneReady", "This function may only be called from the GUI thread.");
	OVITO_CHECK_OBJECT_POINTER(currentSet());
	OVITO_CHECK_OBJECT_POINTER(currentSet()->sceneRoot());

	// Iterate over all object nodes and request an evaluation of their geometry pipeline.
	bool isReady = currentSet()->sceneRoot()->visitObjectNodes([time](ObjectNode* node) {
		return (node->evalPipeline(time).status().type() != ObjectStatus::Pending);
	});

	return isReady;
}

/******************************************************************************
* Calls the given slot as soon as the geometry pipelines of all scene nodes has been
* completely evaluated.
******************************************************************************/
void DataSetManager::runWhenSceneIsReady(std::function<void ()> fn)
{
	OVITO_ASSERT_MSG(QThread::currentThread() == QApplication::instance()->thread(), "DataSetManager::runWhenSceneIsReady", "This function may only be called from the GUI thread.");
	OVITO_CHECK_OBJECT_POINTER(currentSet());
	OVITO_CHECK_OBJECT_POINTER(currentSet()->sceneRoot());

	TimePoint time = currentSet()->animationSettings()->time();

	// Iterate over all object nodes and request an evaluation of their geometry pipeline.
	bool isReady = currentSet()->sceneRoot()->visitObjectNodes([time](ObjectNode* node) {
		return (node->evalPipeline(time).status().type() != ObjectStatus::Pending);
	});

	if(isReady)
		fn();
	else
		_sceneReadyListeners.push_back(fn);
}

/******************************************************************************
* Checks if the scene is ready and calls all registered listeners.
******************************************************************************/
void DataSetManager::notifySceneReadyListeners()
{
	if(!_sceneReadyListeners.empty() && isSceneReady(currentSet()->animationSettings()->time())) {
		auto oldListenerList = _sceneReadyListeners;
		_sceneReadyListeners.clear();
		for(const auto& listener : oldListenerList) {
			listener();
		}
	}
}

/******************************************************************************
* Is called when a target referenced by this object generated an event.
******************************************************************************/
bool DataSetManager::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	OVITO_ASSERT_MSG(QThread::currentThread() == QApplication::instance()->thread(), "DataSetManager::referenceEvent", "Reference events may only be processed in the GUI thread.");

	if(source == currentSet() && event->type() == ReferenceEvent::PendingStateChanged) {
		notifySceneReadyListeners();
	}
	return RefMaker::referenceEvent(source, event);
}

};
