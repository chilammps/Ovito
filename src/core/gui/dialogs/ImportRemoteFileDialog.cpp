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
	layout1->setSpacing(2);

	layout1->addWidget(new QLabel(tr("Remote URL:")));
	_urlEdit = new QComboBox(this);
	_urlEdit->setEditable(true);
	_urlEdit->setInsertPolicy(QComboBox::NoInsert);
	if(_urlEdit->lineEdit())
		_urlEdit->lineEdit()->setPlaceholderText(tr("sftp://username@hostname/path/file"));

	// Load list of recently accessed URLs.
	QSettings settings;
	settings.beginGroup("file/import_remote_file");
	QStringList list = settings.value("history").toStringList();
	for(QString entry : list)
		_urlEdit->addItem(entry);

	layout1->addWidget(_urlEdit);
	layout1->addSpacing(10);

	layout1->addWidget(new QLabel(tr("File type:")));
	_formatSelector = new QComboBox(this);

	_formatSelector->addItem(tr("<Auto-detect file format>"));
	for(const auto& descriptor : ImportExportManager::instance().fileImporters()) {
		_formatSelector->addItem(descriptor.fileFilterDescription());
	}

	layout1->addWidget(_formatSelector);
	layout1->addSpacing(10);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(onOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	layout1->addWidget(buttonBox);
}

/******************************************************************************
* Sets the current URL in the dialog.
******************************************************************************/
void ImportRemoteFileDialog::selectFile(const QUrl& url)
{
	_urlEdit->setCurrentText(url.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded));
}

/******************************************************************************
* This is called when the user has pressed the OK button of the dialog.
* Validates and saves all input made by the user and closes the dialog box.
******************************************************************************/
void ImportRemoteFileDialog::onOk()
{
	try {
		QUrl url = QUrl::fromUserInput(_urlEdit->currentText());
		if(!url.isValid())
			throw Exception(tr("The entered URL is invalid."));

		// Save list of recently accessed URLs.
		QStringList list;
		for(int index = 0; index < _urlEdit->count(); index++) {
			list << _urlEdit->itemText(index);
		}
		QString newEntry = url.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded);
		list.removeAll(newEntry);
		list.prepend(newEntry);
		while(list.size() > 40)
			list.removeLast();
		QSettings settings;
		settings.beginGroup("file/import_remote_file");
		settings.setValue("history", list);

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
	return QUrl::fromUserInput(_urlEdit->currentText());
}

/******************************************************************************
* Returns the selected importer or NULL if auto-detection is requested.
******************************************************************************/
const FileImporterDescription* ImportRemoteFileDialog::selectedFileImporter() const
{
	int importFilterIndex = _formatSelector->currentIndex() - 1;
	OVITO_ASSERT(importFilterIndex >= -1 && importFilterIndex < ImportExportManager::instance().fileImporters().size());

	if(importFilterIndex >= 0)
		return &ImportExportManager::instance().fileImporters()[importFilterIndex];
	else
		return nullptr;
}

};
