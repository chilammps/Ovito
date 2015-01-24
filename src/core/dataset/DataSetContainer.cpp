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
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/dataset/UndoStack.h>
#include <core/animation/AnimationSettings.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/SelectionSet.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/rendering/RenderSettings.h>
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/utilities/io/ObjectSaveStream.h>
#include <core/utilities/io/ObjectLoadStream.h>
#include <core/utilities/io/FileManager.h>

#ifdef Q_OS_UNIX
	#include <signal.h>
#endif

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

IMPLEMENT_OVITO_OBJECT(Core, DataSetContainer, RefMaker);
DEFINE_FLAGS_REFERENCE_FIELD(DataSetContainer, _currentSet, "CurrentSet", DataSet, PROPERTY_FIELD_NO_UNDO);

/******************************************************************************
* Initializes the dataset manager.
******************************************************************************/
DataSetContainer::DataSetContainer(MainWindow* mainWindow) : RefMaker(nullptr),
	_mainWindow(mainWindow), _taskManager(mainWindow)
{
	INIT_PROPERTY_FIELD(DataSetContainer::_currentSet);
}

/******************************************************************************
* Is called when the value of a reference field of this RefMaker changes.
******************************************************************************/
void DataSetContainer::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(DataSetContainer::_currentSet)) {

		if(oldTarget) {
			DataSet* oldDataSet = static_object_cast<DataSet>(oldTarget);

			// Stop animation playback for the old dataset.
			oldDataSet->animationSettings()->stopAnimationPlayback();
		}

		// Forward signals from the current dataset.
		disconnect(_selectionSetReplacedConnection);
		disconnect(_viewportConfigReplacedConnection);
		disconnect(_animationSettingsReplacedConnection);
		disconnect(_renderSettingsReplacedConnection);
		if(currentSet()) {
			_selectionSetReplacedConnection = connect(currentSet(), &DataSet::selectionSetReplaced, this, &DataSetContainer::onSelectionSetReplaced);
			_viewportConfigReplacedConnection = connect(currentSet(), &DataSet::viewportConfigReplaced, this, &DataSetContainer::viewportConfigReplaced);
			_animationSettingsReplacedConnection = connect(currentSet(), &DataSet::animationSettingsReplaced, this, &DataSetContainer::animationSettingsReplaced);
			_renderSettingsReplacedConnection = connect(currentSet(), &DataSet::renderSettingsReplaced, this, &DataSetContainer::renderSettingsReplaced);
			Q_EMIT viewportConfigReplaced(currentSet()->viewportConfig());
			Q_EMIT animationSettingsReplaced(currentSet()->animationSettings());
			Q_EMIT renderSettingsReplaced(currentSet()->renderSettings());
			onSelectionSetReplaced(currentSet()->selection());
			onAnimationSettingsReplaced(currentSet()->animationSettings());
		}
		else {
			onSelectionSetReplaced(nullptr);
			onAnimationSettingsReplaced(nullptr);
			Q_EMIT viewportConfigReplaced(nullptr);
			Q_EMIT animationSettingsReplaced(nullptr);
			Q_EMIT renderSettingsReplaced(nullptr);
		}

		Q_EMIT dataSetChanged(currentSet());
	}
	RefMaker::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* This handler is invoked when the current selection set of the current dataset
* has been replaced.
******************************************************************************/
void DataSetContainer::onSelectionSetReplaced(SelectionSet* newSelectionSet)
{
	// Forward signals from the current selection set.
	disconnect(_selectionSetChangedConnection);
	disconnect(_selectionSetChangeCompleteConnection);
	if(newSelectionSet) {
		_selectionSetChangedConnection = connect(newSelectionSet, &SelectionSet::selectionChanged, this, &DataSetContainer::selectionChanged);
		_selectionSetChangeCompleteConnection = connect(newSelectionSet, &SelectionSet::selectionChangeComplete, this, &DataSetContainer::selectionChangeComplete);
	}
	Q_EMIT selectionSetReplaced(newSelectionSet);
	Q_EMIT selectionChanged(newSelectionSet);
	Q_EMIT selectionChangeComplete(newSelectionSet);
}

/******************************************************************************
* This handler is invoked when the current animation settings of the current
* dataset have been replaced.
******************************************************************************/
void DataSetContainer::onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings)
{
	// Forward signals from the current animation settings object.
	disconnect(_animationTimeChangedConnection);
	disconnect(_animationTimeChangeCompleteConnection);
	if(newAnimationSettings) {
		_animationTimeChangedConnection = connect(newAnimationSettings, &AnimationSettings::timeChanged, this, &DataSetContainer::timeChanged);
		_animationTimeChangeCompleteConnection = connect(newAnimationSettings, &AnimationSettings::timeChangeComplete, this, &DataSetContainer::timeChangeComplete);
	}
	if(newAnimationSettings) {
		Q_EMIT timeChanged(newAnimationSettings->time());
		Q_EMIT timeChangeComplete();
	}
}

/******************************************************************************
* Save the current dataset.
******************************************************************************/
bool DataSetContainer::fileSave()
{
	if(currentSet() == nullptr)
		return false;

	// Ask the user for a filename if there is no one set.
	if(currentSet()->filePath().isEmpty())
		return fileSaveAs();

	// Save dataset to file.
	try {
		currentSet()->saveToFile(currentSet()->filePath());
		currentSet()->undoStack().setClean();
	}
	catch(const Exception& ex) {
		ex.showError();
		return false;
	}

	return true;
}

/******************************************************************************
* This is the implementation of the "Save As" action.
* Returns true, if the scene has been saved.
******************************************************************************/
bool DataSetContainer::fileSaveAs(const QString& filename)
{
	if(currentSet() == nullptr)
		return false;

	if(filename.isEmpty()) {

		if(!mainWindow())
			throw Exception(tr("Cannot save scene. No filename has been set."));

		QFileDialog dialog(mainWindow(), tr("Save Scene As"));
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
bool DataSetContainer::askForSaveChanges()
{
	if(!currentSet() || currentSet()->undoStack().isClean() || currentSet()->filePath().isEmpty() || !mainWindow())
		return true;

	QMessageBox::StandardButton result = QMessageBox::question(mainWindow(), tr("Save changes"),
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
* Creates an empty dataset and makes it the current dataset.
******************************************************************************/
bool DataSetContainer::fileNew()
{
	OORef<DataSet> newSet = new DataSet();
	newSet->loadUserDefaults();
	setCurrentSet(newSet);
	return true;
}

/******************************************************************************
* Loads the given scene file.
******************************************************************************/
bool DataSetContainer::fileLoad(const QString& filename)
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
			if(mainWindow()) {
				QString msg = tr("The scene file has been written with a version of this application that uses %1-bit floating-point precision. "
					   "The version of this application that you are using at this moment only supports %2-bit precision numbers. "
					   "The precision of all numbers stored in the input file will be truncated during loading.").arg(stream.floatingPointPrecision()*8).arg(sizeof(FloatType)*8);
				QMessageBox::warning(mainWindow(), tr("Floating-point precision mismatch"), msg);
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
bool DataSetContainer::importFile(const QUrl& url, const OvitoObjectType* importerType, FileImporter::ImportMode importMode)
{
	if(!url.isValid())
		throw Exception(tr("Failed to import file. URL is not valid: %1").arg(url.toString()));

	OORef<FileImporter> importer;
	if(!importerType) {

		// Download file so we can determine its format.
		Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(*this, url);
		if(!taskManager().waitForTask(fetchFileFuture))
			return false;

		// Detect file format.
		importer = FileImporter::autodetectFileFormat(currentSet(), fetchFileFuture.result(), url.path());
		if(!importer)
			throw Exception(tr("Could not detect the format of the file to be imported. The format might not be supported."));
	}
	else {
		importer = static_object_cast<FileImporter>(importerType->createInstance(currentSet()));
		if(!importer)
			throw Exception(tr("Failed to import file. Could not initialize import service."));
	}

	// Load user-defined default settings for the importer.
	importer->loadUserDefaults();

	return importer->importFile(url, importMode);
}

// Boolean flag which is set by the POSIX signal handler when user
// presses Ctrl+C to interrupt the program. In console mode, the
// DataSetContainer::waitUntil() function breaks out of the waiting loop
// when this flag is set.
static QAtomicInt _userInterrupt;

/******************************************************************************
* This function blocks execution until some operation has been completed.
******************************************************************************/
bool DataSetContainer::waitUntil(const std::function<bool()>& callback, const QString& message, QProgressDialog* progressDialog)
{
	OVITO_ASSERT_MSG(QThread::currentThread() == QApplication::instance()->thread(), "DataSetContainer::waitUntilReady", "This function may only be called from the GUI thread.");

	// Check if operation is already completed.
	if(callback())
		return true;

	// Suspend viewport updates while waiting.
	ViewportSuspender viewportSuspender(currentSet());

	// Check if viewports are currently being rendered.
	// If yes, it's not a good idea to display a progress dialog.
	bool isRendering = currentSet() ? currentSet()->viewportConfig()->isRendering() : false;

	if(!isRendering && Application::instance().guiMode()) {

		// Show a modal progress dialog to block user interface while waiting.
		std::unique_ptr<QProgressDialog> localDialog;
		if(!progressDialog) {
			localDialog.reset(new QProgressDialog(mainWindow()));
			progressDialog = localDialog.get();
			progressDialog->setWindowModality(Qt::WindowModal);
			progressDialog->setAutoClose(false);
			progressDialog->setAutoReset(false);
			progressDialog->setMinimumDuration(0);
			progressDialog->setValue(0);
		}
		progressDialog->setLabelText(message);

		// Poll callback function until it returns true.
		while(!callback()) {
			if(progressDialog->wasCanceled())
				return false;
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 50);
		}
	}
	else {
#ifdef Q_OS_UNIX
		// Install POSIX signal handler to catch Ctrl+C key press in console mode.
		auto oldSignalHandler = ::signal(SIGINT, [](int) { _userInterrupt.storeRelease(1); });
		try {
#endif

		// Poll callback function until it returns true.
		while(!callback() && !_userInterrupt.loadAcquire()) {
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 20);
		}

#ifdef Q_OS_UNIX
		::signal(SIGINT, oldSignalHandler);
		}
		catch(...) {
			::signal(SIGINT, oldSignalHandler);
		}
#endif
		if(_userInterrupt.load()) {
			taskManager().cancelAll();
			return false;
		}
	}

	return true;
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
