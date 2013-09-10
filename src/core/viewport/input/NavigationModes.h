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
#include <core/rendering/ArrowGeometryBuffer.h>
#include "ViewportInputHandler.h"

namespace Ovito {

/**
 * \brief Base class for viewport navigation modes likes zoom, pan and orbit.
 */
class OVITO_CORE_EXPORT NavigationMode : public ViewportInputHandler
{
	Q_OBJECT

public:

	enum OrbitCenterMode {
		ORBIT_CONSTRUCTION_PLANE,	/// Take the current construction plane as orbit center.
		ORBIT_SELECTION_CENTER,		/// Take the center of mass of the current selection as orbit center.
		ORBIT_USER_DEFINED			/// Use the orbit center set by the user.
	};
	
public:

	/// \brief Returns the activation behavior of this input handler.
	///
	/// Viewport navigation modes are temporary.
	virtual InputHandlerType handlerType() override { return TEMPORARY; }

	/// \brief Handles the mouse down event for the given viewport.
	virtual void mousePressEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Handles the mouse up event for the given viewport.
	virtual void mouseReleaseEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Handles the mouse move event for the given viewport.
	virtual void mouseMoveEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Lets the input mode render its overlay content in a viewport.
	virtual void renderOverlay(Viewport* vp, ViewportSceneRenderer* renderer, bool isActive) override;

	/// \brief Computes the bounding box of the visual viewport overlay rendered by the input mode.
	virtual Box3 overlayBoundingBox(Viewport* vp, ViewportSceneRenderer* renderer, bool isActive) override;

	/// \brief Indicates whether this input mode renders into the viewports.
	virtual bool hasOverlay() override { return true; }

	/// Changes the way the center of rotation is chosen.
	static void setOrbitCenterMode(OrbitCenterMode mode);

	/// Returns the current center of orbit mode.
	static OrbitCenterMode orbitCenterMode() { return _orbitCenterMode; }

	/// Sets the world space point around which the camera orbits.
	static void setUserOrbitCenter(const Point3& center);

	/// Returns the world space point around which the camera orbits.
	static Point3 orbitCenter();

protected:

	/// Protected constructor.
	NavigationMode() : _viewport(nullptr) {}

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, const QPoint& delta) {}

	/// \brief This is called by the system after the input handler is
	///        no longer the active handler.
	virtual void deactivated() override;
	
protected:

	/// Mouse position at first click.
	QPoint _startPoint;

	/// The saved camera position.
	Point3 _oldCameraPosition;

	/// The saved camera direction.
	Vector3 _oldCameraDirection;

	/// The saved zoom factor.
	FloatType _oldFieldOfView;
	
	/// The saved world to camera transformation matrix.
	AffineTransformation _oldViewMatrix;

	/// The saved camera to world transformation matrix.
	AffineTransformation _oldInverseViewMatrix;

	/// The current viewport we are working in.
	Viewport* _viewport;

	/// Indicates around which point the camera should orbit.
	static OrbitCenterMode _orbitCenterMode;

	/// The user-defined orbiting center.
	static Point3 _userOrbitCenter;

	/// The geometry buffer used to render the orbit center.
	static OORef<ArrowGeometryBuffer> _orbitCenterMarker;
};

/******************************************************************************
* The orbit viewport input mode.
******************************************************************************/
class OVITO_CORE_EXPORT OrbitMode : public NavigationMode
{
	Q_OBJECT

protected:

	/// \brief Protected constructor to prevent the creation of second instances.
	OrbitMode() {
		setCursor(QCursor(QPixmap(":/core/cursor/viewport/cursor_orbit.png")));
	}

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, const QPoint& delta) override;

public:

	/// \brief Returns the instance of this class.
	/// \return A pointer to the global instance of this singleton class.
	static OrbitMode* instance() {
		static OORef<OrbitMode> instance(new OrbitMode());
		return instance.get();
	}
};

/******************************************************************************
* The pan viewport input mode.
******************************************************************************/
class OVITO_CORE_EXPORT PanMode : public NavigationMode
{
	Q_OBJECT
	
protected:
	
	/// \brief Protected constructor to prevent the creation of multiple instances.
	PanMode() : NavigationMode() {
		setCursor(QCursor(QPixmap(":/core/cursor/viewport/cursor_pan.png")));
	}

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, const QPoint& delta) override;

public:

	/// \brief Returns the instance of this class.
	/// \return A pointer to the global instance of this singleton class.
	static PanMode* instance() {
		static OORef<PanMode> instance(new PanMode());
		return instance.get();
	}
};


/******************************************************************************
* The zoom viewport input mode.
******************************************************************************/
class OVITO_CORE_EXPORT ZoomMode : public NavigationMode
{
	Q_OBJECT

protected:

	/// \brief Protected constructor to prevent the creation of second instances.
	ZoomMode() : NavigationMode() {
		setCursor(QCursor(QPixmap(":/core/cursor/viewport/cursor_zoom.png")));
	}

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, const QPoint& delta) override;

	/// Computes a scaling factor that depends on the total size of the scene which is used to
	/// control the zoom sensitivity in perspective mode.
	static FloatType sceneSizeFactor();

public:

	/// Zooms the viewport in or out.
	void zoom(Viewport* vp, FloatType steps);

	/// \brief Returns the instance of this class.
	/// \return A pointer to the global instance of this singleton class.
	static ZoomMode* instance() { 
		static OORef<ZoomMode> instance(new ZoomMode());
		return instance.get(); 
	}
};

/******************************************************************************
* The field of view input mode.
******************************************************************************/
class OVITO_CORE_EXPORT FOVMode : public NavigationMode
{
	Q_OBJECT

protected:

	/// \brief Protected constructor to prevent the creation of second instances.
	FOVMode() : NavigationMode() {
		setCursor(QCursor(QPixmap(":/core/cursor/viewport/cursor_fov.png")));
	}

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, const QPoint& delta) override;

public:

	/// \brief Returns the instance of this class.
	/// \return A pointer to the global instance of this singleton class.
	static FOVMode* instance() {
		static OORef<FOVMode> instance(new FOVMode());
		return instance.get();
	}
};

/******************************************************************************
* This input mode lets the user pick the center of rotation for the orbit mode.
******************************************************************************/
class OVITO_CORE_EXPORT PickOrbitCenterMode : public ViewportInputHandler
{
public:

	/// Constructor.
	PickOrbitCenterMode() : _showCursor(false) {
		if(Application::instance().guiMode())
			_hoverCursor = QCursor(QPixmap(":/core/cursor/editing/cursor_mode_select.png"));
	}

	/// Returns the activation behavior of this input handler.
	virtual InputHandlerType handlerType() override { return ViewportInputHandler::NORMAL; }

	/// Handles the mouse click event for a Viewport.
	virtual void mousePressEvent(Viewport* vp, QMouseEvent* event) override;

	/// Is called when the user moves the mouse.
	virtual void mouseMoveEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Lets the input mode render its overlay content in a viewport.
	virtual void renderOverlay(Viewport* vp, ViewportSceneRenderer* renderer, bool isActive) override {
		OrbitMode::instance()->renderOverlay(vp, renderer, isActive);
	}

	/// \brief Computes the bounding box of the visual viewport overlay rendered by the input mode.
	virtual Box3 overlayBoundingBox(Viewport* vp, ViewportSceneRenderer* renderer, bool isActive) override {
		return OrbitMode::instance()->overlayBoundingBox(vp, renderer, isActive);
	}

	/// \brief Indicates whether this input mode renders into the viewports.
	virtual bool hasOverlay() override { return true; }

	/// \brief Sets the orbit rotation center to the space location under given mouse coordinates.
	bool pickOrbitCenter(Viewport* vp, const QPoint& pos);

private:

	/// Finds the intersection point between a ray originating from the current mouse cursor position and the scene.
	bool findIntersection(Viewport* vp, const QPoint& mousePos, Point3& intersectionPoint);

	/// The mouse cursor that is shown when over an object.
	QCursor _hoverCursor;

	/// Indicates that the mouse cursor is over an object.
	bool _showCursor;

public:

	/// \brief Returns the instance of this class.
	/// \return A pointer to the global instance of this singleton class.
	static PickOrbitCenterMode* instance() {
		static OORef<PickOrbitCenterMode> instance(new PickOrbitCenterMode());
		return instance.get();
	}
};

};

#endif // __OVITO_NAVIGATION_MODES_H
