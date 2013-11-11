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
#include <core/viewport/ViewportManager.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>

namespace Ovito {

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportWindow::ViewportWindow(Viewport* owner) :
		_viewport(owner), _updateRequested(false), _updatePending(false),
		_context(nullptr), _oglDebugLogger(nullptr)
{
	// Indicate that the window is to be used for OpenGL rendering.
	setSurfaceType(QWindow::OpenGLSurface);

	// Indicate that we want a depth buffer.
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setMajorVersion(OVITO_OPENGL_REQUESTED_VERSION_MAJOR);
	format.setMinorVersion(OVITO_OPENGL_REQUESTED_VERSION_MINOR);
	format.setProfile(QSurfaceFormat::CoreProfile);
#ifdef OVITO_DEBUG
	format.setOption(QSurfaceFormat::DebugContext);
#endif
	setFormat(format);
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
	ViewportInputHandler* handler = ViewportInputManager::instance().currentHandler();
	if(handler)
		handler->mouseDoubleClickEvent(_viewport, event);
}

/******************************************************************************
* Handles mouse press events.
******************************************************************************/
void ViewportWindow::mousePressEvent(QMouseEvent* event)
{
	ViewportManager::instance().setActiveViewport(_viewport);

	// Intercept mouse clicks on the viewport caption.
	if(_viewport->_contextMenuArea.contains(event->pos())) {
		_viewport->showViewportMenu(event->pos());
		return;
	}

	ViewportInputHandler* handler = ViewportInputManager::instance().currentHandler();
	if(handler)
		handler->mousePressEvent(_viewport, event);
}

/******************************************************************************
* Handles mouse release events.
******************************************************************************/
void ViewportWindow::mouseReleaseEvent(QMouseEvent* event)
{
	ViewportInputHandler* handler = ViewportInputManager::instance().currentHandler();
	if(handler)
		handler->mouseReleaseEvent(_viewport, event);
}

/******************************************************************************
* Handles mouse move events.
******************************************************************************/
void ViewportWindow::mouseMoveEvent(QMouseEvent* event)
{
	ViewportInputHandler* handler = ViewportInputManager::instance().currentHandler();
	if(handler)
		handler->mouseMoveEvent(_viewport, event);
}

/******************************************************************************
* Handles mouse wheel events.
******************************************************************************/
void ViewportWindow::wheelEvent(QWheelEvent* event)
{
	ViewportInputHandler* handler = ViewportInputManager::instance().currentHandler();
	if(handler)
		handler->wheelEvent(_viewport, event);
}

/******************************************************************************
* Immediately redraws the contents of this window.
******************************************************************************/
void ViewportWindow::renderNow()
{
	if(!isExposed())
		return;

	if(ViewportManager::instance().isSuspended())
		return;

	_updateRequested = false;

	// Create OpenGL context on first redraw.
	if(!_context) {
		_context = new QOpenGLContext(this);

		// Look for other existing viewport windows that we can share the OpenGL context with.
		QOpenGLContext* shareContext = nullptr;
		for(Viewport* vp : ViewportManager::instance().viewports()) {
			if(vp != _viewport && vp->_viewportWindow) {
				shareContext = vp->_viewportWindow->glcontext();
				if(shareContext) break;
			}
		}
		_context->setShareContext(shareContext);
		_context->setFormat(requestedFormat());
		if(!_context->create())
			throw Exception(tr("Failed to create OpenGL context."));
		if(shareContext && _context->shareContext() != shareContext)
			qWarning() << "Viewport cannot share OpenGL context with other viewports.";
		_context->makeCurrent(this);

#ifdef OVITO_DEBUG
		// Initialize the the OpenGL debug logger object.
		// It will forward error messages from the OpenGL server to the console.
		_oglDebugLogger = new QOpenGLDebugLogger(this);
		if(!_oglDebugLogger->initialize()) {
			delete _oglDebugLogger;
			_oglDebugLogger = nullptr;
		}
		else {
			for(const QOpenGLDebugMessage& message : _oglDebugLogger->loggedMessages())
				openGLDebugMessage(message);
			connect(_oglDebugLogger, &QOpenGLDebugLogger::messageLogged, this, &ViewportWindow::openGLDebugMessage);
			_oglDebugLogger->startLogging();
		}
#endif

		static bool firstTime = true;
		if(!shareContext && firstTime) {
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
			qDebug() << "OpenGL debug logger:        " << (_oglDebugLogger != nullptr);
		}

		if(_context->format().majorVersion() < OVITO_OPENGL_MINIMUM_VERSION_MAJOR || _context->format().minorVersion() < OVITO_OPENGL_MINIMUM_VERSION_MINOR) {
			Exception ex(tr(
					"The OpenGL implementation available on this system does not support OpenGL version %4.%5 or newer.\n\n"
					"Ovito requires modern graphics hardware and up-to-date graphics drivers to display 3D content. Your current system configuration is not compatible with Ovito and the application will quit now.\n\n"
					"To avoid this error message, please install the newest graphics driver, or upgrade your graphics card.\n\n"
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
			ViewportManager::instance().suspendViewportUpdates();
			QCoreApplication::removePostedEvents(nullptr, 0);
			ex.showError();
			QCoreApplication::instance()->quit();
			return;
		}
	}

	if(!_context->makeCurrent(this)) {
		qWarning() << "Failed to make OpenGL context current.";
		return;
	}

	_viewport->render(_context);
	_context->swapBuffers(this);

	_context->doneCurrent();
}

/******************************************************************************
* This receives log messages from the QOpenGLDebugLogger.
******************************************************************************/
void ViewportWindow::openGLDebugMessage(const QOpenGLDebugMessage& debugMessage)
{
	qDebug().nospace() << "Viewport " << _viewport->viewportTitle() << ": " << debugMessage.message() << " (" << debugMessage.id() << ")";
}

};
