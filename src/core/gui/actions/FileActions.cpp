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
#include <core/dataset/importexport/FileImporter.h>
#include <core/dataset/importexport/FileExporter.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportWindow.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>
#include <core/scene/SelectionSet.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui)

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
			"<p>Copyright (C) 2013-2015, Alexander Stukowski</p>"
			"<p>This program comes with ABSOLUTELY NO WARRANTY.<br>"
			"This is free software, and you are welcome to redistribute\n"
			"it under certain conditions. See the source for copying conditions.</p>"
			"<p><a href=\"http://www.ovito.org/\">http://www.ovito.org/</a></p>"));
	msgBox.setDefaultButton(QMessageBox::Ok);
	QPixmap icon = QApplication::windowIcon().pixmap(64 * mainWindow()->devicePixelRatio());
	icon.setDevicePixelRatio(mainWindow()->devicePixelRatio());
	msgBox.setIconPixmap(icon);
	msgBox.exec();
}

/******************************************************************************
* Handles the ACTION_HELP_SHOW_ONLINE_HELP command.
******************************************************************************/
void ActionManager::on_HelpShowOnlineHelp_triggered()
{
	mainWindow()->openHelpTopic(QString());
}

/******************************************************************************
* Handles the ACTION_HELP_OPENGL_INFO command.
******************************************************************************/
void ActionManager::on_HelpOpenGLInfo_triggered()
{
	QDialog dlg(mainWindow());
	dlg.setWindowTitle(tr("OpenGL Information"));
	QVBoxLayout* layout = new QVBoxLayout(&dlg);
	QTextEdit* textEdit = new QTextEdit(&dlg);
	textEdit->setReadOnly(true);
	QString text;
	if(mainWindow()->datasetContainer().currentSet()) {
		Viewport* vp = mainWindow()->datasetContainer().currentSet()->viewportConfig()->activeViewport();
		ViewportSceneRenderer* renderer = mainWindow()->datasetContainer().currentSet()->viewportConfig()->viewportRenderer();
		if(vp && renderer && vp->viewportWindow()->glcontext()) {
			vp->viewportWindow()->glcontext()->makeCurrent(vp->viewportWindow());
			QSurfaceFormat format = vp->viewportWindow()->glcontext()->format();
			QTextStream stream(&text, QIODevice::WriteOnly | QIODevice::Text);
			stream << "======= System info =======" << endl;
			stream << "Date: " << QDateTime::currentDateTime().toString() << endl;
			stream << "Application: " << QApplication::applicationName() << " " << QApplication::applicationVersion() << endl;
#if defined(Q_OS_MAC)
			stream << "OS: Mac OS X (" << QSysInfo::macVersion() << ")" << endl;
#elif defined(Q_OS_WIN)
			stream << "OS: Windows (" << QSysInfo::windowsVersion() << ")" << endl;
#elif defined(Q_OS_LINUX)
			stream << "OS: Linux" << endl;
			// Get 'uname' output.
			QProcess unameProcess;
			unameProcess.start("uname -m -i -o -r -v", QIODevice::ReadOnly);
			unameProcess.waitForFinished();
			QByteArray unameOutput = unameProcess.readAllStandardOutput();
			unameOutput.replace('\n', ' ');
			stream << "uname output: " << unameOutput << endl;
			// Get 'lsb_release' output.
			QProcess lsbProcess;
			lsbProcess.start("lsb_release -s -i -d -r", QIODevice::ReadOnly);
			lsbProcess.waitForFinished();
			QByteArray lsbOutput = lsbProcess.readAllStandardOutput();
			lsbOutput.replace('\n', ' ');
			stream << "LSB output: " << lsbOutput << endl;
#endif
			stream << "Architecture: " << (QT_POINTER_SIZE*8) << " bit" << endl;
			stream << "Qt version: " << QT_VERSION_STR << endl;
			stream << "Command line: " << QCoreApplication::arguments().join(' ') << endl;
			stream << "======= OpenGL info =======" << endl;
			stream << "Version: " << format.majorVersion() << QStringLiteral(".") << format.minorVersion() << endl;
			stream << "Profile: " << (format.profile() == QSurfaceFormat::CoreProfile ? "core" : (format.profile() == QSurfaceFormat::CompatibilityProfile ? "compatibility" : "none")) << endl;
			stream << "Alpha: " << format.hasAlpha() << endl;
			stream << "Vendor: " << QString((const char*)glGetString(GL_VENDOR)) << endl;
			stream << "Renderer: " << QString((const char*)glGetString(GL_RENDERER)) << endl;
			stream << "Version string: " << QString((const char*)glGetString(GL_VERSION)) << endl;
			stream << "Shading language: " << QString((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)) << endl;
			stream << "Shader programs: " << QOpenGLShaderProgram::hasOpenGLShaderPrograms() << endl;
			stream << "Vertex shaders: " << QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Vertex) << endl;
			stream << "Fragment shaders: " << QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Fragment) << endl;
			stream << "Geometry shaders: " << QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Geometry) << endl;
			stream << "Swap behavior: " << (format.swapBehavior() == QSurfaceFormat::SingleBuffer ? QStringLiteral("single buffer") : (format.swapBehavior() == QSurfaceFormat::DoubleBuffer ? QStringLiteral("double buffer") : (format.swapBehavior() == QSurfaceFormat::TripleBuffer ? QStringLiteral("triple buffer") : QStringLiteral("other")))) << endl;
			stream << "Depth buffer size: " << format.depthBufferSize() << endl;
			stream << "Stencil buffer size: " << format.stencilBufferSize() << endl;
			stream << "Deprecated functions: " << format.testOption(QSurfaceFormat::DeprecatedFunctions) << endl;
			stream << "Using point sprites: " << renderer->usePointSprites() << endl;
			stream << "Using geometry shaders: " << renderer->useGeometryShaders() << endl;
			stream << "Context sharing: " << ViewportWindow::contextSharingEnabled() << endl;
			vp->viewportWindow()->glcontext()->doneCurrent();
		}
	}
	if(!text.isEmpty())
		textEdit->setPlainText(text);
	else
		textEdit->setPlainText(tr("Could not obtain OpenGL information."));
	textEdit->setMinimumSize(QSize(600, 400));
	layout->addWidget(textEdit);
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, &dlg);
	connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::accept);
	connect(buttonBox->addButton(tr("Copy to clipboard"), QDialogButtonBox::ActionRole), &QPushButton::clicked, [text]() { QApplication::clipboard()->setText(text); });
	layout->addWidget(buttonBox);
	dlg.exec();
}

/******************************************************************************
* Handles the ACTION_FILE_NEW_WINDOW command.
******************************************************************************/
void ActionManager::on_FileNewWindow_triggered()
{
	try {
		MainWindow* mainWin = new MainWindow();
		mainWin->show();
		mainWin->restoreLayout();
		mainWin->datasetContainer().fileNew();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Handles the ACTION_FILE_NEW command.
******************************************************************************/
void ActionManager::on_FileNew_triggered()
{
	try {
		if(mainWindow()->datasetContainer().askForSaveChanges()) {
			mainWindow()->datasetContainer().fileNew();
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
	ImportFileDialog dialog(FileImporter::availableImporters(), _dataset, mainWindow(), tr("Import Data"));
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
	ImportRemoteFileDialog dialog(FileImporter::availableImporters(), _dataset, mainWindow(), tr("Import Remote File"));
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
	// Create the list of scene nodes to be exported.
	QVector<SceneNode*> nodes = _dataset->selection()->nodes();
	if(nodes.empty()) {
		Exception(tr("Please select an object to be exported first.")).showError();
		return;
	}

	// Build filter string.
	QStringList filterStrings;
	const auto& exporterTypes = FileExporter::availableExporters();
	for(const OvitoObjectType* type : exporterTypes) {
		try {
			OORef<FileExporter> exporter = static_object_cast<FileExporter>(type->createInstance(_dataset));
			filterStrings << QString("%1 (%2)").arg(exporter->fileFilterDescription(), exporter->fileFilter());
		}
		catch(...) { filterStrings << QString(); }
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

		// Create exporter.
		OORef<FileExporter> exporter = static_object_cast<FileExporter>(exporterTypes[exportFilterIndex]->createInstance(_dataset));

		// Load user-defined default settings.
		exporter->loadUserDefaults();

		exporter->exportToFile(nodes, exportFile, false);
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
