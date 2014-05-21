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
 * \file ViewportWindow.h
 * \brief Contains the definition of the Ovito::ViewportWindow class.
 */

#ifndef __OVITO_VIEWPORT_WINDOW_H
#define __OVITO_VIEWPORT_WINDOW_H

#include <core/Core.h>

namespace Ovito {

/**
 * \brief An internal render window class used by Viewport.
 */
class ViewportWindow : public QWindow
{
public:

	/// Constructor.
	ViewportWindow(Viewport* owner);

    /// \brief Puts an update request event for this window on the event loop.
	void renderLater();

	/// \brief Immediately redraws the contents of this window.
	void renderNow();

	/// If an update request is pending for this viewport window, immediately
	/// processes it and redraw the window contents.
	void processUpdateRequest();

	/// Returns the window's OpenGL context used for rendering.
	QOpenGLContext* glcontext() const { return _context; }

	/// Returns whether all viewport windows should share one GL context or not.
	static bool useMultipleContexts();

protected:

	/// Handles the expose events.
	virtual void exposeEvent(QExposeEvent* event) override;

	/// Handles the resize events.
	virtual void resizeEvent(QResizeEvent* event) override;

	/// Handles double click events.
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

	/// Handles mouse press events.
	virtual void mousePressEvent(QMouseEvent* event) override;

	/// Handles mouse release events.
	virtual void mouseReleaseEvent(QMouseEvent* event) override;

	/// Handles mouse move events.
	virtual void mouseMoveEvent(QMouseEvent* event) override;

	/// Handles mouse wheel events.
	virtual void wheelEvent(QWheelEvent* event) override;

	/// This internal method receives events to the viewport window.
	virtual bool event(QEvent* event) override;

private:

	/// The owning viewport of this window.
	Viewport* _viewport;

	/// A flag that indicates that an update has been requested.
	bool _updateRequested;

	/// A flag that indicates that an update request event has been put on the event queue.
	bool _updatePending;

	/// The OpenGL context used for rendering.
	QOpenGLContext* _context;

	/// The parent window of this viewport window.
	MainWindow* _mainWindow;

private:

	Q_OBJECT
};

};

#endif // __OVITO_VIEWPORT_WINDOW_H
