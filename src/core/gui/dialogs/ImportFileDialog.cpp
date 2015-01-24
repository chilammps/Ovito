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
#include "ImportFileDialog.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Constructs the dialog window.
******************************************************************************/
ImportFileDialog::ImportFileDialog(const QVector<OvitoObjectType*>& importerTypes, DataSet* dataset, QWidget* parent, const QString& caption, const QString& directory) :
	HistoryFileDialog("import", parent, caption, directory), _importerTypes(importerTypes)
{
	connect(this, &QFileDialog::fileSelected, this, &ImportFileDialog::onFileSelected);

	// Build filter string.
	for(OvitoObjectType* importerType : _importerTypes) {
		OORef<FileImporter> imp = static_object_cast<FileImporter>(importerType->createInstance(dataset));
		_filterStrings << QString("%1 (%2)").arg(imp->fileFilterDescription(), imp->fileFilter());
	}
	if(_filterStrings.isEmpty())
		throw Exception(tr("There are no importer plugins installed."));

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
	else
		selectNameFilter(_filterStrings.front());
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
	if(_selectedFile.isEmpty()) {
		QStringList filesToImport = selectedFiles();
		if(filesToImport.isEmpty()) return QString();
		return filesToImport.front();
	}
	else return _selectedFile;
}

/******************************************************************************
* Returns the selected importer type or NULL if auto-detection is requested.
******************************************************************************/
const OvitoObjectType* ImportFileDialog::selectedFileImporterType() const
{
	int importFilterIndex = _filterStrings.indexOf(_selectedFilter.isEmpty() ? selectedNameFilter() : _selectedFilter) - 1;
	OVITO_ASSERT(importFilterIndex >= -1 && importFilterIndex < _importerTypes.size());

	if(importFilterIndex >= 0)
		return _importerTypes[importFilterIndex];
	else
		return nullptr;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
