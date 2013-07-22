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
#include <core/utilities/concurrent/ProgressManager.h>
#include <core/utilities/io/FileManager.h>
#include "ImportRemoteFileDialog.h"

namespace Ovito {

/******************************************************************************
* Constructs the dialog window.
******************************************************************************/
ImportRemoteFileDialog::ImportRemoteFileDialog(QWidget* parent, const QString& caption) : QDialog(parent)
{
	setWindowTitle(caption);

	QVBoxLayout* layout1 = new QVBoxLayout(this);

	layout1->addWidget(new QLabel(tr("Remote URL:")));
	_urlEdit = new QLineEdit(this);
	_urlEdit->setPlaceholderText(tr("sftp://username@hostname/path/file"));
	layout1->addWidget(_urlEdit);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(onOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	layout1->addWidget(buttonBox);
}

/******************************************************************************
* This is called when the user has pressed the OK button of the dialog.
* Validates and saves all input made by the user and closes the dialog box.
******************************************************************************/
void ImportRemoteFileDialog::onOk()
{
	try {
		QUrl url = QUrl::fromUserInput(_urlEdit->text());
		if(!url.isValid())
			throw Exception(tr("The entered URL is invalid."));

		// Close dialog box.
		accept();
	}
	catch(const Exception& ex) {
		ex.showError();
		return;
	}
}

/******************************************************************************
* Returns the file to import after the dialog has been closed with "OK".
******************************************************************************/
QUrl ImportRemoteFileDialog::fileToImport() const
{
	return QUrl::fromUserInput(_urlEdit->text());
}

/******************************************************************************
* After the dialog has been closed with "OK", this method creates a parser
* object for the selected file.
******************************************************************************/
OORef<FileImporter> ImportRemoteFileDialog::createFileImporter()
{
	QUrl importFile = fileToImport();
	if(importFile.isEmpty()) return nullptr;

#if 0
	int importFilterIndex = _filterStrings.indexOf(selectedNameFilter()) - 1;
	OVITO_ASSERT(importFilterIndex >= -1 && importFilterIndex < _filterStrings.size());
#else
	int importFilterIndex = -1;
#endif

	OORef<FileImporter> importer;
	if(importFilterIndex >= 0)
		return ImportExportManager::instance().fileImporters()[importFilterIndex].createService();
	else {

		// Download file so we can determine its format.
		Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(importFile);
		if(!ProgressManager::instance().waitForTask(fetchFileFuture))
			return nullptr;

		OORef<FileImporter> importer = ImportExportManager::instance().autodetectFileFormat(fetchFileFuture.result(), importFile.path());
		if(!importer)
			throw Exception(tr("Could not auto-detect the format of the selected file. Please specify its format explicitly"));

		return importer;
	}
}

};
