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

#ifndef __OVITO_ACTION_MANAGER_H
#define __OVITO_ACTION_MANAGER_H

#include <core/Core.h>
#include <core/animation/TimeInterval.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui)

//////////////////////// Action identifiers ///////////////////////////

/// This action closes the main window and exits the application.
#define ACTION_QUIT				"Quit"
/// This action loads an empty scene file.
#define ACTION_FILE_NEW			"FileNew"
/// This action shows the file open dialog.
#define ACTION_FILE_OPEN		"FileOpen"
/// This action saves the current file.
#define ACTION_FILE_SAVE		"FileSave"
/// This action shows the file save as dialog.
#define ACTION_FILE_SAVEAS		"FileSaveAs"
/// This action shows the file import dialog.
#define ACTION_FILE_IMPORT		"FileImport"
/// This action shows the remote file import dialog.
#define ACTION_FILE_REMOTE_IMPORT "FileRemoteImport"
/// This action shows the file export dialog.
#define ACTION_FILE_EXPORT		"FileExport"
/// This action opens another main window.
#define ACTION_FILE_NEW_WINDOW	"FileNewWindow"

/// This action shows the about dialog.
#define ACTION_HELP_ABOUT				"HelpAbout"
/// This action shows the online help.
#define ACTION_HELP_SHOW_ONLINE_HELP	"HelpShowOnlineHelp"
/// This action displays OpenGL diagnostics.
#define ACTION_HELP_OPENGL_INFO			"HelpOpenGLInfo"

/// This action undoes the last operation.
#define ACTION_EDIT_UNDO		"EditUndo"
/// This action does the last undone operation again.
#define ACTION_EDIT_REDO		"EditRedo"
/// This action deletes the selected objects.
#define ACTION_EDIT_DELETE		"EditDelete"

/// This action maximizes the active viewport.
#define ACTION_VIEWPORT_MAXIMIZE					"ViewportMaximize"
/// This action activates the viewport zoom mode.
#define ACTION_VIEWPORT_ZOOM						"ViewportZoom"
/// This action activates the viewport pan mode.
#define ACTION_VIEWPORT_PAN							"ViewportPan"
/// This action activates the viewport orbit mode.
#define ACTION_VIEWPORT_ORBIT						"ViewportOrbit"
/// This action activates the field of view viewport mode.
#define ACTION_VIEWPORT_FOV							"ViewportFOV"
/// This action activates the 'pick center of rotation' input mode.
#define ACTION_VIEWPORT_PICK_ORBIT_CENTER			"ViewportOrbitPickCenter"
/// This zooms the current viewport to the scene extents
#define ACTION_VIEWPORT_ZOOM_SCENE_EXTENTS			"ViewportZoomSceneExtents"
/// This zooms the current viewport to the selection extents
#define ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS		"ViewportZoomSelectionExtents"
/// This zooms all viewports to the scene extents
#define ACTION_VIEWPORT_ZOOM_SCENE_EXTENTS_ALL		"ViewportZoomSceneExtentsAll"
/// This zooms all viewports to the selection extents
#define ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS_ALL	"ViewportZoomSelectionExtentsAll"

/// This action deletes the currently selected modifier from the modifier stack.
#define ACTION_MODIFIER_DELETE				"ModifierDelete"
/// This action moves the currently selected modifer up one entry in the modifier stack.
#define ACTION_MODIFIER_MOVE_UP				"ModifierMoveUp"
/// This action moves the currently selected modifer up down entry in the modifier stack.
#define ACTION_MODIFIER_MOVE_DOWN			"ModifierMoveDown"
/// This action toggles the enabled/disable state of the currently selected modifier.
#define ACTION_MODIFIER_TOGGLE_STATE		"ModifierToggleEnabledState"

/// This action jumps to the start of the animation
#define ACTION_GOTO_START_OF_ANIMATION		"AnimationGotoStart"
/// This action jumps to the end of the animation
#define ACTION_GOTO_END_OF_ANIMATION		"AnimationGotoEnd"
/// This action jumps to previous frame in the animation
#define ACTION_GOTO_PREVIOUS_FRAME			"AnimationGotoPreviousFrame"
/// This action jumps to next frame in the animation
#define ACTION_GOTO_NEXT_FRAME				"AnimationGotoNextFrame"
/// This action toggles animation playback
#define ACTION_TOGGLE_ANIMATION_PLAYBACK	"AnimationTogglePlayback"
/// This action starts the animation playback
#define ACTION_START_ANIMATION_PLAYBACK		"AnimationStartPlayback"
/// This action starts the animation playback
#define ACTION_STOP_ANIMATION_PLAYBACK		"AnimationStopPlayback"
/// This action shows the animation settings dialog
#define ACTION_ANIMATION_SETTINGS			"AnimationSettings"
/// This action activates/deactivates the animation mode
#define ACTION_AUTO_KEY_MODE_TOGGLE		"AnimationToggleRecording"

/// This action starts rendering of the current view.
#define ACTION_RENDER_ACTIVE_VIEWPORT		"RenderActiveViewport"
/// This action displays the frame buffer windows showing the last rendered image.
#define ACTION_SHOW_FRAME_BUFFER			"ShowFrameBuffer"

/// This actions open the application's "Settings" dialog.
#define ACTION_SETTINGS_DIALOG				"Settings"

/// This actions activates the scene node selection mode.
#define ACTION_SELECTION_MODE				"SelectionMode"
/// This actions activates the scene node translation mode.
#define ACTION_XFORM_MOVE_MODE				"XFormMoveMode"
/// This actions activates the scene node rotation mode.
#define ACTION_XFORM_ROTATE_MODE			"XFormRotateMode"

/**
 * \brief Manages the application's actions.
 */
class OVITO_CORE_EXPORT ActionManager : public QObject
{
	Q_OBJECT

public:

	/// Constructor.
	ActionManager(MainWindow* mainWindow);

	/// Returns the associated main window.
	MainWindow* mainWindow() const { return reinterpret_cast<MainWindow*>(parent()); }

	/// \brief Returns the action with the given ID or NULL.
	/// \param actionId The identifier string of the action to return.
	QAction* findAction(const QString& actionId) const {
		return findChild<QAction*>(actionId);
	}

	/// \brief Returns the action with the given ID.
	/// \param actionId The unique identifier string of the action to return.
	QAction* getAction(const QString& actionId) const {
		QAction* action = findAction(actionId);
		OVITO_ASSERT_MSG(action != nullptr, "ActionManager::getAction()", "Action does not exist.");
		return action;
	}

	/// \brief Invokes the command action with the given ID.
	/// \param actionId The unique identifier string of the action to invoke.
	/// \throw Exception if the action does not exist or if an error occurs during the action invocation.
	void invokeAction(const QString& actionId);

	/// \brief Registers a new action with the ActionManager.
	/// \param action The action to be registered. The ActionManager will take ownership of the object.
	void addAction(QAction* action);

	/// \brief Creates and registers a new command action with the ActionManager.
	QAction* createCommandAction(const QString& id,
						const QString& title,
						const char* iconPath = nullptr,
						const QString& statusTip = QString(),
						const QKeySequence& shortcut = QKeySequence());

	/// \brief Creates and registers a new action with the ActionManager.
	QAction* createViewportModeAction(const QString& id,
						ViewportInputMode* inputHandler,
						const QString& title,
						const char* iconPath = nullptr,
						const QString& statusTip = QString(),
						const QKeySequence& shortcut = QKeySequence());

private Q_SLOTS:

	/// This is called when a new dataset has been loaded.
	void onDataSetChanged(DataSet* newDataSet);

	/// This is called when new animation settings have been loaded.
	void onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings);

	/// This is called when the active animation interval has changed.
	void onAnimationIntervalChanged(TimeInterval newAnimationInterval);

	void on_Quit_triggered();
	void on_HelpAbout_triggered();
	void on_HelpOpenGLInfo_triggered();
	void on_HelpShowOnlineHelp_triggered();
	void on_FileNew_triggered();
	void on_FileOpen_triggered();
	void on_FileSave_triggered();
	void on_FileSaveAs_triggered();
	void on_FileImport_triggered();
	void on_FileRemoteImport_triggered();
	void on_FileExport_triggered();
	void on_FileNewWindow_triggered();
	void on_ViewportMaximize_triggered();
	void on_ViewportZoomSceneExtents_triggered();
	void on_ViewportZoomSelectionExtents_triggered();
	void on_ViewportZoomSceneExtentsAll_triggered();
	void on_ViewportZoomSelectionExtentsAll_triggered();
	void on_Settings_triggered();
	void on_AnimationGotoStart_triggered();
	void on_AnimationGotoEnd_triggered();
	void on_AnimationGotoPreviousFrame_triggered();
	void on_AnimationGotoNextFrame_triggered();
	void on_AnimationStartPlayback_triggered();
	void on_AnimationStopPlayback_triggered();
	void on_AnimationSettings_triggered();
	void on_RenderActiveViewport_triggered();
	void on_EditDelete_triggered();

private:

	QMetaObject::Connection _canUndoChangedConnection;
	QMetaObject::Connection _canRedoChangedConnection;
	QMetaObject::Connection _undoTextChangedConnection;
	QMetaObject::Connection _redoTextChangedConnection;
	QMetaObject::Connection _undoTriggeredConnection;
	QMetaObject::Connection _redoTriggeredConnection;
	QMetaObject::Connection _autoKeyModeChangedConnection;
	QMetaObject::Connection _autoKeyModeToggledConnection;
	QMetaObject::Connection _animationIntervalChangedConnection;

	/// The current dataset being edited in the main window.
	OORef<DataSet> _dataset;

	friend class AnimationPlaybackViewportMode;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_ACTION_MANAGER_H
