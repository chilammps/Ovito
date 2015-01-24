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

#ifndef __OVITO_IMPORT_REMOTE_FILE_DIALOG_H
#define __OVITO_IMPORT_REMOTE_FILE_DIALOG_H

#include <core/Core.h>
#include <core/dataset/importexport/FileImporter.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * This dialog lets the user select a remote file to be imported.
 */
class OVITO_CORE_EXPORT ImportRemoteFileDialog : public QDialog
{
	Q_OBJECT
	
public:

	/// \brief Constructs the dialog window.
	ImportRemoteFileDialog(const QVector<OvitoObjectType*>& importerTypes, DataSet* dataset, QWidget* parent = nullptr, const QString& caption = QString());

	/// \brief Sets the current URL in the dialog.
	void selectFile(const QUrl& url);

	/// \brief Returns the file to import after the dialog has been closed with "OK".
	QUrl fileToImport() const;

	/// \brief Returns the selected importer type or NULL if auto-detection is requested.
	const OvitoObjectType* selectedFileImporterType() const;

	virtual QSize sizeHint() const override {
		return QDialog::sizeHint().expandedTo(QSize(500, 0));
	}

protected Q_SLOTS:

	/// This is called when the user has pressed the OK button of the dialog.
	/// Validates and saves all input made by the user and closes the dialog box.
	void onOk();

private:

	QVector<OvitoObjectType*> _importerTypes;

	QComboBox* _urlEdit;
	QComboBox* _formatSelector;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_IMPORT_REMOTE_FILE_DIALOG_H
