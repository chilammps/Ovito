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
#include <core/gui/actions/ViewportModeAction.h>
#include <core/gui/app/Application.h>
#include <core/gui/undo/UndoManager.h>
#include <core/viewport/input/NavigationModes.h>
#include <core/animation/AnimManager.h>
#include <core/dataset/DataSetManager.h>

namespace Ovito {

/// The singleton instance of the class.
ActionManager* ActionManager::_instance = nullptr;

/******************************************************************************
* Initializes the ActionManager.
******************************************************************************/
ActionManager::ActionManager()
{
	OVITO_ASSERT_MSG(!_instance, "ActionManager constructor", "Multiple instances of this singleton class have been created.");

	createCommandAction(ACTION_QUIT, tr("Exit"), ":/core/actions/file/file_quit.png", tr("Quit the application."), QKeySequence::Quit);
	createCommandAction(ACTION_FILE_NEW, tr("Reset State"), ":/core/actions/file/file_new.png", tr("Resets the program to its initial state."), QKeySequence::New);
	createCommandAction(ACTION_FILE_OPEN, tr("Load State"), ":/core/actions/file/file_open.png", tr("Load a saved state from a file."), QKeySequence::Open);
	createCommandAction(ACTION_FILE_SAVE, tr("Save State"), ":/core/actions/file/file_save.png", tr("Save the current program state to a file."), QKeySequence::Save);
	createCommandAction(ACTION_FILE_SAVEAS, tr("Save State As"), ":/core/actions/file/file_save_as.png", tr("Save the current program state to a new file."), QKeySequence::SaveAs);
	createCommandAction(ACTION_FILE_IMPORT, tr("Open Local File"), ":/core/actions/file/file_import.png", tr("Import data from a file on this computer."), Qt::CTRL + Qt::Key_I);
	createCommandAction(ACTION_FILE_REMOTE_IMPORT, tr("Open Remote File"), ":/core/actions/file/file_import_remote.png", tr("Import a file from a remote location."), Qt::CTRL + Qt::SHIFT + Qt::Key_I);
	createCommandAction(ACTION_FILE_EXPORT, tr("Export File"), ":/core/actions/file/file_export.png", tr("Export data to a file."), Qt::CTRL + Qt::Key_E);
	createCommandAction(ACTION_HELP_ABOUT, tr("About Ovito"), NULL, tr("Show information about the application."));
	createCommandAction(ACTION_HELP_SHOW_ONLINE_HELP, tr("Manual"), NULL, tr("Open the online manual."));

	createCommandAction(ACTION_EDIT_DELETE, tr("Delete"), ":/core/actions/edit/edit_delete.png", tr("Deletes the selected objects."), QKeySequence::Delete);

	createCommandAction(ACTION_SETTINGS_DIALOG, tr("&Settings..."));

	createCommandAction(ACTION_RENDER_ACTIVE_VIEWPORT, tr("Render Active Viewport"), ":/core/actions/rendering/render_active_viewport.png");

	createCommandAction(ACTION_VIEWPORT_MAXIMIZE, tr("Maximize Active Viewport"), ":/core/actions/viewport/maximize_viewport.png", tr("Enlarge/reduce the active viewport."));
	createCommandAction(ACTION_VIEWPORT_ZOOM_SCENE_EXTENTS, tr("Zoom Scene Extents"), ":/core/actions/viewport/zoom_scene_extents.png", tr("Zoom to show everything."));
	createCommandAction(ACTION_VIEWPORT_ZOOM_SCENE_EXTENTS_ALL, tr("Zoom Scene Extents All"), ":/core/actions/viewport/zoom_scene_extents.png", tr("Zoom all viewports to show everything."));
	createCommandAction(ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS, tr("Zoom Selection Extents"), ":/core/actions/viewport/zoom_selection_extents.png", tr("Zoom to show the selected objects."));
	createCommandAction(ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS_ALL, tr("Zoom Selection Extents All"), ":/core/actions/viewport/zoom_selection_extents.png", tr("Zoom all viewports to show the selected objects."));
	createViewportModeAction(ACTION_VIEWPORT_ZOOM, ZoomMode::instance(), tr("Zoom"), ":/core/actions/viewport/mode_zoom.png", tr("Activate zoom mode."));
	createViewportModeAction(ACTION_VIEWPORT_PAN, PanMode::instance(), tr("Pan"), ":/core/actions/viewport/mode_pan.png", tr("Activate pan mode to shift the region visible in the viewports."));
	createViewportModeAction(ACTION_VIEWPORT_ORBIT, OrbitMode::instance(), tr("Orbit"), ":/core/actions/viewport/mode_orbit.png", tr("Activate orbit mode to rotate the camera around the scene."));
	createViewportModeAction(ACTION_VIEWPORT_FOV, FOVMode::instance(), tr("Field Of View"), ":/core/actions/viewport/mode_fov.png", tr("Activate field of view mode to change the perspective projection."));
	createViewportModeAction(ACTION_VIEWPORT_PICK_ORBIT_CENTER, PickOrbitCenterMode::instance(), tr("Set Orbit Center"), ":/core/actions/viewport/mode_set_orbit_center.png", tr("Set the center of rotation."));

	createCommandAction(ACTION_GOTO_START_OF_ANIMATION, tr("Goto Start of Animation"), ":/core/actions/animation/goto_animation_start.png", QString(), Qt::Key_Home);
	createCommandAction(ACTION_GOTO_END_OF_ANIMATION, tr("Goto End of Animation"), ":/core/actions/animation/goto_animation_end.png", QString(), Qt::Key_End);
	createCommandAction(ACTION_GOTO_PREVIOUS_FRAME, tr("Goto Previous Frame"), ":/core/actions/animation/goto_previous_frame.png", QString(), Qt::Key_Minus);
	createCommandAction(ACTION_GOTO_NEXT_FRAME, tr("Goto Next Frame"), ":/core/actions/animation/goto_next_frame.png", QString(), Qt::Key_Plus);
	createCommandAction(ACTION_START_ANIMATION_PLAYBACK, tr("Start Animation Playback"), ":/core/actions/animation/play_animation.png");
	createCommandAction(ACTION_STOP_ANIMATION_PLAYBACK, tr("Stop Animation Playback"), ":/core/actions/animation/stop_animation.png");
	createCommandAction(ACTION_ANIMATION_SETTINGS, tr("Animation Settings"), ":/core/actions/animation/animation_settings.png");
	createViewportModeAction(ACTION_TOGGLE_ANIMATION_PLAYBACK, createAnimationPlaybackViewportMode(), tr("Play Animation"), ":/core/actions/animation/play_animation.png");

	// Create action that toggles animation mode.
	QAction* animModeAction = createCommandAction(ACTION_ANIMATION_MODE_TOGGLE, tr("Animation Mode"), ":/core/actions/animation/animation_mode.png");
	animModeAction->setCheckable(true);
	animModeAction->setChecked(AnimManager::instance().animationMode());
	connect(animModeAction, SIGNAL(toggled(bool)), &AnimManager::instance(), SLOT(setAnimationMode(bool)));
	connect(&AnimManager::instance(), SIGNAL(animationModeChanged(bool)), animModeAction, SLOT(setChecked(bool)));

	// Create Edit->Undo action.
	QAction* undoAction = UndoManager::instance().createUndoAction(this);
	undoAction->setObjectName(ACTION_EDIT_UNDO);
	if(Application::instance().guiMode()) undoAction->setIcon(QIcon(QString(":/core/actions/edit/edit_undo.png")));
	addAction(undoAction);

	// Create Edit->Redo action.
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
* Creates and registers a new command action with the ActionManager.
******************************************************************************/
QAction* ActionManager::createCommandAction(const QString& id, const QString& title, const char* iconPath, const QString& statusTip, const QKeySequence& shortcut)
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

/******************************************************************************
* Creates and registers a new viewport mode action with the ActionManager.
******************************************************************************/
QAction* ActionManager::createViewportModeAction(const QString& id, const OORef<ViewportInputHandler>& inputHandler, const QString& title, const char* iconPath, const QString& statusTip, const QKeySequence& shortcut)
{
	QAction* action = new ViewportModeAction(title, this, inputHandler);
	action->setObjectName(id);
	if(!shortcut.isEmpty()) action->setShortcut(shortcut);
	action->setStatusTip(statusTip);
	if(iconPath && Application::instance().guiMode())
		action->setIcon(QIcon(QString(iconPath)));
	addAction(action);
	return action;
}

/******************************************************************************
* Handles ACTION_EDIT_DELETE command
******************************************************************************/
void ActionManager::on_EditDelete_triggered()
{
	UndoableTransaction::handleExceptions(tr("Delete"), []() {
		// Delete all nodes in selection set.
		for(SceneNode* node : DataSetManager::instance().currentSelection()->nodes())
			node->deleteNode();
	});
}


};
