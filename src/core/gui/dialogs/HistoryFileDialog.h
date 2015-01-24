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

#ifndef __OVITO_HISTORY_FILE_DIALOG_H
#define __OVITO_HISTORY_FILE_DIALOG_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Dialogs)

/**
 * \brief The file chooser dialog that saves a history of recently visited directories.
 */
class OVITO_CORE_EXPORT HistoryFileDialog : public QFileDialog
{
	Q_OBJECT
	
public:

	/// \brief Constructs the dialog window.
	HistoryFileDialog(const QString& dialogClass, QWidget* parent = NULL, const QString& caption = QString(), const QString& directory = QString(), const QString& filter = QString());
	
private Q_SLOTS:

	/// This is called when the user has pressed the OK button of the dialog box.
	void onFileSelected(const QString& file);

protected:

	/// Loads the list of most recently visited directories from the settings store.
	QStringList loadDirHistory() const;

	/// Saves the list of most recently visited directories to the settings store.
	void saveDirHistory(const QStringList& list) const;

private:

	/// The type of file dialog: "import", "export" etc.
	QString _dialogClass;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_HISTORY_FILE_DIALOG_H
