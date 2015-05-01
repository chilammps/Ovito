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

#ifndef __OVITO_VIEWPORT_WINDOW_H
#define __OVITO_VIEWPORT_WINDOW_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief The internal render window/widget used by the Viewport class.
 */
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
class ViewportWindow : public QWindow
#else
class ViewportWindow : public QOpenGLWidget
#endif
{
public:

	/// Constructor.
	ViewportWindow(Viewport* owner, QWidget* parentWidget);

    /// \brief Puts an update request event for this window on the event loop.
	void renderLater();

	/// \brief Immediately redraws the contents of this window.
	void renderNow();

	/// If an update request is pending for this viewport window, immediately
	/// processes it and redraw the window contents.
	void processUpdateRequest();

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
	/// Returns the window's OpenGL context used for rendering.
	QOpenGLContext* context() const { return _context; }
#else
	/// Mimic isExposed() function of QWindow.
	bool isExposed() const { return isVisible(); }
#endif

	/// Determines whether all viewport windows should share one GL context or not.
	static bool contextSharingEnabled(bool forceDefaultSetting = false);

	/// Determines whether OpenGL point sprites should be used or not.
	static bool pointSpritesEnabled(bool forceDefaultSetting = false);

	/// Determines whether OpenGL geometry shader programs should be used or not.
	static bool geometryShadersEnabled(bool forceDefaultSetting = false);

	/// Determines whether OpenGL geometry shader programs are supported by the hardware.
	static bool geometryShadersSupported() { return _openglSupportsGeomShaders; }

	/// Returns the vendor name of the OpenGL implementation in use.
	static const QByteArray& openGLVendor() { return _openGLVendor; }

	/// Returns the renderer name of the OpenGL implementation in use.
	static const QByteArray& openGLRenderer() { return _openGLRenderer; }

	/// Returns the version string of the OpenGL implementation in use.
	static const QByteArray& openGLVersion() { return _openGLVersion; }

	/// Returns the version of the OpenGL shading language supported by the system.
	static const QByteArray& openGLSLVersion() { return _openGLSLVersion; }

	/// Returns the current surface format used by the OpenGL implementation.
	static const QSurfaceFormat& openglSurfaceFormat() { return _openglSurfaceFormat; }

protected:

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
	/// Handles the expose events.
	virtual void exposeEvent(QExposeEvent* event) override;

	/// Handles the resize events.
	virtual void resizeEvent(QResizeEvent* event) override;

	/// Is called in periodic intervals.
	virtual void timerEvent(QTimerEvent* event) override;

	/// This internal method receives events to the viewport window.
	virtual bool event(QEvent* event) override;
#else
	/// Is called whenever the widget needs to be painted.
	virtual void paintGL() override;

	/// Is called when the mouse cursor leaves the widget.
	virtual void leaveEvent(QEvent* event) override;
#endif

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

	/// Determines the capabilities of the current OpenGL implementation.
	static void determineOpenGLInfo();

private:

	/// The owning viewport of this window.
	Viewport* _viewport;

	/// A flag that indicates that a viewport update has been requested.
	bool _updateRequested;

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
	/// A flag that indicates that an update request event has been put on the event queue.
	bool _updatePending;

	/// The OpenGL context used for rendering.
	QOpenGLContext* _context;
#endif

	/// The parent window of this viewport window.
	MainWindow* _mainWindow;

	/// The vendor of the OpenGL implementation in use.
	static QByteArray _openGLVendor;

	/// The renderer name of the OpenGL implementation in use.
	static QByteArray _openGLRenderer;

	/// The version string of the OpenGL implementation in use.
	static QByteArray _openGLVersion;

	/// The version of the OpenGL shading language supported by the system.
	static QByteArray _openGLSLVersion;

	/// The current surface format used by the OpenGL implementation.
	static QSurfaceFormat _openglSurfaceFormat;

	/// Indicates whether the current OpenGL implementation supports geometry shader programs.
	static bool _openglSupportsGeomShaders;

private:

	Q_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_VIEWPORT_WINDOW_H
