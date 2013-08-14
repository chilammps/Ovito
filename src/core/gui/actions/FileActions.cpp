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
#include <core/dataset/DataSetManager.h>
#include <core/dataset/importexport/ImportExportManager.h>

namespace Ovito {

/******************************************************************************
* Handles the ACTION_QUIT command.
******************************************************************************/
void ActionManager::on_Quit_triggered()
{
	if(!Application::instance().guiMode()) return;
	MainWindow::instance().close();
}

/******************************************************************************
* Handles the ACTION_HELP_ABOUT command.
******************************************************************************/
void ActionManager::on_HelpAbout_triggered()
{
	if(!Application::instance().guiMode()) return;

	QMessageBox msgBox(QMessageBox::NoIcon, QCoreApplication::applicationName(),
			tr("<h3>Ovito (Open Visualization Tool)</h3>"
				"<p>Version %1</p>").arg(QCoreApplication::applicationVersion()),
			QMessageBox::Ok);
	msgBox.setInformativeText(tr(
			"<p>A visualization and analysis software for atomistic simulation data.</p>"
			"<p>Copyright (C) 2013, Alexander Stukowski</p>"
			"<p><a href=\"http://www.ovito.org/\">http://www.ovito.org/</a></p>"));
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.setIconPixmap(QApplication::windowIcon().pixmap(64));
	msgBox.exec();
}

/******************************************************************************
* Handles the ACTION_HELP_SHOW_ONLINE_HELP command.
******************************************************************************/
void ActionManager::on_HelpShowOnlineHelp_triggered()
{
	if(!Application::instance().guiMode()) return;

	// Use the web browser to display online help.
	QString fullURL = QString("http://www.ovito.org/manual_v%1.%2.%3/")
					.arg(OVITO_VERSION_MAJOR)
					.arg(OVITO_VERSION_MINOR)
					.arg(OVITO_VERSION_REVISION);

	if(!QDesktopServices::openUrl(QUrl(fullURL))) {
		Exception(tr("Could not launch web browser to display online manual. The requested URL is %1").arg(fullURL)).showError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_NEW command.
******************************************************************************/
void ActionManager::on_FileNew_triggered()
{
	if(DataSetManager::instance().askForSaveChanges()) {
		OORef<DataSet> newSet(new DataSet());
		DataSetManager::instance().setCurrentSet(newSet);
	}
}

/******************************************************************************
* Handles the ACTION_FILE_OPEN command.
******************************************************************************/
void ActionManager::on_FileOpen_triggered()
{
	if(!DataSetManager::instance().askForSaveChanges())
		return;

	QSettings settings;
	settings.beginGroup("file/scene");

	// Go the last directory used.
	QString defaultPath;
	OORef<DataSet> dataSet = DataSetManager::instance().currentSet();
	if(dataSet == NULL || dataSet->filePath().isEmpty())
		defaultPath = settings.value("last_directory").toString();
	else
		defaultPath = dataSet->filePath();

	QString filename = QFileDialog::getOpenFileName(&MainWindow::instance(), tr("Load Scene"),
			defaultPath, tr("Scene Files (*.ovito);;All Files (*)"));
    if(filename.isEmpty())
		return;

	// Remember directory for the next time...
	settings.setValue("last_directory", QFileInfo(filename).absolutePath());

	DataSetManager::instance().fileLoad(filename);
}

/******************************************************************************
* Handles the ACTION_FILE_SAVE command.
******************************************************************************/
void ActionManager::on_FileSave_triggered()
{
	DataSetManager::instance().fileSave();
}

/******************************************************************************
* Handles the ACTION_FILE_SAVEAS command.
******************************************************************************/
void ActionManager::on_FileSaveAs_triggered()
{
	DataSetManager::instance().fileSaveAs();
}

/******************************************************************************
* Handles the ACTION_SETTINGS_DIALOG command.
******************************************************************************/
void ActionManager::on_Settings_triggered()
{
	if(Application::instance().guiMode()) {
		ApplicationSettingsDialog dlg(&MainWindow::instance());
		dlg.exec();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_IMPORT command.
******************************************************************************/
void ActionManager::on_FileImport_triggered()
{
	// Let the user select a file.
	ImportFileDialog dialog(&MainWindow::instance(), tr("Import Data"));
	if(dialog.exec() != QDialog::Accepted)
		return;

	// Import file.
	DataSetManager::instance().importFile(QUrl::fromLocalFile(dialog.fileToImport()), dialog.selectedFileImporter());
}

/******************************************************************************
* Handles the ACTION_FILE_REMOTE_IMPORT command.
******************************************************************************/
void ActionManager::on_FileRemoteImport_triggered()
{
	// Let the user enter the URL of the remote file.
	ImportRemoteFileDialog dialog(&MainWindow::instance(), tr("Import Remote File"));
	if(dialog.exec() != QDialog::Accepted)
		return;

	// Import URL.
	DataSetManager::instance().importFile(dialog.fileToImport(), dialog.selectedFileImporter());
}

/******************************************************************************
* Handles the ACTION_FILE_EXPORT command.
******************************************************************************/
void ActionManager::on_FileExport_triggered()
{
#if 0
	// Build filter string.
	QStringList filterStrings;
	Q_FOREACH(const ImportExportDescriptor& descr, IMPORTEXPORT_MANAGER.exporters()) {
		filterStrings << QString("%1 (%2)").arg(descr.fileFilterDescription(), descr.fileFilter());
	}
	if(filterStrings.isEmpty()) {
		Exception(tr("This function is disabled. There are no export plugins installed.")).showError();
		return;
	}

	QSettings settings;
	settings.beginGroup("file/export");

	// Let the user select a destination file.
	HistoryFileDialog dialog("export", MAIN_FRAME, tr("Export Data"));
	dialog.setFilters(filterStrings);
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
		dialog.selectFilter(lastExportFilter);

	if(!dialog.exec())
		return;

	QStringList files = dialog.selectedFiles();
	if(files.isEmpty())
		return;
	QString exportFile = files.front();

	// Remember directory for the next time...
	settings.setValue("last_export_dir", dialog.directory().absolutePath());
	// Remember export filter for the next time...
	settings.setValue("last_export_filter", dialog.selectedFilter());

	// Export to selected file.
	try {
		int exportFilterIndex = filterStrings.indexOf(dialog.selectedFilter());
		OVITO_ASSERT(exportFilterIndex >= 0 && exportFilterIndex < filterStrings.size());
		ImporterExporter::SmartPtr exporter = IMPORTEXPORT_MANAGER.exporters()[exportFilterIndex].createService();

		exporter->exportToFile(exportFile, DATASET_MANAGER.currentSet());
	}
	catch(const Exception& ex) {
		ex.showError();
	}
#endif
}

};
