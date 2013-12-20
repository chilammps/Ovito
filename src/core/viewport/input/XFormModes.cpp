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
#include <core/dataset/UndoStack.h>
#include <core/animation/AnimationSettings.h>
#include <core/scene/SelectionSet.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/viewport/input/XFormModes.h>

namespace Ovito {

/******************************************************************************
* Handles the mouse down event for the given viewport.
******************************************************************************/
void SelectionMode::mousePressEvent(Viewport* vp, QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton) {
		_viewport = vp;
		_clickPoint = event->localPos();
	}
	else if(event->button() == Qt::RightButton) {
		_viewport = nullptr;
	}
	ViewportInputMode::mousePressEvent(vp, event);
}

/******************************************************************************
* Handles the mouse up event for the given viewport.
******************************************************************************/
void SelectionMode::mouseReleaseEvent(Viewport* vp, QMouseEvent* event)
{
	if(_viewport != nullptr) {
		// Select object under mouse cursor.
		ViewportPickResult pickResult = vp->pick(_clickPoint);
		if(pickResult.valid && pickResult.objectNode) {
			_viewport->dataset()->undoStack().beginCompoundOperation(tr("Select"));
			_viewport->dataset()->selection()->setNode(pickResult.objectNode.get());
			_viewport->dataset()->undoStack().endCompoundOperation();
		}
		_viewport = nullptr;
	}
	ViewportInputMode::mouseReleaseEvent(vp, event);
}

/******************************************************************************
* This is called by the system after the input handler is
* no longer the active handler.
******************************************************************************/
void SelectionMode::deactivated(bool temporary)
{
	_viewport = nullptr;
	ViewportInputMode::deactivated(temporary);
}

/******************************************************************************
* Handles the mouse move event for the given viewport.
******************************************************************************/
void SelectionMode::mouseMoveEvent(Viewport* vp, QMouseEvent* event)
{
	// Change mouse cursor while hovering over an object.
	ViewportPickResult pickResult = vp->pick(event->localPos());
	setCursor(pickResult.valid ? _hoverCursor : QCursor());
	ViewportInputMode::mouseMoveEvent(vp, event);
}

/******************************************************************************
* This is called by the system after the input handler is
* no longer the active handler.
******************************************************************************/
void XFormMode::deactivated(bool temporary)
{
	if(_viewport) {
		// Restore old state if change has not been committed.
		_viewport->dataset()->undoStack().endCompoundOperation(false);
		_viewport->dataset()->undoStack().endCompoundOperation(false);
		_viewport = nullptr;
	}
	ViewportInputMode::deactivated(temporary);
}

/******************************************************************************
* Handles the mouse down event for the given viewport.
******************************************************************************/
void XFormMode::mousePressEvent(Viewport* vp, QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton) {
		if(_viewport == nullptr) {

			// Select object under mouse cursor.
			ViewportPickResult pickResult = vp->pick(event->localPos());
			if(pickResult.valid && pickResult.objectNode) {
				_viewport = vp;
				_startPoint = event->localPos();
				_viewport->dataset()->undoStack().beginCompoundOperation(undoDisplayName());
				_viewport->dataset()->selection()->setNode(pickResult.objectNode.get());
				_viewport->dataset()->undoStack().beginCompoundOperation(undoDisplayName());
				startXForm();
				return;
			}
		}
	}
	else if(event->button() == Qt::RightButton) {
		if(_viewport != nullptr) {
			// Restore old state when aborting the operation.
			_viewport->dataset()->undoStack().endCompoundOperation(false);
			_viewport->dataset()->undoStack().endCompoundOperation(false);
			_viewport = nullptr;
			return;
		}
	}
	ViewportInputMode::mousePressEvent(vp, event);
}

/******************************************************************************
* Handles the mouse up event for the given viewport.
******************************************************************************/
void XFormMode::mouseReleaseEvent(Viewport* vp, QMouseEvent* event)
{
	if(_viewport) {
		// Commit change.
		_viewport->dataset()->undoStack().endCompoundOperation();
		_viewport->dataset()->undoStack().endCompoundOperation();
		_viewport = nullptr;
	}
	ViewportInputMode::mouseReleaseEvent(vp, event);
}

/******************************************************************************
* Handles the mouse move event for the given viewport.
******************************************************************************/
void XFormMode::mouseMoveEvent(Viewport* vp, QMouseEvent* event)
{
	if(_viewport == vp) {
#if 1
		// Take the current mouse cursor position to make the input mode
		// look more responsive. The cursor position recorded when the mouse event was
		// generates may be too old.
		_currentPoint = vp->widget()->mapFromGlobal(QCursor::pos());
#else
		_currentPoint = event->localPos();
#endif

		vp->dataset()->undoStack().resetCurrentCompoundOperation();
		doXForm();

		// Force immediate viewport repaints.
		vp->dataset()->mainWindow()->processViewportUpdates();
	}
	else {

		// Change mouse cursor while hovering over an object.
		ViewportPickResult pickResult = vp->pick(event->localPos());
		setCursor(pickResult.valid ? _xformCursor : QCursor());

	}
	ViewportInputMode::mouseMoveEvent(vp, event);
}

/******************************************************************************
* Returns the origin of the transformation system to use for xform modes.
******************************************************************************/
Point3 XFormMode::transformationCenter()
{
	Point3 center = Point3::Origin();
	SelectionSet* selection = viewport()->dataset()->selection();
	if(selection && !selection->empty()) {
		TimeInterval interval;
		TimePoint time = selection->dataset()->animationSettings()->time();
		for(SceneNode* node : selection->nodes()) {
			const AffineTransformation& nodeTM = node->getWorldTransform(time, interval);
			center += nodeTM.translation();
		}
		center /= (FloatType)selection->count();
	}
	return center;
}

/******************************************************************************
* Determines the coordinate system to use for transformation.
*******************************************************************************/
AffineTransformation XFormMode::transformationSystem()
{
	return viewport()->gridMatrix();
}

/******************************************************************************
* Is called when the transformation operation begins.
******************************************************************************/
void MoveMode::startXForm()
{
	_translationSystem = transformationSystem();
	_initialPoint = Point3::Origin();
	viewport()->snapPoint(_startPoint, _initialPoint, _translationSystem);
}

/******************************************************************************
* Is repeatedly called during the transformation operation.
******************************************************************************/
void MoveMode::doXForm()
{
    Point3 point2;
	if(viewport()->snapPoint(_currentPoint, point2, _translationSystem)) {

		// Get movement in world space.
		_delta = _translationSystem * (point2 - _initialPoint);

		// Apply transformation to selected nodes.
		applyXForm(viewport()->dataset()->selection()->nodes(), 1);
	}
}

/******************************************************************************
* Applies the current transformation to a set of nodes.
******************************************************************************/
void MoveMode::applyXForm(const QVector<SceneNode*>& nodeSet, FloatType multiplier)
{
	for(SceneNode* node : nodeSet) {
		OVITO_CHECK_OBJECT_POINTER(node);
		OVITO_CHECK_OBJECT_POINTER(node->transformationController());

		// Get node's transformation system.
		AffineTransformation transformSystem = _translationSystem;

		// Get parent's system.
		TimeInterval iv;
		TimePoint time = node->dataset()->animationSettings()->time();
		const AffineTransformation& translationSys = node->parentNode()->getWorldTransform(time, iv);

		// Move node in parent's system.
		node->transformationController()->translate(time, _delta * multiplier, translationSys.inverse());
	}
}

/******************************************************************************
* Is called when the transformation operation begins.
******************************************************************************/
void RotateMode::startXForm()
{
	_transformationCenter = transformationCenter();
}

/******************************************************************************
* Is repeatedly called during the transformation operation.
******************************************************************************/
void RotateMode::doXForm()
{
	FloatType angle1 = (FloatType)(_currentPoint.y() - _startPoint.y()) / 100;
	FloatType angle2 = (FloatType)(_currentPoint.x() - _startPoint.x()) / 100;

	// Constrain rotation to z-axis.
	_rotation = Rotation(Vector3(0,0,1), angle1);

	// Apply transformation to selected nodes.
	applyXForm(viewport()->dataset()->selection()->nodes(), 1);
}

/******************************************************************************
* Applies the current transformation to a set of nodes.
******************************************************************************/
void RotateMode::applyXForm(const QVector<SceneNode*>& nodeSet, FloatType multiplier)
{
	for(SceneNode* node : nodeSet) {
		OVITO_CHECK_OBJECT_POINTER(node);
		OVITO_CHECK_OBJECT_POINTER(node->transformationController());

		// Get transformation system.
		AffineTransformation transformSystem = transformationSystem();
		transformSystem.translation() = _transformationCenter - Point3::Origin();

		// Make transformation system relative to parent's tm.
		TimeInterval iv;
		TimePoint time = node->dataset()->animationSettings()->time();
		const AffineTransformation& parentTM = node->parentNode()->getWorldTransform(time, iv);
		transformSystem = transformSystem * parentTM.inverse();

		// Rotate node in transformation system.
		Rotation scaledRot = Rotation(_rotation.axis(), _rotation.angle() * multiplier);
		node->transformationController()->rotate(time, scaledRot, transformSystem);

#if 0
		// Translate node for off-center rotation.
		if(!ANIM_MANAGER.isAnimating()) {
			AffineTransformation inverseSys = transformSystem.inverse();
			/// Get node position in parent's space.
			AffineTransformation curTM;
			node->transformationController()->getValue(time, curTM, iv);
			Point3 nodePos = Point3::Origin() + curTM.translation();
			nodePos = inverseSys * nodePos;
			Vector3 translation = (AffineTransformation::rotation(scaledRot) * nodePos) - nodePos;
			node->transformationController()->translate(time, translation, transformSystem);
		}
#endif
	}
}

};
