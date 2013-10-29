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
#include <core/scene/objects/camera/AbstractCameraObject.h>
#include <core/scene/objects/camera/CameraObject.h>
#include <core/dataset/DataSetManager.h>
#include <core/dataset/DataSet.h>
#include <core/gui/dialogs/AdjustCameraDialog.h>
#include <core/gui/mainwin/MainWindow.h>
#include "ViewportMenu.h"

namespace Ovito {

/******************************************************************************
* Initializes the menu.
******************************************************************************/
ViewportMenu::ViewportMenu(Viewport* vp) : _viewport(vp)
{
	QAction* action;

	// Build menu.
	action = addAction(tr("Show Render Frame"), this, SLOT(onShowRenderFrame(bool)));
	action->setCheckable(true);
	action->setChecked(_viewport->renderFrameShown());
	addSeparator();

	_viewTypeMenu = addMenu(tr("View type"));
	connect(_viewTypeMenu, SIGNAL(aboutToShow()), this, SLOT(onShowViewTypeMenu()));

	QActionGroup* viewTypeGroup = new QActionGroup(this);
	action = viewTypeGroup->addAction(tr("Top"));
	action->setCheckable(true);
	action->setChecked(_viewport->viewType() == Viewport::VIEW_TOP);
	action->setData((int)Viewport::VIEW_TOP);
	action = viewTypeGroup->addAction(tr("Bottom"));
	action->setCheckable(true);
	action->setChecked(_viewport->viewType() == Viewport::VIEW_BOTTOM);
	action->setData((int)Viewport::VIEW_BOTTOM);
	action = viewTypeGroup->addAction(tr("Front"));
	action->setCheckable(true);
	action->setChecked(_viewport->viewType() == Viewport::VIEW_FRONT);
	action->setData((int)Viewport::VIEW_FRONT);
	action = viewTypeGroup->addAction(tr("Back"));
	action->setCheckable(true);
	action->setChecked(_viewport->viewType() == Viewport::VIEW_BACK);
	action->setData((int)Viewport::VIEW_BACK);
	action = viewTypeGroup->addAction(tr("Left"));
	action->setCheckable(true);
	action->setChecked(_viewport->viewType() == Viewport::VIEW_LEFT);
	action->setData((int)Viewport::VIEW_LEFT);
	action = viewTypeGroup->addAction(tr("Right"));
	action->setCheckable(true);
	action->setChecked(_viewport->viewType() == Viewport::VIEW_RIGHT);
	action->setData((int)Viewport::VIEW_RIGHT);
	action = viewTypeGroup->addAction(tr("Ortho"));
	action->setCheckable(true);
	action->setChecked(_viewport->viewType() == Viewport::VIEW_ORTHO);
	action->setData((int)Viewport::VIEW_ORTHO);
	action = viewTypeGroup->addAction(tr("Perspective"));
	action->setCheckable(true);
	action->setChecked(_viewport->viewType() == Viewport::VIEW_PERSPECTIVE);
	action->setData((int)Viewport::VIEW_PERSPECTIVE);
	_viewTypeMenu->addActions(viewTypeGroup->actions());
	connect(viewTypeGroup, SIGNAL(triggered(QAction*)), this, SLOT(onViewType(QAction*)));

	addSeparator();
	addAction(tr("Adjust View..."), this, SLOT(onAdjustView()));

	//connect(QGuiApplication::instance(), SIGNAL(focusWindowChanged(QWindow*)), this, SLOT(onWindowFocusChanged()));
}

/******************************************************************************
* Displays the menu.
******************************************************************************/
void ViewportMenu::show(const QPoint& pos)
{
	exec(_viewport->widget()->mapToGlobal(pos));
}

/******************************************************************************
* Is called just before the "View Type" sub-menu is shown.
******************************************************************************/
void ViewportMenu::onShowViewTypeMenu()
{
	QActionGroup* viewNodeGroup = new QActionGroup(this);
	connect(viewNodeGroup, SIGNAL(triggered(QAction*)), this, SLOT(onViewNode(QAction*)));

	// Find all camera nodes in the scene.
	DataSetManager::instance().currentSet()->sceneRoot()->visitObjectNodes([this, viewNodeGroup](ObjectNode* node) -> bool {
		PipelineFlowState state = node->evalPipeline(AnimManager::instance().time());
		OORef<AbstractCameraObject> camera = state.convertObject<AbstractCameraObject>(AnimManager::instance().time());
		if(camera) {
			// Add a menu entry for this camera node.
			QAction* action = viewNodeGroup->addAction(node->name());
			action->setCheckable(true);
			action->setChecked(_viewport->viewNode() == node);
			action->setData(qVariantFromValue((void*)node));
		}
		return true;
	});

	// Add menu entries to menu.
	if(viewNodeGroup->actions().isEmpty() == false) {
		_viewTypeMenu->addSeparator();
		_viewTypeMenu->addActions(viewNodeGroup->actions());
	}

	_viewTypeMenu->addSeparator();
	_viewTypeMenu->addAction(tr("Create new camera"), this, SLOT(onCreateCamera()))->setEnabled(_viewport->viewNode() == nullptr);

	disconnect(_viewTypeMenu, SIGNAL(aboutToShow()), this, SLOT(onShowViewTypeMenu()));
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onShowGrid(bool checked)
{
	_viewport->setGridShown(checked);
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onShowRenderFrame(bool checked)
{
	_viewport->setRenderFrameShown(checked);
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onViewType(QAction* action)
{
	_viewport->setViewType((Viewport::ViewType)action->data().toInt());
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onAdjustView()
{
	AdjustCameraDialog dialog(_viewport, &MainWindow::instance());
	dialog.exec();
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onViewNode(QAction* action)
{
	ObjectNode* viewNode = static_cast<ObjectNode*>(action->data().value<void*>());
	OVITO_CHECK_OBJECT_POINTER(viewNode);

	_viewport->setViewType(Viewport::VIEW_SCENENODE);
	_viewport->setViewNode(viewNode);
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onCreateCamera()
{
	UndoableTransaction::handleExceptions(tr("Create camera"), [this]() {
		SceneRoot* scene = DataSetManager::instance().currentSet()->sceneRoot();
		AnimationSuspender animSuspender;

		// Create and initialize the camera object.
		OORef<CameraObject> cameraObj;
		OORef<ObjectNode> cameraNode;
		{
			UndoSuspender noUndo;
			cameraObj = new CameraObject();
			cameraObj->setIsPerspective(_viewport->isPerspectiveProjection());
			if(_viewport->isPerspectiveProjection())
				cameraObj->fovController()->setValue(0, _viewport->fieldOfView());
			else
				cameraObj->zoomController()->setValue(0, _viewport->fieldOfView());

			// Create an object node to insert camera object into scene.
			cameraNode = new ObjectNode();
			cameraNode->setSceneObject(cameraObj);

			// Give the new node a name.
			cameraNode->setName(scene->makeNameUnique(tr("Camera")));

			// Position camera node to match the current view.
			cameraNode->transformationController()->setValue(0, _viewport->inverseViewMatrix());
		}

		// Insert node into scene.
		scene->addChild(cameraNode);

		// Select new camera node.
		DataSetManager::instance().currentSet()->selection()->setNode(cameraNode.get());

		// Set new camera as view node for current viewport.
		_viewport->setViewType(Viewport::VIEW_SCENENODE);
		_viewport->setViewNode(cameraNode.get());
	});
}

};
