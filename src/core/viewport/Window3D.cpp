///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2008) Alexander Stukowski
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
#include <core/viewport/Window3D.h>
#include <core/viewport/ViewportManager.h>

namespace Core {

IMPLEMENT_ABSTRACT_CLASS(Window3D, QGLWidget)

QSet<Window3D*> Window3D::windowsWithPendingUpdates;
size_t Window3D::_sharingContextCounter = 0;

const static QGLFormat defaultGLFormat(QGL::FormatOptions(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba | QGL::DirectRendering | QGL::NoAccumBuffer | QGL::NoStencilBuffer));

static Window3D* findSharedContextWidget(QWidget* parent) {
	return parent->findChild<Window3D*>();
}

/******************************************************************************
* The constructor of the 3d window class.
******************************************************************************/
Window3D::Window3D(QWidget* parent) : QGLWidget(defaultGLFormat, parent, findSharedContextWidget(parent)),
	_isRendering(false), _needsUpdate(true),
	_viewMatrix(IDENTITY), _worldMatrix(IDENTITY), _projMatrix(IDENTITY),
	_viewMatrixInv(IDENTITY), _worldMatrixInv(IDENTITY), _projMatrixInv(IDENTITY),
	_objToViewMatrix(IDENTITY), _objToViewMatrixInv(IDENTITY), _objToScreenMatrix(IDENTITY),
	_depthTest(true), _backfaceCulling(true),
	_isEnabled(true), _isVisible(true), _lightingEnabled(false),
	_pickRegion(NULL), _closestHitDistance(HIT_TEST_NONE), _realizedMaterial(-1)
{
	CHECK_POINTER(parent);

	// Do not need to paint the widget background because OpenGL fills the whole canvas.
	setAutoFillBackground(false);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_NoSystemBackground);

	setMouseTracking(true);

	Window3D::windowsWithPendingUpdates.insert(this);
	if(isSharing()) {
		Window3D* partner = findSharedContextWidget(parent);
		OVITO_ASSERT(partner != this);
		OVITO_ASSERT(partner->isSharing());
		_sharingContextID = partner->_sharingContextID;
	}
	else _sharingContextID = _sharingContextCounter++;
}


/******************************************************************************
* Destructor
******************************************************************************/
Window3D::~Window3D()
{
	windowsWithPendingUpdates.remove(this);
}

/******************************************************************************
* Sets up the OpenGL rendering context.
******************************************************************************/
void Window3D::initializeGL()
{
	// Detect supported OpenGL extensions.
	detectOpenGLExtensions();

	static bool firstWidget = true;

	if(firstWidget) {
		QGLFormat fmt = format();
		VerboseLogger() << "OpenGL display format:" << endl;
		VerboseLogger() << "  Depth buffer:" << fmt.depth() << endl;
		VerboseLogger() << "  Depth buffer size:" << fmt.depthBufferSize() << endl;
		VerboseLogger() << "  Direct rendering:" << fmt.directRendering() << endl;
		VerboseLogger() << "  Double buffer:" << fmt.doubleBuffer() << endl;
		VerboseLogger() << "  RGBA mode:" << fmt.rgba() << endl;
		VerboseLogger() << "  Color buffer sizes:" << fmt.redBufferSize() << fmt.greenBufferSize() << fmt.blueBufferSize() << endl;

		VerboseLogger() << "OpenGL vendor:    " << driverVendor() << endl;
		VerboseLogger() << "OpenGL renderer:  " << rendererName() << endl;
		VerboseLogger() << "OpenGL ARB_point_parameters extension:      " << hasPointParametersExtension() << endl;
		VerboseLogger() << "OpenGL EXT_fog_coord extension:             " << hasFogCoordExtension() << endl;
	}

	// Setup initial OpenGL state.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);
	glCullFace(GL_BACK);
}

/******************************************************************************
* Event handler.
******************************************************************************/
void Window3D::paintEvent(QPaintEvent* event)
{
	if(_needsUpdate && VIEWPORT_MANAGER.isSuspended()) {
		update();
		return;
	}

	// Render the contents of this viewport in Window3D::paintGL().
	_needsUpdate = true;

	QGLWidget::paintEvent(event);
}

/******************************************************************************
* Renders the contents of the window.
******************************************************************************/
void Window3D::paintGL()
{
	if(_needsUpdate && !isHidden() && !geometry().isEmpty() && !isRendering()) {
		setAttribute(Qt::WA_ForceUpdatesDisabled, true);
		beginFrame();
		renderWindow();
		endFrame();
		setAttribute(Qt::WA_ForceUpdatesDisabled, false);
	}
}

/******************************************************************************
* Sets the rendering viewport rectangle.
******************************************************************************/
void Window3D::setViewportRectangle(const QRect& rect)
{
	_viewportRect = rect;
	if(isRendering()) {
		// Resize GL viewport to window bounds.
		if(_viewportRect.width() > 0 && _viewportRect.height() > 0) {
			glViewport(_viewportRect.x(), height() - _viewportRect.bottom() - 1, _viewportRect.width(), _viewportRect.height());
		}
	}

	// Calculate aspect ratio of viewport.
	if(_viewportRect.width() > 0 && _viewportRect.height() > 0)
		_aspectRatio = (FloatType)_viewportRect.height() / (FloatType)_viewportRect.width();
	else
		_aspectRatio = 1;
}

/******************************************************************************
* Prepares the window for rendering.
******************************************************************************/
void Window3D::beginFrame()
{
	OVITO_ASSERT_MSG(!_isRendering, "Window3D", "The rendering of the window is already in progress.");

	// We are now in rendering mode.
	_isRendering = true;
	_needsUpdate = false;

	// Update OpenGL viewport rectangle.
	setViewportRectangle(viewportRectangle());

	// Reset scene bounding box.
	_lastSceneExtent = _sceneExtent;
	_sceneExtent.setEmpty();

	// Update OpenGL transformation matrices.
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrix(_objToViewMatrix.constData());
	glMatrixMode(GL_PROJECTION);
	glLoadMatrix(projectionMatrix().constData());

	// Restore GL render mode.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(depthTest() ? GL_LEQUAL : GL_ALWAYS);
	if(backfaceCulling()) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if(lightingEnabled()) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);
}

/******************************************************************************
* Shows the rendered graphics.
******************************************************************************/
void Window3D::endFrame()
{
	OVITO_ASSERT(isRendering() && !pickingRegion());
	_isRendering = false;
}

/******************************************************************************
* Clears the drawing buffer with the background color.
******************************************************************************/
void Window3D::clearBuffer(const Color& backgroundColor)
{
	if(isRendering()) {
		Color clampedColor = backgroundColor;
		clampedColor.clampMinMax();
		glClearColor((GLclampf)clampedColor.r, (GLclampf)clampedColor.g, (GLclampf)clampedColor.b, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

/******************************************************************************
* Sets the current view matrix.
******************************************************************************/
void Window3D::setViewMatrix(const AffineTransformation& tm)
{
	_viewMatrix = tm;
	_viewMatrixInv = _viewMatrix.inverse();
	_objToViewMatrix = _viewMatrix * _worldMatrix;
	_objToViewMatrixInv = _worldMatrixInv * _viewMatrixInv;
	_objToScreenMatrix = _projMatrix * _objToViewMatrix;
	if(isRendering()) {
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrix(_objToViewMatrix.constData());
	}
}

/******************************************************************************
* Sets the current world transformation matrix.
******************************************************************************/
void Window3D::setWorldMatrix(const AffineTransformation& tm)
{
	_worldMatrix = tm;
	_worldMatrixInv = _worldMatrix.inverse();
	_objToViewMatrix = _viewMatrix * _worldMatrix;
	_objToViewMatrixInv = _worldMatrixInv * _viewMatrixInv;
	_objToScreenMatrix = _projMatrix * _objToViewMatrix;
	if(isRendering()) {
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrix(_objToViewMatrix.constData());
	}
}

/******************************************************************************
* Sets the current projection matrix.
******************************************************************************/
void Window3D::setProjectionMatrix(const Matrix4& tm)
{
	_projMatrix = tm;
	_projMatrixInv = _projMatrix.inverse();
	_objToScreenMatrix = _projMatrix * _objToViewMatrix;
	_isPerspectiveProjection = (tm(3, 0) != 0) || (tm(3, 1) != 0) || (tm(3, 2) != 0);
	if(isRendering()) {
		glMatrixMode(GL_PROJECTION);
		glLoadMatrix(_projMatrix.constData());
	}
}

/******************************************************************************
* Marks the window for update.
******************************************************************************/
void Window3D::update()
{
	if(_needsUpdate && !Window3D::windowsWithPendingUpdates.contains(this)) {
		Window3D::windowsWithPendingUpdates.insert(this);
	}
	else if(!_needsUpdate && isVisible() && !rect().isEmpty()) {
		_needsUpdate = true;
		if(Window3D::windowsWithPendingUpdates.contains(this) == false) {
			Window3D::windowsWithPendingUpdates.insert(this);
		}
	}
}

/******************************************************************************
* Immediately repaints all 3d windows that have been flagged for
* an update using Window3D::update().
******************************************************************************/
void Window3D::processWindowUpdates()
{
	if(windowsWithPendingUpdates.empty()) {
		return;
	}
	// Process any pending paint requests.
	QSet<Window3D*> windowsWithPendingUpdatesCopy(windowsWithPendingUpdates);
	windowsWithPendingUpdates.clear();
	Q_FOREACH(Window3D* win, windowsWithPendingUpdatesCopy) {
		CHECK_POINTER(win);
		if(!win->_needsUpdate || win->isHidden() || win->geometry().isEmpty()) {
			continue;
		}
		win->repaint();
	}
	windowsWithPendingUpdates.clear();
}

/******************************************************************************
* Posts paint events to all 3d windows that have been flagged for
* an update so they will be redrawn as soon as possible.
******************************************************************************/
void Window3D::postWindowUpdates()
{
	if(windowsWithPendingUpdates.empty()) return;
	if(VIEWPORT_MANAGER.isSuspended()) return;

	// Process any pending paint requests.
	Q_FOREACH(Window3D* win, windowsWithPendingUpdates) {
		CHECK_POINTER(win);
		if(!win->_needsUpdate || win->isHidden() || win->geometry().isEmpty()) continue;
		win->QGLWidget::update();
	}
	windowsWithPendingUpdates.clear();
}

};
