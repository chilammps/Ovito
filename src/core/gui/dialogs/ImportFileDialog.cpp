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
#include <core/dataset/importexport/ImportExportManager.h>
#include "ImportFileDialog.h"

namespace Ovito {

/******************************************************************************
* Constructs the dialog window.
******************************************************************************/
ImportFileDialog::ImportFileDialog(QWidget* parent, const QString& caption) :
	HistoryFileDialog("import", parent, caption)
{
	connect(this, SIGNAL(fileSelected(const QString&)), this, SLOT(onFileSelected(const QString&)));

	// Build filter string.
	for(const auto& descriptor : ImportExportManager::instance().fileImporters()) {
		_filterStrings << QString("%1 (%2)").arg(descriptor.fileFilterDescription(), descriptor.fileFilter());
	}
	if(_filterStrings.isEmpty())
		throw Exception(tr("There are no import plugins installed."));

	_filterStrings.prepend(tr("<Auto-detect file format> (*)"));

	setNameFilters(_filterStrings);
	setAcceptMode(QFileDialog::AcceptOpen);
	setFileMode(QFileDialog::ExistingFile);

	QSettings settings;
	settings.beginGroup("file/import");

	// Select the last import filter that was used last time.
	QString lastImportFilter = settings.value("last_import_filter").toString();
	if(!lastImportFilter.isEmpty())
		selectNameFilter(lastImportFilter);
}

/******************************************************************************
* This is called when the user has pressed the OK button of the dialog.
******************************************************************************/
void ImportFileDialog::onFileSelected(const QString& file)
{
	if(file.isEmpty())
		return;

	// Remember selected import filter for the next time...
	QSettings settings;
	settings.beginGroup("file/import");
	settings.setValue("last_import_filter", selectedNameFilter());
}


/******************************************************************************
* Returns the file to import after the dialog has been closed with "OK".
******************************************************************************/
QString ImportFileDialog::fileToImport() const
{
	QStringList filesToImport = selectedFiles();
	if(filesToImport.isEmpty()) return QString();
	return filesToImport.front();
}

/******************************************************************************
* After the dialog has been closed with "OK", this method creates a parser
* object for the selected file.
******************************************************************************/
OORef<FileImporter> ImportFileDialog::createFileImporter()
{
	QString importFile = fileToImport();
	if(importFile.isEmpty()) return nullptr;

	int importFilterIndex = _filterStrings.indexOf(selectedNameFilter()) - 1;
	OVITO_ASSERT(importFilterIndex >= -1 && importFilterIndex < _filterStrings.size());

	OORef<FileImporter> importer;
	if(importFilterIndex >= 0)
		return ImportExportManager::instance().fileImporters()[importFilterIndex].createService();
	else {
		OORef<FileImporter> importer = ImportExportManager::instance().autodetectFileFormat(importFile);
		if(!importer)
			throw Exception(tr("Could not auto-detect the format of the selected file. Please specify its format explicitly in the file selector dialog."));
		return importer;
	}
}

#ifdef Q_WS_MAC

/******************************************************************************
* Shows the dialog box.
******************************************************************************/
int ImportFileDialog::exec()
{
	// On Mac OS X, use the native dialog box (by calling QFileDialog::getOpenFileName) instead of QFileDialog,
	// because it provides easier access to external drives.

	QString filterString;
	Q_FOREACH(QString f, nameFilters())
		filterString += f + ";;";

	QString selFilter = selectedNameFilter();
	QString filename = QFileDialog::getOpenFileName(parentWidget(), windowTitle(), directory().path(), filterString, &selFilter);
	if(filename.isEmpty()) return 0;

	selectNameFilter(selFilter);
	onFileSelected(filename);
	selectFile(filename);

	return 1;
}

#endif


};
