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
#include <core/gui/widgets/display/CoordinateDisplayWidget.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/viewport/input/XFormModes.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

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
			_viewport->dataset()->selection()->setNode(pickResult.objectNode);
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
	inputManager()->mainWindow()->statusBar()->clearMessage();
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

	// Display a description of the object under the mouse cursor in the status bar.
	if(pickResult.valid && pickResult.pickInfo)
		inputManager()->mainWindow()->statusBar()->showMessage(pickResult.pickInfo->infoString(pickResult.objectNode, pickResult.subobjectId));
	else
		inputManager()->mainWindow()->statusBar()->clearMessage();

	ViewportInputMode::mouseMoveEvent(vp, event);
}

/******************************************************************************
* This is called by the system after the input handler has
* become the active handler.
******************************************************************************/
void XFormMode::activated(bool temporaryActivation)
{
	ViewportInputMode::activated(temporaryActivation);

	// Listen to selection change events to update the coordinate display.
	DataSetContainer& datasetContainer = inputManager()->mainWindow()->datasetContainer();
	connect(&datasetContainer, &DataSetContainer::selectionChangeComplete, this, &XFormMode::onSelectionChangeComplete);
	connect(&datasetContainer, &DataSetContainer::timeChanged, this, &XFormMode::onTimeChanged);
	onSelectionChangeComplete(datasetContainer.currentSet() ? datasetContainer.currentSet()->selection() : nullptr);
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
	disconnect(&inputManager()->mainWindow()->datasetContainer(), &DataSetContainer::selectionChangeComplete, this, &XFormMode::onSelectionChangeComplete);
	disconnect(&inputManager()->mainWindow()->datasetContainer(), &DataSetContainer::timeChanged, this, &XFormMode::onTimeChanged);
	_selectedNode.setTarget(nullptr);
	onSelectionChangeComplete(nullptr);
	ViewportInputMode::deactivated(temporary);
}

/******************************************************************************
* Is called when the user has selected a different scene node.
******************************************************************************/
void XFormMode::onSelectionChangeComplete(SelectionSet* selection)
{
	CoordinateDisplayWidget* coordDisplay = inputManager()->mainWindow()->coordinateDisplay();
	if(selection) {
		if(selection->size() == 1) {
			_selectedNode.setTarget(selection->node(0));
			updateCoordinateDisplay(coordDisplay);
			coordDisplay->activate(undoDisplayName());
			connect(coordDisplay, &CoordinateDisplayWidget::valueEntered, this, &XFormMode::onCoordinateValueEntered, Qt::ConnectionType(Qt::AutoConnection | Qt::UniqueConnection));
			return;
		}
	}
	_selectedNode.setTarget(nullptr);
	disconnect(coordDisplay, &CoordinateDisplayWidget::valueEntered, this, &XFormMode::onCoordinateValueEntered);
	coordDisplay->deactivate();
}

/******************************************************************************
* Is called when the selected scene node generates a notification event.
******************************************************************************/
void XFormMode::onSceneNodeEvent(ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::TransformationChanged) {
		updateCoordinateDisplay(inputManager()->mainWindow()->coordinateDisplay());
	}
}

/******************************************************************************
* Is called when the current animation time has changed.
******************************************************************************/
void XFormMode::onTimeChanged(TimePoint time)
{
	updateCoordinateDisplay(inputManager()->mainWindow()->coordinateDisplay());
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
				_viewport->dataset()->selection()->setNode(pickResult.objectNode);
				_viewport->dataset()->undoStack().beginCompoundOperation(undoDisplayName());
				startXForm();
			}
		}
		return;
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
		center /= (FloatType)selection->size();
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
* Updates the values displayed in the coordinate display widget.
******************************************************************************/
void MoveMode::updateCoordinateDisplay(CoordinateDisplayWidget* coordDisplay)
{
	if(_selectedNode.target()) {
		DataSet* dataset = _selectedNode.target()->dataset();
		coordDisplay->setUnit(dataset->unitsManager().worldUnit());
		Controller* ctrl = _selectedNode.target()->transformationController();
		if(ctrl) {
			TimeInterval iv;
			Vector3 translation;
			ctrl->getPositionValue(dataset->animationSettings()->time(), translation, iv);
			coordDisplay->setValues(translation);
			return;
		}
	}
	coordDisplay->setValues(Vector3::Zero());
}

/******************************************************************************
* This signal handler is called by the coordinate display widget when the user
* has changed the value of one of the vector components.
******************************************************************************/
void MoveMode::onCoordinateValueEntered(int component, FloatType value)
{
	if(_selectedNode.target()) {
		Controller* ctrl = _selectedNode.target()->transformationController();
		if(ctrl) {
			TimeInterval iv;
			Vector3 translation;
			DataSet* dataset = _selectedNode.target()->dataset();
			ctrl->getPositionValue(dataset->animationSettings()->time(), translation, iv);
			translation[component] = value;
			ctrl->setPositionValue(dataset->animationSettings()->time(), translation, true);
		}
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

/******************************************************************************
* Updates the values displayed in the coordinate display widget.
******************************************************************************/
void RotateMode::updateCoordinateDisplay(CoordinateDisplayWidget* coordDisplay)
{
	if(_selectedNode.target()) {
		DataSet* dataset = _selectedNode.target()->dataset();
		coordDisplay->setUnit(dataset->unitsManager().angleUnit());
		Controller* ctrl = _selectedNode.target()->transformationController();
		if(ctrl) {
			TimeInterval iv;
			Rotation rotation;
			ctrl->getRotationValue(dataset->animationSettings()->time(), rotation, iv);
			Vector3 euler = rotation.toEuler(Matrix3::szyx);
			coordDisplay->setValues(Vector3(euler[2], euler[1], euler[0]));
			return;
		}
	}
	coordDisplay->setValues(Vector3::Zero());
}

/******************************************************************************
* This signal handler is called by the coordinate display widget when the user
* has changed the value of one of the vector components.
******************************************************************************/
void RotateMode::onCoordinateValueEntered(int component, FloatType value)
{
	if(_selectedNode.target()) {
		Controller* ctrl = _selectedNode.target()->transformationController();
		if(ctrl) {
			TimeInterval iv;
			Vector3 translation;
			DataSet* dataset = _selectedNode.target()->dataset();
			CoordinateDisplayWidget* coordDisplay = inputManager()->mainWindow()->coordinateDisplay();
			Vector3 euler = coordDisplay->getValues();
			Rotation rotation = Rotation::fromEuler(Vector3(euler[2], euler[1], euler[0]), Matrix3::szyx);
			ctrl->setRotationValue(dataset->animationSettings()->time(), rotation, true);
		}
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
