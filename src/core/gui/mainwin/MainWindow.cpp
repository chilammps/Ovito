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
#include "MainWindow.h"
#include "AnimationTimeSlider.h"
#include "ViewportsPanel.h"
#include <core/gui/actions/ActionManager.h>

namespace Ovito {

/// The global instance of this window class.
MainWindow* MainWindow::_instance = NULL;

/******************************************************************************
* The constructor of the main window class.
******************************************************************************/
MainWindow::MainWindow(const QString& title) :
	QMainWindow()
{
	OVITO_ASSERT_MSG(_instance == NULL, "MainWindow constructor", "Only one main window should be created.");
	_instance = this;

	setWindowTitle(title);
	setAttribute(Qt::WA_DeleteOnClose);

	// Create the main menu
	createMainMenu();

	// Create the main toolbar.
	createMainToolbar();

	// Setup the layout of docking widgets.
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

	ViewportsPanel* viewportsPanel = new ViewportsPanel(this);
	setCentralWidget(viewportsPanel);

	// Create the animation panel below the viewports.
	QWidget* animationPanel = new QWidget();
	QVBoxLayout* animationPanelLayout = new QVBoxLayout(animationPanel);
	animationPanelLayout->setSpacing(0);
	animationPanelLayout->setContentsMargins(0, 2, 0, 0);
	animationPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	// Create animation time slider
	AnimationTimeSlider* timeSlider = new AnimationTimeSlider();
	timeSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	animationPanelLayout->addWidget(timeSlider);
	animationPanelLayout->addStretch(1);

	// Create status bar.
	_statusBar = new QStatusBar(animationPanel);
	_statusBar->setSizeGripEnabled(false);
	_statusBar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
	setStatusBar(_statusBar);
	animationPanelLayout->addWidget(_statusBar);

	QDockWidget* animationPanelDockWidget = new QDockWidget(tr("Animation Panel"), this);
	animationPanelDockWidget->setObjectName("AnimationPanel");
	animationPanelDockWidget->setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
	animationPanelDockWidget->setFeatures(QDockWidget::DockWidgetClosable);
	animationPanelDockWidget->setWidget(animationPanel);
	animationPanelDockWidget->setTitleBarWidget(new QWidget());
	addDockWidget(Qt::BottomDockWidgetArea, animationPanelDockWidget);

   // Create the viewport control toolbar.
	QToolBar* viewportControlBar1 = new QToolBar();
	//viewportControlBar1->addAction(ActionManager::instance().getAction(ACTION_VIEWPORT_ZOOM));
	//viewportControlBar1->addAction(ActionManager::instance().getAction(ACTION_VIEWPORT_PAN));
	//viewportControlBar1->addAction(ActionManager::instance().getAction(ACTION_VIEWPORT_ORBIT));
	//viewportControlBar1->addAction(ActionManager::instance().getAction(ACTION_VIEWPORT_PICK_ORBIT_CENTER));
	viewportControlBar1->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; } QToolButton { padding: 0px; margin: 0px }");
	QToolBar* viewportControlBar2 = new QToolBar();
	//viewportControlBar2->addAction(ActionManager::instance().getAction(ACTION_VIEWPORT_ZOOM_SCENE_EXTENTS));
	//viewportControlBar2->addAction(ActionManager::instance().getAction(ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS));
	//viewportControlBar2->addAction(ActionManager::instance().getAction(ACTION_VIEWPORT_FOV));
	viewportControlBar2->addAction(ActionManager::instance().getAction(ACTION_VIEWPORT_MAXIMIZE));
	viewportControlBar2->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; } QToolButton { padding: 0px; margin: 0px }");
	QWidget* viewportControlPanel = new QWidget();
	QVBoxLayout* viewportControlPanelLayout = new QVBoxLayout(viewportControlPanel);
	viewportControlPanelLayout->setSpacing(0);
	viewportControlPanelLayout->setContentsMargins(0, 1, 0, 0);
	viewportControlPanelLayout->addWidget(viewportControlBar1);
	viewportControlPanelLayout->addWidget(viewportControlBar2);
	viewportControlPanelLayout->addStretch(1);
	viewportControlPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	QDockWidget* viewportControlDockWidget = new QDockWidget(tr("Viewport Control"), this);
	viewportControlDockWidget->setObjectName("ViewportControlPanel");
	viewportControlDockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
	viewportControlDockWidget->setFeatures(QDockWidget::DockWidgetClosable);
	viewportControlDockWidget->setWidget(viewportControlPanel);
	viewportControlDockWidget->setTitleBarWidget(new QWidget());
	addDockWidget(Qt::BottomDockWidgetArea, viewportControlDockWidget);
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
	fileMenu->addAction(ActionManager::instance().getAction(ACTION_FILE_IMPORT));
	fileMenu->addAction(ActionManager::instance().getAction(ACTION_FILE_EXPORT));
	fileMenu->addSeparator();
	fileMenu->addAction(ActionManager::instance().getAction(ACTION_FILE_OPEN));
	fileMenu->addAction(ActionManager::instance().getAction(ACTION_FILE_SAVE));
	fileMenu->addAction(ActionManager::instance().getAction(ACTION_FILE_SAVEAS));
	fileMenu->addSeparator();
	fileMenu->addAction(ActionManager::instance().getAction(ACTION_QUIT));

	// Build the edit menu.
	QMenu* editMenu = menuBar->addMenu(tr("&Edit"));
	editMenu->addAction(ActionManager::instance().getAction(ACTION_EDIT_UNDO));
	editMenu->addAction(ActionManager::instance().getAction(ACTION_EDIT_REDO));

	// Build the options menu.
	QMenu* optionsMenu = menuBar->addMenu(tr("&Options"));
	optionsMenu->addAction(ActionManager::instance().getAction(ACTION_SETTINGS_DIALOG));

	// Build the help menu.
	QMenu* helpMenu = menuBar->addMenu(tr("&Help"));
	helpMenu->addAction(ActionManager::instance().getAction(ACTION_HELP_SHOW_ONLINE_HELP));
	helpMenu->addSeparator();
	helpMenu->addAction(ActionManager::instance().getAction(ACTION_HELP_ABOUT));

	setMenuBar(menuBar);
}

/******************************************************************************
* Creates the main toolbar.
******************************************************************************/
void MainWindow::createMainToolbar()
{
	_mainToolbar = addToolBar(tr("Main Toolbar"));
	_mainToolbar->setObjectName("MainToolbar");

	_mainToolbar->addAction(ActionManager::instance().getAction(ACTION_FILE_OPEN));
	_mainToolbar->addAction(ActionManager::instance().getAction(ACTION_FILE_SAVE));

	_mainToolbar->addSeparator();

	_mainToolbar->addAction(ActionManager::instance().getAction(ACTION_FILE_IMPORT));
	_mainToolbar->addAction(ActionManager::instance().getAction(ACTION_FILE_EXPORT));

	_mainToolbar->addSeparator();

	_mainToolbar->addAction(ActionManager::instance().getAction(ACTION_EDIT_UNDO));
	_mainToolbar->addAction(ActionManager::instance().getAction(ACTION_EDIT_REDO));
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
#if 0
	// Save changes.
	if(!DATASET_MANAGER.askForSaveChanges()) {
		event->ignore();
		return;
	}

	// Close current scene file.
	DATASET_MANAGER.setCurrentSet(new DataSet());
#endif

	// Save window layout.
	saveLayout();

	// Destroy main window.
	event->accept();
}

};
