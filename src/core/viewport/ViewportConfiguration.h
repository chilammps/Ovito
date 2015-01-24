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

#ifndef __OVITO_VIEWPORT_CONFIGURATION_H
#define __OVITO_VIEWPORT_CONFIGURATION_H

#include <core/Core.h>
#include <core/viewport/Viewport.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(View)

/**
 * \brief This class holds a collection of Viewport objects.
 *
 * It also keeps track of the current viewport and the maximized viewport.
 */
class OVITO_CORE_EXPORT ViewportConfiguration : public RefTarget
{

public:

	enum OrbitCenterMode {
		ORBIT_SELECTION_CENTER,		///< Take the center of mass of the current selection as orbit center.
									///< If there is no selection, use scene bounding box.
		ORBIT_USER_DEFINED			///< Use the orbit center set by the user.
	};
	Q_ENUMS(OrbitCenterMode);

public:

	/// Constructor.
	Q_INVOKABLE ViewportConfiguration(DataSet* dataset);

	/// Returns the list of viewports.
	const QVector<Viewport*>& viewports() const { return _viewports; }

	/// Add a record for a new viewport.
	void addViewport(const OORef<Viewport>& vp) { _viewports.push_back(vp); }

	/// \brief Returns the active viewport.
	/// \return The active Viewport or \c NULL if no viewport is currently active.
	Viewport* activeViewport() { return _activeViewport; }

	/// \brief Returns the maximized viewport.
	/// \return The maximized viewport or \c NULL if no one is currently maximized.
	Viewport* maximizedViewport() { return _maximizedViewport; }

	/// \brief Immediately repaints all viewports that have been scheduled for an update using updateViewports().
	/// \sa updateViewports()
	void processViewportUpdates();

	/// \brief A call to this method suspends redrawing of the viewports.
	///
	/// To resume redrawing of viewports call resumeViewportUpdates().
	///
	/// Calling updateViewports() while redrawing is suspended will update the
	/// viewports as soon as redrawing is resumed.
	///
	/// Normally you should use the ViewportSuspender helper class to suspend viewport update.
	/// It has the advantage of being exception-safe.
	///
	/// \sa resumeViewportUpdates(), isSuspended(), ViewportSuspender
	void suspendViewportUpdates() { _viewportSuspendCount++; }

	/// \brief This will resume redrawing of the viewports after a call to suspendViewportUpdates().
	/// \sa suspendViewportUpdates(), isSuspended()
	void resumeViewportUpdates();

	/// \brief Returns whether viewport updates are suspended.
	/// \return \c true if suspendViewportUpdates() has been called to suspend any viewport updates.
	/// \sa suspendViewportUpdates(), resumeViewportUpdates()
	bool isSuspended() const { return _viewportSuspendCount > 0; }

	/// \brief Returns whether any of the viewports
	///        is currently being updated.
	/// \return \c true if there is currently a rendering operation going on.
	bool isRendering() const;

	/// Returns the renderer to be used for rendering the interactive viewports.
	ViewportSceneRenderer* viewportRenderer();

	/// Changes the way the center of rotation is chosen.
	void setOrbitCenterMode(OrbitCenterMode mode) { _orbitCenterMode = mode; }

	/// Returns the current center of orbit mode.
	OrbitCenterMode orbitCenterMode() { return _orbitCenterMode; }

	/// Sets the user-defined location around which the camera orbits.
	void setUserOrbitCenter(const Point3& center) { _userOrbitCenter = center; }

	/// Returns the user-defined location around which the camera orbits.
	const Point3& userOrbitCenter() const { return _userOrbitCenter; }

	/// Returns the current location around which the viewport camera orbits.
	Point3 orbitCenter();

public Q_SLOTS:

	/// \brief Sets the active viewport.
	/// \param vp The viewport to be made active.
	void setActiveViewport(Viewport* vp) {
		OVITO_ASSERT_MSG(vp == NULL || _viewports.contains(vp), "ViewportConfiguration::setActiveViewport", "Viewport is not in current configuration.");
		_activeViewport = vp;
	}

	/// \brief Maximizes a viewport.
	/// \param vp The viewport to be maximized or \c NULL to restore the currently maximized viewport to
	///           its original state.
	void setMaximizedViewport(Viewport* vp) {
		OVITO_ASSERT_MSG(vp == NULL || _viewports.contains(vp), "ViewportConfiguration::setMaximizedViewport", "Viewport is not in current configuration.");
		_maximizedViewport = vp;
	}

	/// \brief Zooms all viewports to the extents of the currently selected nodes.
	void zoomToSelectionExtents() {
		for(Viewport* vp : viewports())
			vp->zoomToSelectionExtents();
	}

	/// \brief Zooms to the extents of the scene.
	void zoomToSceneExtents() {
		for(Viewport* vp : viewports())
			vp->zoomToSceneExtents();
	}

	/// \brief This will flag all viewports for redrawing.
	///
	/// This function does not cause an immediate repaint of the viewports; instead it schedules a
	/// paint event for processing when Qt returns to the main event loop. You can call this method as often
	/// as you want; it will return immediately and will cause only one viewport repaint when Qt returns to the
	/// main event loop.
	///
	/// To update only a single viewport, Viewport::updateViewport() should be used.
	///
	/// To redraw all viewports immediately without waiting for the paint event to be processed,
	/// call processViewportUpdates() subsequently.
	void updateViewports();

protected:

	/// Is called when the value of a reference field of this RefMaker changes.
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) override;

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

Q_SIGNALS:

	/// This signal is emitted when another viewport became active.
	void activeViewportChanged(Viewport* activeViewport);

	/// This signal is emitted when one of the viewports has been maximized.
	void maximizedViewportChanged(Viewport* maximizedViewport);

	/// This signal is emitted when the camera orbit center haa changed.
	void cameraOrbitCenterChanged();

private:

	/// The list of viewports.
	VectorReferenceField<Viewport> _viewports;

	/// The active viewport. May be NULL.
	ReferenceField<Viewport> _activeViewport;

	/// The maximized viewport or NULL.
	ReferenceField<Viewport> _maximizedViewport;

	/// This counter is for suspending the viewport updates.
	int _viewportSuspendCount;

	/// Indicates that the viewports have been invalidated while updates were suspended.
	bool _viewportsNeedUpdate;

	/// The renderer for the interactive viewports.
	OORef<ViewportSceneRenderer> _viewportRenderer;

	/// Controls around which point the viewport camera should orbit.
    PropertyField<OrbitCenterMode, int> _orbitCenterMode;

	/// Position of the orbiting center picked by the user.
	PropertyField<Point3> _userOrbitCenter;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_viewports);
	DECLARE_REFERENCE_FIELD(_activeViewport);
	DECLARE_REFERENCE_FIELD(_maximizedViewport);
	DECLARE_PROPERTY_FIELD(_orbitCenterMode);
	DECLARE_PROPERTY_FIELD(_userOrbitCenter);
};


/**
 * \brief Small helper class that suspends viewport redrawing while it
 * exists.
 *
 * The constructor of this class calls ViewportConfiguration::suspendViewportUpdates() and
 * the destructor calls ViewportConfiguration::resumeViewportUpdates().
 *
 * Use this to make your code exception-safe.
 * Just create an instance of this class on the stack to suspend viewport updates
 * during the lifetime of the class instance.
 */
class ViewportSuspender {
public:
	ViewportSuspender(ViewportConfiguration* vpconf) : _vpconf(*vpconf) { _vpconf.suspendViewportUpdates(); }
	ViewportSuspender(RefMaker* object) : _vpconf(*object->dataset()->viewportConfig()) { _vpconf.suspendViewportUpdates(); }
	~ViewportSuspender() { _vpconf.resumeViewportUpdates(); }
private:
	ViewportConfiguration& _vpconf;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::ViewportConfiguration::OrbitCenterMode);
Q_DECLARE_TYPEINFO(Ovito::ViewportConfiguration::OrbitCenterMode, Q_PRIMITIVE_TYPE);

#endif		// __OVITO_VIEWPORT_CONFIGURATION_H
