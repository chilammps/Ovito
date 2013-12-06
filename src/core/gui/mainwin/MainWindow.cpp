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
#include <core/dataset/DataSetContainer.h>
#include <core/gui/actions/ActionManager.h>
#include <core/gui/widgets/animation/AnimationTimeSpinner.h>
#include <core/gui/widgets/animation/AnimationFramesToolButton.h>
#include <core/gui/widgets/animation/AnimationTimeSlider.h>
#include <core/gui/widgets/rendering/FrameBufferWindow.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/input/ViewportInputManager.h>
#include "MainWindow.h"
#include "ViewportsPanel.h"
#include "cmdpanel/CommandPanel.h"

namespace Ovito {

/******************************************************************************
* The constructor of the main window class.
******************************************************************************/
MainWindow::MainWindow() :
		_datasetContainer(this)
{
	setWindowTitle(tr("Ovito (Open Visualization Tool)"));
	setAttribute(Qt::WA_DeleteOnClose);

	// Setup the layout of docking widgets.
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

	// Create input manager.
	_viewportInputManager = new ViewportInputManager(this);

	// Create actions.
	_actionManager = new ActionManager(this);

	// Create the main menu
	createMainMenu();

	// Create the main toolbar.
	createMainToolbar();

	ViewportsPanel* viewportsPanel = new ViewportsPanel(this);
	setCentralWidget(viewportsPanel);

	// Create the animation panel below the viewports.
	QWidget* animationPanel = new QWidget();
	QVBoxLayout* animationPanelLayout = new QVBoxLayout(animationPanel);
	animationPanelLayout->setSpacing(0);
	animationPanelLayout->setContentsMargins(0, 0, 0, 0);
	animationPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	// Create animation time slider
	AnimationTimeSlider* timeSlider = new AnimationTimeSlider(this);
	timeSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	animationPanelLayout->addWidget(timeSlider);

	// Create status bar.
	_statusBar = new QStatusBar(animationPanel);
	_statusBar->setSizeGripEnabled(false);
	_statusBar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
	setStatusBar(_statusBar);
	animationPanelLayout->addWidget(_statusBar, 1);

	// Create the animation control toolbar.
	QToolBar* animationControlBar1 = new QToolBar();
	animationControlBar1->addAction(actionManager()->getAction(ACTION_GOTO_START_OF_ANIMATION));
	animationControlBar1->addSeparator();
	animationControlBar1->addAction(actionManager()->getAction(ACTION_GOTO_PREVIOUS_FRAME));
	animationControlBar1->addAction(actionManager()->getAction(ACTION_TOGGLE_ANIMATION_PLAYBACK));
	animationControlBar1->addAction(actionManager()->getAction(ACTION_GOTO_NEXT_FRAME));
	animationControlBar1->addSeparator();
	animationControlBar1->addAction(actionManager()->getAction(ACTION_GOTO_END_OF_ANIMATION));
	animationControlBar1->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; } QToolButton { padding: 0px; margin: 0px }");
	QToolBar* animationControlBar2 = new QToolBar();
#if 0
	animationControlBar2->addAction(actionManager()->getAction(ACTION_AUTO_KEY_MODE_TOGGLE));
#else
	animationControlBar2->addWidget(new AnimationFramesToolButton(datasetContainer()));
#endif
	class TimeEditBox : public QLineEdit {
	public:
		virtual QSize sizeHint() const { return minimumSizeHint(); }
	};
	QLineEdit* timeEditBox = new TimeEditBox();
	timeEditBox->setToolTip(tr("Current Animation Time"));
	AnimationTimeSpinner* currentTimeSpinner = new AnimationTimeSpinner(this);
	currentTimeSpinner->setTextBox(timeEditBox);
	animationControlBar2->addWidget(timeEditBox);
	animationControlBar2->addWidget(currentTimeSpinner);
	animationControlBar2->addAction(actionManager()->getAction(ACTION_ANIMATION_SETTINGS));
	animationControlBar2->addWidget(new QWidget());
	animationControlBar2->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; } QToolButton { padding: 0px; margin: 0px }");

	QWidget* animationControlPanel = new QWidget();
	QVBoxLayout* animationControlPanelLayout = new QVBoxLayout(animationControlPanel);
	animationControlPanelLayout->setSpacing(0);
	animationControlPanelLayout->setContentsMargins(0, 1, 0, 0);
	animationControlPanelLayout->addWidget(animationControlBar1);
	animationControlPanelLayout->addWidget(animationControlBar2);
	animationControlPanelLayout->addStretch(1);
	animationControlPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	// Create the viewport control toolbar.
	QToolBar* viewportControlBar1 = new QToolBar();
	viewportControlBar1->addAction(actionManager()->getAction(ACTION_VIEWPORT_ZOOM));
	viewportControlBar1->addAction(actionManager()->getAction(ACTION_VIEWPORT_PAN));
	viewportControlBar1->addAction(actionManager()->getAction(ACTION_VIEWPORT_ORBIT));
	viewportControlBar1->addAction(actionManager()->getAction(ACTION_VIEWPORT_PICK_ORBIT_CENTER));
	viewportControlBar1->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; } QToolButton { padding: 0px; margin: 0px }");
	QToolBar* viewportControlBar2 = new QToolBar();
	viewportControlBar2->addAction(actionManager()->getAction(ACTION_VIEWPORT_ZOOM_SCENE_EXTENTS));
	viewportControlBar2->addAction(actionManager()->getAction(ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS));
	viewportControlBar2->addAction(actionManager()->getAction(ACTION_VIEWPORT_FOV));
	viewportControlBar2->addAction(actionManager()->getAction(ACTION_VIEWPORT_MAXIMIZE));
	viewportControlBar2->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; } QToolButton { padding: 0px; margin: 0px }");
	QWidget* viewportControlPanel = new QWidget();
	QVBoxLayout* viewportControlPanelLayout = new QVBoxLayout(viewportControlPanel);
	viewportControlPanelLayout->setSpacing(0);
	viewportControlPanelLayout->setContentsMargins(0, 1, 0, 0);
	viewportControlPanelLayout->addWidget(viewportControlBar1);
	QHBoxLayout* sublayout = new QHBoxLayout();
	sublayout->addStretch(1);
	sublayout->addWidget(viewportControlBar2);
	viewportControlPanelLayout->addLayout(sublayout);
	viewportControlPanelLayout->addStretch(1);
	viewportControlPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	// Create the command panel.
	_commandPanel = new CommandPanel(this, this);

	createDockPanel(tr("Animation Panel"), "AnimationPanel", Qt::BottomDockWidgetArea, Qt::BottomDockWidgetArea, animationPanel);
	createDockPanel(tr("Animation Control Panel"), "AnimationControlPanel", Qt::BottomDockWidgetArea, Qt::BottomDockWidgetArea, animationControlPanel);
	createDockPanel(tr("Viewport Control"), "ViewportControlPanel", Qt::BottomDockWidgetArea, Qt::BottomDockWidgetArea, viewportControlPanel);
	createDockPanel(tr("Command Panel"), "CommandPanel", Qt::RightDockWidgetArea, Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea), _commandPanel);

	// Create the frame buffer window.
	_frameBufferWindow = new FrameBufferWindow(this);
}

/******************************************************************************
* Creates a dock panel.
******************************************************************************/
void MainWindow::createDockPanel(const QString& caption, const QString& objectName, Qt::DockWidgetArea dockArea, Qt::DockWidgetAreas allowedAreas, QWidget* contents)
{
	QDockWidget* dockWidget = new QDockWidget(caption, this);
	dockWidget->setObjectName(objectName);
	dockWidget->setAllowedAreas(allowedAreas);
	dockWidget->setFeatures(QDockWidget::DockWidgetClosable);
	dockWidget->setWidget(contents);
	dockWidget->setTitleBarWidget(new QWidget());
	addDockWidget(dockArea, dockWidget);
}

/******************************************************************************
* Loads the layout of the docked widgets from the settings store.
******************************************************************************/
void MainWindow::restoreLayout()
{
	QSettings settings;
	settings.beginGroup("app/mainwindow");
	QVariant state = settings.value("state");
	if(state.canConvert<QByteArray>())
		restoreState(state.toByteArray());
}

/******************************************************************************
* Saves the layout of the docked widgets to the settings store.
******************************************************************************/
void MainWindow::saveLayout()
{
	QSettings settings;
	settings.beginGroup("app/mainwindow");
	settings.setValue("state", saveState());
}

/******************************************************************************
* Creates the main menu.
******************************************************************************/
void MainWindow::createMainMenu()
{
	QMenuBar* menuBar = new QMenuBar(this);

	// Build the file menu.
	QMenu* fileMenu = menuBar->addMenu(tr("&File"));
	fileMenu->addAction(actionManager()->getAction(ACTION_FILE_NEW_WINDOW));
	fileMenu->addAction(actionManager()->getAction(ACTION_FILE_IMPORT));
	fileMenu->addAction(actionManager()->getAction(ACTION_FILE_REMOTE_IMPORT));
	fileMenu->addAction(actionManager()->getAction(ACTION_FILE_EXPORT));
	fileMenu->addSeparator();
	fileMenu->addAction(actionManager()->getAction(ACTION_FILE_OPEN));
	fileMenu->addAction(actionManager()->getAction(ACTION_FILE_SAVE));
	fileMenu->addAction(actionManager()->getAction(ACTION_FILE_SAVEAS));
	fileMenu->addSeparator();
	fileMenu->addAction(actionManager()->getAction(ACTION_QUIT));

	// Build the edit menu.
	QMenu* editMenu = menuBar->addMenu(tr("&Edit"));
	editMenu->addAction(actionManager()->getAction(ACTION_EDIT_UNDO));
	editMenu->addAction(actionManager()->getAction(ACTION_EDIT_REDO));
	editMenu->addSeparator();
	editMenu->addAction(actionManager()->getAction(ACTION_EDIT_DELETE));

	// Build the options menu.
	QMenu* optionsMenu = menuBar->addMenu(tr("&Options"));
	optionsMenu->addAction(actionManager()->getAction(ACTION_SETTINGS_DIALOG));

	// Build the help menu.
	QMenu* helpMenu = menuBar->addMenu(tr("&Help"));
	helpMenu->addAction(actionManager()->getAction(ACTION_HELP_SHOW_ONLINE_HELP));
	helpMenu->addSeparator();
	helpMenu->addAction(actionManager()->getAction(ACTION_HELP_ABOUT));

	setMenuBar(menuBar);
}

/******************************************************************************
* Creates the main toolbar.
******************************************************************************/
void MainWindow::createMainToolbar()
{
	_mainToolbar = addToolBar(tr("Main Toolbar"));
	_mainToolbar->setObjectName("MainToolbar");

	_mainToolbar->addAction(actionManager()->getAction(ACTION_FILE_IMPORT));
	_mainToolbar->addAction(actionManager()->getAction(ACTION_FILE_REMOTE_IMPORT));
	_mainToolbar->addAction(actionManager()->getAction(ACTION_FILE_EXPORT));

	_mainToolbar->addSeparator();

	_mainToolbar->addAction(actionManager()->getAction(ACTION_FILE_OPEN));
	_mainToolbar->addAction(actionManager()->getAction(ACTION_FILE_SAVE));

	_mainToolbar->addSeparator();

	_mainToolbar->addAction(actionManager()->getAction(ACTION_EDIT_UNDO));
	_mainToolbar->addAction(actionManager()->getAction(ACTION_EDIT_REDO));

	_mainToolbar->addSeparator();

	_mainToolbar->addAction(actionManager()->getAction(ACTION_RENDER_ACTIVE_VIEWPORT));
}

/******************************************************************************
* Is called when the window receives an event.
******************************************************************************/
bool MainWindow::event(QEvent* event)
{
	if(event->type() == QEvent::StatusTip) {
		statusBar()->showMessage(static_cast<QStatusTipEvent*>(event)->tip());
		return true;
	}
	return QMainWindow::event(event);
}

/******************************************************************************
* Is called when the user closes the window.
******************************************************************************/
void MainWindow::closeEvent(QCloseEvent* event)
{
	try {
		// Save changes.
		if(!datasetContainer().askForSaveChanges()) {
			event->ignore();
			return;
		}

		// Save window layout.
		saveLayout();

		// Destroy main window.
		event->accept();
	}
	catch(const Exception& ex) {
		event->ignore();
		ex.showError();
	}
}

/******************************************************************************
* Immediately repaints all viewports that are flagged for an update.
******************************************************************************/
void MainWindow::processViewportUpdates()
{
	datasetContainer().currentSet()->viewportConfig()->processViewportUpdates();
}

};
