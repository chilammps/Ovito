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
#include <core/viewport/ViewportWindow.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>
#include <core/gui/mainwin/MainWindow.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/// The vendor of the OpenGL implementation in use.
QByteArray ViewportWindow::_openGLVendor;

/// Indicates whether the OpenGL implementation supports geometry shader programs.
bool ViewportWindow::_openglSupportsGeomShaders = false;

/******************************************************************************
* Returns whether all viewport windows should share one GL context or not.
******************************************************************************/
bool ViewportWindow::contextSharingEnabled(bool forceDefaultSetting)
{
	if(!forceDefaultSetting) {
		// The user can override the use of multiple GL contexts.
		QVariant userSetting = QSettings().value("display/share_opengl_context");
		if(userSetting.isValid())
			return userSetting.toBool();
	}

	if(_openGLVendor.isEmpty() && QOpenGLContext::currentContext())
		_openGLVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));

#if defined(Q_OS_OSX)
	// On Mac OS X 10.9 with Intel graphics, using a single context for multiple viewports doesn't work very well.
	return false;
#elif defined(Q_OS_LINUX)
	// On Intel graphics under Linux, sharing a single context doesn't work very well either.
	if(_openGLVendor.contains("Intel"))
		return false;
#endif

	// By default, all viewports of a main window use the same GL context.
	return true;
}

/******************************************************************************
* Determines whether OpenGL point sprites should be used or not.
******************************************************************************/
bool ViewportWindow::pointSpritesEnabled(bool forceDefaultSetting)
{
	if(!forceDefaultSetting) {
		// The user can override the use of point sprites.
		QVariant userSetting = QSettings().value("display/use_point_sprites");
		if(userSetting.isValid())
			return userSetting.toBool();
	}

	if(_openGLVendor.isEmpty() && QOpenGLContext::currentContext())
		_openGLVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));

#if defined(Q_OS_WIN)
	// Point sprites don't seem to work well on Intel graphics under Windows.
	if(_openGLVendor.contains("Intel"))
		return false;
#elif defined(Q_OS_OSX)
	// Point sprites don't seem to work well on ATI graphics under Mac OS X.
	if(_openGLVendor.contains("ATI"))
		return false;
#endif

	// Use point sprites by default.
	return true;
}

/******************************************************************************
* Determines whether OpenGL geometry shader programs should be used or not.
******************************************************************************/
bool ViewportWindow::geometryShadersEnabled(bool forceDefaultSetting)
{
	if(!forceDefaultSetting) {
		// The user can override the use of geometry shaders.
		QVariant userSetting = QSettings().value("display/use_geometry_shaders");
		if(userSetting.isValid())
			return userSetting.toBool() && _openglSupportsGeomShaders;
	}
	if(Application::instance().guiMode())
		return _openglSupportsGeomShaders;
	else if(QOpenGLContext::currentContext())
		return QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Geometry);
	else
		return false;
}

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportWindow::ViewportWindow(Viewport* owner) :
		_viewport(owner), _updateRequested(false), _updatePending(false),
		_context(nullptr),
		_mainWindow(owner->dataset()->mainWindow())
{
	if(contextSharingEnabled()) {
		// Get the master OpenGL context, which is managed by the main window.
		OVITO_CHECK_POINTER(_mainWindow);
		_context = _mainWindow->getOpenGLContext();
	}
	else {
		// Create a dedicated OpenGL context for this viewport window.
		// All contexts still share OpenGL resources.
		_context = new QOpenGLContext(this);
		_context->setFormat(ViewportSceneRenderer::getDefaultSurfaceFormat());
		_context->setShareContext(_mainWindow->getOpenGLContext());
		if(!_context->create())
			throw Exception(tr("Failed to create OpenGL context."));
	}

	// Determine OpenGL vendor string so other parts of the code can decide
	// which OpenGL features are save to use.
	if(_openGLVendor.isEmpty()) {
		QOffscreenSurface offscreenSurface;
		offscreenSurface.setFormat(_context->format());
		offscreenSurface.create();
		if(!offscreenSurface.isValid())
			throw Exception(tr("Failed to create offscreen surface. Cannot query OpenGL vendor string."));
		if(!_context->makeCurrent(&offscreenSurface))
			throw Exception(tr("Failed to make OpenGL context current on offscreen surface. Cannot query OpenGL vendor string."));
		_openGLVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		_openglSupportsGeomShaders = QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Geometry);
		_context->doneCurrent();
	}

	// Indicate that the window is to be used for OpenGL rendering.
	setSurfaceType(QWindow::OpenGLSurface);
	setFormat(_context->format());
}

/******************************************************************************
* Puts an update request event for this viewport on the event loop.
******************************************************************************/
void ViewportWindow::renderLater()
{
	_updateRequested = true;
	// If not already done so, put an update request event on the event loop,
	// which leads to renderNow() being called once the event gets processed.
	if(!_updatePending) {
		_updatePending = true;
		QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateLater));
	}
}

/******************************************************************************
* This internal method receives events to the viewport window.
******************************************************************************/
bool ViewportWindow::event(QEvent* event)
{
	// Handle update request events creates by renderLater().
	if(event->type() == QEvent::UpdateLater) {
		_updatePending = false;
		processUpdateRequest();
		return true;
	}
	return QWindow::event(event);
}

/******************************************************************************
* If an update request is pending for this viewport window, immediately
* processes it and redraw the window contents.
******************************************************************************/
void ViewportWindow::processUpdateRequest()
{
	if(_updateRequested)
		renderNow();
}

/******************************************************************************
* Handles the expose events.
******************************************************************************/
void ViewportWindow::exposeEvent(QExposeEvent*)
{
	if(isExposed()) {
		renderNow();
	}
}

/******************************************************************************
* Handles the resize events.
******************************************************************************/
void ViewportWindow::resizeEvent(QResizeEvent*)
{
	if(isExposed()) {
		renderNow();
	}
}

/******************************************************************************
* Handles double click events.
******************************************************************************/
void ViewportWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
	ViewportInputMode* mode = _mainWindow->viewportInputManager()->activeMode();
	if(mode) {
		try {
			mode->mouseDoubleClickEvent(_viewport, event);
		}
		catch(const Exception& ex) {
			qWarning() << "Uncaught exception in mouse event handler:";
			ex.logError();
		}
	}
}

/******************************************************************************
* Handles mouse press events.
******************************************************************************/
void ViewportWindow::mousePressEvent(QMouseEvent* event)
{
	_viewport->dataset()->viewportConfig()->setActiveViewport(_viewport);

	// Intercept mouse clicks on the viewport caption.
	if(_viewport->_contextMenuArea.contains(event->pos())) {
		_viewport->showViewportMenu(event->pos());
		return;
	}

	ViewportInputMode* mode = _mainWindow->viewportInputManager()->activeMode();
	if(mode) {
		try {
			mode->mousePressEvent(_viewport, event);
		}
		catch(const Exception& ex) {
			qWarning() << "Uncaught exception in mouse event handler:";
			ex.logError();
		}
	}
}

/******************************************************************************
* Handles mouse release events.
******************************************************************************/
void ViewportWindow::mouseReleaseEvent(QMouseEvent* event)
{
	ViewportInputMode* mode = _mainWindow->viewportInputManager()->activeMode();
	if(mode) {
		try {
			mode->mouseReleaseEvent(_viewport, event);
		}
		catch(const Exception& ex) {
			qWarning() << "Uncaught exception in mouse event handler:";
			ex.logError();
		}
	}
}

/******************************************************************************
* Handles mouse move events.
******************************************************************************/
void ViewportWindow::mouseMoveEvent(QMouseEvent* event)
{
	if(_viewport->_contextMenuArea.contains(event->pos()) && !_viewport->_cursorInContextMenuArea) {
		_viewport->_cursorInContextMenuArea = true;
		_viewport->updateViewport();
		startTimer(0);
	}
	else if(!_viewport->_contextMenuArea.contains(event->pos()) && _viewport->_cursorInContextMenuArea) {
		_viewport->_cursorInContextMenuArea = false;
		_viewport->updateViewport();
	}

	ViewportInputMode* mode = _mainWindow->viewportInputManager()->activeMode();
	if(mode) {
		try {
			mode->mouseMoveEvent(_viewport, event);
		}
		catch(const Exception& ex) {
			qWarning() << "Uncaught exception in mouse event handler:";
			ex.logError();
		}
	}
}

/******************************************************************************
* Handles mouse wheel events.
******************************************************************************/
void ViewportWindow::wheelEvent(QWheelEvent* event)
{
	ViewportInputMode* mode = _mainWindow->viewportInputManager()->activeMode();
	if(mode) {
		try {
			mode->wheelEvent(_viewport, event);
		}
		catch(const Exception& ex) {
			qWarning() << "Uncaught exception in mouse event handler:";
			ex.logError();
		}
	}
}

/******************************************************************************
* Is called in periodic intervals.
******************************************************************************/
void ViewportWindow::timerEvent(QTimerEvent* event)
{
	if(_viewport->_contextMenuArea.contains(mapFromGlobal(QCursor::pos())))
		return;

	if(_viewport->_cursorInContextMenuArea) {
		_viewport->_cursorInContextMenuArea = false;
		_viewport->updateViewport();
	}
	killTimer(event->timerId());
}

/******************************************************************************
* Immediately redraws the contents of this window.
******************************************************************************/
void ViewportWindow::renderNow()
{
	if(!isExposed())
		return;

	_updateRequested = false;

	// Do not re-enter rendering function of the same viewport.
	if(_viewport->isRendering())
		return;

	// Before making our GL context current, remember the old context that
	// is currently active so we can restore it when we are done.
	// This is necessary, because multiple viewport repaint requests can be
	// processed simultaneously.
	QPointer<QOpenGLContext> oldContext = QOpenGLContext::currentContext();
	QSurface* oldSurface = oldContext ? oldContext->surface() : nullptr;

	if(!_context->makeCurrent(this)) {
		qWarning() << "Failed to make OpenGL context current.";
		return;
	}
	OVITO_REPORT_OPENGL_ERRORS();

	QSurfaceFormat format = _context->format();
	// OpenGL in a VirtualBox machine Windows guest reports "2.1 Chromium 1.9" as version string, which is
	// not correctly parsed by Qt. We have to workaround this.
	if(qstrncmp((const char*)glGetString(GL_VERSION), "2.1 ", 4) == 0) {
		format.setMajorVersion(2);
		format.setMinorVersion(1);
	}

#ifdef OVITO_DEBUG
	static bool firstTime = true;
	if(firstTime) {
		firstTime = false;
		qDebug() << "OpenGL depth buffer size:   " << format.depthBufferSize();
		(qDebug() << "OpenGL version:             ").nospace() << format.majorVersion() << "." << format.minorVersion();
		qDebug() << "OpenGL profile:             " << (format.profile() == QSurfaceFormat::CoreProfile ? "core" : (format.profile() == QSurfaceFormat::CompatibilityProfile ? "compatibility" : "none"));
		qDebug() << "OpenGL has alpha:           " << format.hasAlpha();
		qDebug() << "OpenGL vendor:              " << QString((const char*)glGetString(GL_VENDOR));
		qDebug() << "OpenGL renderer:            " << QString((const char*)glGetString(GL_RENDERER));
		qDebug() << "OpenGL version string:      " << QString((const char*)glGetString(GL_VERSION));
		qDebug() << "OpenGL shading language:    " << QString((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		qDebug() << "OpenGL shader programs:     " << QOpenGLShaderProgram::hasOpenGLShaderPrograms();
		qDebug() << "OpenGL vertex shaders:      " << QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Vertex);
		qDebug() << "OpenGL fragment shaders:    " << QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Fragment);
		qDebug() << "OpenGL geometry shaders:    " << QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Geometry);
		qDebug() << "OpenGL swap behavior:       " << (format.swapBehavior() == QSurfaceFormat::SingleBuffer ? QStringLiteral("single buffer") : (format.swapBehavior() == QSurfaceFormat::DoubleBuffer ? QStringLiteral("double buffer") : (format.swapBehavior() == QSurfaceFormat::TripleBuffer ? QStringLiteral("triple buffer") : QStringLiteral("other"))));
		qDebug() << "OpenGL stencil buffer size: " << format.stencilBufferSize();
		qDebug() << "OpenGL deprecated func:     " << format.testOption(QSurfaceFormat::DeprecatedFunctions);
		//qDebug() << "OpenGL extensions:          " << QString((const char*)glGetString(GL_EXTENSIONS));
	}
#endif

	if(format.majorVersion() < OVITO_OPENGL_MINIMUM_VERSION_MAJOR || (format.majorVersion() == OVITO_OPENGL_MINIMUM_VERSION_MAJOR && format.minorVersion() < OVITO_OPENGL_MINIMUM_VERSION_MINOR)) {
		// Avoid infinite recursion.
		static bool errorMessageShown = false;
		if(!errorMessageShown) {
			errorMessageShown = true;
			_viewport->dataset()->viewportConfig()->suspendViewportUpdates();
			Exception ex(tr(
					"The OpenGL graphics driver installed on this system does not support OpenGL version %6.%7 or newer.\n\n"
					"Ovito requires modern graphics hardware and up-to-date graphics drivers to display 3D content. Your current system configuration is not compatible with Ovito and the application will quit now.\n\n"
					"To avoid this error, please install the newest graphics driver of the hardware vendor or, if necessary, consider replacing your graphics card with a newer model.\n\n"
					"The installed OpenGL graphics driver reports the following information:\n\n"
					"OpenGL vendor: %1\n"
					"OpenGL renderer: %2\n"
					"OpenGL version: %3.%4 (%5)\n\n"
					"Ovito requires at least OpenGL version %6.%7.")
					.arg(QString((const char*)glGetString(GL_VENDOR)))
					.arg(QString((const char*)glGetString(GL_RENDERER)))
					.arg(_context->format().majorVersion())
					.arg(_context->format().minorVersion())
					.arg(QString((const char*)glGetString(GL_VERSION)))
					.arg(OVITO_OPENGL_MINIMUM_VERSION_MAJOR)
					.arg(OVITO_OPENGL_MINIMUM_VERSION_MINOR)
				);
			ex.showError();
			QCoreApplication::removePostedEvents(nullptr, 0);
			QCoreApplication::instance()->quit();
		}
		return;
	}

	OVITO_REPORT_OPENGL_ERRORS();
	if(!_viewport->dataset()->viewportConfig()->isSuspended()) {
		_viewport->render(_context);
	}
	else {
		Color backgroundColor = Viewport::viewportColor(ViewportSettings::COLOR_VIEWPORT_BKG);
		OVITO_CHECK_OPENGL(glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), 1));
		OVITO_CHECK_OPENGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
		_viewport->dataset()->viewportConfig()->updateViewports();
	}
	_context->swapBuffers(this);

	OVITO_REPORT_OPENGL_ERRORS();

	// Restore old GL context.
	if(oldSurface && oldContext) {
		if(!oldContext->makeCurrent(oldSurface))
			qWarning() << "Failed to restore old OpenGL context.";
	}
	else {
		_context->doneCurrent();
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
