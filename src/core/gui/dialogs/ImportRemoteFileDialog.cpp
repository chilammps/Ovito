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
#include <core/utilities/io/FileManager.h>
#include "ImportRemoteFileDialog.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Constructs the dialog window.
******************************************************************************/
ImportRemoteFileDialog::ImportRemoteFileDialog(const QVector<OvitoObjectType*>& importerTypes, DataSet* dataset, QWidget* parent, const QString& caption) : QDialog(parent),
		_importerTypes(importerTypes)
{
	setWindowTitle(caption);

	QVBoxLayout* layout1 = new QVBoxLayout(this);
	layout1->setSpacing(2);

	layout1->addWidget(new QLabel(tr("Remote URL:")));

	QHBoxLayout* layout2 = new QHBoxLayout();
	layout2->setContentsMargins(0,0,0,0);
	layout2->setSpacing(4);

	_urlEdit = new QComboBox(this);
	_urlEdit->setEditable(true);
	_urlEdit->setInsertPolicy(QComboBox::NoInsert);
	_urlEdit->setMinimumContentsLength(40);
	if(_urlEdit->lineEdit())
		_urlEdit->lineEdit()->setPlaceholderText(tr("sftp://username@hostname/path/file"));

	// Load list of recently accessed URLs.
	QSettings settings;
	settings.beginGroup("file/import_remote_file");
	QStringList list = settings.value("history").toStringList();
	for(QString entry : list)
		_urlEdit->addItem(entry);

	layout2->addWidget(_urlEdit);
	QToolButton* clearURLHistoryButton = new QToolButton();
	clearURLHistoryButton->setIcon(QIcon(":/core/actions/edit/edit_clear.png"));
	clearURLHistoryButton->setToolTip(tr("Clear history"));
	connect(clearURLHistoryButton, &QToolButton::clicked, [this]() {
		QString text = _urlEdit->currentText();
		_urlEdit->clear();
		_urlEdit->setCurrentText(text);
	});
	layout2->addWidget(clearURLHistoryButton);

	layout1->addLayout(layout2);
	layout1->addSpacing(10);

	layout1->addWidget(new QLabel(tr("File type:")));
	_formatSelector = new QComboBox(this);

	_formatSelector->addItem(tr("<Auto-detect format>"));
	for(OvitoObjectType* importerType : importerTypes) {
		OORef<FileImporter> imp = static_object_cast<FileImporter>(importerType->createInstance(dataset));
		_formatSelector->addItem(imp->fileFilterDescription());
	}

	layout1->addWidget(_formatSelector);
	layout1->addSpacing(10);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Open | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &ImportRemoteFileDialog::onOk);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &ImportRemoteFileDialog::reject);
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
* Returns the selected importer type or NULL if auto-detection is requested.
******************************************************************************/
const OvitoObjectType* ImportRemoteFileDialog::selectedFileImporterType() const
{
	int importFilterIndex = _formatSelector->currentIndex() - 1;
	OVITO_ASSERT(importFilterIndex >= -1 && importFilterIndex < _importerTypes.size());

	if(importFilterIndex >= 0)
		return _importerTypes[importFilterIndex];
	else
		return nullptr;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
