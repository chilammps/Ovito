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
#include <core/viewport/ViewportManager.h>
#include <core/gui/undo/UndoManager.h>
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/animation/AnimManager.h>
#include <core/io/ObjectSaveStream.h>
#include <core/io/ObjectLoadStream.h>

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(DataSetManager, RefMaker)
DEFINE_FLAGS_REFERENCE_FIELD(DataSetManager, _currentSet, "CurrentSet", DataSet, PROPERTY_FIELD_NO_UNDO)
#if 0
DEFINE_FLAGS_REFERENCE_FIELD(DataSetManager, _selectionSetProxy, "SelectionSetProxy", CurrentSelectionProxy, PROPERTY_FIELD_NO_UNDO)
#endif

/// The singleton instance of the class.
QScopedPointer<DataSetManager> DataSetManager::_instance;

/******************************************************************************
* Initializes the dataset manager.
******************************************************************************/
DataSetManager::DataSetManager()
{
	INIT_PROPERTY_FIELD(DataSetManager::_currentSet);
#if 0
	INIT_PROPERTY_FIELD(DataSetManager::_selectionSetProxy);
	_selectionSetProxy = new CurrentSelectionProxy();
#endif

	// Reset the undo stack when a new scene has been loaded.
	connect(this, SIGNAL(dataSetReset(DataSet*)), &UndoManager::instance(), SLOT(reset()));
}

/******************************************************************************
* Sets the current data set being edited by the user.
******************************************************************************/
void DataSetManager::setCurrentSet(const OORef<DataSet>& set)
{
	OVITO_ASSERT_MSG(!UndoManager::instance().isRecording(), "DataSetManager::setCurrentSet", "The replacement of the current dataset cannot be undone.");
	_currentSet = set;

#if 0
	// Reset selection set
	_selectionSetProxy->setCurrentSelectionSet(set ? set->selection() : NULL);
#endif

	// Do not record any operations while resetting the application.
	UndoSuspender noUndo;

	// Inform listeners.
	dataSetReset(currentSet());

	// Update viewports to show the new scene.
	ViewportManager::instance().updateViewports();
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

	// Clear dirty flag of data set.
	currentSet()->setDirty(false);

	return true;
}

/******************************************************************************
* This is the implementation of the "Save As" action.
* Returns true, if the scene has been saved.
******************************************************************************/
bool DataSetManager::fileSaveAs(const QString& filename)
{
	if(currentSet() == NULL)
		return false;

	if(filename.isEmpty()) {
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
	if(!currentSet() || !currentSet()->hasBeenChanged() || Application::instance().consoleMode())
		return true;

	QMessageBox::StandardButton result = QMessageBox::question(&MainWindow::instance(), tr("Save changes"),
		tr("The current scene has been modified. Do you want to save the changes?"),
		QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Cancel);
	if(result == QMessageBox::Cancel)
		return false; // operation canceled by user
	else if(result == QMessageBox::No)
		return true; // go on without saving
	else {
		// Save scene
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
	try {
		OORef<DataSet> dataSet;
		{
			QFile fileStream(filename);
		    if(!fileStream.open(QIODevice::ReadOnly))
				throw Exception(tr("Failed to open input file '%1' for reading.").arg(filename));

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
	}
	catch(const Exception& ex) {
		ex.showError();
		return false;
	}
	return true;
}

};
