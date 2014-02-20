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
#include "HistoryFileDialog.h"

#define MAX_DIRECTORY_HISTORY_SIZE	1

namespace Ovito {

/******************************************************************************
* Constructs the dialog window.
******************************************************************************/
HistoryFileDialog::HistoryFileDialog(const QString& dialogClass, QWidget* parent, const QString& caption, const QString& directory, const QString& filter) :
	QFileDialog(parent, caption, directory, filter), _dialogClass(dialogClass)
{
	connect(this, &QFileDialog::fileSelected, this, &HistoryFileDialog::onFileSelected);

	if(directory.isEmpty()) {
		QStringList history = loadDirHistory();
		if(history.isEmpty() == false) {
			setDirectory(history.front());
		}
	}
}

/******************************************************************************
* This is called when the user has pressed the OK button of the dialog.
******************************************************************************/
void HistoryFileDialog::onFileSelected(const QString& file)
{
	if(file.isEmpty()) return;
	QString currentDir = QFileInfo(file).absolutePath();

	QStringList history = loadDirHistory();
	history.removeAll(currentDir);
	if(history.size() >= MAX_DIRECTORY_HISTORY_SIZE)
		history.erase(history.begin() + (MAX_DIRECTORY_HISTORY_SIZE - 1), history.end());
	history.push_front(currentDir);
	saveDirHistory(history);
}

/******************************************************************************
* Loads the list of most recently visited directories from the settings store.
******************************************************************************/
QStringList HistoryFileDialog::loadDirHistory() const
{
	QStringList list;
	QSettings settings;
	settings.beginGroup("file/dir_history/" + _dialogClass);
	for(int index = 0; ; index++) {
		QString d = settings.value(QString("dir%1").arg(index++)).toString();
		if(d.isEmpty()) break;
		list.push_back(d);
	}
	return list;
}

/******************************************************************************
* Saves the list of most recently visited directories to the settings store.
******************************************************************************/
void HistoryFileDialog::saveDirHistory(const QStringList& list) const
{
	QSettings settings;
	settings.beginGroup("file/dir_history/" + _dialogClass);
	settings.remove("");
	for(int index = 0; index < list.size(); index++) {
		settings.setValue(QString("dir%1").arg(index), list[index]);
	}
}

};
