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
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/ViewportSettings.h>
#include <core/viewport/picking/PickingSceneRenderer.h>
#include <core/animation/AnimationSettings.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>
#include <core/rendering/RenderSettings.h>
#include <core/scene/SelectionSet.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/objects/camera/AbstractCameraObject.h>
#include "ViewportMenu.h"

/// The default field of view in world units used for orthogonal view types when the scene is empty.
#define DEFAULT_ORTHOGONAL_FIELD_OF_VIEW		200.0f

/// The default field of view angle in radians used for perspective view types when the scene is empty.
#define DEFAULT_PERSPECTIVE_FIELD_OF_VIEW		(35.0f*FLOATTYPE_PI/180.0f)

/// Controls the margin size between the overlay render frame and the viewport border.
#define VIEWPORT_RENDER_FRAME_SIZE				0.93f

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(View)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, Viewport, RefTarget);
DEFINE_FLAGS_REFERENCE_FIELD(Viewport, _viewNode, "ViewNode", ObjectNode, PROPERTY_FIELD_NEVER_CLONE_TARGET | PROPERTY_FIELD_NO_SUB_ANIM);
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _viewType, "ViewType", PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _gridMatrix, "GridMatrix", PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _fieldOfView, "FieldOfView", PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _cameraPosition, "CameraPosition", PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _cameraDirection, "CameraDirection", PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _renderPreviewMode, "ShowRenderFrame", PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _viewportTitle, "Title", PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _cameraTM, "CameraTransformation", PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _showGrid, "ShowGrid", PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_PROPERTY_FIELD(Viewport, _stereoscopicMode, "StereoscopicMode", PROPERTY_FIELD_NO_UNDO);
DEFINE_VECTOR_REFERENCE_FIELD(Viewport, _overlays, "Overlays", ViewportOverlay);

/******************************************************************************
* Constructor.
******************************************************************************/
Viewport::Viewport(DataSet* dataset) : RefTarget(dataset),
		_widget(nullptr), _viewportWindow(nullptr),
		_viewType(VIEW_NONE),
		_fieldOfView(100),
		_renderPreviewMode(false),
		_isRendering(false),
		_cameraPosition(Point3::Origin()), _cameraDirection(Vector3::Zero()),
		_renderDebugCounter(0),
		_pickingRenderer(new PickingSceneRenderer(dataset)),
		_cameraTM(AffineTransformation::Identity()),
		_gridMatrix(AffineTransformation::Identity()),
		_showGrid(false),
		_stereoscopicMode(false),
		_cursorInContextMenuArea(false)
{
	INIT_PROPERTY_FIELD(Viewport::_viewNode);
	INIT_PROPERTY_FIELD(Viewport::_viewType);
	INIT_PROPERTY_FIELD(Viewport::_gridMatrix);
	INIT_PROPERTY_FIELD(Viewport::_fieldOfView);
	INIT_PROPERTY_FIELD(Viewport::_cameraPosition);
	INIT_PROPERTY_FIELD(Viewport::_cameraDirection);
	INIT_PROPERTY_FIELD(Viewport::_renderPreviewMode);
	INIT_PROPERTY_FIELD(Viewport::_viewportTitle);
	INIT_PROPERTY_FIELD(Viewport::_cameraTM);
	INIT_PROPERTY_FIELD(Viewport::_showGrid);
	INIT_PROPERTY_FIELD(Viewport::_overlays);
	INIT_PROPERTY_FIELD(Viewport::_stereoscopicMode);

	connect(&ViewportSettings::getSettings(), &ViewportSettings::settingsChanged, this, &Viewport::viewportSettingsChanged);
}

/******************************************************************************
* Destructor
******************************************************************************/
Viewport::~Viewport()
{
	if(_widget) _widget->deleteLater();
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
	Matrix3 coordSys = ViewportSettings::getSettings().coordinateSystemOrientation();
	switch(type) {
		case VIEW_TOP:
			setCameraTransformation(AffineTransformation(coordSys));
			setGridMatrix(cameraTransformation());
			break;
		case VIEW_BOTTOM:
			setCameraTransformation(AffineTransformation(coordSys * Matrix3(1,0,0, 0,-1,0, 0,0,-1)));
			setGridMatrix(cameraTransformation());
			break;
		case VIEW_LEFT:
			setCameraTransformation(AffineTransformation(coordSys * Matrix3(0,0,-1, -1,0,0, 0,1,0)));
			setGridMatrix(cameraTransformation());
			break;
		case VIEW_RIGHT:
			setCameraTransformation(AffineTransformation(coordSys * Matrix3(0,0,1, 1,0,0, 0,1,0)));
			setGridMatrix(cameraTransformation());
			break;
		case VIEW_FRONT:
			setCameraTransformation(AffineTransformation(coordSys * Matrix3(1,0,0, 0,0,-1, 0,1,0)));
			setGridMatrix(cameraTransformation());
			break;
		case VIEW_BACK:
			setCameraTransformation(AffineTransformation(coordSys * Matrix3(-1,0,0, 0,0,1, 0,1,0)));
			setGridMatrix(cameraTransformation());
			break;
		case VIEW_ORTHO:
			if(!keepCurrentView) {
				setCameraPosition(Point3::Origin());
				if(viewType() == VIEW_NONE)
					setCameraTransformation(AffineTransformation(coordSys));
			}
			setGridMatrix(AffineTransformation(coordSys));
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
			setGridMatrix(AffineTransformation(coordSys));
			break;
		case VIEW_SCENENODE:
			setGridMatrix(AffineTransformation(coordSys));
			break;
		case VIEW_NONE:
			setGridMatrix(AffineTransformation(coordSys));
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
* Changes the viewing direction of the camera.
******************************************************************************/
void Viewport::setCameraDirection(const Vector3& newDir)
{
	if(newDir != Vector3::Zero()) {
		Vector3 upVector = ViewportSettings::getSettings().upVector();
		if(!ViewportSettings::getSettings().restrictVerticalRotation()) {
			if(upVector.dot(cameraTransformation().column(1)) < 0)
				upVector = -upVector;
		}
		setCameraTransformation(AffineTransformation::lookAlong(cameraPosition(), newDir, upVector).inverse());
	}
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
		// Get camera transformation.
		params.inverseViewMatrix = viewNode()->getWorldTransform(time, params.validityInterval);
		params.viewMatrix = params.inverseViewMatrix.inverse();

		PipelineFlowState state = viewNode()->evalPipeline(time);
		if(OORef<AbstractCameraObject> camera = state.convertObject<AbstractCameraObject>(time)) {

			// Get remaining parameters from camera object.
			camera->projectionParameters(time, params);
		}
		else {
			params.fieldOfView = 1;
			params.isPerspective = false;
		}
	}
	else {
		params.inverseViewMatrix = cameraTransformation();
		params.viewMatrix = params.inverseViewMatrix.inverse();
		params.fieldOfView = fieldOfView();
		params.isPerspective = (viewType() == VIEW_PERSPECTIVE);
	}

	// Transform scene bounding box to camera space.
	Box3 bb = sceneBoundingBox.transformed(params.viewMatrix).centerScale(1.01f);

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
		params.projectionMatrix = Matrix4::perspective(params.fieldOfView, 1.0f / params.aspectRatio, params.znear, params.zfar);
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
	Box3 sceneBoundingBox = dataset()->sceneRoot()->worldBoundingBox(dataset()->animationSettings()->time());
	zoomToBox(sceneBoundingBox);
}

/******************************************************************************
* Zooms to the extents of the currently selected nodes.
******************************************************************************/
void Viewport::zoomToSelectionExtents()
{
	Box3 selectionBoundingBox;
	for(SceneNode* node : dataset()->selection()->nodes()) {
		selectionBoundingBox.addBox(node->worldBoundingBox(dataset()->animationSettings()->time()));
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
		return;	// Do not reposition the camera node.

	if(isPerspectiveProjection()) {
		FloatType dist = box.size().length() * 0.5 / tan(fieldOfView() * 0.5);
		setCameraPosition(box.center() - cameraDirection().resized(dist));
	}
	else {

		// Setup projection.
		FloatType aspectRatio = (FloatType)size().height() / size().width();
		if(renderPreviewMode()) {
			if(RenderSettings* renderSettings = dataset()->renderSettings())
				aspectRatio = renderSettings->outputImageAspectRatio();
		}
		ViewProjectionParameters projParams = projectionParameters(dataset()->animationSettings()->time(), aspectRatio, box);

		FloatType minX = FLOATTYPE_MAX, minY = FLOATTYPE_MAX;
		FloatType maxX = FLOATTYPE_MIN, maxY = FLOATTYPE_MIN;
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
	if(event->type() == ReferenceEvent::TargetChanged) {
		if(source == viewNode()) {
			// Update viewport when camera node has moved.
			updateViewport();
			return false;
		}
		else if(_overlays.contains(source)) {
			// Update viewport when one of the overlays has changed.
			updateViewport();
		}
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
			// If the camera node has been deleted, switch to Orthographic or Perspective view type.
			// Keep current camera orientation.
			setFieldOfView(_projParams.fieldOfView);
			setCameraTransformation(_projParams.inverseViewMatrix);
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
* Is called when a RefTarget has been added to a VectorReferenceField.
******************************************************************************/
void Viewport::referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex)
{
	if(field == PROPERTY_FIELD(Viewport::_overlays)) {
		updateViewport();
	}
	RefTarget::referenceInserted(field, newTarget, listIndex);
}

/******************************************************************************
* Is called when a RefTarget has been removed from a VectorReferenceField.
******************************************************************************/
void Viewport::referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex)
{
	if(field == PROPERTY_FIELD(Viewport::_overlays)) {
		updateViewport();
	}
	RefTarget::referenceRemoved(field, oldTarget, listIndex);
}


/******************************************************************************
* Loads the class' contents from an input stream.
******************************************************************************/
void Viewport::loadFromStream(ObjectLoadStream& stream)
{
	RefTarget::loadFromStream(stream);

	// The old OVITO versions stored the camera transformation not as a matrix but
	// as a position and a direction vector. After loading an old OVITO file, we
	// have to convert the position/direction representation to a matrix representation.
	if(_cameraDirection.value() != Vector3::Zero()) {
		setCameraPosition(_cameraPosition);
		setCameraDirection(_cameraDirection);
		_cameraDirection = Vector3(Vector3::Zero());
	}

	// The global viewport settings may have changed.
	// Adjust camera orientation according to new settings.
	setCameraDirection(cameraDirection());
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
* This is called when the global viewport settings have changed.
******************************************************************************/
void Viewport::viewportSettingsChanged(ViewportSettings* newSettings)
{
	// Update camera TM if up axis ha changed.
	setCameraDirection(cameraDirection());

	// Redraw viewport.
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
	notifyDependents(ReferenceEvent::TitleChanged);
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

	// Invalidate picking buffer every time the visible contents of the viewport change.
	_pickingRenderer->reset();

	try {
		_isRendering = true;
		TimePoint time = dataset()->animationSettings()->time();
		RenderSettings* renderSettings = dataset()->renderSettings();
		OVITO_ASSERT(renderSettings != nullptr);
		ViewportSceneRenderer* renderer = dataset()->viewportConfig()->viewportRenderer();

		QSize vpSize = size();
		glViewport(0, 0, vpSize.width(), vpSize.height());

		// Set up the viewport renderer.
		renderer->startRender(dataset(), renderSettings);

		// Request scene bounding box.
		Box3 boundingBox = renderer->sceneBoundingBox(time);

		// Set up preliminary projection.
		FloatType aspectRatio = (FloatType)vpSize.height() / vpSize.width();
		_projParams = projectionParameters(time, aspectRatio, boundingBox);

		// Adjust projection if render frame is shown.
		if(renderPreviewMode())
			adjustProjectionForRenderFrame(_projParams);

		// Set up the viewport renderer.
		renderer->beginFrame(time, _projParams, this);

		// Add bounding box of interactive elements.
		boundingBox.addBox(renderer->boundingBoxInteractive(time, this));

		// Set up final projection.
		_projParams = projectionParameters(time, aspectRatio, boundingBox);

		// Adjust projection if render frame is shown.
		if(renderPreviewMode())
			adjustProjectionForRenderFrame(_projParams);

		if(!_projParams.isPerspective || !stereoscopicMode()) {

			// Pass final projection parameters to renderer.
			renderer->setProjParams(_projParams);

			// Call the viewport renderer to render the scene objects.
			renderer->renderFrame(nullptr, nullptr);

		}
		else {

			// Stereoscopic parameters
			FloatType eyeSeparation = 16.0;
			FloatType convergence = (orbitCenter() - Point3::Origin() - _projParams.inverseViewMatrix.translation()).length();
			convergence = std::max(convergence, _projParams.znear);
			ViewProjectionParameters params = _projParams;

			// Setup project of left eye.
			FloatType top = params.znear * tan(params.fieldOfView / 2);
			FloatType bottom = -top;
			FloatType a = tan(params.fieldOfView / 2) / params.aspectRatio * convergence;
			FloatType b = a - eyeSeparation / 2;
			FloatType c = a + eyeSeparation / 2;
			FloatType left = -b * params.znear / convergence;
			FloatType right = c * params.znear / convergence;
			params.projectionMatrix = Matrix4::frustum(left, right, bottom, top, params.znear, params.zfar);
			params.inverseProjectionMatrix = params.projectionMatrix.inverse();
			params.viewMatrix = AffineTransformation::translation(Vector3(eyeSeparation / 2, 0, 0)) * _projParams.viewMatrix;
			params.inverseViewMatrix = params.viewMatrix.inverse();
			renderer->setProjParams(params);

			// Render image of left eye.
			glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
			renderer->renderFrame(nullptr, nullptr);

			// Setup project of right eye.
			left = -c * params.znear / convergence;
			right = b * params.znear / convergence;
			params.projectionMatrix = Matrix4::frustum(left, right, bottom, top, params.znear, params.zfar);
			params.inverseProjectionMatrix = params.projectionMatrix.inverse();
			params.viewMatrix = AffineTransformation::translation(Vector3(-eyeSeparation / 2, 0, 0)) * _projParams.viewMatrix;
			params.inverseViewMatrix = params.viewMatrix.inverse();
			renderer->setProjParams(params);

			// Render image of right eye.
			glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
			renderer->renderFrame(nullptr, nullptr);

			// Restore default OpenGL state.
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}

		if(renderPreviewMode()) {
			// Render render frame.
			renderRenderFrame();

			// Paint overlays.
			if(!overlays().empty()) {
				// Let overlays paint into QImage buffer, which will then
				// be painted over the OpenGL frame buffer.
				QImage overlayBuffer(size(), QImage::Format_ARGB32_Premultiplied);
				overlayBuffer.fill(0);
				Box2 renderFrameBox = renderFrameRect();
				QRect renderFrameRect(
						(renderFrameBox.minc.x()+1.0f)*overlayBuffer.width()/2,
						(renderFrameBox.minc.y()+1.0f)*overlayBuffer.height()/2,
						renderFrameBox.width()*overlayBuffer.width()/2,
						renderFrameBox.height()*overlayBuffer.height()/2);
				ViewProjectionParameters renderProjParams = projectionParameters(time, renderSettings->outputImageAspectRatio(), boundingBox);
				for(ViewportOverlay* overlay : overlays()) {
					QPainter painter(&overlayBuffer);
					painter.setWindow(QRect(0, 0, renderSettings->outputImageWidth(), renderSettings->outputImageHeight()));
					painter.setViewport(renderFrameRect);
					painter.setRenderHint(QPainter::Antialiasing);
					overlay->render(this, painter, renderProjParams, renderSettings);
				}
				std::shared_ptr<ImagePrimitive> overlayBufferPrim = renderer->createImagePrimitive();
				overlayBufferPrim->setImage(overlayBuffer);
				overlayBufferPrim->renderViewport(renderer, Point2(-1,-1), Vector2(2, 2));
			}
		}
		else {
			// Render orientation tripod.
			renderOrientationIndicator();
		}

		// Render viewport caption.
		renderViewportTitle();

		// Stop rendering.
		renderer->endFrame();
		renderer->endRender();

		_isRendering = false;
	}
	catch(Exception& ex) {
		_isRendering = false;
		ex.prependGeneralMessage(tr("An unexpected error occurred while rendering the viewport contents. The program will quit."));

		QString openGLReport;
		QTextStream stream(&openGLReport, QIODevice::WriteOnly | QIODevice::Text);
		stream << "OpenGL version: " << context->format().majorVersion() << QStringLiteral(".") << context->format().minorVersion() << endl;
		stream << "OpenGL profile: " << (context->format().profile() == QSurfaceFormat::CoreProfile ? "core" : (context->format().profile() == QSurfaceFormat::CompatibilityProfile ? "compatibility" : "none")) << endl;
		stream << "OpenGL vendor: " << QString((const char*)glGetString(GL_VENDOR)) << endl;
		stream << "OpenGL renderer: " << QString((const char*)glGetString(GL_RENDERER)) << endl;
		stream << "OpenGL version string: " << QString((const char*)glGetString(GL_VERSION)) << endl;
		stream << "OpenGL shading language: " << QString((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)) << endl;
		stream << "OpenGL shader programs: " << QOpenGLShaderProgram::hasOpenGLShaderPrograms() << endl;
		stream << "OpenGL geometry shaders: " << QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Geometry, context) << endl;
		stream << "Using point sprites: " << ViewportWindow::pointSpritesEnabled() << endl;
		stream << "Using geometry shaders: " << ViewportWindow::geometryShadersEnabled() << endl;
		stream << "Context sharing: " << ViewportWindow::contextSharingEnabled() << endl;
		ex.appendDetailMessage(openGLReport);

		dataset()->viewportConfig()->suspendViewportUpdates();
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
	ViewportSceneRenderer* renderer = dataset()->viewportConfig()->viewportRenderer();
	if(!_captionBuffer || !_captionBuffer->isValid(renderer)) {
		_captionBuffer = renderer->createTextPrimitive();
		_captionBuffer->setFont(ViewportSettings::getSettings().viewportFont());
	}

	if(_cursorInContextMenuArea && !_captionBuffer->font().underline()) {
		QFont font = _captionBuffer->font();
		font.setUnderline(true);
		_captionBuffer->setFont(font);
	}
	else if(!_cursorInContextMenuArea && _captionBuffer->font().underline()) {
		QFont font = _captionBuffer->font();
		font.setUnderline(false);
		_captionBuffer->setFont(font);
	}

	QString str = viewportTitle();
	if(renderPreviewMode())
		str += tr(" (preview)");
#ifdef OVITO_DEBUG
	str += QString(" [%1]").arg(++_renderDebugCounter);
#endif
	_captionBuffer->setText(str);
	Color textColor = viewportColor(ViewportSettings::COLOR_VIEWPORT_CAPTION);
	if(renderPreviewMode() && textColor == renderer->renderSettings()->backgroundColor())
		textColor = Vector3(1,1,1) - (Vector3)textColor;
	_captionBuffer->setColor(ColorA(textColor));

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
	// Changing the cursor leads to program crash on MacOS and Qt <= 5.2.0.
#if !defined(Q_OS_MACX) || (QT_VERSION >= QT_VERSION_CHECK(5, 2, 1))
	if(_viewportWindow)
		_viewportWindow->setCursor(cursor);
#endif
}

/******************************************************************************
* Restores the default arrow cursor for this viewport window.
******************************************************************************/
void Viewport::unsetCursor()
{
#if !defined(Q_OS_MACX) || (QT_VERSION >= QT_VERSION_CHECK(5, 2, 1))
	if(_viewportWindow)
		_viewportWindow->unsetCursor();
#endif
}

/******************************************************************************
* Render the axis tripod symbol in the corner of the viewport that indicates
* the coordinate system orientation.
******************************************************************************/
void Viewport::renderOrientationIndicator()
{
	const FloatType tripodSize = 60.0f * viewportWindow()->devicePixelRatio();			// pixels
	const FloatType tripodArrowSize = 0.17f; 	// percentage of the above value.
	ViewportSceneRenderer* renderer = dataset()->viewportConfig()->viewportRenderer();

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

    static const ColorA axisColors[3] = { ColorA(1, 0, 0), ColorA(0, 1, 0), ColorA(0.2f, 0.2f, 1) };
	static const QString labels[3] = { QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("z") };

	// Create line buffer.
	if(!_orientationTripodGeometry || !_orientationTripodGeometry->isValid(renderer)) {
		_orientationTripodGeometry = renderer->createLinePrimitive();
		_orientationTripodGeometry->setVertexCount(18);
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
			_orientationTripodLabels[axis] = renderer->createTextPrimitive();
			_orientationTripodLabels[axis]->setFont(ViewportSettings::getSettings().viewportFont());
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
	RenderSettings* renderSettings = dataset()->renderSettings();
	if(!renderSettings || vpSize.width() == 0 || vpSize.height() == 0)
		return;

	FloatType renderAspectRatio = renderSettings->outputImageAspectRatio();
	FloatType windowAspectRatio = (FloatType)vpSize.height() / (FloatType)vpSize.width();

	if(_projParams.isPerspective) {
		if(renderAspectRatio < windowAspectRatio)
			params.fieldOfView = atan(tan(params.fieldOfView*0.5f) / (VIEWPORT_RENDER_FRAME_SIZE / windowAspectRatio * renderAspectRatio))*2.0f;
		else
			params.fieldOfView = atan(tan(params.fieldOfView*0.5f) / VIEWPORT_RENDER_FRAME_SIZE)*2.0f;
		params.projectionMatrix = Matrix4::perspective(params.fieldOfView, 1.0f / params.aspectRatio, params.znear, params.zfar);
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
	RenderSettings* renderSettings = dataset()->renderSettings();
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
	// Create a rendering buffer that is responsible for rendering the frame.
	ViewportSceneRenderer* renderer = dataset()->viewportConfig()->viewportRenderer();
	if(!_renderFrameOverlay || !_renderFrameOverlay->isValid(renderer)) {
		_renderFrameOverlay = renderer->createImagePrimitive();
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
	int height = size().height();
	if(height == 0) return 1.0f;

	const FloatType baseSize = 60.0f;

	if(isPerspectiveProjection()) {

		Point3 p = viewMatrix() * worldPosition;
        if(std::abs(p.z()) < FLOATTYPE_EPSILON) return 1.0f;

        Point3 p1 = projectionMatrix() * p;
		Point3 p2 = projectionMatrix() * (p + Vector3(1,0,0));

		return 0.8f * baseSize / (p1 - p2).length() / (FloatType)height;
	}
	else {
		return _projParams.fieldOfView / (FloatType)height * baseSize;
	}
}

/******************************************************************************
* Determines the object that is visible under the given mouse cursor position.
******************************************************************************/
ViewportPickResult Viewport::pick(const QPointF& pos)
{
	// Cannot perform picking while viewport is not visible or currently rendering.
	if(!viewportWindow() || !viewportWindow()->isExposed() || isRendering()) {
		ViewportPickResult result;
		result.valid = false;
		return result;
	}
	
	try {
		if(_pickingRenderer->isRefreshRequired()) {

			// Set up the picking renderer.
			_pickingRenderer->startRender(dataset(), dataset()->renderSettings());

			try {
				// Request scene bounding box.
				TimePoint time = dataset()->animationSettings()->time();
				Box3 boundingBox = _pickingRenderer->sceneBoundingBox(time);

				// Setup projection.
				QSize vpSize = size();
				FloatType aspectRatio = (FloatType)vpSize.height() / vpSize.width();
				ViewProjectionParameters projParams = projectionParameters(time, aspectRatio, boundingBox);

				// Adjust projection if render frame is shown.
				if(renderPreviewMode())
					adjustProjectionForRenderFrame(projParams);

				// Set up the picking renderer.
				_pickingRenderer->beginFrame(time, projParams, this);

				try {
					// Add bounding box of interactive elements.
					boundingBox.addBox(_pickingRenderer->boundingBoxInteractive(time, this));

					// Set up final projection.
					_projParams = projectionParameters(time, aspectRatio, boundingBox);

					// Adjust projection if render frame is shown.
					if(renderPreviewMode())
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
		}

		// Query which object is located at the given window position.
		ViewportPickResult result;
		const PickingSceneRenderer::ObjectRecord* objInfo;
		std::tie(objInfo, result.subobjectId) = _pickingRenderer->objectAtLocation((pos * viewportWindow()->devicePixelRatio()).toPoint());
		result.valid = (objInfo != nullptr);
		if(objInfo) {
			result.objectNode = objInfo->objectNode;
			result.pickInfo = objInfo->pickInfo;
			result.worldPosition = _pickingRenderer->worldPositionFromLocation((pos * viewportWindow()->devicePixelRatio()).toPoint());
		}
		return result;
	}
	catch(const Exception& ex) {
		ex.showError();
		ViewportPickResult result;
		result.valid = false;
		return result;
	}
}

/******************************************************************************
* Computes a point in the given coordinate system based on the given screen
* position and the current snapping settings.
******************************************************************************/
bool Viewport::snapPoint(const QPointF& screenPoint, Point3& snapPoint, const AffineTransformation& snapSystem)
{
	// Compute the intersection point of the ray with the X-Y plane of the snapping coordinate system.
    Ray3 ray = snapSystem.inverse() * screenRay(screenPoint);

    Plane3 plane(Vector3(0,0,1), 0);
	FloatType t = plane.intersectionT(ray, FloatType(1e-3));
	if(t == FLOATTYPE_MAX) return false;
	if(isPerspectiveProjection() && t <= 0) return false;

	snapPoint = ray.point(t);
	snapPoint.z() = 0;

	return true;
}

/******************************************************************************
* Computes a ray in world space going through a pixel of the viewport window.
******************************************************************************/
Ray3 Viewport::screenRay(const QPointF& screenPoint)
{
	return viewportRay(Point2(
			(FloatType)screenPoint.x() / _widget->width() * 2.0f - 1.0f,
			1.0f - (FloatType)screenPoint.y() / _widget->height() * 2.0f));
}

/******************************************************************************
* Computes a ray in world space going through a viewport pixel.
******************************************************************************/
Ray3 Viewport::viewportRay(const Point2& viewportPoint)
{
	if(_projParams.isPerspective) {
		Point3 ndc1(viewportPoint.x(), viewportPoint.y(), 1);
		Point3 ndc2(viewportPoint.x(), viewportPoint.y(), 0);
		Point3 p1 = _projParams.inverseViewMatrix * (_projParams.inverseProjectionMatrix * ndc1);
		Point3 p2 = _projParams.inverseViewMatrix * (_projParams.inverseProjectionMatrix * ndc2);
		return Ray3(Point3::Origin() + _projParams.inverseViewMatrix.translation(), p1 - p2);
	}
	else {
		Point3 ndc(viewportPoint.x(), viewportPoint.y(), -1);
		return Ray3(_projParams.inverseViewMatrix * (_projParams.inverseProjectionMatrix * ndc), _projParams.inverseViewMatrix * Vector3(0,0,-1));
	}
}

/******************************************************************************
* Computes the intersection of a ray going through a point in the
* viewport projection plane and the grid plane.
*
* Returns true if an intersection has been found.
******************************************************************************/
bool Viewport::computeConstructionPlaneIntersection(const Point2& viewportPosition, Point3& intersectionPoint, FloatType epsilon)
{
	// The construction plane in grid coordinates.
	Plane3 gridPlane = Plane3(Vector3(0,0,1), 0);

	// Compute the ray and transform it to the grid coordinate system.
	Ray3 ray = gridMatrix().inverse() * viewportRay(viewportPosition);

	// Compute intersection point.
	FloatType t = gridPlane.intersectionT(ray, epsilon);
    if(t == std::numeric_limits<FloatType>::max()) return false;
	if(isPerspectiveProjection() && t <= 0.0f) return false;

	intersectionPoint = ray.point(t);
	intersectionPoint.z() = 0;

	return true;
}

/******************************************************************************
* Returns the current orbit center for this viewport.
******************************************************************************/
Point3 Viewport::orbitCenter()
{
	// Use the target of a camera as the orbit center.
	if(viewNode() && viewType() == Viewport::VIEW_SCENENODE && viewNode()->lookatTargetNode()) {
		TimeInterval iv;
		TimePoint time = dataset()->animationSettings()->time();
		return Point3::Origin() + viewNode()->lookatTargetNode()->getWorldTransform(time, iv).translation();
	}
	else {
		Point3 currentOrbitCenter = dataset()->viewportConfig()->orbitCenter();

		if(viewNode() && isPerspectiveProjection()) {
			// If a free camera node is selected, the current orbit center is at the same location as the camera.
			// In this case, we should shift the orbit center such that it is in front of the camera.
			Point3 camPos = Point3::Origin() + inverseViewMatrix().translation();
			if(currentOrbitCenter.equals(camPos))
				currentOrbitCenter = camPos - 50.0f * inverseViewMatrix().column(2);
		}
		return currentOrbitCenter;
	}
}


OVITO_END_INLINE_NAMESPACE
}	// End of namespace
