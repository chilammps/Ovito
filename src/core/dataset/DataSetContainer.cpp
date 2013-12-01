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
#include <core/dataset/importexport/ImportExportManager.h>
#include <core/dataset/UndoStack.h>
#include <core/scene/SceneRoot.h>
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/utilities/io/ObjectSaveStream.h>
#include <core/utilities/io/ObjectLoadStream.h>
#include <core/utilities/io/FileManager.h>
#include <core/utilities/concurrent/ProgressManager.h>

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, DataSetContainer, RefMaker)
DEFINE_FLAGS_REFERENCE_FIELD(DataSetContainer, _currentSet, "CurrentSet", DataSet, PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_REFERENCE_FIELD(DataSetContainer, _selectionSetProxy, "SelectionSetProxy", CurrentSelectionProxy, PROPERTY_FIELD_NO_UNDO)

/******************************************************************************
* Initializes the dataset manager.
******************************************************************************/
DataSetContainer::DataSetContainer() : RefMaker(nullptr)
{
	INIT_PROPERTY_FIELD(DataSetContainer::_currentSet);
	INIT_PROPERTY_FIELD(DataSetContainer::_selectionSetProxy);

	// Create internal selection proxy object.
	_selectionSetProxy = new CurrentSelectionProxy();
	connect(_selectionSetProxy, SIGNAL(selectionChanged(SelectionSet*)), this, SIGNAL(selectionChanged(SelectionSet*)));
	connect(_selectionSetProxy, SIGNAL(selectionChangeComplete(SelectionSet*)), this, SIGNAL(selectionChangeComplete(SelectionSet*)));
}

/******************************************************************************
* Sets the current data set being edited by the user.
******************************************************************************/
void DataSetContainer::setCurrentSet(const OORef<DataSet>& set)
{
	_currentSet = set;

	// Reset selection set
	_selectionSetProxy->setCurrentSelectionSet(set ? set->selection() : nullptr);

	// Inform listeners.
	Q_EMIT dataSetChanged(currentSet());
}

/******************************************************************************
* This is the implementation of the "Save" action.
* Returns true, if the scene has been saved.
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
		if(Application::instance().guiMode() == false)
			throw Exception(tr("Cannot save scene. No filename has been set."));

		QFileDialog dialog(currentSet()->mainWindow(), tr("Save Scene As"));
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
	if(!currentSet() || currentSet()->undoStack().isClean() || Application::instance().consoleMode())
		return true;

	QMessageBox::StandardButton result = QMessageBox::question(currentSet()->mainWindow(), tr("Save changes"),
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
bool DataSetContainer::importFile(const QUrl& url, const FileImporterDescription* importerType, FileImporter::ImportMode importMode)
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
		importer = ImportExportManager::instance().autodetectFileFormat(currentSet(), fetchFileFuture.result(), url.path());
		if(!importer)
			throw Exception(tr("Could not detect the format of the file to be imported. The format might not be supported."));
	}
	else {
		importer = importerType->createService(currentSet());
		if(!importer)
			throw Exception(tr("Failed to import file. Could not initialize import service."));
	}

	return importer->importFile(url, importMode);
}

};
