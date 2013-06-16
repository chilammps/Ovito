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
#include "ViewportInputHandler.h"

namespace Ovito {

/**
 * \brief Base class for viewport navigation modes likes zoom, pan and orbit.
 */
class NavigationMode : public ViewportInputHandler
{
	Q_OBJECT
	
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
};

/******************************************************************************
* The pan viewport input mode.
******************************************************************************/
class PanMode : public NavigationMode
{
	Q_OBJECT
	
protected:
	
	/// \brief Protected constructor to prevent the creation of multiple instances.
	PanMode() : NavigationMode() {}

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, const QPoint& delta) override;

	/// Gets the mouse cursor of this viewport mode.
	virtual QCursor getCursor() override { return QCursor(QPixmap(":/core/cursor/viewport/cursor_pan.png")); }

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
class ZoomMode : public NavigationMode
{
	Q_OBJECT

protected:

	/// \brief Protected constructor to prevent the creation of second instances.
	ZoomMode() : NavigationMode() {}

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, const QPoint& delta) override;

	/// Gets the mouse cursor of this viewport mode.
	virtual QCursor getCursor() override { return QCursor(QPixmap(":/core/cursor/viewport/cursor_zoom.png")); }

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
class FOVMode : public NavigationMode
{
	Q_OBJECT

protected:

	/// \brief Protected constructor to prevent the creation of second instances.
	FOVMode() : NavigationMode() {}

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, const QPoint& delta) override;

	/// Gets the mouse cursor of this viewport mode.
	virtual QCursor getCursor() override { return QCursor(QPixmap(":/core/cursor/viewport/cursor_fov.png")); }

public:

	/// \brief Returns the instance of this class.
	/// \return A pointer to the global instance of this singleton class.
	static FOVMode* instance() {
		static OORef<FOVMode> instance(new FOVMode());
		return instance.get();
	}
};

/******************************************************************************
* The orbit viewport input mode.
******************************************************************************/
class OrbitMode : public NavigationMode
{
	Q_OBJECT
	
public:

	enum CenterMode {
		ORBIT_CONSTRUCTION_PLANE,	/// Take the current construction plane as orbit center.
		ORBIT_SELECTION_CENTER,		/// Take the center of mass of the current selection as orbit center.
		ORBIT_USER_DEFINED			/// Use the orbit center set by the user.
	};

	/// Changes the way the center of rotation is chosen.
	void setCenterMode(CenterMode mode);

	/// Returns the current center of orbit mode.
	CenterMode centerMode() const { return _centerMode; }

	/// Sets the world space point around which the camera orbits.
	void setOrbitCenter(const Point3& center);

	/// Returns the world space point around which the camera orbits.
	const Point3& orbitCenter() const { return _orbitCenter; }

	/// \brief Lets the input mode render its overlay content in a viewport.
	virtual void renderOverlay(Viewport* vp, bool isActive) override;

	/// \brief Indicates whether this input mode renders into the viewports.
	virtual bool hasOverlay() override { return true; }

protected:

	/// \brief Protected constructor to prevent the creation of second instances.
	OrbitMode() : NavigationMode(), _centerMode(ORBIT_SELECTION_CENTER), _orbitCenter(Point3::Origin()) {}

	/// Computes the new view based on the new mouse position.
	virtual void modifyView(Viewport* vp, const QPoint& delta) override;

	/// Gets the mouse cursor of this viewport mode.
	virtual QCursor getCursor() override { return QCursor(QPixmap(":/core/cursor/viewport/cursor_orbit.png")); }

	/// \brief Handles the mouse down event for the given viewport.
	virtual void mousePressEvent(Viewport* vp, QMouseEvent* event) override;

protected:

	/// Contains the current orbiting center.
	Point3 _orbitCenter;
	
	/// Indicates around which point the camera should orbit.
	CenterMode _centerMode;

public:

	/// \brief Returns the instance of this class.
	/// \return A pointer to the global instance of this singleton class.
	static OrbitMode* instance() { 
		static OORef<OrbitMode> instance(new OrbitMode());
		return instance.get(); 
	}
};

#if 0

/******************************************************************************
* This input mode lets the user pick the center of rotation for the orbit mode.
******************************************************************************/
class PickOrbitCenterMode : public SimpleInputHandler
{
public:

	/// Constructor.
	PickOrbitCenterMode() : showCursor(false) {
		if(APPLICATION_MANAGER.guiMode())
			hoverCursor = QCursor(QPixmap(":/core/main/cursor_mode_select.png"));
	}

	/// Returns the activation behaviour of this input handler.
	virtual InputHandlerType handlerActivationType() { return ViewportInputHandler::NORMAL; }

	/// Handles the mouse click event for a Viewport.
	virtual void onMousePressed(QMouseEvent* event);

	/// Is called when the user moves the mouse without pressing the button.
	virtual void onMouseFreeMove(Viewport& vp, QMouseEvent* event);

	/// Gets the current cursor of this viewport mode.
	virtual QCursor getCursor() {
		if(showCursor)
			return hoverCursor;
		else
			return SimpleInputHandler::getCursor();
	}

	/// \brief Lets the input mode render its overlay content in a viewport.
	virtual void renderOverlay(Viewport* vp, bool isActive) { OrbitMode::instance()->renderOverlay(vp, isActive); }

	/// \brief Indicates whether this input mode renders into the viewports.
	virtual bool hasOverlay() { return true; }

	/// \brief Returns the instance of this class.
	/// \return A pointer to the global instance of this singleton class.
	static PickOrbitCenterMode* instance() {
		static intrusive_ptr<PickOrbitCenterMode> instance(new PickOrbitCenterMode());
		return instance.get();
	}

private:

	/// Finds the intersection point between a ray originating from the current mouse cursor position and the scene.
	bool findIntersection(Viewport* vp, const Point2I& mousePos, Point3& intersectionPoint);

	/// The mouse cursor that is shown when over an object.
	QCursor hoverCursor;

	/// Indicates that the mouse cursor is over an object.
	bool showCursor;
};

#endif

};

#endif // __OVITO_NAVIGATION_MODES_H
