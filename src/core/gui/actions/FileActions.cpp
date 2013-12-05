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
#include <core/gui/actions/ActionManager.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/dialogs/ApplicationSettingsDialog.h>
#include <core/gui/dialogs/ImportFileDialog.h>
#include <core/gui/dialogs/ImportRemoteFileDialog.h>
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/importexport/ImportExportManager.h>

namespace Ovito {

/******************************************************************************
* Handles the ACTION_QUIT command.
******************************************************************************/
void ActionManager::on_Quit_triggered()
{
	mainWindow()->close();
}

/******************************************************************************
* Handles the ACTION_HELP_ABOUT command.
******************************************************************************/
void ActionManager::on_HelpAbout_triggered()
{
	QMessageBox msgBox(QMessageBox::NoIcon, QCoreApplication::applicationName(),
			tr("<h3>Ovito (Open Visualization Tool)</h3>"
				"<p>Version %1</p>").arg(QCoreApplication::applicationVersion()),
			QMessageBox::Ok, mainWindow());
	msgBox.setInformativeText(tr(
			"<p>A visualization and analysis software for atomistic simulation data.</p>"
			"<p>Copyright (C) 2013, Alexander Stukowski</p>"
			"<p>This program comes with ABSOLUTELY NO WARRANTY.<br>"
			"This is free software, and you are welcome to redistribute\n"
			"it under certain conditions. See the source for copying conditions.</p>"
			"<p><a href=\"http://www.ovito.org/\">http://www.ovito.org/</a></p>"));
	msgBox.setDefaultButton(QMessageBox::Ok);
	QPixmap icon = QApplication::windowIcon().pixmap(64 * Application::instance().devicePixelRatio());
	icon.setDevicePixelRatio(Application::instance().devicePixelRatio());
	msgBox.setIconPixmap(icon);
	msgBox.exec();
}

/******************************************************************************
* Handles the ACTION_HELP_SHOW_ONLINE_HELP command.
******************************************************************************/
void ActionManager::on_HelpShowOnlineHelp_triggered()
{
	Application::instance().openHelpTopic(QString());
}

/******************************************************************************
* Handles the ACTION_FILE_NEW command.
******************************************************************************/
void ActionManager::on_FileNew_triggered()
{
	try {
		if(mainWindow()->datasetContainer().askForSaveChanges()) {
			OORef<DataSet> newSet(new DataSet());
			mainWindow()->datasetContainer().setCurrentSet(newSet);
		}
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_OPEN command.
******************************************************************************/
void ActionManager::on_FileOpen_triggered()
{
	try {
		if(!mainWindow()->datasetContainer().askForSaveChanges())
			return;

		QSettings settings;
		settings.beginGroup("file/scene");

		// Go the last directory used.
		QString defaultPath;
		OORef<DataSet> dataSet = mainWindow()->datasetContainer().currentSet();
		if(dataSet == NULL || dataSet->filePath().isEmpty())
			defaultPath = settings.value("last_directory").toString();
		else
			defaultPath = dataSet->filePath();

		QString filename = QFileDialog::getOpenFileName(mainWindow(), tr("Load Scene"),
				defaultPath, tr("Scene Files (*.ovito);;All Files (*)"));
		if(filename.isEmpty())
			return;

		// Remember directory for the next time...
		settings.setValue("last_directory", QFileInfo(filename).absolutePath());

		mainWindow()->datasetContainer().fileLoad(filename);
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_SAVE command.
******************************************************************************/
void ActionManager::on_FileSave_triggered()
{
	if(Application::instance().guiMode()) {
		// Set focus to main window.
		// This will process any pending user inputs in QLineEdit fields.
		mainWindow()->setFocus();
	}

	try {
		mainWindow()->datasetContainer().fileSave();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_SAVEAS command.
******************************************************************************/
void ActionManager::on_FileSaveAs_triggered()
{
	try {
		mainWindow()->datasetContainer().fileSaveAs();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Handles the ACTION_SETTINGS_DIALOG command.
******************************************************************************/
void ActionManager::on_Settings_triggered()
{
	if(Application::instance().guiMode()) {
		ApplicationSettingsDialog dlg(mainWindow());
		dlg.exec();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_IMPORT command.
******************************************************************************/
void ActionManager::on_FileImport_triggered()
{
	// Let the user select a file.
	ImportFileDialog dialog(ImportExportManager::instance().fileImporters(_dataset.get()), mainWindow(), tr("Import Data"));
	if(dialog.exec() != QDialog::Accepted)
		return;

	try {
		// Import file.
		mainWindow()->datasetContainer().importFile(QUrl::fromLocalFile(dialog.fileToImport()), dialog.selectedFileImporterType());
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_REMOTE_IMPORT command.
******************************************************************************/
void ActionManager::on_FileRemoteImport_triggered()
{
	// Let the user enter the URL of the remote file.
	ImportRemoteFileDialog dialog(ImportExportManager::instance().fileImporters(_dataset.get()), mainWindow(), tr("Import Remote File"));
	if(dialog.exec() != QDialog::Accepted)
		return;

	try {
		// Import URL.
		mainWindow()->datasetContainer().importFile(dialog.fileToImport(), dialog.selectedFileImporterType());
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_EXPORT command.
******************************************************************************/
void ActionManager::on_FileExport_triggered()
{
	// Build filter string.
	QStringList filterStrings;
	const auto& exporterTypes = ImportExportManager::instance().fileExporters(_dataset.get());
	for(FileExporterDescription* descriptor : exporterTypes) {
		filterStrings << QString("%1 (%2)").arg(descriptor->fileFilterDescription(), descriptor->fileFilter());
	}
	if(filterStrings.isEmpty()) {
		Exception(tr("This function is disabled, because there are no export services available.")).showError();
		return;
	}

	QSettings settings;
	settings.beginGroup("file/export");

	// Let the user select a destination file.
	HistoryFileDialog dialog("export", mainWindow(), tr("Export Data"));
	dialog.setNameFilters(filterStrings);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setConfirmOverwrite(true);

	// Go the last directory used.
	QString lastExportDirectory = settings.value("last_export_dir").toString();
	if(!lastExportDirectory.isEmpty())
		dialog.setDirectory(lastExportDirectory);
	// Select the last export filter being used ...
	QString lastExportFilter = settings.value("last_export_filter").toString();
	if(!lastExportFilter.isEmpty())
		dialog.selectNameFilter(lastExportFilter);

	if(!dialog.exec())
		return;

	QStringList files = dialog.selectedFiles();
	if(files.isEmpty())
		return;
	QString exportFile = files.front();

	// Remember directory for the next time...
	settings.setValue("last_export_dir", dialog.directory().absolutePath());
	// Remember export filter for the next time...
	settings.setValue("last_export_filter", dialog.selectedNameFilter());

	// Export to selected file.
	try {
		int exportFilterIndex = filterStrings.indexOf(dialog.selectedNameFilter());
		OVITO_ASSERT(exportFilterIndex >= 0 && exportFilterIndex < exporterTypes.size());

		OORef<FileExporter> exporter = exporterTypes[exportFilterIndex]->createService(_dataset.get());
		exporter->exportToFile(exportFile);
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

};
