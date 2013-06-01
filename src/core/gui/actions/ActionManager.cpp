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
#include <core/gui/app/Application.h>
#include <core/gui/undo/UndoManager.h>

namespace Ovito {

/// The singleton instance of the class.
QScopedPointer<ActionManager> ActionManager::_instance;

/******************************************************************************
* Initializes the ActionManager.
******************************************************************************/
ActionManager::ActionManager()
{
	createAction(ACTION_QUIT, tr("Exit"), ":/core/actions/file/file_quit.png", tr("Quit the application."), QKeySequence::Quit);
	createAction(ACTION_FILE_NEW, tr("New Scene"), ":/core/actions/file/file_new.png", tr("Resets the scene."), QKeySequence::New);
	createAction(ACTION_FILE_OPEN, tr("Open Scene"), ":/core/actions/file/file_open.png", tr("Open a saved scene from a file."), QKeySequence::Open);
	createAction(ACTION_FILE_SAVE, tr("Save Scene"), ":/core/actions/file/file_save.png", tr("Save the current scene to a file."), QKeySequence::Save);
	createAction(ACTION_FILE_SAVEAS, tr("Save Scene As"), ":/core/actions/file/file_save_as.png", tr("Save the current scene to a new file."), QKeySequence::SaveAs);
	createAction(ACTION_FILE_IMPORT, tr("Import Simulation"), ":/core/actions/file/file_import.png", tr("Import simulation data into the current scene."));
	createAction(ACTION_FILE_EXPORT, tr("Export Simulation"), ":/core/actions/file/file_export.png", tr("Export data to a file."), Qt::CTRL + Qt::Key_E);
	createAction(ACTION_HELP_ABOUT, tr("About"), NULL, tr("Show information about the application."));
	createAction(ACTION_HELP_SHOW_ONLINE_HELP, tr("Manual"), NULL, tr("Open the online manual."));

	QAction* undoAction = UndoManager::instance().createUndoAction(this);
	undoAction->setObjectName(ACTION_EDIT_UNDO);
	if(Application::instance().guiMode()) undoAction->setIcon(QIcon(QString(":/core/actions/edit/edit_undo.png")));
	addAction(undoAction);

	QAction* redoAction = UndoManager::instance().createRedoAction(this);
	redoAction->setObjectName(ACTION_EDIT_REDO);
	if(Application::instance().guiMode()) redoAction->setIcon(QIcon(QString(":/core/actions/edit/edit_redo.png")));
	addAction(redoAction);

	QMetaObject::connectSlotsByName(this);
}

/******************************************************************************
* Invokes the command action with the given ID.
******************************************************************************/
void ActionManager::invokeAction(const QString& actionId)
{
	QAction* action = getAction(actionId);
	if(!action) throw Exception(tr("Action with id '%1' is not defined.").arg(actionId));
	action->trigger();
}

/******************************************************************************
* Registers an action with the ActionManager.
******************************************************************************/
void ActionManager::addAction(QAction* action)
{
	OVITO_CHECK_POINTER(action);
	OVITO_ASSERT_MSG(action->parent() == this || findAction(action->objectName()) == NULL, "ActionManager::addAction()", "There is already an action with the same ID.");

	// Make it a child of this manager.
	action->setParent(this);
}

/******************************************************************************
* Creates and registers a new action with the ActionManager.
******************************************************************************/
QAction* ActionManager::createAction(const QString& id, const QString& title, const char* iconPath, const QString& statusTip, const QKeySequence& shortcut)
{
	QAction* action = new QAction(title, this);
	action->setObjectName(id);
	if(!shortcut.isEmpty()) action->setShortcut(shortcut);
	action->setStatusTip(statusTip);
	if(iconPath && Application::instance().guiMode())
		action->setIcon(QIcon(QString(iconPath)));
	addAction(action);
	return action;
}

};
