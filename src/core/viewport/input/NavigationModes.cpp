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
#include <core/viewport/ViewportManager.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/viewport/input/NavigationModes.h>
#include <core/viewport/ViewportSettings.h>
#include <core/animation/AnimManager.h>

namespace Ovito {

/******************************************************************************
* This is called by the system after the input handler is
* no longer the active handler.
******************************************************************************/
void NavigationMode::deactivated()
{
	if(_viewport) {
		_viewport->setMouseGrabEnabled(false);
		// Restore old settings.
		_viewport->setCameraPosition(_oldCameraPosition);
		_viewport->setCameraDirection(_oldCameraDirection);
		_viewport->setFieldOfView(_oldFieldOfView);
		_viewport = NULL;
	}
	ViewportInputHandler::deactivated();
}

/******************************************************************************
* Handles the mouse down event for the given viewport.
******************************************************************************/
void NavigationMode::mousePressEvent(Viewport* vp, QMouseEvent* event)
{
	if(event->button() == Qt::RightButton) {
		ViewportInputHandler::mousePressEvent(vp, event);
	}
	else {
		vp->setMouseGrabEnabled(true);
		_viewport = vp;
		_startPoint = event->pos();
		_oldCameraPosition = vp->cameraPosition();
		_oldCameraDirection = vp->cameraDirection();
		_oldFieldOfView = vp->fieldOfView();
		_oldViewMatrix = vp->viewMatrix();
		_oldInverseViewMatrix = vp->inverseViewMatrix();
	}
}

/******************************************************************************
* Handles the mouse up event for the given viewport.
******************************************************************************/
void NavigationMode::mouseReleaseEvent(Viewport* vp, QMouseEvent* event)
{
	if(_viewport) {
		_viewport->setMouseGrabEnabled(false);
		_viewport = NULL;
	}
}

/******************************************************************************
* Handles the mouse move event for the given viewport.
******************************************************************************/
void NavigationMode::mouseMoveEvent(Viewport* vp, QMouseEvent* event)
{
	if(_viewport == vp) {
		modifyView(vp, event->pos() - _startPoint);
		vp->updateViewport();
		ViewportManager::instance().processViewportUpdates();
	}
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Pan Mode ///////////////////////////////////

/******************************************************************************
* Computes the new view matrix based on the new mouse position.
******************************************************************************/
void PanMode::modifyView(Viewport* vp, const QPoint& delta)
{
	FloatType scaling;
	if(vp->isPerspectiveProjection())
		scaling = 50.0 / vp->size().height();
	else
		scaling = _oldFieldOfView / vp->size().height();
	FloatType deltaX = -scaling * delta.x();
	FloatType deltaY =  scaling * delta.y();
	Vector3 displacement = _oldInverseViewMatrix * Vector3(deltaX, deltaY, 0);
	vp->setCameraPosition(_oldCameraPosition + displacement);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Zoom Mode ///////////////////////////////////

/******************************************************************************
* Computes the new view matrix based on the new mouse position.
******************************************************************************/
void ZoomMode::modifyView(Viewport* vp, const QPoint& delta)
{
	if(vp->isPerspectiveProjection()) {
		FloatType amount =  -0.5 * delta.y();
		vp->setCameraPosition(_oldCameraPosition + _oldCameraDirection.resized(amount));
	}
	else {
		FloatType scaling = (FloatType)exp(0.006 * delta.y());
		vp->setFieldOfView(_oldFieldOfView * scaling);
	}
}

/******************************************************************************
* Zooms the viewport in or out.
******************************************************************************/
void ZoomMode::zoom(Viewport* vp, FloatType steps)
{
	if(vp->isPerspectiveProjection()) {
		vp->setCameraPosition(vp->cameraPosition() + vp->cameraDirection().resized(0.1 * steps));
	}
	else {
		vp->setFieldOfView(vp->fieldOfView() * exp(-steps * 0.001));
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////// FOV Mode ///////////////////////////////////

/******************************************************************************
* Computes the new field of view based on the new mouse position.
******************************************************************************/
void FOVMode::modifyView(Viewport* vp, const QPoint& delta)
{
	if(vp->isPerspectiveProjection()) {
		FloatType newFOV = _oldFieldOfView + (FloatType)delta.y() * 0.002;
		newFOV = std::max(newFOV, (FloatType)(5.0 * FLOATTYPE_PI / 180.0));
		newFOV = std::min(newFOV, (FloatType)(170.0 * FLOATTYPE_PI / 180.0));
		vp->setFieldOfView(newFOV);
	}
	else {
		FloatType scaling = (FloatType)exp(0.006 * delta.y());
		vp->setFieldOfView(_oldFieldOfView * scaling);
	}
}

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Orbit Mode ///////////////////////////////////

/******************************************************************************
* Handles the mouse down event for the given viewport.
******************************************************************************/
void OrbitMode::mousePressEvent(Viewport* vp, QMouseEvent* event)
{
	NavigationMode::mousePressEvent(vp, event);
	if(event->button() == Qt::LeftButton) {
		// Update orbiting center.
		if(centerMode() == ORBIT_CONSTRUCTION_PLANE || centerMode() == ORBIT_SELECTION_CENTER) {
			Point3 center = Point3::Origin();
			FloatType d = (center - vp->cameraPosition()).dot(vp->cameraDirection());
			_orbitCenter = vp->cameraPosition() + vp->cameraDirection() * (d / vp->cameraDirection().squaredLength());
		}
	}
}

/******************************************************************************
* Computes the new view matrix based on the new mouse position.
******************************************************************************/
void OrbitMode::modifyView(Viewport* vp, const QPoint& delta)
{
	if(!vp->isPerspectiveProjection())
		vp->setViewType(Viewport::VIEW_ORTHO);

	Matrix3 coordSys = ViewportSettings::getSettings().coordinateSystemOrientation();
	Vector3 v;
	if(vp->isPerspectiveProjection())
		v = (_oldCameraPosition - _orbitCenter);
	else
		v = -_oldCameraDirection;
	v = coordSys.inverse() * v;

	FloatType theta, phi;
	if(v.x() == 0 && v.y() == 0)
		theta = FLOATTYPE_PI;
	else
		theta = atan2(v.x(), v.y());
	phi = atan2(sqrt(v.x() * v.x() + v.y() * v.y()), v.z());

	FloatType speed = 4.0 / vp->size().height();
	theta += speed * delta.x();
	phi -= speed * delta.y();

	phi = std::max(FLOATTYPE_EPSILON, std::min(FLOATTYPE_PI- FLOATTYPE_EPSILON, phi));
	FloatType radius = v.length();

	v.x() = sin(phi) * sin(theta);
	v.y() = sin(phi) * cos(theta);
	v.z() = cos(phi);
	v = coordSys * v;

	vp->setCameraPosition(_orbitCenter + v * radius);
	if(vp->isPerspectiveProjection())
		vp->setCameraDirection(_orbitCenter - vp->cameraPosition());
	else
		vp->setCameraDirection(-v);
}

/******************************************************************************
* Changes the way the center of rotation is chosen.
******************************************************************************/
void OrbitMode::setCenterMode(OrbitMode::CenterMode mode)
{
	if(_centerMode == mode) return;
	_centerMode = mode;
	ViewportManager::instance().updateViewports();
}

/******************************************************************************
* Sets the world space point around which the camera orbits.
******************************************************************************/
void OrbitMode::setOrbitCenter(const Point3& center)
{
	if(_orbitCenter == center) return;
	_orbitCenter = center;
	ViewportManager::instance().updateViewports();
}

/******************************************************************************
* Lets the input mode render its overlay content in a viewport.
******************************************************************************/
void OrbitMode::renderOverlay(Viewport* vp, bool isActive)
{
	NavigationMode::renderOverlay(vp, isActive);
#if 0
	// Render center of rotation when in user mode.
	if(_centerMode == ORBIT_USER_DEFINED) {
		vp->setDepthTest(true);
		vp->setBackfaceCulling(true);
		vp->setLightingEnabled(false);
		vp->setWorldMatrix(AffineTransformation::translation(orbitCenter() - ORIGIN));

		FloatType symbolSize = 1.0 * vp->nonScalingSize(orbitCenter());

		Box3 bbox(Point3(-symbolSize), Point3(+symbolSize));
		Point3 verts[2];
		verts[0] = Point3(-symbolSize,0,0);
		verts[1] = Point3(+symbolSize,0,0);
		vp->setRenderingColor(Color(1,0,0));
		vp->renderLines(2, bbox, verts);
		verts[0] = Point3(0,-symbolSize,0);
		verts[1] = Point3(0,+symbolSize,0);
		vp->setRenderingColor(Color(0,1,0));
		vp->renderLines(2, bbox, verts);
		verts[0] = Point3(0,0,-symbolSize);
		verts[1] = Point3(0,0,+symbolSize);
		vp->setRenderingColor(Color(0,0,1));
		vp->renderLines(2, bbox, verts);
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Pick Orbit Center Mode ////////////////////////////////

#if 0
/******************************************************************************
* Handles the mouse down events for a Viewport.
******************************************************************************/
void PickOrbitCenterMode::onMousePressed(QMouseEvent* event)
{
	SimpleInputHandler::onMousePressed(event);

	Point3 p;
	if(findIntersection(viewport(), event->pos(), p)) {
		OrbitMode::instance()->setCenterMode(OrbitMode::ORBIT_USER_DEFINED);
		OrbitMode::instance()->setOrbitCenter(p);
	}
	else {
		OrbitMode::instance()->setCenterMode(OrbitMode::ORBIT_SELECTION_CENTER);
		OrbitMode::instance()->setOrbitCenter(ORIGIN);
	}

	onFinish();
}

/******************************************************************************
* Is called when the user moves the mouse while the operation is not active.
******************************************************************************/
void PickOrbitCenterMode::onMouseFreeMove(Viewport& vp, QMouseEvent* event)
{
	SimpleInputHandler::onMouseFreeMove(vp, event);

	Point3 p;
	bool isOverObject = findIntersection(&vp, event->pos(), p);

	if(!isOverObject && showCursor) {
		showCursor = false;
		updateCursor();
	}
	else if(isOverObject && !showCursor) {
		showCursor = true;
		updateCursor();
	}
}

/******************************************************************************
* Finds the closest intersection point between a ray originating from the
* current mouse cursor position and the whole scene.
******************************************************************************/
bool PickOrbitCenterMode::findIntersection(Viewport* vp, const Point2I& mousePos, Point3& intersectionPoint)
{
	OVITO_ASSERT(vp != NULL);

	TimeTicks time = ANIM_MANAGER.time();
	Ray3 ray = vp->screenRay(mousePos);

	// Iterate over all object nodes in the scene.
	SceneRoot* rootNode = DATASET_MANAGER.currentSet()->sceneRoot();
	if(rootNode == NULL) return false;

	FloatType closestDistance = FLOATTYPE_MAX;

	for(SceneNodesIterator iter(rootNode); !iter.finished(); iter.next()) {
		ObjectNode* objNode = dynamic_object_cast<ObjectNode>(iter.current());
		if(!objNode) continue;

		const PipelineFlowState& flowState = objNode->evalPipeline(time);
		if(flowState.result() == NULL) continue;

		TimeInterval iv;
		const AffineTransformation& nodeTM = objNode->getWorldTransform(time, iv);

		Ray3 transformedRay = nodeTM.inverse() * ray;
		Vector3 normal;
		FloatType t;

		if(flowState.result()->intersectRay(transformedRay, time, objNode, t, normal)) {
			if(t < closestDistance) {
				closestDistance = t;
				intersectionPoint = nodeTM * transformedRay.point(t);
			}
		}
	}

	return closestDistance != FLOATTYPE_MAX;
}
#endif

};
