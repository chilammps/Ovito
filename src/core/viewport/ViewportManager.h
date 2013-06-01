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

/**
 * \file ViewportManager.h
 * \brief Contains the definition of the Ovito::ViewportManager class.
 */

#ifndef __OVITO_VIEWPORT_MANAGER_H
#define __OVITO_VIEWPORT_MANAGER_H

#include <core/Core.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportConfiguration.h>

namespace Ovito {

/**
 * \brief Manages the viewports.
 */
class ViewportManager : public QObject
{
	Q_OBJECT

public:

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the ViewportManager singleton class.
	inline static ViewportManager& instance() {
		if(!_instance) _instance.reset(new ViewportManager());
		return *_instance.data();
	}

	/// \brief Returns the active viewport.
	/// \return The active Viewport or \c NULL if no viewport is currently active.
	/// \sa setActiveViewport()
	Viewport* activeViewport() { return _activeViewport; }

	/// \brief Sets the active viewport.
	/// \param vp The viewport to be made active.
	/// \note Calling this method will redraw all viewports.
	/// \sa activeViewport()
	void setActiveViewport(Viewport* vp);

	/// \brief Returns the maximized viewport.
	/// \return The maximized viewport or \c NULL if no one is currently maximized.
	/// \sa setMaximizedViewport()
	Viewport* maximizedViewport() { return _maximizedViewport; }

	/// \brief Maximizes a viewport.
	/// \param vp The viewport to be maximized or \c NULL to restore the currently maximized viewport to
	///           its original state.
	/// \sa maximizedViewport()
	void setMaximizedViewport(Viewport* vp);

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
	///
	/// \sa Viewport::updateViewport(), processViewportUpdates()
	void updateViewports();

	/// \brief Immediately repaints all viewport that have been flagged for
	///        an update using updateViewports().
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

	/// \brief Returns whether any of the viewports in the main viewport panel
	///        is currently being updated.
	/// \return \c true if there is currently a rendering operation going on.
	///
	/// No new windows or dialogs should be shown during this phase
	/// to prevent an infinite update loop.
	///
	/// \sa Window3DContainer::isRendering()
	bool isRendering() const;

	/// \brief Returns all viewports of the main viewport panel.
	/// \return The list of viewports.
	const QVector<Viewport*>& viewports() const { return _viewports; }

	/// \brief Returns the default ViewportConfiguration used for new scene files.
	/// \return Pointer to the default viewport configuration stored by the ViewportManager.
	/// \note You may change the returned ViewportConfiguration object to modify the
	///       default configuration used for new scene files.
	/// \note Instead of using the returned object in a scene file you should make a copy of it first
	///       and leave the original object intact.
	OORef<ViewportConfigurationSet> defaultConfiguration();

Q_SIGNALS:

	/// This signal is emitted when another viewport became active.
	void activeViewportChanged(Viewport* activeViewport);

	/// This signal is emitted when a viewport has been maximized.
	void maximizedViewportChanged(Viewport* maximizedViewport);

private:

	/// The active viewport.
	Viewport* _activeViewport;

	/// The maximized viewport if there is one.
	Viewport* _maximizedViewport;

	/// This counter is for suspending the viewport updates.
	int _viewportSuspendCount;

	/// Indicates that the viewports have been invalidated while updates were suspended.
	bool _viewportsNeedUpdate;

	/// This holds the default configuration of viewports to use.
	OORef<ViewportConfigurationSet> _defaultConfig;

	/// The list of all viewports.
	QVector<Viewport*> _viewports;

private:

	/// Private constructor.
	/// This is a singleton class; no public instances are allowed.
	ViewportManager();

	/// The singleton instance of this class.
	static QScopedPointer<ViewportManager> _instance;
};

/**
 * \brief Small helper class that suspends viewport redrawing while it
 * exists. It can be used to make your code exception-safe.
 *
 * The constructor of this class calls ViewportManager::suspendViewportUpdates() and
 * the destructor calls ViewportManager::resumeViewportUpdates().
 *
 * Just create an instance of this class on the stack to suspend viewport updates
 * during the lifetime of the class instance.
 */
class ViewportSuspender {
public:
	ViewportSuspender() { ViewportManager::instance().suspendViewportUpdates(); }
	~ViewportSuspender() { ViewportManager::instance().resumeViewportUpdates(); }
};

};

#endif // __OVITO_VIEWPORT_MANAGER_H
