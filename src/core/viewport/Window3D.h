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
 * \file Window3D.h
 * \brief Contains the definition of the Ovito::Window3D class and some helper classes.
 */

#ifndef __OVITO_WINDOW3D_H
#define __OVITO_WINDOW3D_H

#include <core/Core.h>
#include <base/utilities/Color.h>
#include <mesh/tri/TriMesh.h>
#include <core/scene/bezier/BezierShape.h>
#include "PickRegion.h"

namespace Ovito {

/**
 * \brief Generic viewport widget that can be used to display 3d graphics.
 *
 * \author Alexander Stukowski
 * \sa Window3DContainer, Viewport
 */
class Window3D : public QGLWidget, public OpenGLExtensions
{
	Q_OBJECT
public:

	/// This enumeration is used by the drawPolyLine() method.
	enum RenderEdgeFlag {
		RENDER_EDGE_HIDDEN,		//< The edge is not rendered at all.
		RENDER_EDGE_VISIBLE,	//< The edge is rendered as a continuous line.
	};

	/// List of marker types that can be rendered with the renderMarker() method.
	enum MarkerType {
		MARKER_BOX,
		MARKER_CROSS,
	};

public:

	/// \brief Constructs a new 3d window.
	/// \param container The parent for the new 3d window. It will be embedded into the given container.
	Window3D(QWidget* parent);

	/// \brief Destructor.
	virtual ~Window3D();

	/// \brief Returns true if this widget's GL context is shared with GL context
	///        of the given widget.
	bool isSharingWith(Window3D* other) const { return _sharingContextID == other->_sharingContextID; }

	/////////////////////////////////// Viewport /////////////////////////////////////

	/// \brief Returns the current rendering viewport rectangle.
	/// \return The viewport rectangle relative to the 3d window.
	///
	/// The viewport rectangle is the region inside the 3d window that contains
	/// 3d OpenGL graphics. By default the viewport rectangle fills the complete
	/// 3d window but sometimes viewport might be smaller than the 3d window area. Then
	/// the remaining parts of the 3d window are rendered with a constant color and
	/// only receive mouse messages.
	///
	/// \sa geometry(), setViewportRectangle()
	const QRect& viewportRectangle() const { return _viewportRect; }

	/// \brief Sets the rendering viewport rectangle.
	/// \param rect The new viewport rectangle. This must not be larger than the size of the 3d window.
	/// \sa viewportRectangle()
	void setViewportRectangle(const QRect& rect);

	/// \brief Returns the aspect ratio (height/width) of the viewport rectangle.
	/// \return The aspect ratio between height and width of the viewportRectangle().
	/// \sa viewportRectangle()
	FloatType aspectRatio() { return _aspectRatio; }

	/// \brief Maps a 2d point from viewport coordinates to screen coordinates.
	/// \param viewportPoint A point in viewport coordinates. These are in the range [-1,+1].
	/// \return The point in screen coordinates. These are measured in pixels relative to the upper left
	///         corner of the 3d window.
	/// \sa viewportRectangle(), screenToViewport()
	Point2I viewportToScreen(const Point2& viewportPoint) const {
		return Point2I((int)((viewportPoint.X + 1.0) * viewportRectangle().width() * 0.5) + viewportRectangle().x(),
						(int)((1.0 - viewportPoint.Y) * viewportRectangle().height() * 0.5) + viewportRectangle().y());
	}

	/// \brief Maps a 2d point from screen coordinates to viewport coordinates.
	/// \param screenPoint A point in screen coordinates. These are measured in pixels relative to the upper left
	///                    corner of the 3d window.
	/// \return The point in viewport coordinates. These are in the range [-1,+1]. If the given screen point
	///         lies outside the viewportRectangle() then the returned point may be outside of the unit square.
	/// \sa viewportRectangle(), viewportToScreen()
	Point2 screenToViewport(const Point2I& screenPoint) const {
		return Point2(((FloatType)(screenPoint.X - viewportRectangle().x()) / viewportRectangle().width()) * 2.0 - 1.0,
				1.0 - ((FloatType)(screenPoint.Y - viewportRectangle().y()) / viewportRectangle().height()) * 2.0);
	}

	/////////////////////////// Projection/Transformation /////////////////////////////

	/// Returns the current view matrix. It transforms from world space to view space.
	const AffineTransformation& viewMatrix() const { return _viewMatrix; }
	/// Returns the inverse of the current view matrix. It transforms from view space to world space.
	const AffineTransformation& inverseViewMatrix() const { return _viewMatrixInv; }
	/// Sets the current view matrix. It transforms from world space to view space.
	void setViewMatrix(const AffineTransformation& tm);

	/// Returns the current world transformation matrix. It transforms from object space to world space.
	const AffineTransformation& worldMatrix() const { return _worldMatrix; }
	/// Returns the inverse of the current world transformation matrix. It transforms from world space to object space.
	const AffineTransformation& inverseWorldMatrix() const { return _worldMatrixInv; }
	/// Sets the current world transformation matrix. It transforms from object space to world space.
	void setWorldMatrix(const AffineTransformation& tm);

	/// Returns the current projection matrix. It transforms from view space to screen space.
	const Matrix4& projectionMatrix() const { return _projMatrix; }
	/// Returns the inverse of the current projection matrix. It transforms from screen space to view space.
	const Matrix4& inverseProjectionMatrix() const { return _projMatrixInv; }
	/// Sets the current projection matrix. It transforms from view space to screen space.
	void setProjectionMatrix(const Matrix4& tm);
	/// Returns true if the projection matrix is a perspective projection.
	bool isPerspectiveProjection() const { return _isPerspectiveProjection; }

	/// Returns the matrix that transforms from current object space to screen space.
	const Matrix4& objectToScreenMatrix() const { return _objToScreenMatrix; }
	/// Returns the matrix that transforms from screen space to current object space.
	Matrix4 screenToObjectMatrix() const { return _objToViewMatrixInv * _projMatrixInv; }
	/// Returns the matrix that transforms from world space to screen space.
	Matrix4 worldToScreenMatrix() const { return _projMatrix * _viewMatrix; }
	/// Returns the matrix that transforms from screen space to world space.
	Matrix4 screenToWorldMatrix() const { return _viewMatrixInv * _projMatrixInv; }

	//////////////////////////////// Color & Material //////////////////////////////////

	/// \brief Sets the current rendering color.
	/// \param c The new rendering color.
	///
	/// The rendering color affects the drawing of lines and text strings.
	/// \note Setting the rendering color has only an effect when the 3d window is
	/// currently being rendered. This can be checked via the isRendering() method.
	void setRenderingColor(const ColorA& c) {
		if(isRendering()) glColor4v(c.constData());
	}

	/// \brief Clears the drawing buffer with the background color.
	/// \note The depth buffer is also cleared by this method.
	/// \sa setClearColor()
	void clearBuffer(const Color& backgroundColor);

	/// \brief Sets the number of real-time materials in use.
	/// \param count The number of materials to use for primitive rendering.
	///
	/// This method resizes the internal materials array. Existing materials
	/// are preserved. Setting the number of materials to zero turns off shading.
	void setMaterialCount(int count) {
		_materials.resize(count);
		_realizedMaterial = -1;
	}

	/// \brief Returns the number of materials in use.
	/// \sa setMaterialCount()
	int materialCount() const { return _materials.size(); }

	/// \brief Specifies a real-time material.
	/// \param index The index of the material to set. This must be in the range specified by setMaterialCount().
	/// \param material A pointer to a Window3DMaterial structure that specifies the material properties.
	///                 The Window3D saves only the pointer and does NOT make a copy of the material structure.
	///                 Therefore it must be made sure that the pointer remains valid as long as the material is in use.
	///                 The parameter \a material can be set to \c NULL. In this case shading is turned off for faces with
	///                 the given material index.
	///
	/// The material will be used for all faces with the material index.
	///
	/// \note Changing the contents of the Window3DMaterial structure pointed to by \a material after a call to setMaterial()
	///       does not automatically update the material used by the Window3D to render primitives. You have to call setMaterial()
	///       again to update the material properties.
	///
	/// \sa setMaterialCount()
	void setMaterial(int index, const Window3DMaterial* material) {
		OVITO_ASSERT_MSG(index >= 0 && index < _materials.size(), "Window3D::setMaterial()", "Material index out of range.");
		_materials[index] = material;
		if(index == _realizedMaterial) _realizedMaterial = -1;
	}

	/// \brief Returns the material definition with the given index.
	/// \param index The index of the material to return. This must be in the range specified by materialCount().
	/// \return A pointer to the material descriptor or \c NULL if this material is undefined.
	/// \sa setMaterial()
	const Window3DMaterial* getMaterial(int index) const {
		OVITO_ASSERT_MSG(index >= 0 && index < _materials.size(), "Window3D::getMaterial()", "Material index out of range.");
		return _materials[index];
	}

	/// \brief Activates the given material in the OpenGL rendering context.
	/// \param index The material to be activated.
	/// \note This is an internal method. Normally you don't need to call it because the Window3D
	///       will realize the correct material as required.
	void realizeMaterial(int index);

	/////////////////////////////////// Lighting //////////////////////////////////////

	/// \brief Returns the maximum number of real-time light supported by the platform.
	/// \return The maximum number of real-time light. Every platform supports at least 8 lights.
	/// \sa setLight()
	int maximumLightCount() const { return GL_MAX_LIGHTS; }

	/// \brief Specifies a real-time light.
	/// \param which The index of the light to set. This must be in the range given by maximumLightCount().
	/// \param light A Window3DLight structure that specifies the light properties.
	/// \note The setLight() method may only be called while the window is in rendering mode (see isRendering()).
	/// \sa setLightingEnabled()
	void setLight(int which, const Window3DLight& light);

	/// \brief Returns whether lighting is currently enabled.
	/// \return \c true if lighting is currently enabled; \c false otherwise.
	/// \sa setLightingEnabled()
	bool lightingEnabled() const { return _lightingEnabled; }

	/// \brief Enabled or disable backface culling.
	/// \param enable Controls whether backface culling is enabled for primitive rendering.
	/// \sa lightingEnabled()
	void setLightingEnabled(bool enable) {
		_lightingEnabled = enable;
		if(isRendering()) {
			if(enable) glEnable(GL_LIGHTING);
			else glDisable(GL_LIGHTING);
		}
	}

	////////////////////////////////// Render mode /////////////////////////////////////

	/// \brief Returns whether the 3d window is currently being rendered.
	/// \return \c true if the rendering is in progress; \c false otherwise.
	bool isRendering() const { return _isRendering; }

	/// \brief Returns whether depth testing is currently enabled.
	/// \return \c true if depth testing is currently enabled; \c false otherwise.
	/// \sa setDepthTest()
	bool depthTest() const { return _depthTest; }

	/// \brief Enabled or disable depth testing.
	/// \param enable Controls whether depth testing (z-buffer) is enabled for primitive rendering.
	/// \sa depthTest()
	void setDepthTest(bool enable) {
		_depthTest = enable;
		if(isRendering())
			glDepthFunc(enable ? GL_LEQUAL : GL_ALWAYS);
	}

	/// \brief Returns whether backface culling is currently enabled.
	/// \return \c true if backface culling is currently enabled; \c false otherwise.
	/// \sa setBackfaceCulling()
	bool backfaceCulling() const { return _backfaceCulling; }

	/// \brief Enabled or disable backface culling.
	/// \param enable Controls whether backface culling is enabled for primitive rendering.
	/// \sa backfaceCulling()
	void setBackfaceCulling(bool enable) {
		_backfaceCulling = enable;
		if(isRendering()) {
			if(enable) glEnable(GL_CULL_FACE);
			else glDisable(GL_CULL_FACE);
		}
	}

	//////////////////////////////// Text Rendering ////////////////////////////////////

	/// \brief Draw a text string using the current rendering color.
	/// \param x The horizontal position of the text in screen coordinates relative to the upper left corner of the viewport rectangle.
	/// \param y The vertical position of the text in screen coordinates relative to the upper left corner of the viewport rectangle.
	/// \param text The text string to render.
	/// \sa setRenderingColor(), viewportRectangle(), textExtent()
	void renderText(int x, int y, const QString& text);

	/// \brief Computes the bounding rectangle of the given text string when rendered with the current font.
	/// \param text The text string to measure.
	/// \return The bounding rectangle for the given text.
	/// \sa renderText()
	QRect textExtent(const QString& text);

	/// \brief Returns the ascender height of the current font.
	/// \return The ascender height in pixels.
	int textAscender();

	////////////////////////////// Rendering Primitives ////////////////////////////////

	/// \brief Renders a polyline.
	/// \param numberOfVertices The number of vertices of the polyline. This must be at least 2.
	/// \param close Specifies whether the polyline should be closed to a polygon. Setting \a close to \c true will render one additional line segment.
	/// \param boundingBox The object space bounding box that contains all vertices.
	/// \param vertices Pointer to an array of object space points that define the polyline.
	/// \param vertexColors Pointer to an array of color values that specify the vertex colors. Vertex colors
	///                     are interpolated to draw the polyline segments. If \a vertexColors is \c NULL then
	///                     the current rendering color is used for all vertices.
	/// \param edgeFlags Pointer to an array of RenderEdgeFlags values, one for each line segment. If \a edgeFlags is not \c NULL then
	///                  these values are used to individually control the rendering of the line segments that connect the vertices.
	///                  Please note that the number of edge flags dependents on the value of the \a close parameter.
	void renderPolyLine(size_t numberOfVertices, bool close, const Box3& boundingBox, const Point3* vertices, const ColorA* vertexColors = NULL, const RenderEdgeFlag* edgeFlags = NULL);

	/// \brief Renders a set of independent line segments.
	/// \param numberOfVertices The number of vertices in the \a vertices array. This must always be an even number.
	/// \param boundingBox The object space bounding box that contains all vertices.
	/// \param vertices Pointer to an array of object space points that define the line segments.
	/// \param vertexColors Pointer to an array of color values that specify the vertex colors. Vertex colors
	///                     are interpolated to draw the line segments. If \a vertexColors is \c NULL then
	///                     the current rendering color is used for all vertices.
	/// \param edgeFlags Pointer to an array of RenderEdgeFlags values, one for each line segment. If \a edgeFlags is not \c NULL then
	///                  these values are used to individually control the rendering of the line segments that connect the vertex pairs.
	void renderLines(size_t numberOfVertices, const Box3& boundingBox, const Point3* vertices, const ColorA* vertexColors = NULL, const RenderEdgeFlag* edgeFlags = NULL);

	/// \brief Renders a triangle mesh in wireframe mode.
	/// \param mesh The mesh to be rendered.
	void renderMeshWireframe(const TriMesh& mesh);

	/// \brief Renders a triangle mesh in shaded mode.
	/// \param mesh The mesh to be rendered.
	void renderMeshShaded(const TriMesh& mesh);

	/// \brief Renders a marker into the viewport window.
	/// \param mtype The marker type to render.
	/// \param pos The object space coordinates of the marker.
	void renderMarker(MarkerType mtype, const Point3& pos);

	/// \brief Renders a shape.
	/// \param shape The shape to be rendered.
	void renderBezierShape(const BezierShape& shape);

	////////////////////////////////// Rendering Setup /////////////////////////////////

	/// \brief Enqueues the window for an update.
	///
	/// Calling this method will redraw the 3d window unless it is hidden.
	/// This function does not cause an immediate repaint; instead it schedules a
	/// paint event for processing when Qt returns to the main event loop.
	///
	/// Calling update() several times normally results in just one rendering pass.
	void update();

	/// \brief Immediately repaints all 3d windows that have been flagged for
	///        an update using Window3D::update().
	/// \sa ViewportManager::processViewportUpdates()
	static void processWindowUpdates();

	/// \brief Posts paint events to all 3d windows that have been flagged for
	///        an update so they will be redrawn as soon as possible.
	/// \note This is an internal method and should not be called.
	static void postWindowUpdates();

	/// \brief Prepares the window for the rendering pass.
	void beginFrame();

	/// \brief Finishes the rendering pass.
	void endFrame();

	/// \brief Enlarges the bounding box that encompasses the whole rendered scene.
	/// \param box The box to be added to the internal scene extent measure. This box must be given in world space.
	///
	/// Most of the primitive rendering routines of the Window3D class automatically call this method.
	/// After the scene has been completely rendered, its extent can be queried using the lastSceneExtent() method.
	///
	/// \sa enlargeSceneExtentOS(), lastSceneExtent()
	void enlargeSceneExtent(const Box3& box) {
		if(!box.isEmpty())
			_sceneExtent.addBox(box);
	}

	/// \brief Enlarges the bounding box that encompasses the whole rendered scene.
	/// \param box The box to be added to the internal scene extent measure. This box must be given in object space defined by the current worldMatrix().
	///
	/// Most of the primitive rendering routines of the Window3D class automatically call this method.
	/// After the scene has been completely rendered, its extent can be queried using the lastSceneExtent() method.
	///
	/// \sa enlargeSceneExtent(), lastSceneExtent()
	void enlargeSceneExtentOS(const Box3& box) {
		if(!box.isEmpty())
			_sceneExtent.addBox(box.transformed(worldMatrix()));
	}

	/// \brief Returns the bounding box that contains the complete scene as it was rendered in the last rendering pass.
	/// \return The world space bounding box of the visible scene.
	/// \note The returned box contains also non-scene geometry like the construction grid.
	/// \sa enlargeSceneExtent()
	const Box3& lastSceneExtent() const { return _lastSceneExtent; }

	/////////////////////////////////// Mouse grabbing /////////////////////////////////////

	/// \brief Returns whether the mouse has been grabbed by this window.
	/// \return \c true when the mouse has been grabbed and all mouse events are redirected to this window.
	bool isGrabbingMouse() const { return QWidget::mouseGrabber()  == this; }

	///////////////////////////////////// Picking ///////////////////////////////////////

	/// \brief Enables or disables picking mode.
	/// \param region A pointer to the region that should be used for hit testing or \c NULL to turn off picking mode.
	///               The developer has to make sure that the pointer \a region stays valid until
	///               picking mode is turned off by calling \c setPickingRegion(NULL).
	/// \sa isPicking(), closestHit()
	void setPickingRegion(const PickRegion* region) {
		_pickRegion = region;
		if(region) _closestHitDistance = HIT_TEST_NONE;
	}

	/// \brief Returns the current picking region used for hit testing.
	/// \return The current picking region or \c NULL if picking mode is not active.
	/// \sa setPickingRegion()
	const PickRegion* pickingRegion() const { return _pickRegion; }

	/// \brief Returns whether picking mode is currently enabled.
	/// \return \c true if picking mode is active; \c false otherwise.
	bool isPicking() const { return _pickRegion != NULL; }

	/// \brief Returns the distance of the closest hit detected in picking mode.
	/// \return The Z-distance to the closest hit detected since
	///         the last call to setPickingRegion() or resetHitLog().
	///         The special return value HIT_TEST_NONE is returned to indicate that no hit has been detected.
	/// \sa resetClosestHit()
	FloatType closestHit() const { return _closestHitDistance; }

	/// \brief Resets the hit records to restart picking.
	/// \sa closestHit()
	void resetHitLog() { _closestHitDistance = HIT_TEST_NONE; }

protected:

	////////////////////////////// Window rendering //////////////////////////////////

	/// \brief Sets up the OpenGL rendering context.
	///
	/// This is called by the system after the widget has been created.
	virtual void initializeGL();

	/// \brief Renders the contents of the window.
	///
	/// This is called by the system when a paint event comes in.
	virtual void paintGL();

	/// \brief Renders the 3d contents of the window.
	/// \note This method must be implemented in derived classes.
	virtual void renderWindow() = 0;

	/// \brief Event handler for the widget's paint events.
	virtual void paintEvent(QPaintEvent* event);

private:

	/// A unique ID assigned to each Window3D that share an OpenGL context.
	/// This number if used to determine whether two Window3D instances share a context.
	size_t _sharingContextID;

	/// The current rendering viewport rectangle.
	QRect _viewportRect;

	/// The aspect ration of the window geometry (
	FloatType _aspectRatio;

	/// A bounding box that contains everything rendered during the last frame.
	Box3 _sceneExtent;

	/// The last bounding box.
	Box3 _lastSceneExtent;

	/// Is window currently rendering?
	bool _isRendering;

	/// The view matrix. It transforms from world space to view space.
	AffineTransformation _viewMatrix;
	/// The inverse of the view matrix. It transforms from view space to world space.
	AffineTransformation _viewMatrixInv;

	/// The object to world matrix. It transforms from object space to world space.
	AffineTransformation _worldMatrix;
	/// The inverse of the object to world matrix. It transforms from world space to object space.
	AffineTransformation _worldMatrixInv;

	/// The object to view matrix (= viewMatrix * worldMatrix)
	Matrix4 _objToViewMatrix;
	/// The view to object matrix.
	Matrix4 _objToViewMatrixInv;

	/// The projection matrix. It transforms from view space to screen space.
	Matrix4 _projMatrix;
	/// The inverse of the projection matrix. It transforms from screen space to view space.
	Matrix4 _projMatrixInv;

	/// The full transformation and projection matrix. It transforms from object space to screen space.
	Matrix4 _objToScreenMatrix;

	/// True when the projection matrix is a perspective projection.
	bool _isPerspectiveProjection;

	/// Indicates whether this window receives mouse messages.
	bool _isEnabled;

	/// Indicates whether this 3d window is visible.
	bool _isVisible;

	/// Indicates that the window must be redrawn as soon as possible.
	bool _needsUpdate;

	/// Controls depth testing for primitive rendering.
	bool _depthTest;

	/// Controls backface culling for primitive rendering.
	bool _backfaceCulling;

	/// Controls lighting of faces.
	bool _lightingEnabled;

	//////////////////////////////////// Materials /////////////////////////////////////

	/// The list of active materials.
	QVarLengthArray<const Window3DMaterial*, 16> _materials;

	/// The index of the material that is currently active in the OpenGL rendering context.
	int _realizedMaterial;

	/////////////////////////////////// Hit Testing //////////////////////////////////////

	/// The current pick region if picking mode is active.
	const PickRegion* _pickRegion;

	/// The distance of the closest hit record.
	FloatType _closestHitDistance;

	/// Performs a hit test on a polyline.
	void hitTestPolyLine(size_t numberOfVertices, bool close, const Point3* vertices, const RenderEdgeFlag* edgeFlags);
	/// Performs a hit test on a set of line segments.
	void hitTestLines(size_t numberOfVertices, const Point3* vertices, const RenderEdgeFlag* edgeFlags);
	/// Performs a hit test on a triangle mesh in wireframe mode.
	void hitTestMeshWireframe(const TriMesh& mesh);
	/// Performs a hit test on a triangle mesh in shaded mode.
	void hitTestMeshShaded(const TriMesh& mesh);
	/// Performs a hit test on a single line segment.
	void hitTestLineSegment(const Point3& v1, const Point3& v2);
	///  Performs a hit test on a single triangle face.
	void hitTestFace(const Point3& v1, const Point3& v2, const Point3& v3, const Vector3& normal);
	/// Clips a triangle and performs a hit test.
	void hitTestClippedTriangle(const Vector4 clipPoints[3]);

	/// This logs a single hit that has been detected.
	void logHit(FloatType zvalue = 0) {
		// Look only for the closest hit.
		if(_closestHitDistance == HIT_TEST_NONE || zvalue < _closestHitDistance)
			_closestHitDistance = zvalue;
	}

	/// These windows will have to be updated.
	static QSet<Window3D*> windowsWithPendingUpdates;

	/// This counter is used to generate unique IDs.
	static size_t _sharingContextCounter;
};

};

#endif // __OVITO_WINDOW3D_H
