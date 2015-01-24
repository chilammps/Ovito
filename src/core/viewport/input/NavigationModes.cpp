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
#include <core/viewport/Viewport.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/viewport/input/NavigationModes.h>
#include <core/viewport/ViewportSettings.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/animation/AnimationSettings.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>
#include <core/scene/objects/camera/AbstractCameraObject.h>
#include <core/scene/SceneRoot.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/dataset/UndoStack.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* This is called by the system after the input handler has
* become the active handler.
******************************************************************************/
void NavigationMode::activated(bool temporaryActivation)
{
	_temporaryActivation = temporaryActivation;
	ViewportInputMode::activated(temporaryActivation);
}

/******************************************************************************
* This is called by the system after the input handler is
* no longer the active handler.
******************************************************************************/
void NavigationMode::deactivated(bool temporary)
{
	if(_viewport) {
		// Restore old settings if view change has not been committed.
		_viewport->setCameraTransformation(_oldCameraTM);
		_viewport->setFieldOfView(_oldFieldOfView);
		_viewport->dataset()->undoStack().endCompoundOperation(false);
		_viewport = nullptr;
	}
	ViewportInputMode::deactivated(temporary);
}

/******************************************************************************
* Handles the mouse down event for the given viewport.
******************************************************************************/
void NavigationMode::mousePressEvent(Viewport* vp, QMouseEvent* event)
{
	if(event->button() == Qt::RightButton) {
		ViewportInputMode::mousePressEvent(vp, event);
		return;
	}

	if(_viewport == nullptr) {
		_viewport = vp;
		_startPoint = event->localPos();
		_oldCameraTM = vp->cameraTransformation();
		_oldCameraPosition = vp->cameraPosition();
		_oldCameraDirection = vp->cameraDirection();
		_oldFieldOfView = vp->fieldOfView();
		_oldViewMatrix = vp->viewMatrix();
		_oldInverseViewMatrix = vp->inverseViewMatrix();
		_currentOrbitCenter = vp->orbitCenter();
		_viewport->dataset()->undoStack().beginCompoundOperation(tr("Modify camera"));
	}
}

/******************************************************************************
* Handles the mouse up event for the given viewport.
******************************************************************************/
void NavigationMode::mouseReleaseEvent(Viewport* vp, QMouseEvent* event)
{
	if(_viewport) {
		// Commit view change.
		_viewport->dataset()->undoStack().endCompoundOperation();
		_viewport = nullptr;

		if(_temporaryActivation)
			inputManager()->removeInputMode(this);
	}
}

/******************************************************************************
* Handles the mouse move event for the given viewport.
******************************************************************************/
void NavigationMode::mouseMoveEvent(Viewport* vp, QMouseEvent* event)
{
	if(_viewport == vp) {
#if 1
		// Take the current mouse cursor position to make the navigation mode
		// look more responsive. The cursor position recorded at the time the mouse event was
		// generates may already be too old.
		QPointF pos = vp->widget()->mapFromGlobal(QCursor::pos());
#else
		QPointF pos = event->localPos();
#endif

		vp->dataset()->undoStack().resetCurrentCompoundOperation();
		modifyView(vp, pos - _startPoint);

		// Force immediate viewport repaint.
		vp->dataset()->mainWindow()->processViewportUpdates();
	}
}

/******************************************************************************
* Lets the input mode render its overlay content in a viewport.
******************************************************************************/
void NavigationMode::renderOverlay3D(Viewport* vp, ViewportSceneRenderer* renderer)
{
	if(renderer->isPicking())
		return;

	// Render center of rotation.
	Point3 center = vp->dataset()->viewportConfig()->orbitCenter();
	FloatType symbolSize = vp->nonScalingSize(center);
	renderer->setWorldTransform(AffineTransformation::translation(center - Point3::Origin()) * AffineTransformation::scaling(symbolSize));

	// Create line buffer.
	if(!_orbitCenterMarker || !_orbitCenterMarker->isValid(renderer)) {
		_orbitCenterMarker = renderer->createArrowPrimitive(ArrowPrimitive::CylinderShape, ArrowPrimitive::NormalShading, ArrowPrimitive::HighQuality);
		_orbitCenterMarker->startSetElements(3);
        _orbitCenterMarker->setElement(0, Point3(-1,0,0), Vector3(2,0,0), ColorA(1,0,0), 0.05f);
        _orbitCenterMarker->setElement(1, Point3(0,-1,0), Vector3(0,2,0), ColorA(0,1,0), 0.05f);
        _orbitCenterMarker->setElement(2, Point3(0,0,-1), Vector3(0,0,2), ColorA(0.2f,0.2f,1), 0.05f);
		_orbitCenterMarker->endSetElements();
	}
	_orbitCenterMarker->render(renderer);
}

/******************************************************************************
* Computes the bounding box of the visual viewport overlay rendered by the input mode.
******************************************************************************/
Box3 NavigationMode::overlayBoundingBox(Viewport* vp, ViewportSceneRenderer* renderer)
{
	Point3 center = vp->dataset()->viewportConfig()->orbitCenter();
	FloatType symbolSize = vp->nonScalingSize(center);
	return Box3(center, symbolSize);
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Pan Mode ///////////////////////////////////

/******************************************************************************
* Computes the new view matrix based on the new mouse position.
******************************************************************************/
void PanMode::modifyView(Viewport* vp, QPointF delta)
{
	FloatType scaling;
	if(vp->isPerspectiveProjection())
		scaling = 10.0f * vp->nonScalingSize(_currentOrbitCenter) / vp->size().height();
	else
		scaling = 2.0f * _oldFieldOfView * vp->viewportWindow()->devicePixelRatio() / vp->size().height();
	FloatType deltaX = -scaling * delta.x();
	FloatType deltaY =  scaling * delta.y();
	Vector3 displacement = _oldInverseViewMatrix * Vector3(deltaX, deltaY, 0);
	if(vp->viewNode() == nullptr || vp->viewType() != Viewport::VIEW_SCENENODE) {
		vp->setCameraPosition(_oldCameraPosition + displacement);
	}
	else {
		// Get parent's system.
		TimeInterval iv;
		const AffineTransformation& parentSys =
				vp->viewNode()->parentNode()->getWorldTransform(vp->dataset()->animationSettings()->time(), iv);

		// Move node in parent's system.
		vp->viewNode()->transformationController()->translate(
				vp->dataset()->animationSettings()->time(), displacement, parentSys.inverse());

		// If it's a target camera, move target as well.
		if(vp->viewNode()->lookatTargetNode()) {
			vp->viewNode()->lookatTargetNode()->transformationController()->translate(
					vp->dataset()->animationSettings()->time(), displacement, parentSys.inverse());
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Zoom Mode ///////////////////////////////////

/******************************************************************************
* Computes the new view matrix based on the new mouse position.
******************************************************************************/
void ZoomMode::modifyView(Viewport* vp, QPointF delta)
{
	if(vp->isPerspectiveProjection()) {
		FloatType amount =  -5.0f * sceneSizeFactor(vp) * delta.y();
		if(vp->viewNode() == nullptr || vp->viewType() != Viewport::VIEW_SCENENODE) {
			vp->setCameraPosition(_oldCameraPosition + _oldCameraDirection.resized(amount));
		}
		else {
			TimeInterval iv;
			const AffineTransformation& sys = vp->viewNode()->getWorldTransform(vp->dataset()->animationSettings()->time(), iv);
			vp->viewNode()->transformationController()->translate(
					vp->dataset()->animationSettings()->time(), Vector3(0,0,-amount), sys);
		}
	}
	else {

		AbstractCameraObject* cameraObj = nullptr;
		FloatType oldFOV = _oldFieldOfView;
		if(vp->viewNode() && vp->viewType() == Viewport::VIEW_SCENENODE) {
			cameraObj = dynamic_object_cast<AbstractCameraObject>(vp->viewNode()->sourceObject());
			if(cameraObj) {
				TimeInterval iv;
				oldFOV = cameraObj->fieldOfView(vp->dataset()->animationSettings()->time(), iv);
			}
		}

		FloatType newFOV = oldFOV * (FloatType)exp(0.003f * delta.y());

		if(vp->viewNode() == nullptr || vp->viewType() != Viewport::VIEW_SCENENODE) {
			vp->setFieldOfView(newFOV);
		}
		else if(cameraObj) {
			cameraObj->setFieldOfView(vp->dataset()->animationSettings()->time(), newFOV);
		}
	}
}

/******************************************************************************
* Computes a scaling factor that depends on the total size of the scene which is used to
* control the zoom sensitivity in perspective mode.
******************************************************************************/
FloatType ZoomMode::sceneSizeFactor(Viewport* vp)
{
	OVITO_CHECK_OBJECT_POINTER(vp);
	Box3 sceneBoundingBox = vp->dataset()->sceneRoot()->worldBoundingBox(vp->dataset()->animationSettings()->time());
	if(!sceneBoundingBox.isEmpty())
		return sceneBoundingBox.size().length() * 5e-4f;
	else
		return 0.1f;
}

/******************************************************************************
* Zooms the viewport in or out.
******************************************************************************/
void ZoomMode::zoom(Viewport* vp, FloatType steps)
{
	if(vp->viewNode() == nullptr || vp->viewType() != Viewport::VIEW_SCENENODE) {
		if(vp->isPerspectiveProjection()) {
			vp->setCameraPosition(vp->cameraPosition() + vp->cameraDirection().resized(sceneSizeFactor(vp) * steps));
		}
		else {
			vp->setFieldOfView(vp->fieldOfView() * exp(-steps * 0.001f));
		}
	}
	else {
		UndoableTransaction::handleExceptions(vp->dataset()->undoStack(), tr("Zoom viewport"), [this, steps, vp]() {
			if(vp->isPerspectiveProjection()) {
				FloatType amount = sceneSizeFactor(vp) * steps;
				TimeInterval iv;
				const AffineTransformation& sys = vp->viewNode()->getWorldTransform(vp->dataset()->animationSettings()->time(), iv);
				vp->viewNode()->transformationController()->translate(vp->dataset()->animationSettings()->time(), Vector3(0,0,-amount), sys);
			}
			else {
				AbstractCameraObject* cameraObj = dynamic_object_cast<AbstractCameraObject>(vp->viewNode()->sourceObject());
				if(cameraObj) {
					TimeInterval iv;
					FloatType oldFOV = cameraObj->fieldOfView(vp->dataset()->animationSettings()->time(), iv);
					cameraObj->setFieldOfView(vp->dataset()->animationSettings()->time(), oldFOV * exp(-steps * 0.001f));
				}
			}
		});
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// FOV Mode ///////////////////////////////////

/******************************************************************************
* Computes the new field of view based on the new mouse position.
******************************************************************************/
void FOVMode::modifyView(Viewport* vp, QPointF delta)
{
	AbstractCameraObject* cameraObj = nullptr;
	FloatType oldFOV = _oldFieldOfView;
	if(vp->viewNode() && vp->viewType() == Viewport::VIEW_SCENENODE) {
		cameraObj = dynamic_object_cast<AbstractCameraObject>(vp->viewNode()->sourceObject());
		if(cameraObj) {
			TimeInterval iv;
			oldFOV = cameraObj->fieldOfView(vp->dataset()->animationSettings()->time(), iv);
		}
	}

	FloatType newFOV;
	if(vp->isPerspectiveProjection()) {
		newFOV = oldFOV + (FloatType)delta.y() * 0.002f;
		newFOV = std::max(newFOV, (FloatType)(5.0f * FLOATTYPE_PI / 180.0f));
		newFOV = std::min(newFOV, (FloatType)(170.0f * FLOATTYPE_PI / 180.0f));
	}
	else {
		newFOV = oldFOV * (FloatType)exp(0.006f * delta.y());
	}

	if(vp->viewNode() == nullptr || vp->viewType() != Viewport::VIEW_SCENENODE) {
		vp->setFieldOfView(newFOV);
	}
	else if(cameraObj) {
		cameraObj->setFieldOfView(vp->dataset()->animationSettings()->time(), newFOV);
	}
}

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Orbit Mode ///////////////////////////////////

/******************************************************************************
* Computes the new view matrix based on the new mouse position.
******************************************************************************/
void OrbitMode::modifyView(Viewport* vp, QPointF delta)
{
	if(vp->viewType() < Viewport::VIEW_ORTHO)
		vp->setViewType(Viewport::VIEW_ORTHO, true);

	Matrix3 coordSys = ViewportSettings::getSettings().coordinateSystemOrientation();
	Vector3 v = _oldViewMatrix * coordSys.column(2);

	FloatType theta, phi;
	if(v.x() == 0 && v.y() == 0)
		theta = FLOATTYPE_PI;
	else
		theta = atan2(v.x(), v.y());
	phi = atan2(sqrt(v.x() * v.x() + v.y() * v.y()), v.z());

	FloatType speed = 4.0f / vp->size().height();
	FloatType deltaTheta = speed * delta.x();
	FloatType deltaPhi = -speed * delta.y();

	if(ViewportSettings::getSettings().restrictVerticalRotation()) {
		if(phi + deltaPhi < FLOATTYPE_EPSILON)
			deltaPhi = -phi + FLOATTYPE_EPSILON;
		else if(phi + deltaPhi > FLOATTYPE_PI - FLOATTYPE_EPSILON)
			deltaPhi = FLOATTYPE_PI - FLOATTYPE_EPSILON - phi;
	}

	Vector3 t1 = _currentOrbitCenter - Point3::Origin();
	Vector3 t2 = (_oldViewMatrix * _currentOrbitCenter) - Point3::Origin();

	if(vp->viewNode() == nullptr || vp->viewType() != Viewport::VIEW_SCENENODE) {
		AffineTransformation newTM =
				AffineTransformation::translation(t1) *
				AffineTransformation::rotation(Rotation(ViewportSettings::getSettings().upVector(), -deltaTheta)) *
				AffineTransformation::translation(-t1) * _oldInverseViewMatrix *
				AffineTransformation::translation(t2) *
				AffineTransformation::rotationX(deltaPhi) *
				AffineTransformation::translation(-t2);
		newTM.orthonormalize();
		vp->setCameraTransformation(newTM);
	}
	else {
		Controller* ctrl = vp->viewNode()->transformationController();
		TimePoint time = vp->dataset()->animationSettings()->time();
		Rotation rotX(Vector3(1,0,0), deltaPhi, false);
		ctrl->rotate(time, rotX, _oldInverseViewMatrix);
		Rotation rotZ(ViewportSettings::getSettings().upVector(), -deltaTheta);
		ctrl->rotate(time, rotZ, AffineTransformation::Identity());
		Vector3 shiftVector = _oldInverseViewMatrix.translation() - (_currentOrbitCenter - Point3::Origin());
		Vector3 translationZ = (Matrix3::rotation(rotZ) * shiftVector) - shiftVector;
		Vector3 translationX = Matrix3::rotation(rotZ) * _oldInverseViewMatrix * ((Matrix3::rotation(rotX) * t2) - t2);
		ctrl->translate(time, translationZ - translationX, AffineTransformation::Identity());
	}
}

/////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Pick Orbit Center Mode ////////////////////////////////

/******************************************************************************
* Sets the orbit rotation center to the space location under given mouse coordinates.
******************************************************************************/
bool PickOrbitCenterMode::pickOrbitCenter(Viewport* vp, const QPointF& pos)
{
	Point3 p;
	if(findIntersection(vp, pos, p)) {
		vp->dataset()->viewportConfig()->setOrbitCenterMode(ViewportConfiguration::ORBIT_USER_DEFINED);
		vp->dataset()->viewportConfig()->setUserOrbitCenter(p);
		return true;
	}
	else {
		vp->dataset()->viewportConfig()->setOrbitCenterMode(ViewportConfiguration::ORBIT_SELECTION_CENTER);
		vp->dataset()->viewportConfig()->setUserOrbitCenter(Point3::Origin());
		if(MainWindow* mainWindow = vp->dataset()->mainWindow())
			mainWindow->statusBar()->showMessage(tr("No object has been picked. Resetting orbit center to default position."), 1200);
		return false;
	}
}

/******************************************************************************
* Handles the mouse down events for a Viewport.
******************************************************************************/
void PickOrbitCenterMode::mousePressEvent(Viewport* vp, QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton) {
		if(pickOrbitCenter(vp, event->localPos()))
			return;
	}
	ViewportInputMode::mousePressEvent(vp, event);
}

/******************************************************************************
* Is called when the user moves the mouse while the operation is not active.
******************************************************************************/
void PickOrbitCenterMode::mouseMoveEvent(Viewport* vp, QMouseEvent* event)
{
	ViewportInputMode::mouseMoveEvent(vp, event);

	Point3 p;
	bool isOverObject = findIntersection(vp, event->localPos(), p);

	if(!isOverObject && _showCursor) {
		_showCursor = false;
		setCursor(QCursor());
	}
	else if(isOverObject && !_showCursor) {
		_showCursor = true;
		setCursor(_hoverCursor);
	}
}

/******************************************************************************
* Finds the closest intersection point between a ray originating from the
* current mouse cursor position and the whole scene.
******************************************************************************/
bool PickOrbitCenterMode::findIntersection(Viewport* vp, const QPointF& mousePos, Point3& intersectionPoint)
{
	ViewportPickResult pickResults = vp->pick(mousePos);
	if(!pickResults.valid)
		return false;

	intersectionPoint = pickResults.worldPosition;
	return true;
}

/******************************************************************************
* Lets the input mode render its overlay content in a viewport.
******************************************************************************/
void PickOrbitCenterMode::renderOverlay3D(Viewport* vp, ViewportSceneRenderer* renderer)
{
	inputManager()->orbitMode()->renderOverlay3D(vp, renderer);
}

/******************************************************************************
* Computes the bounding box of the visual viewport overlay rendered by the input mode.
******************************************************************************/
Box3 PickOrbitCenterMode::overlayBoundingBox(Viewport* vp, ViewportSceneRenderer* renderer)
{
	return inputManager()->orbitMode()->overlayBoundingBox(vp, renderer);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
