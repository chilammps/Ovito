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
 * \file Viewport.h
 * \brief Contains the definition of the Ovito::Viewport class.
 */

#ifndef __OVITO_VIEWPORT_H
#define __OVITO_VIEWPORT_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/animation/TimeInterval.h>
#include <core/scene/ObjectNode.h>
#include <core/rendering/TextPrimitive.h>
#include <core/rendering/ImagePrimitive.h>
#include <core/rendering/LinePrimitive.h>
#include "ViewportSettings.h"
#include "ViewportWindow.h"

namespace Ovito {

class PickingSceneRenderer;		// defined in PickingSceneRenderer.h

/******************************************************************************
* This data structure describes a projection parameters used to render
* the 3D contents in a viewport.
******************************************************************************/
struct ViewProjectionParameters
{
	/// The aspect ratio (height/width) of the viewport.
	FloatType aspectRatio;

	/// Indicates whether this is a orthogonal or perspective projection.
	bool isPerspective;

	/// The distance of the front clipping plane in world units.
	FloatType znear;

	/// The distance of the back clipping plane in world units.
	FloatType zfar;

	/// When using an orthogonal projection this is the vertical field of view in world units.
	/// When using a perspective projection this is the vertical field of view angle in radians.
	FloatType fieldOfView;

	/// The world to view space transformation matrix.
	AffineTransformation viewMatrix;

	/// The view space to world space transformation matrix.
	AffineTransformation inverseViewMatrix;

	/// The view space to screen space projection matrix.
	Matrix4 projectionMatrix;

	/// The screen space to view space transformation matrix.
	Matrix4 inverseProjectionMatrix;

	/// The bounding box of the scene.
	Box3 boundingBox;

	/// Specifies the time interval during which the stored parameters stay constant.
	TimeInterval validityInterval;
};

/******************************************************************************
* This data structure is returned by the Viewport::pick() method.
*******************************************************************************/
struct ViewportPickResult
{
	/// Indicates whether an object was picked or not.
	bool valid;

	/// The coordinates of the hit point in world space.
	Point3 worldPosition;

	/// The object node that was picked.
	OORef<ObjectNode> objectNode;

	/// The scene object that was picked.
	OORef<SceneObject> sceneObject;

	/// The display object that has rendered the picked scene object.
	OORef<DisplayObject> displayObject;

	/// The subobject that was picked.
	quint32 subobjectId;
};

/**
 * \brief A viewport window that displays the current scene.
 */
class OVITO_CORE_EXPORT Viewport : public RefTarget
{
public:

	/// View types.
	enum ViewType {
		VIEW_NONE,
		VIEW_TOP,
		VIEW_BOTTOM,
		VIEW_FRONT,
		VIEW_BACK,
		VIEW_LEFT,
		VIEW_RIGHT,
		VIEW_ORTHO,
		VIEW_PERSPECTIVE,
		VIEW_SCENENODE,
	};
	Q_ENUMS(ViewType);

	/// The shading modes for viewports.
	enum ShadingMode {
		SHADING_WIREFRAME,
		SHADING_SHADED,
		SHADING_SHADED_WITH_EDGES,
	};
	Q_ENUMS(ShadingMode);

public:

	/// \brief Constructs a new viewport.
	Q_INVOKABLE Viewport(DataSet* dataset);

	/// \brief Destructor.
	~Viewport();

    /// \brief Puts an update request event for this viewport on the event loop.
    ///
	/// Calling this method is going to redraw the viewport contents unless the viewport is hidden.
	/// This function does not cause an immediate repaint; instead it schedules an
	/// update request event, which is processed when execution returns to the main event loop.
	///
	/// To update all viewports at once you should use ViewportManager::updateViewports().
	void updateViewport();

	/// \brief Immediately redraws the contents of this viewport.
	void redrawViewport();

	/// \brief If an update request is pending for this viewport, immediately processes it and redraw the viewport.
	void processUpdateRequest();

	/// Creates the widget that contains the viewport's rendering window.
	QWidget* createWidget(QWidget* parent);

	/// Returns the widget that contains the viewport's rendering window.
	QWidget* widget() { return _widget; }

	/// Indicates whether the rendering of the viewport contents is currently in progress.
	bool isRendering() const { return _isRendering; }

	/// \brief Displays the context menu for the viewport.
	/// \param pos The position in where the context menu should be displayed.
	void showViewportMenu(const QPoint& pos = QPoint(0,0));

	/// \brief Computes the projection matrix and other parameters.
	/// \param time The animation time for which the view is requested.
	/// \param aspectRatio Specifies the desired aspect ratio (height/width) of the output image.
	/// \param sceneBoundingBox The bounding box of the scene in world coordinates. This must be provided by the caller and
	///                         is used to calculate the near and far z-clipping planes.
	/// \return The returned structure describes the projection used to render the contents of the viewport.
	ViewProjectionParameters projectionParameters(TimePoint time, FloatType aspectRatio, const Box3& sceneBoundingBox);

	/// \brief Returns the view type of the viewport.
	/// \return The type of view used in the viewport.
	ViewType viewType() const { return _viewType; }

	/// \brief Changes the view type.
	/// \param type The new view type.
	/// \param keepViewParams When setting the view type to ViewType::VIEW_ORTHO or ViewType::VIEW_PERSPECTIVE,
	///        this controls whether the camera is reset to the default position/orientation.
	/// \note if \a type is set to ViewType::VIEW_SCENENODE then a view node should be set
	///       using setViewNode().
	void setViewType(ViewType type, bool keepCurrentView = false);

	/// \brief Returns true if the viewport is using a perspective project;
	///        returns false if it is using an orthogonal projection.
	bool isPerspectiveProjection() const;

	/// \brief Returns the current shading mode used for scene rendering.
	/// \return The current shading mode.
	ShadingMode shadingMode() const { return _shadingMode; }

	/// \brief Sets the shading mode to use for scene rendering.
	/// \param mode The new shading mode.
	/// \note This method should not be called while the scene is being rendered.
	///       It causes the viewport to be updated.
	void setShadingMode(ShadingMode mode) { _shadingMode = mode; }

	/// \brief Returns the field of view value of the viewport.
	/// \return Vertical camera angle in radians if the viewport uses a perspective projection or
	///         the field of view in the vertical direction in world units if the viewport
	///         uses an orthogonal projection.
	FloatType fieldOfView() const { return _fieldOfView; }

	/// \brief Sets the zoom of the viewport.
	/// \param fov Vertical camera angle in radians if the viewport uses a perspective projection or
	///            the field of view in the vertical direction in world units if the viewport
	///            uses an orthogonal projection.
	void setFieldOfView(FloatType fov) {
		// Clamp FOV to reasonable interval.
		if(fov > 1e12f) fov = 1e12f;
		else if(fov < -1e12f) fov = -1e12f;
		_fieldOfView = fov;
	}

	/// \brief Returns the orientation of the viewport's camera in space.
	/// \note This is only used when viewNode() == NULL.
	const AffineTransformation& cameraTransformation() const { return _cameraTM; }

	/// \brief Sets the position and orientation of the viewport's camera in space.
	/// \note This only has effect when viewNode() == NULL.
	void setCameraTransformation(const AffineTransformation& tm) { _cameraTM = tm; }

	/// \brief Returns the viewing direction of the camera.
	Vector3 cameraDirection() const {
		if(cameraTransformation().column(2) == Vector3::Zero()) return Vector3(0,0,1);
		else return -cameraTransformation().column(2);
	}

	/// \brief Changes the viewing direction of the camera.
	void setCameraDirection(const Vector3& newDir);

	/// \brief Returns the position of the camera.
	Point3 cameraPosition() const {
		return Point3::Origin() + cameraTransformation().translation();
	}

	/// \brief Sets the position of the camera.
	void setCameraPosition(const Point3& p) {
		AffineTransformation tm = cameraTransformation();
		tm.translation() = p - Point3::Origin();
		setCameraTransformation(tm);
	}

	/// \brief Returns the current world to camera transformation matrix.
	const AffineTransformation& viewMatrix() const { return _projParams.viewMatrix; }

	/// \brief Returns the inverse of current camera to world matrix.
	const AffineTransformation& inverseViewMatrix() const { return _projParams.inverseViewMatrix; }

	/// \brief Returns the current projection transformation matrix.
	const Matrix4& projectionMatrix() const { return _projParams.projectionMatrix; }

	/// \brief Returns the inverse of the current projection matrix.
	const Matrix4& inverseProjectionMatrix() const { return _projParams.inverseProjectionMatrix; }

	/// \brief Returns whether the render frame is shown in the viewport.
	bool renderFrameShown() const { return _showRenderFrame; }

	/// \brief Sets whether the render frame is shown in the viewport.
	void setRenderFrameShown(bool show) { _showRenderFrame = show; }

	/// \brief Returns whether the grid is shown in the viewport.
	bool isGridVisible() const { return _showGrid; }

	/// \brief Sets whether the grid is shown in the viewport.
	void setGridVisible(bool showGrid) { _showGrid = showGrid; }

	/// \brief Gets the scene node used as camera for the viewport.
	/// \return The scene node or \c NULL if no scene node has been set.
	ObjectNode* viewNode() const { return _viewNode; }

	/// \brief Sets the scene node used as camera for the viewport.
	/// \param node The scene node to be used as view point. The scene node must be a camera object and the
	///             viewport type must have been set to ViewType::VIEW_SCENENODE using setViewType()
	///             to enable the camera tracking mode for this viewport.
	void setViewNode(ObjectNode* node) { _viewNode = node; }

	/// \brief Computes the scaling factor of an object that should always appear in the same size on the screen, independent of its
	///        position with respect to the camera.
	FloatType nonScalingSize(const Point3& worldPosition);

	/// \brief Computes a point in the given coordinate system based on the given screen position and the current snapping settings.
	/// \param[in] screenPoint A screen point relative to the upper left corner of the viewport.
	/// \param[out] snapPoint The resulting point in the coordinate system specified by \a snapSystem. If the method returned
	///                       \c false then the value of this output variable is undefined.
	/// \param[in] snapSystem Specifies the coordinate system in which the snapping point should be determined.
	/// \return \c true if a snapping point has been found; \c false if no snapping point was found for the given screen position.
	bool snapPoint(const QPointF& screenPoint, Point3& snapPoint, const AffineTransformation& snapSystem);

	/// \brief Computes a point in the grid coordinate system based on a screen position and the current snap settings.
	/// \param[in] screenPoint A screen point relative to the upper left corner of the 3d window.
	/// \param[out] snapPoint The resulting snap point in the viewport's grid coordinate system. If the method returned
	///                       \c false then the value of this output variable is undefined.
	/// \return \c true if a snapping point has been found; \c false if no snapping point was found for the given screen position.
	bool snapPoint(const QPointF& screenPoint, Point3& snapPoint) {
		return this->snapPoint(screenPoint, snapPoint, gridMatrix());
	}

	/// \brief Computes a ray in world space going through a pixel of the viewport window.
	/// \param screenPoint A screen point relative to the upper left corner of the viewport window.
	/// \return The ray that goes from the camera point through the specified pixel of the viewport window.
	Ray3 screenRay(const QPointF& screenPoint);

	/// \brief Computes a ray in world space going through a viewport pixel.
	/// \param viewportPoint Viewport coordinates of the point in the range [-1,+1].
	/// \return The ray that goes from the viewpoint through the specified position in the viewport.
	Ray3 viewportRay(const Point2& viewportPoint);

	/// \brief Computes the intersection point of a ray going through a point in the
	///        viewport plane with the construction grid plane.
	/// \param[in] viewportPosition A 2d point in viewport coordinates (in the range [-1,+1]).
	/// \param[out] intersectionPoint The coordinates of the intersection point in grid plane coordinates.
	///                               The point can be transformed to world coordinates using the gridMatrix() transform.
	/// \param[in] epsilon This threshold value is used to test whether the ray is parallel to the grid plane.
	/// \return \c true if an intersection has been found; \c false if not.
	bool computeConstructionPlaneIntersection(const Point2& viewportPosition, Point3& intersectionPoint, FloatType epsilon = FLOATTYPE_EPSILON);

	/// Returns the geometry of the render frame, i.e., the region of the viewport that
	/// will be visible in a rendered image.
	/// The returned box is given in viewport coordinates (interval [-1,+1]).
	Box2 renderFrameRect() const;

	/// \brief Determines the object that is visible under the given mouse cursor position.
	ViewportPickResult pick(const QPointF& pos);

	/// \brief Zooms to the extents of the scene.
	Q_INVOKABLE void zoomToSceneExtents();

	/// \brief Zooms to the extents of the currently selected nodes.
	Q_INVOKABLE void zoomToSelectionExtents();

	/// \brief Zooms to the extents of the given bounding box.
	Q_INVOKABLE void zoomToBox(const Box3& box);

	/// \brief Returns the transformation matrix that defines the orientation of the viewport grid in world space.
	const AffineTransformation& gridMatrix() const { return _gridMatrix; }

	/// \brief Sets the orientation of the viewport grid in world space.
	void setGridMatrix(const AffineTransformation& tm) { _gridMatrix = tm; }

	/// \brief Returns the caption of the viewport.
	/// \return The title of the viewport.
	const QString& viewportTitle() const { return _viewportTitle; }

	/// \brief Returns a color value for drawing something in the viewport. The user can configure the color for each element.
	/// \param which The enum constant that specifies what type of element to draw.
	/// \return The color that should be used for the given element type.
	static const Color& viewportColor(ViewportSettings::ViewportColor which) {
		return ViewportSettings::getSettings().viewportColor(which);
	}

	/// If the return value is true, the viewport window receives all mouse events until
	/// setMouseGrabEnabled(false) is called; other windows get no mouse events at all.
	bool setMouseGrabEnabled(bool grab);

	/// Sets the cursor shape for this viewport window.
	/// The mouse cursor will assume this shape when it is over this viewport window,
	/// unless an override cursor is set.
	void setCursor(const QCursor& cursor);

	/// Restores the default arrow cursor for this viewport window.
	void unsetCursor();

	/// Returns the current size of the viewport window.
	QSize size() const { return _widget ? (_widget->size() * viewportWindow()->devicePixelRatio()) : QSize(); }

	/// Returns a pointer to the internal OpenGL rendering window.
	ViewportWindow* viewportWindow() const { return _viewportWindow.data(); }

public:

	Q_PROPERTY(QString title READ viewportTitle);
	Q_PROPERTY(ObjectNode* viewNode READ viewNode WRITE setViewNode);
	Q_PROPERTY(Point3 cameraPosition READ cameraPosition WRITE setCameraPosition);
	Q_PROPERTY(Vector3 cameraDirection READ cameraDirection WRITE setCameraDirection);
	Q_PROPERTY(AffineTransformation cameraTransformation READ cameraTransformation WRITE setCameraTransformation);
	Q_PROPERTY(FloatType fov READ fieldOfView WRITE setFieldOfView);
	Q_PROPERTY(ViewType type READ viewType WRITE setViewType);
	Q_PROPERTY(bool renderFrameVisible READ renderFrameShown WRITE setRenderFrameShown);
	Q_PROPERTY(bool gridVisible READ isGridVisible WRITE setGridVisible);

protected:

	/// Is called when the value of a property field of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Is called when a RefTarget referenced by this object has generated an event.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Is called when the value of a reference field of this RefMaker changes.
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) override;

	/// Loads the class' contents from an input stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

protected:

	/// Renders the contents of the viewport into the surface associated with the given context.
	void render(QOpenGLContext* context);

	/// Updates the title text of the viewport based on the current view type.
	void updateViewportTitle();

	/// Renders the viewport caption text.
	void renderViewportTitle();

	/// Render the axis tripod symbol in the corner of the viewport that indicates
	/// the coordinate system orientation.
	void renderOrientationIndicator();

	/// Renders the frame on top of the scene that indicates the visible rendering area.
	void renderRenderFrame();

	/// Modifies the projection such that the render frame painted over the 3d scene exactly
	/// matches the true visible area.
	void adjustProjectionForRenderFrame(ViewProjectionParameters& params);

private Q_SLOTS:

	/// This is called when the global viewport settings have changed.
	void viewportSettingsChanged(ViewportSettings* newSettings);

private:

	/// The type of the viewport (top, left, perspective, etc.)
	PropertyField<ViewType> _viewType;

	/// Shading mode of viewport (shaded, wireframe, etc..)
	PropertyField<ShadingMode> _shadingMode;

	/// The orientation of the grid.
	PropertyField<AffineTransformation> _gridMatrix;

	/// The zoom or field of view.
	PropertyField<FloatType> _fieldOfView;

	/// The position of the camera in world space.
	/// Note: This property field is obsolete and no longer used. We only keep it for file compatibility.
	PropertyField<Point3> _cameraPosition;

	/// The viewing direction of the camera in world space.
	/// Note: This property field is obsolete and no longer used. We only keep it for file compatibility.
	PropertyField<Vector3> _cameraDirection;

	/// The orientation of the camera.
	PropertyField<AffineTransformation> _cameraTM;

	/// Indicates whether the rendering frame is shown.
	PropertyField<bool> _showRenderFrame;

	/// Indicates whether the grid is shown.
	PropertyField<bool> _showGrid;

	/// The scene node (camera) that has been selected as the view node.
	ReferenceField<ObjectNode> _viewNode;

	/// The title of the viewport.
	PropertyField<QString> _viewportTitle;

	/// The widget that contains the viewport's rendering window.
	QPointer<QWidget> _widget;

	/// The internal OpenGL rendering window.
	QPointer<ViewportWindow> _viewportWindow;

	/// The zone in the upper left corner of the viewport where
	/// the context menu can be activated by the user.
	QRect _contextMenuArea;

	/// This flag is true during the rendering phase.
	bool _isRendering;

	/// Describes the current 3D projection used to render the contents of the viewport.
	ViewProjectionParameters _projParams;

	/// Counts how often this viewport has been rendered.
	int _renderDebugCounter;

	/// The rendering buffer maintained to render the viewport's caption text.
	std::shared_ptr<TextPrimitive> _captionBuffer;

	/// The geometry buffer used to render the viewport's orientation indicator.
	std::shared_ptr<LinePrimitive> _orientationTripodGeometry;

	/// The rendering buffer used to render the viewport's orientation indicator labels.
	std::shared_ptr<TextPrimitive> _orientationTripodLabels[3];

	/// This is used to render the render frame around the viewport.
	std::shared_ptr<ImagePrimitive> _renderFrameOverlay;

	/// This renderer generates an offscreen rendering of the scene that allows picking of objects.
	OORef<PickingSceneRenderer> _pickingRenderer;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_viewNode);
	DECLARE_PROPERTY_FIELD(_viewType);
	DECLARE_PROPERTY_FIELD(_shadingMode);
	DECLARE_PROPERTY_FIELD(_gridMatrix);
	DECLARE_PROPERTY_FIELD(_fieldOfView);
	DECLARE_PROPERTY_FIELD(_cameraPosition);
	DECLARE_PROPERTY_FIELD(_cameraDirection);
	DECLARE_PROPERTY_FIELD(_showRenderFrame);
	DECLARE_PROPERTY_FIELD(_viewportTitle);
	DECLARE_PROPERTY_FIELD(_cameraTM);
	DECLARE_PROPERTY_FIELD(_showGrid);

	friend class ViewportWindow;
	friend class ViewportMenu;
};

};

Q_DECLARE_METATYPE(Ovito::Viewport::ViewType);
Q_DECLARE_METATYPE(Ovito::Viewport::ShadingMode);
Q_DECLARE_TYPEINFO(Ovito::Viewport::ViewType, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Viewport::ShadingMode, Q_PRIMITIVE_TYPE);


#endif // __OVITO_VIEWPORT_H
