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

namespace Ovito {

/// The vendor of the OpenGL implementation in use.
QByteArray ViewportWindow::_openGLVendor;

/******************************************************************************
* Returns whether all viewport windows should share one GL context or not.
******************************************************************************/
bool ViewportWindow::contextSharingEnabled(bool forceDefaultSetting)
{
	if(!forceDefaultSetting) {
		// The user can control the use of multiple GL contexts.
		QVariant userSetting = QSettings().value("display/share_opengl_context");
		if(userSetting.isValid())
			return userSetting.toBool();
	}

#if defined(Q_OS_OSX)
	// On Mac OS X 10.9, sharing a single context doesn't work very well.
	return false;
#elif defined(Q_OS_LINUX)
	// On Intel graphics under Linux, sharing a single context doesn't work very well.
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
		// The user can control the use of point sprites.
		QVariant userSetting = QSettings().value("display/use_point_sprites");
		if(userSetting.isValid())
			return userSetting.toBool();
	}

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
	if(mode)
		mode->mouseDoubleClickEvent(_viewport, event);
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
	if(mode)
		mode->mousePressEvent(_viewport, event);
}

/******************************************************************************
* Handles mouse release events.
******************************************************************************/
void ViewportWindow::mouseReleaseEvent(QMouseEvent* event)
{
	ViewportInputMode* mode = _mainWindow->viewportInputManager()->activeMode();
	if(mode)
		mode->mouseReleaseEvent(_viewport, event);
}

/******************************************************************************
* Handles mouse move events.
******************************************************************************/
void ViewportWindow::mouseMoveEvent(QMouseEvent* event)
{
	ViewportInputMode* mode = _mainWindow->viewportInputManager()->activeMode();
	if(mode)
		mode->mouseMoveEvent(_viewport, event);
}

/******************************************************************************
* Handles mouse wheel events.
******************************************************************************/
void ViewportWindow::wheelEvent(QWheelEvent* event)
{
	ViewportInputMode* mode = _mainWindow->viewportInputManager()->activeMode();
	if(mode)
		mode->wheelEvent(_viewport, event);
}

/******************************************************************************
* Immediately redraws the contents of this window.
******************************************************************************/
void ViewportWindow::renderNow()
{
	if(!isExposed())
		return;

	_updateRequested = false;

	if(!_context->makeCurrent(this)) {
		qWarning() << "Failed to make OpenGL context current.";
		return;
	}
	OVITO_CHECK_OPENGL();

#ifdef OVITO_DEBUG
	static bool firstTime = true;
	if(firstTime) {
		firstTime = false;
		QSurfaceFormat format = _context->format();
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
	}
#endif

	if(_context->format().majorVersion() < OVITO_OPENGL_MINIMUM_VERSION_MAJOR || (_context->format().majorVersion() == OVITO_OPENGL_MINIMUM_VERSION_MAJOR && _context->format().minorVersion() < OVITO_OPENGL_MINIMUM_VERSION_MINOR)) {
		Exception ex(tr(
				"The OpenGL implementation available on this system does not support OpenGL version %4.%5 or newer.\n\n"
				"Ovito requires modern graphics hardware and up-to-date graphics drivers to display 3D content. Your current system configuration is not compatible with Ovito and the application will now quit.\n\n"
				"To avoid this error message, please install the newest graphics driver of the hardware vendor, or upgrade your graphics card.\n\n"
				"The installed OpenGL graphics driver reports the following information:\n\n"
				"OpenGL Vendor: %1\n"
				"OpenGL Renderer: %2\n"
				"OpenGL Version: %3\n\n"
				"Ovito requires OpenGL version %4.%5 or higher.")
				.arg(QString((const char*)glGetString(GL_VENDOR)))
				.arg(QString((const char*)glGetString(GL_RENDERER)))
				.arg(QString((const char*)glGetString(GL_VERSION)))
				.arg(OVITO_OPENGL_MINIMUM_VERSION_MAJOR)
				.arg(OVITO_OPENGL_MINIMUM_VERSION_MINOR)
				);
		_viewport->dataset()->viewportConfig()->suspendViewportUpdates();
		QCoreApplication::removePostedEvents(nullptr, 0);
		ex.showError();
		QCoreApplication::instance()->quit();
		return;
	}

	// Store OpenGL vendor string so other parts of the code can decide
	// which OpenGL features are save to use.
	if(_openGLVendor.isEmpty())
		_openGLVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));

	OVITO_CHECK_OPENGL();
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

	OVITO_CHECK_OPENGL();
	_context->doneCurrent();
}

};
