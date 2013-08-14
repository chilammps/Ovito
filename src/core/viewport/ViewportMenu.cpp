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
#include <core/scene/SceneRoot.h>
#include <core/scene/ObjectNode.h>
#include <core/dataset/DataSetManager.h>
#include <core/dataset/DataSet.h>
#include <core/gui/dialogs/AdjustCameraDialog.h>
#include <core/gui/mainwin/MainWindow.h>
#include "ViewportMenu.h"

namespace Ovito {

/******************************************************************************
* Initializes the menu.
******************************************************************************/
ViewportMenu::ViewportMenu(Viewport* vp) : viewport(vp)
{
	QAction* action;

	// Build menu.
	action = addAction(tr("Show Render Frame"), this, SLOT(onShowRenderFrame(bool)));
	action->setCheckable(true);
	action->setChecked(viewport->renderFrameShown());
	addSeparator();

	QActionGroup* viewTypeGroup = new QActionGroup(this);
	action = viewTypeGroup->addAction(tr("Top"));
	action->setCheckable(true);
	action->setChecked(viewport->viewType() == Viewport::VIEW_TOP);
	action->setData((int)Viewport::VIEW_TOP);
	action = viewTypeGroup->addAction(tr("Bottom"));
	action->setCheckable(true);
	action->setChecked(viewport->viewType() == Viewport::VIEW_BOTTOM);
	action->setData((int)Viewport::VIEW_BOTTOM);
	action = viewTypeGroup->addAction(tr("Front"));
	action->setCheckable(true);
	action->setChecked(viewport->viewType() == Viewport::VIEW_FRONT);
	action->setData((int)Viewport::VIEW_FRONT);
	action = viewTypeGroup->addAction(tr("Back"));
	action->setCheckable(true);
	action->setChecked(viewport->viewType() == Viewport::VIEW_BACK);
	action->setData((int)Viewport::VIEW_BACK);
	action = viewTypeGroup->addAction(tr("Left"));
	action->setCheckable(true);
	action->setChecked(viewport->viewType() == Viewport::VIEW_LEFT);
	action->setData((int)Viewport::VIEW_LEFT);
	action = viewTypeGroup->addAction(tr("Right"));
	action->setCheckable(true);
	action->setChecked(viewport->viewType() == Viewport::VIEW_RIGHT);
	action->setData((int)Viewport::VIEW_RIGHT);
	action = viewTypeGroup->addAction(tr("Ortho"));
	action->setCheckable(true);
	action->setChecked(viewport->viewType() == Viewport::VIEW_ORTHO);
	action->setData((int)Viewport::VIEW_ORTHO);
	action = viewTypeGroup->addAction(tr("Perspective"));
	action->setCheckable(true);
	action->setChecked(viewport->viewType() == Viewport::VIEW_PERSPECTIVE);
	action->setData((int)Viewport::VIEW_PERSPECTIVE);
	addActions(viewTypeGroup->actions());
	connect(viewTypeGroup, SIGNAL(triggered(QAction*)), this, SLOT(onViewType(QAction*)));

	addSeparator();
	addAction(tr("Adjust Camera"), this, SLOT(onAdjustCamera()));

	//connect(QGuiApplication::instance(), SIGNAL(focusWindowChanged(QWindow*)), this, SLOT(onWindowFocusChanged()));
}

/******************************************************************************
* Displays the menu.
******************************************************************************/
void ViewportMenu::show(const QPoint& pos)
{
	exec(viewport->widget()->mapToGlobal(pos));
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onShowGrid(bool checked)
{
	viewport->setGridShown(checked);
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onShowRenderFrame(bool checked)
{
	viewport->setRenderFrameShown(checked);
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onViewType(QAction* action)
{
	viewport->setViewType((Viewport::ViewType)action->data().toInt());
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onAdjustCamera()
{
	AdjustCameraDialog dialog(viewport, &MainWindow::instance());
	dialog.exec();
}

};
