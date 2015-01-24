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

#ifndef __OVITO_NAVIGATION_MODES_H
#define __OVITO_NAVIGATION_MODES_H

#include <core/Core.h>
#include <core/gui/app/Application.h>
#include <core/rendering/ArrowPrimitive.h>
#include "ViewportInputMode.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief Base class for viewport navigation modes likes zoom, pan and orbit.
 */
class OVITO_CORE_EXPORT NavigationMode : public ViewportInputMode
{
	Q_OBJECT
	
public:

	/// \brief Returns the activation behavior of this input mode.
	virtual InputModeType modeType() override { return TemporaryMode; }

	/// \brief Handles the mouse down event for the given viewport.
	virtual void mousePressEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Handles the mouse up event for the given viewport.
	virtual void mouseReleaseEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Handles the mouse move event for the given viewport.
	virtual void mouseMoveEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Lets the input mode render its overlay content in a viewport.
	virtual void renderOverlay3D(Viewport* vp, ViewportSceneRenderer* renderer) override;

	/// \brief Computes the bounding box of the visual viewport overlay rendered by the input mode.
	virtual Box3 overlayBoundingBox(Viewport* vp, ViewportSceneRenderer* renderer) override;

	/// \brief Indicates whether this input mode renders into the viewports.
	virtual bool hasOverlay() override { return true; }

protected:

	/// Protected constructor.
	NavigationMode(QObject* parent) : ViewportInputMode(parent), _viewport(nullptr) {}

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, QPointF delta) {}

	/// \brief This is called by the system after the input handler has
	///        become the active handler.
	virtual void activated(bool temporaryActivation) override;

	/// \brief This is called by the system after the input handler is
	///        no longer the active handler.
	virtual void deactivated(bool temporary) override;
	
protected:

	/// Mouse position at first click.
	QPointF _startPoint;

	/// The saved camera position.
	Point3 _oldCameraPosition;

	/// The saved camera direction.
	Vector3 _oldCameraDirection;

	/// The saved camera transformation.
	AffineTransformation _oldCameraTM;

	/// The saved zoom factor.
	FloatType _oldFieldOfView;
	
	/// The saved world to camera transformation matrix.
	AffineTransformation _oldViewMatrix;

	/// The saved camera to world transformation matrix.
	AffineTransformation _oldInverseViewMatrix;

	/// The current viewport we are working in.
	Viewport* _viewport;

	/// Indicates whether this navigation mode is only temporarily activated.
	bool _temporaryActivation;

	/// The cached orbit center as determined when the navigation mode was activated.
	Point3 _currentOrbitCenter;

	/// The geometry buffer used to render the orbit center.
	std::shared_ptr<ArrowPrimitive> _orbitCenterMarker;
};

/******************************************************************************
* The orbit viewport input mode.
******************************************************************************/
class OVITO_CORE_EXPORT OrbitMode : public NavigationMode
{
	Q_OBJECT

public:

	/// \brief Constructor.
	OrbitMode(QObject* parent) : NavigationMode(parent) {
		setCursor(QCursor(QPixmap(":/core/cursor/viewport/cursor_orbit.png")));
	}

protected:

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, QPointF delta) override;
};

/******************************************************************************
* The pan viewport input mode.
******************************************************************************/
class OVITO_CORE_EXPORT PanMode : public NavigationMode
{
	Q_OBJECT
	
public:
	
	/// \brief Constructor.
	PanMode(QObject* parent) : NavigationMode(parent) {
		setCursor(QCursor(QPixmap(":/core/cursor/viewport/cursor_pan.png")));
	}

protected:

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, QPointF delta) override;
};


/******************************************************************************
* The zoom viewport input mode.
******************************************************************************/
class OVITO_CORE_EXPORT ZoomMode : public NavigationMode
{
	Q_OBJECT

public:

	/// \brief Constructor.
	ZoomMode(QObject* parent) : NavigationMode(parent) {
		setCursor(QCursor(QPixmap(":/core/cursor/viewport/cursor_zoom.png")));
	}

	/// Zooms the given viewport in or out.
	void zoom(Viewport* vp, FloatType steps);

protected:

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, QPointF delta) override;

	/// Computes a scaling factor that depends on the total size of the scene which is used to
	/// control the zoom sensitivity in perspective mode.
	FloatType sceneSizeFactor(Viewport* vp);
};

/******************************************************************************
* The field of view input mode.
******************************************************************************/
class OVITO_CORE_EXPORT FOVMode : public NavigationMode
{
	Q_OBJECT

public:

	/// \brief Protected constructor to prevent the creation of second instances.
	FOVMode(QObject* parent) : NavigationMode(parent) {
		setCursor(QCursor(QPixmap(":/core/cursor/viewport/cursor_fov.png")));
	}

protected:

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, QPointF delta) override;
};

/******************************************************************************
* This input mode lets the user pick the center of rotation for the orbit mode.
******************************************************************************/
class OVITO_CORE_EXPORT PickOrbitCenterMode : public ViewportInputMode
{
	Q_OBJECT

public:

	/// Constructor.
	PickOrbitCenterMode(QObject* parent) : ViewportInputMode(parent), _showCursor(false) {
		_hoverCursor = QCursor(QPixmap(":/core/cursor/editing/cursor_mode_select.png"));
	}

	/// Handles the mouse click event for a Viewport.
	virtual void mousePressEvent(Viewport* vp, QMouseEvent* event) override;

	/// Is called when the user moves the mouse.
	virtual void mouseMoveEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Lets the input mode render its overlay content in a viewport.
	virtual void renderOverlay3D(Viewport* vp, ViewportSceneRenderer* renderer) override;

	/// \brief Computes the bounding box of the visual viewport overlay rendered by the input mode.
	virtual Box3 overlayBoundingBox(Viewport* vp, ViewportSceneRenderer* renderer) override;

	/// \brief Indicates whether this input mode renders into the viewports.
	virtual bool hasOverlay() override { return true; }

	/// \brief Sets the orbit rotation center to the space location under given mouse coordinates.
	bool pickOrbitCenter(Viewport* vp, const QPointF& pos);

private:

	/// Finds the intersection point between a ray originating from the current mouse cursor position and the scene.
	bool findIntersection(Viewport* vp, const QPointF& mousePos, Point3& intersectionPoint);

	/// The mouse cursor that is shown when over an object.
	QCursor _hoverCursor;

	/// Indicates that the mouse cursor is over an object.
	bool _showCursor;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_NAVIGATION_MODES_H
