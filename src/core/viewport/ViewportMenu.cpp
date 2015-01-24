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
#include <core/dataset/DataSet.h>
#include <core/animation/AnimationSettings.h>
#include <core/gui/dialogs/AdjustCameraDialog.h>
#include <core/gui/mainwin/MainWindow.h>
#include "ViewportMenu.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Initializes the menu.
******************************************************************************/
ViewportMenu::ViewportMenu(Viewport* vp) : QMenu(vp->widget()), _viewport(vp)
{
	QAction* action;

	// Build menu.
	action = addAction(tr("Render Preview"), this, SLOT(onRenderPreviewMode(bool)));
	action->setCheckable(true);
	action->setChecked(_viewport->renderPreviewMode());
	action = addAction(tr("Show Grid"), this, SLOT(onShowGrid(bool)));
	action->setCheckable(true);
	action->setChecked(_viewport->isGridVisible());
#ifdef OVITO_DEBUG
	action = addAction(tr("Stereoscopic Mode (anaglyphs)"), this, SLOT(onStereoscopicMode(bool)));
	action->setCheckable(true);
	action->setChecked(_viewport->stereoscopicMode());
#endif
	addSeparator();

	_viewTypeMenu = addMenu(tr("View Type"));
	connect(_viewTypeMenu, &QMenu::aboutToShow, this, &ViewportMenu::onShowViewTypeMenu);

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
	connect(viewTypeGroup, &QActionGroup::triggered, this, &ViewportMenu::onViewType);

	addSeparator();
	addAction(tr("Adjust View..."), this, SLOT(onAdjustView()))->setEnabled(_viewport->viewType() != Viewport::VIEW_SCENENODE);

#ifdef Q_OS_MACX
	connect(static_cast<QGuiApplication*>(QGuiApplication::instance()), &QGuiApplication::focusWindowChanged, this, &ViewportMenu::onWindowFocusChanged);
#endif
}

/******************************************************************************
* Displays the menu.
******************************************************************************/
void ViewportMenu::show(const QPoint& pos)
{
	// Make sure deleteLater() calls are executed first.
	QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

	// Show context menu.
	exec(_viewport->widget()->mapToGlobal(pos));
}

/******************************************************************************
* Is called just before the "View Type" sub-menu is shown.
******************************************************************************/
void ViewportMenu::onShowViewTypeMenu()
{
	QActionGroup* viewNodeGroup = new QActionGroup(this);
	connect(viewNodeGroup, &QActionGroup::triggered, this, &ViewportMenu::onViewNode);

	// Find all camera nodes in the scene.
	_viewport->dataset()->sceneRoot()->visitObjectNodes([this, viewNodeGroup](ObjectNode* node) -> bool {
		PipelineFlowState state = node->evalPipeline(_viewport->dataset()->animationSettings()->time());
		OORef<AbstractCameraObject> camera = state.convertObject<AbstractCameraObject>(_viewport->dataset()->animationSettings()->time());
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
	_viewTypeMenu->addAction(tr("Create Camera"), this, SLOT(onCreateCamera()))->setEnabled(_viewport->viewNode() == nullptr);

	disconnect(_viewTypeMenu, &QMenu::aboutToShow, this, &ViewportMenu::onShowViewTypeMenu);
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onRenderPreviewMode(bool checked)
{
	_viewport->setRenderPreviewMode(checked);
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onShowGrid(bool checked)
{
	_viewport->setGridVisible(checked);
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onStereoscopicMode(bool checked)
{
	_viewport->setStereoscopicMode(checked);
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
	AdjustCameraDialog dialog(_viewport, _viewport->dataset()->mainWindow());
	dialog.exec();
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onViewNode(QAction* action)
{
	ObjectNode* viewNode = static_cast<ObjectNode*>(action->data().value<void*>());
	OVITO_CHECK_OBJECT_POINTER(viewNode);

	UndoableTransaction::handleExceptions(_viewport->dataset()->undoStack(), tr("Set camera"), [this, viewNode]() {
		_viewport->setViewType(Viewport::VIEW_SCENENODE);
		_viewport->setViewNode(viewNode);
	});
}

/******************************************************************************
* Handles the menu item event.
******************************************************************************/
void ViewportMenu::onCreateCamera()
{
	UndoableTransaction::handleExceptions(_viewport->dataset()->undoStack(), tr("Create camera"), [this]() {
		SceneRoot* scene = _viewport->dataset()->sceneRoot();
		AnimationSuspender animSuspender(_viewport->dataset()->animationSettings());

		// Create and initialize the camera object.
		OORef<CameraObject> cameraObj;
		OORef<ObjectNode> cameraNode;
		{
			UndoSuspender noUndo(_viewport->dataset()->undoStack());
			cameraObj = new CameraObject(_viewport->dataset());
			cameraObj->setIsPerspective(_viewport->isPerspectiveProjection());
			if(_viewport->isPerspectiveProjection())
				cameraObj->fovController()->setFloatValue(0, _viewport->fieldOfView());
			else
				cameraObj->zoomController()->setFloatValue(0, _viewport->fieldOfView());

			// Create an object node for the camera.
			cameraNode = new ObjectNode(_viewport->dataset());
			cameraNode->setDataProvider(cameraObj);

			// Give the new node a name.
			cameraNode->setName(scene->makeNameUnique(tr("Camera")));

			// Position camera node to match the current view.
			AffineTransformation tm = _viewport->inverseViewMatrix();
			if(_viewport->isPerspectiveProjection() == false) {
				// Position camera with parallel projection outside of scene bounding box.
				tm = tm * AffineTransformation::translation(
						Vector3(0, 0, -_viewport->_projParams.znear + 0.2f * (_viewport->_projParams.zfar-_viewport->_projParams.znear)));
			}
			cameraNode->transformationController()->setTransformationValue(0, tm, true);
		}

		// Insert node into scene.
		scene->addChild(cameraNode);

		// Set new camera as view node for current viewport.
		_viewport->setViewType(Viewport::VIEW_SCENENODE);
		_viewport->setViewNode(cameraNode);
	});
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
