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
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportWindow.h>
#include <core/viewport/ViewportManager.h>
#include <core/viewport/picking/PickingSceneRenderer.h>
#include <core/animation/AnimManager.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>
#include <core/rendering/RenderSettings.h>
#include <core/dataset/DataSetManager.h>
#include <core/scene/objects/camera/AbstractCameraObject.h>
#include "ViewportMenu.h"

/// The default field of view in world units used for orthogonal view types when the scene is empty.
#define DEFAULT_ORTHOGONAL_FIELD_OF_VIEW		200.0

/// The default field of view angle in radians used for perspective view types when the scene is empty.
#define DEFAULT_PERSPECTIVE_FIELD_OF_VIEW		(FLOATTYPE_PI/4.0)

/// Controls the margin size between the overlay render frame and the viewport border.
#define VIEWPORT_RENDER_FRAME_SIZE				0.95

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, Viewport, RefTarget);
DEFINE_FLAGS_REFERENCE_FIELD(Viewport, _viewNode, "ViewNode", ObjectNode, PROPERTY_FIELD_NEVER_CLONE_TARGET)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _viewType, "ViewType", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _shadingMode, "ShadingMode", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _showGrid, "ShowGrid", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _gridMatrix, "GridMatrix", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _fieldOfView, "FieldOfView", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _cameraPosition, "CameraPosition", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _cameraDirection, "CameraDirection", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _showRenderFrame, "ShowRenderFrame", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _viewportTitle, "Title", PROPERTY_FIELD_NO_UNDO)

/******************************************************************************
* Constructor.
******************************************************************************/
Viewport::Viewport() :
		_widget(nullptr), _viewportWindow(nullptr),
		_viewType(VIEW_NONE), _shadingMode(SHADING_WIREFRAME), _showGrid(false),
		_fieldOfView(100),
		_showRenderFrame(false),
		_isRendering(false),
		_cameraPosition(Point3::Origin()), _cameraDirection(0,0,-1),
		_renderDebugCounter(0),
		_pickingRenderer(new PickingSceneRenderer())
{
	INIT_PROPERTY_FIELD(Viewport::_viewNode);
	INIT_PROPERTY_FIELD(Viewport::_viewType);
	INIT_PROPERTY_FIELD(Viewport::_shadingMode);
	INIT_PROPERTY_FIELD(Viewport::_showGrid);
	INIT_PROPERTY_FIELD(Viewport::_gridMatrix);
	INIT_PROPERTY_FIELD(Viewport::_fieldOfView);
	INIT_PROPERTY_FIELD(Viewport::_cameraPosition);
	INIT_PROPERTY_FIELD(Viewport::_cameraDirection);
	INIT_PROPERTY_FIELD(Viewport::_showRenderFrame);
	INIT_PROPERTY_FIELD(Viewport::_viewportTitle);
}

/******************************************************************************
* Destructor
******************************************************************************/
Viewport::~Viewport()
{
	delete _widget;

	OVITO_ASSERT(!_widget);
	OVITO_ASSERT(!_viewportWindow);
}

/******************************************************************************
* Displays the context menu for this viewport.
******************************************************************************/
void Viewport::showViewportMenu(const QPoint& pos)
{
	if(_viewportWindow)
		_viewportWindow->requestActivate();

	// Create the context menu for the viewport.
	ViewportMenu contextMenu(this);

	// Show menu.
	contextMenu.show(pos);
}

/******************************************************************************
* Changes the view type.
******************************************************************************/
void Viewport::setViewType(ViewType type, bool keepCurrentView)
{
	if(type == viewType())
		return;

	// Reset camera node.
	if(type != VIEW_SCENENODE)
		setViewNode(nullptr);

	// Setup default view.
	switch(type) {
		case VIEW_TOP:
			setCameraPosition(Point3::Origin());
			setCameraDirection(-ViewportSettings::getSettings().coordinateSystemOrientation().column(2));
			break;
		case VIEW_BOTTOM:
			setCameraPosition(Point3::Origin());
			setCameraDirection(ViewportSettings::getSettings().coordinateSystemOrientation().column(2));
			break;
		case VIEW_LEFT:
			setCameraPosition(Point3::Origin());
			setCameraDirection(ViewportSettings::getSettings().coordinateSystemOrientation().column(0));
			break;
		case VIEW_RIGHT:
			setCameraPosition(Point3::Origin());
			setCameraDirection(-ViewportSettings::getSettings().coordinateSystemOrientation().column(0));
			break;
		case VIEW_FRONT:
			setCameraPosition(Point3::Origin());
			setCameraDirection(ViewportSettings::getSettings().coordinateSystemOrientation().column(1));
			break;
		case VIEW_BACK:
			setCameraPosition(Point3::Origin());
			setCameraDirection(-ViewportSettings::getSettings().coordinateSystemOrientation().column(1));
			break;
		case VIEW_ORTHO:
			if(!keepCurrentView) {
				setCameraPosition(Point3::Origin());
				if(viewType() == VIEW_NONE)
					setCameraDirection(-ViewportSettings::getSettings().coordinateSystemOrientation().column(2));
			}
			break;
		case VIEW_PERSPECTIVE:
			if(!keepCurrentView) {
				if(viewType() >= VIEW_TOP && viewType() <= VIEW_ORTHO) {
					setCameraPosition(cameraPosition() - (cameraDirection().normalized() * fieldOfView()));
				}
				else if(viewType() != VIEW_PERSPECTIVE) {
					setCameraPosition(ViewportSettings::getSettings().coordinateSystemOrientation() * Point3(0,0,-50));
					setCameraDirection(ViewportSettings::getSettings().coordinateSystemOrientation() * Vector3(0,0,1));
				}
			}
			break;
		case VIEW_SCENENODE:
			break;
		case VIEW_NONE:
			break;
	}

	if(!keepCurrentView) {
		// Setup default zoom.
		if(type == VIEW_PERSPECTIVE) {
			if(viewType() != VIEW_PERSPECTIVE)
				setFieldOfView(DEFAULT_PERSPECTIVE_FIELD_OF_VIEW);
		}
		else {
			if(viewType() == VIEW_PERSPECTIVE || viewType() == VIEW_NONE)
				setFieldOfView(DEFAULT_ORTHOGONAL_FIELD_OF_VIEW);
		}
	}

	_viewType = type;
}

/******************************************************************************
* Returns true if the viewport is using a perspective project;
* returns false if it is using an orthogonal projection.
******************************************************************************/
bool Viewport::isPerspectiveProjection() const
{
	if(viewType() <= VIEW_ORTHO)
		return false;
	else if(viewType() == VIEW_PERSPECTIVE)
		return true;
	else
		return _projParams.isPerspective;
}

/******************************************************************************
* Computes the projection matrix and other parameters.
******************************************************************************/
ViewProjectionParameters Viewport::projectionParameters(TimePoint time, FloatType aspectRatio, const Box3& sceneBoundingBox)
{
	OVITO_ASSERT(aspectRatio > FLOATTYPE_EPSILON);
	OVITO_ASSERT(!sceneBoundingBox.isEmpty());

	ViewProjectionParameters params;
	params.aspectRatio = aspectRatio;
	params.validityInterval.setInfinite();
	params.boundingBox = sceneBoundingBox;

	// Get transformation from view scene node.
	if(viewType() == VIEW_SCENENODE && viewNode()) {
		PipelineFlowState state = viewNode()->evalPipeline(time);
		if(OORef<AbstractCameraObject> camera = state.convertObject<AbstractCameraObject>(time)) {

			// Get camera transformation.
			params.inverseViewMatrix = viewNode()->getWorldTransform(time, params.validityInterval);
			params.viewMatrix = params.inverseViewMatrix.inverse();

			// Get remaining parameters from camera object.
			camera->projectionParameters(time, params);
		}
	}
	else {
		params.viewMatrix = AffineTransformation::lookAlong(cameraPosition(), cameraDirection(), ViewportSettings::getSettings().upVector());
		params.inverseViewMatrix = params.viewMatrix.inverse();
		params.fieldOfView = fieldOfView();
		params.isPerspective = (viewType() == VIEW_PERSPECTIVE);
	}

	// Transform scene bounding box to camera space.
	Box3 bb = sceneBoundingBox.transformed(params.viewMatrix).centerScale(1.01);

	// Compute projection matrix.
	if(params.isPerspective) {
		if(bb.minc.z() < -FLOATTYPE_EPSILON) {
			params.zfar = -bb.minc.z();
			params.znear = std::max(-bb.maxc.z(), params.zfar * 1e-4f);
		}
		else {
			params.zfar = std::max(sceneBoundingBox.size().length(), FloatType(1));
			params.znear = params.zfar * 1e-4f;
		}
		params.zfar = std::max(params.zfar, params.znear * 1.01f);
		params.projectionMatrix = Matrix4::perspective(params.fieldOfView, 1.0 / params.aspectRatio, params.znear, params.zfar);
	}
	else {
		if(!bb.isEmpty()) {
			params.znear = -bb.maxc.z();
			params.zfar  = std::max(-bb.minc.z(), params.znear + 1.0f);
		}
		else {
			params.znear = 1;
			params.zfar = 100;
		}
		params.projectionMatrix = Matrix4::ortho(-params.fieldOfView / params.aspectRatio, params.fieldOfView / params.aspectRatio,
							-params.fieldOfView, params.fieldOfView,
							params.znear, params.zfar);
	}
	params.inverseProjectionMatrix = params.projectionMatrix.inverse();

	return params;
}

/******************************************************************************
* Zooms to the extents of the scene.
******************************************************************************/
void Viewport::zoomToSceneExtents()
{
	Box3 sceneBoundingBox = DataSetManager::instance().currentSet()->sceneRoot()->worldBoundingBox(AnimManager::instance().time());
	zoomToBox(sceneBoundingBox);
}

/******************************************************************************
* Zooms to the extents of the currently selected nodes.
******************************************************************************/
void Viewport::zoomToSelectionExtents()
{
	Box3 selectionBoundingBox;
	for(SceneNode* node : DataSetManager::instance().currentSelection()->nodes()) {
		selectionBoundingBox.addBox(node->worldBoundingBox(AnimManager::instance().time()));
	}
	if(selectionBoundingBox.isEmpty() == false)
		zoomToBox(selectionBoundingBox);
	else
		zoomToSceneExtents();
}

/******************************************************************************
* Zooms to the extents of the given bounding box.
******************************************************************************/
void Viewport::zoomToBox(const Box3& box)
{
	if(box.isEmpty())
		return;

	if(viewType() == VIEW_SCENENODE)
		return;	// Cannot reposition the view node.

	if(isPerspectiveProjection()) {
		FloatType dist = box.size().length() * 0.5 / tan(fieldOfView() * 0.5);
		setCameraPosition(box.center() - cameraDirection().resized(dist));
	}
	else {

		// Setup projection.
		FloatType aspectRatio = (FloatType)size().height() / size().width();
		if(renderFrameShown()) {
			if(RenderSettings* renderSettings = DataSetManager::instance().currentSet()->renderSettings())
				aspectRatio = renderSettings->outputImageAspectRatio();
		}
		ViewProjectionParameters projParams = projectionParameters(AnimManager::instance().time(), aspectRatio, box);

		FloatType minX =  FLOATTYPE_MAX, minY =  FLOATTYPE_MAX;
		FloatType maxX = -FLOATTYPE_MAX, maxY = -FLOATTYPE_MAX;
		for(int i = 0; i < 8; i++) {
			Point3 trans = projParams.viewMatrix * box[i];
			if(trans.x() < minX) minX = trans.x();
			if(trans.x() > maxX) maxX = trans.x();
			if(trans.y() < minY) minY = trans.y();
			if(trans.y() > maxY) maxY = trans.y();
		}
		FloatType w = std::max(maxX - minX, FloatType(1e-5));
		FloatType h = std::max(maxY - minY, FloatType(1e-5));
		if(aspectRatio > h/w)
			setFieldOfView(w * aspectRatio * 0.55);
		else
			setFieldOfView(h * 0.55);
		setCameraPosition(box.center());
	}
}

/******************************************************************************
* Is called when a RefTarget referenced by this object has generated an event.
******************************************************************************/
bool Viewport::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == viewNode() && event->type() == ReferenceEvent::TargetChanged) {
		// Update viewport when camera node has moved.
		updateViewport();
		return false;
	}
	else if(source == viewNode() && event->type() == ReferenceEvent::TitleChanged) {
		// Update viewport title when camera node has been renamed.
		updateViewportTitle();
		updateViewport();
		return false;
	}
	return RefTarget::referenceEvent(source, event);
}

/******************************************************************************
* Is called when the value of a reference field of this RefMaker changes.
******************************************************************************/
void Viewport::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(Viewport::_viewNode)) {
		if(viewType() == VIEW_SCENENODE && newTarget == nullptr) {
			// If the camera node has been deleted, switch to Ortho or Perspective view mode.
			// Keep current camera orientation.
			setFieldOfView(_projParams.fieldOfView);
			setCameraPosition(Point3::Origin() + _projParams.inverseViewMatrix.translation());
			setCameraDirection(_projParams.inverseViewMatrix * Vector3(0,0,-1));
			setViewType(isPerspectiveProjection() ? VIEW_PERSPECTIVE : VIEW_ORTHO, true);
		}
		else if(viewType() != VIEW_SCENENODE && newTarget != nullptr) {
			setViewType(VIEW_SCENENODE);
		}

		// Update viewport when the camera has been replaced by another scene node.
		updateViewportTitle();
	}
	RefTarget::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* Is called when the value of a property field of this object has changed.
******************************************************************************/
void Viewport::propertyChanged(const PropertyFieldDescriptor& field)
{
	RefTarget::propertyChanged(field);
	if(field == PROPERTY_FIELD(Viewport::_viewType)) {
		updateViewportTitle();
	}
	updateViewport();
}

/******************************************************************************
* Updates the title text of the viewport based on the current view type.
******************************************************************************/
void Viewport::updateViewportTitle()
{
	// Load viewport caption string.
	switch(viewType()) {
		case VIEW_TOP: _viewportTitle = tr("Top"); break;
		case VIEW_BOTTOM: _viewportTitle = tr("Bottom"); break;
		case VIEW_FRONT: _viewportTitle = tr("Front"); break;
		case VIEW_BACK: _viewportTitle = tr("Back"); break;
		case VIEW_LEFT: _viewportTitle = tr("Left"); break;
		case VIEW_RIGHT: _viewportTitle = tr("Right"); break;
		case VIEW_ORTHO: _viewportTitle = tr("Ortho"); break;
		case VIEW_PERSPECTIVE: _viewportTitle = tr("Perspective"); break;
		case VIEW_SCENENODE:
			if(viewNode() != nullptr)
				_viewportTitle = viewNode()->name();
			else
				_viewportTitle = tr("No view node");
		break;
		default: OVITO_ASSERT(false); _viewportTitle = QString(); // unknown viewport type
	}
}

/******************************************************************************
* Returns the widget that contains the viewport's rendering window.
******************************************************************************/
QWidget* Viewport::createWidget(QWidget* parent)
{
	OVITO_ASSERT(_widget == nullptr && _viewportWindow == nullptr);
	if(!_widget) {
		_viewportWindow = new ViewportWindow(this);
		_widget = QWidget::createWindowContainer(_viewportWindow, parent);
		_widget->setAttribute(Qt::WA_DeleteOnClose);
	}
	return _widget;
}

/******************************************************************************
* Puts an update request event for this viewport on the event loop.
******************************************************************************/
void Viewport::updateViewport()
{
	if(_viewportWindow)
		_viewportWindow->renderLater();
}

/******************************************************************************
* Immediately redraws the contents of this viewport.
******************************************************************************/
void Viewport::redrawViewport()
{
	if(_viewportWindow)
		_viewportWindow->renderNow();
}

/******************************************************************************
* If an update request is pending for this viewport, immediately processes it
* and redraw the viewport.
******************************************************************************/
void Viewport::processUpdateRequest()
{
	if(_viewportWindow)
		_viewportWindow->processUpdateRequest();
}

/******************************************************************************
* Renders the contents of the viewport.
******************************************************************************/
void Viewport::render(QOpenGLContext* context)
{
	OVITO_ASSERT_MSG(!isRendering(), "Viewport::render", "Viewport is already rendering.");

	try {
		_isRendering = true;

		QSize vpSize = size();
		glViewport(0, 0, vpSize.width(), vpSize.height());

		// Set up the viewport renderer.
		ViewportManager::instance().renderer()->startRender(DataSetManager::instance().currentSet(), DataSetManager::instance().currentSet()->renderSettings());

		// Request scene bounding box.
		Box3 boundingBox = ViewportManager::instance().renderer()->sceneBoundingBox(AnimManager::instance().time());

		// Set up preliminary projection.
		FloatType aspectRatio = (FloatType)vpSize.height() / vpSize.width();
		_projParams = projectionParameters(AnimManager::instance().time(), aspectRatio, boundingBox);

		// Adjust projection if render frame is shown.
		if(renderFrameShown())
			adjustProjectionForRenderFrame(_projParams);

		// Set up the viewport renderer.
		ViewportManager::instance().renderer()->beginFrame(AnimManager::instance().time(), _projParams, this);

		// Add bounding box of interactive elements.
		boundingBox.addBox(ViewportManager::instance().renderer()->boundingBoxInteractive(AnimManager::instance().time(), this));

		// Set up final projection.
		_projParams = projectionParameters(AnimManager::instance().time(), aspectRatio, boundingBox);

		// Adjust projection if render frame is shown.
		if(renderFrameShown())
			adjustProjectionForRenderFrame(_projParams);

		// Pass final projection parameters to renderer.
		ViewportManager::instance().renderer()->setProjParams(_projParams);

		// Call the viewport renderer to render the scene objects.
		ViewportManager::instance().renderer()->renderFrame(nullptr, nullptr);

		// Render render frame.
		renderRenderFrame();

		// Render orientation tripod.
		renderOrientationIndicator();

		// Render viewport caption.
		renderViewportTitle();

		// Stop rendering.
		ViewportManager::instance().renderer()->endFrame();
		ViewportManager::instance().renderer()->endRender();

		_isRendering = false;
	}
	catch(Exception& ex) {
		ex.prependGeneralMessage(tr("An unexpected error occurred while rendering the viewport contents. The program will quit."));
		ViewportManager::instance().suspendViewportUpdates();
		QCoreApplication::removePostedEvents(nullptr, 0);
		ex.showError();
		QCoreApplication::instance()->quit();
	}
}

/******************************************************************************
* Renders the viewport caption text.
******************************************************************************/
void Viewport::renderViewportTitle()
{
	// Create a rendering buffer that is responsible for rendering the viewport's caption text.
	SceneRenderer* renderer = ViewportManager::instance().renderer();
	if(!_captionBuffer || !_captionBuffer->isValid(renderer)) {
		_captionBuffer = renderer->createTextGeometryBuffer();
		_captionBuffer->setFont(ViewportManager::instance().viewportFont());
	}

#ifndef OVITO_DEBUG
	_captionBuffer->setText(viewportTitle());
#else
	_captionBuffer->setText(QString("%1 [%2]").arg(viewportTitle()).arg(++_renderDebugCounter));
#endif
	_captionBuffer->setColor(ColorA(viewportColor(ViewportSettings::COLOR_VIEWPORT_CAPTION)));

	QFontMetricsF metrics(_captionBuffer->font());
	QPointF pos = QPointF(2, 2) * viewportWindow()->devicePixelRatio();
	_contextMenuArea = QRect(0, 0, std::max(metrics.width(_captionBuffer->text()), 30.0) + pos.x(), metrics.height() + pos.y());
	_captionBuffer->renderWindow(renderer, Point2(pos.x(), pos.y()), Qt::AlignLeft | Qt::AlignTop);
}

/******************************************************************************
* Sets whether mouse grab should be enabled or not for this viewport window.
******************************************************************************/
bool Viewport::setMouseGrabEnabled(bool grab)
{
	if(_viewportWindow)
		return _viewportWindow->setMouseGrabEnabled(grab);
	else
		return false;
}

/******************************************************************************
* Sets the cursor shape for this viewport window.
******************************************************************************/
void Viewport::setCursor(const QCursor& cursor)
{
	if(_viewportWindow)
		_viewportWindow->setCursor(cursor);
}

/******************************************************************************
* Restores the default arrow cursor for this viewport window.
******************************************************************************/
void Viewport::unsetCursor()
{
	if(_viewportWindow)
		_viewportWindow->unsetCursor();
}

/******************************************************************************
* Render the axis tripod symbol in the corner of the viewport that indicates
* the coordinate system orientation.
******************************************************************************/
void Viewport::renderOrientationIndicator()
{
	const FloatType tripodSize = 60.0f * viewportWindow()->devicePixelRatio();			// pixels
	const FloatType tripodArrowSize = 0.17f; 	// percentage of the above value.
	SceneRenderer* renderer = ViewportManager::instance().renderer();

	// Turn off depth-testing.
	OVITO_CHECK_OPENGL(glDisable(GL_DEPTH_TEST));

	// Setup projection matrix.
	ViewProjectionParameters projParams = _projParams;
	FloatType xscale = size().width() / tripodSize;
	FloatType yscale = size().height() / tripodSize;
	projParams.projectionMatrix = Matrix4::translation(Vector3(-1.0 + 1.3f/xscale, -1.0 + 1.3f/yscale, 0))
									* Matrix4::ortho(-xscale, xscale, -yscale, yscale, -2, 2);
	projParams.inverseProjectionMatrix = projParams.projectionMatrix.inverse();
	projParams.viewMatrix.setIdentity();
	projParams.inverseViewMatrix.setIdentity();
	renderer->setProjParams(projParams);
	renderer->setWorldTransform(AffineTransformation::Identity());

	static const ColorA axisColors[3] = { ColorA(1, 0, 0), ColorA(0, 1, 0), ColorA(0.2, 0.2, 1) };
	static const QString labels[3] = { "x", "y", "z" };

	// Create line buffer.
	if(!_orientationTripodGeometry || !_orientationTripodGeometry->isValid(renderer)) {
		_orientationTripodGeometry = renderer->createLineGeometryBuffer();
		_orientationTripodGeometry->setSize(18);
		ColorA vertexColors[18];
		for(int i = 0; i < 18; i++)
			vertexColors[i] = axisColors[i / 6];
		_orientationTripodGeometry->setVertexColors(vertexColors);
	}

	// Render arrows.
	Point3 vertices[18];
	for(int axis = 0, index = 0; axis < 3; axis++) {
		Vector3 dir = _projParams.viewMatrix.column(axis).normalized();
		vertices[index++] = Point3::Origin();
		vertices[index++] = Point3::Origin() + dir;
		vertices[index++] = Point3::Origin() + dir;
		vertices[index++] = Point3::Origin() + (dir + tripodArrowSize * Vector3(dir.y() - dir.x(), -dir.x() - dir.y(), dir.z()));
		vertices[index++] = Point3::Origin() + dir;
		vertices[index++] = Point3::Origin() + (dir + tripodArrowSize * Vector3(-dir.y() - dir.x(), dir.x() - dir.y(), dir.z()));
	}
	_orientationTripodGeometry->setVertexPositions(vertices);
	_orientationTripodGeometry->render(renderer);

	// Render x,y,z labels.
	for(int axis = 0; axis < 3; axis++) {

		// Create a rendering buffer that is responsible for rendering the text label.
		if(!_orientationTripodLabels[axis] || !_orientationTripodLabels[axis]->isValid(renderer)) {
			_orientationTripodLabels[axis] = renderer->createTextGeometryBuffer();
			_orientationTripodLabels[axis]->setFont(ViewportManager::instance().viewportFont());
			_orientationTripodLabels[axis]->setColor(axisColors[axis]);
			_orientationTripodLabels[axis]->setText(labels[axis]);
		}

		Point3 p = Point3::Origin() + _projParams.viewMatrix.column(axis).resized(1.2f);
		Point3 ndcPoint = projParams.projectionMatrix * p;
		Point2 windowPoint(( ndcPoint.x() + 1.0) * size().width()  / 2,
							(-ndcPoint.y() + 1.0) * size().height() / 2);
		_orientationTripodLabels[axis]->renderWindow(renderer, windowPoint, Qt::AlignHCenter | Qt::AlignVCenter);
	}

	// Restore old rendering attributes.
	OVITO_CHECK_OPENGL(glEnable(GL_DEPTH_TEST));
}

/******************************************************************************
* Modifies the projection such that the render frame painted over the 3d scene exactly
* matches the true visible area.
******************************************************************************/
void Viewport::adjustProjectionForRenderFrame(ViewProjectionParameters& params)
{
	QSize vpSize = size();
	RenderSettings* renderSettings = DataSetManager::instance().currentSet()->renderSettings();
	if(!renderSettings || vpSize.width() == 0 || vpSize.height() == 0)
		return;

	FloatType renderAspectRatio = renderSettings->outputImageAspectRatio();
	FloatType windowAspectRatio = (FloatType)vpSize.height() / (FloatType)vpSize.width();

	if(_projParams.isPerspective) {
		if(renderAspectRatio < windowAspectRatio)
			params.fieldOfView = atan(tan(params.fieldOfView*0.5) / (VIEWPORT_RENDER_FRAME_SIZE / windowAspectRatio * renderAspectRatio))*2.0;
		else
			params.fieldOfView = atan(tan(params.fieldOfView*0.5) / VIEWPORT_RENDER_FRAME_SIZE)*2.0;
		params.projectionMatrix = Matrix4::perspective(params.fieldOfView, 1.0 / params.aspectRatio, params.znear, params.zfar);
	}
	else {
		if(renderAspectRatio < windowAspectRatio)
			params.fieldOfView /= VIEWPORT_RENDER_FRAME_SIZE / windowAspectRatio * renderAspectRatio;
		else
			params.fieldOfView /= VIEWPORT_RENDER_FRAME_SIZE;
		params.projectionMatrix = Matrix4::ortho(-params.fieldOfView / params.aspectRatio, params.fieldOfView / params.aspectRatio,
							-params.fieldOfView, params.fieldOfView,
							params.znear, params.zfar);
	}
	params.inverseProjectionMatrix = params.projectionMatrix.inverse();
}

/******************************************************************************
* Returns the geometry of the render frame, i.e., the region of the viewport that
* will be visible in a rendered image.
* The returned box is given in viewport coordinates (interval [-1,+1]).
******************************************************************************/
Box2 Viewport::renderFrameRect() const
{
	QSize vpSize = size();
	RenderSettings* renderSettings = DataSetManager::instance().currentSet()->renderSettings();
	if(!renderSettings || vpSize.width() == 0 || vpSize.height() == 0)
		return Box2(Point2(-1), Point2(+1));

	// Compute a rectangle that has the same aspect ratio as the rendered image.
	FloatType renderAspectRatio = renderSettings->outputImageAspectRatio();
	FloatType windowAspectRatio = (FloatType)vpSize.height() / (FloatType)vpSize.width();
	FloatType frameWidth, frameHeight;
	if(renderAspectRatio < windowAspectRatio) {
		frameWidth = VIEWPORT_RENDER_FRAME_SIZE;
		frameHeight = frameWidth / windowAspectRatio * renderAspectRatio;
	}
	else {
		frameHeight = VIEWPORT_RENDER_FRAME_SIZE;
		frameWidth = frameHeight / renderAspectRatio * windowAspectRatio;
	}

	return Box2(-frameWidth, -frameHeight, frameWidth, frameHeight);
}

/******************************************************************************
* Renders the frame on top of the scene that indicates the visible rendering area.
******************************************************************************/
void Viewport::renderRenderFrame()
{
	if(!renderFrameShown())
		return;

	// Create a rendering buffer that is responsible for rendering the frame.
	SceneRenderer* renderer = ViewportManager::instance().renderer();
	if(!_renderFrameOverlay || !_renderFrameOverlay->isValid(renderer)) {
		_renderFrameOverlay = renderer->createImageGeometryBuffer();
		QImage image(1, 1, QImage::Format_ARGB32_Premultiplied);
		image.fill(0xA0FFFFFF);
		_renderFrameOverlay->setImage(image);
	}

	Box2 rect = renderFrameRect();

	// Render rectangle borders
	_renderFrameOverlay->renderViewport(renderer, Point2(-1,-1), Vector2(1.0 + rect.minc.x(), 2));
	_renderFrameOverlay->renderViewport(renderer, Point2(rect.maxc.x(),-1), Vector2(1.0 - rect.maxc.x(), 2));
	_renderFrameOverlay->renderViewport(renderer, Point2(rect.minc.x(),-1), Vector2(rect.width(), 1.0 + rect.minc.y()));
	_renderFrameOverlay->renderViewport(renderer, Point2(rect.minc.x(),rect.maxc.y()), Vector2(rect.width(), 1.0 - rect.maxc.y()));
}

/******************************************************************************
* Computes the world size of an object that should appear always in the
* same size on the screen.
******************************************************************************/
FloatType Viewport::nonScalingSize(const Point3& worldPosition)
{
	if(isPerspectiveProjection()) {

		Point3 p = viewMatrix() * worldPosition;
        if(std::abs(p.z()) < FLOATTYPE_EPSILON) return 1.0f;

        Point3 p1 = projectionMatrix() * p;
		Point3 p2 = projectionMatrix() * (p + Vector3(1,0,0));

		return 0.1f / (p1 - p2).length();
	}
	else {
		int height = size().height();
		if(height == 0) return 1.0f;
		return fieldOfView() / (FloatType)height * 60.0f;
	}
}

/******************************************************************************
* Determines the object that is visible under the given mouse cursor position.
******************************************************************************/
ViewportPickResult Viewport::pick(const QPointF& pos)
{
	OVITO_ASSERT_MSG(!isRendering(), "Viewport::pick", "Object picking is not possible while rendering viewport contents.");
	
	try {

		// Set up the picking renderer.
		_pickingRenderer->startRender(DataSetManager::instance().currentSet(), DataSetManager::instance().currentSet()->renderSettings());

		try {
			// Request scene bounding box.
			Box3 boundingBox = _pickingRenderer->sceneBoundingBox(AnimManager::instance().time());

			// Setup projection.
			QSize vpSize = size();
			FloatType aspectRatio = (FloatType)vpSize.height() / vpSize.width();
			ViewProjectionParameters projParams = projectionParameters(AnimManager::instance().time(), aspectRatio, boundingBox);

			// Adjust projection if render frame is shown.
			if(renderFrameShown())
				adjustProjectionForRenderFrame(projParams);

			// Set up the picking renderer.
			_pickingRenderer->beginFrame(AnimManager::instance().time(), projParams, this);
			
			try {
				// Add bounding box of interactive elements.
				boundingBox.addBox(_pickingRenderer->boundingBoxInteractive(AnimManager::instance().time(), this));

				// Set up final projection.
				_projParams = projectionParameters(AnimManager::instance().time(), aspectRatio, boundingBox);

				// Adjust projection if render frame is shown.
				if(renderFrameShown())
					adjustProjectionForRenderFrame(_projParams);

				// Pass final projection parameters to renderer.
				_pickingRenderer->setProjParams(_projParams);

				// Call the viewport renderer to render the scene objects.
				_pickingRenderer->renderFrame(nullptr, nullptr);
			}
			catch(...) {
				_pickingRenderer->endFrame();
				throw;
			}

			// Stop rendering.
			_pickingRenderer->endFrame();
		}
		catch(...) {
			_pickingRenderer->endRender();
			throw;
		}
		_pickingRenderer->endRender();

		// Query which object is located at the given window position.
		ViewportPickResult result;
		const PickingSceneRenderer::ObjectRecord* objInfo;
		std::tie(objInfo, result.subobjectId) = _pickingRenderer->objectAtLocation((pos * viewportWindow()->devicePixelRatio()).toPoint());
		result.valid = (objInfo != nullptr);
		if(objInfo) {
			result.objectNode = objInfo->objectNode;
			result.sceneObject = objInfo->sceneObject;
			result.worldPosition = _pickingRenderer->worldPositionFromLocation((pos * viewportWindow()->devicePixelRatio()).toPoint());
		}
		_pickingRenderer->reset();
		return result;
	}
	catch(const Exception& ex) {
		ex.showError();
		ViewportPickResult result;
		result.valid = false;
		return result;
	}
}

};
