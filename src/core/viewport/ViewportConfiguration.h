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
 * \file ViewportConfiguration.h
 * \brief Contains the definition of the Ovito::ViewportRecord and the Ovito::ViewportConfiguration classes.
 */

#ifndef __OVITO_VIEWPORT_CONFIGURATION_H
#define __OVITO_VIEWPORT_CONFIGURATION_H

#include <core/Core.h>
#include <core/viewport/Viewport.h>

namespace Ovito {


/**
 * \brief Stores the configuration for a single viewport.
 *
 * This class is used to save the configuration of a Viewport
 * in the scene file.
 */
class ViewportConfiguration : public RefTarget
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
	Q_ENUMS(ViewType)

	/// The shading modes for viewports.
	enum ShadingMode {
		SHADING_WIREFRAME,
		SHADING_SHADED,
		SHADING_SHADED_WITH_EDGES,
	};
	Q_ENUMS(ShadingMode)

public:

	/// Default constructor.
	Q_INVOKABLE ViewportConfiguration(Viewport* owner = NULL);

#if 0
	/// \brief Returns a description the viewport's view at the given animation time.
	/// \param time The animation time for which the view description is requested.
	/// \param aspectRatio Specifies the desired aspect ratio (height/width) of the output image.
	/// \param sceneBoundingBox The bounding box of the scene in world coordinates. This is used to calculate the near and far z-clipping planes.
	/// \return This structure can be used by a PluginRenderer to render the scene as it is currently displayed in the viewport.
	CameraViewDescription getViewDescription(TimeTicks time, FloatType aspectRatio, const Box3& sceneBoundingBox = Box3());
#endif

	/// \brief Returns the view type of the viewport.
	/// \return The type of view used in the viewport.
	ViewType viewType() const { return _viewType; }

	/// \brief Changes the view type.
	/// \param type The new view type.
	/// \note if \a type is set to ViewType::VIEW_SCENENODE then a view node should be set
	///       using setViewNode().
	void setViewType(ViewType type);

	/// \brief Returns the current shading mode used for scene rendering.
	/// \return The current shading mode.
	ShadingMode shadingMode() const { return _shadingMode; }

	/// \brief Sets the shading mode to use for scene rendering.
	/// \param mode The new shading mode.
	/// \note This method should not be called while the scene is being rendered.
	///       It causes the viewport to be updated.
	void setShadingMode(ShadingMode mode) { _shadingMode = mode; }

	/// \brief Returns whether the grid display is currently enabled.
	/// \return \c true if the grid is displayed in the viewport; \c false otherwise.
	bool isGridShown() const { return _showGrid; }

	/// \brief Turns the grid display on or off.
	/// \param visible Controls the display of the construction grid.
	void setGridShown(bool visible) { _showGrid = visible; }

	/// \brief Returns the orientation of the grid plane.
	const AffineTransformation& gridMatrix() const { return _gridMatrix; }

	/// \brief Sets the orientation of the grid plane.
	/// \param tm The transformation matrix that defines the grid orientation.
	///           It transforms from grid coordinates to world coordinates.
	void setGridMatrix(const AffineTransformation& tm) { _gridMatrix = tm; }

	/// \brief Returns the field of view value of the viewport.
	/// \return Horizontal camera angle in radians if the viewport uses a perspective projection or
	///         the field of view in the horizontal direction in world units if the viewport
	///         uses an orthogonal projection.
	FloatType fieldOfView() const { return _fieldOfView; }

	/// \brief Sets the zoom of the viewport.
	/// \param fov Horizontal camera angle in radians if the viewport uses a perspective projection or
	///            the field of view in the horizontal direction in world units if the viewport
	///            uses an orthogonal projection.
	void setFieldOfView(FloatType fov) {
		// Clamp FOV to reasonable interval.
		if(fov > 1e12f) fov = 1e12f;
		else if(fov < -1e12f) fov = -1e12f;
		_fieldOfView = fov;
	}

	/// \brief Returns the world to camera (view) transformation without projection.
	const AffineTransformation& viewMatrix() const { return _viewMatrix; }

	/// \brief Returns the camera (view) to world transformation without the projection part.
	AffineTransformation inverseViewMatrix() const { return viewMatrix().inverse(); }

	/// \brief Sets the view matrix of the viewport.
	void setViewMatrix(const AffineTransformation& tm) { _viewMatrix = tm; }

	/// \brief Returns whether the render frame is shown in the viewport.
	bool renderFrameShown() const { return _showRenderFrame; }

	/// \brief Sets whether the render frame is shown in the viewport.
	/// \param show Specifies whether the render frame is shown or not.
	void setRenderFrameShown(bool show) { _showRenderFrame = show; }

#if 0
	/// \brief Gets the scene node used as camera for the viewport.
	/// \return The scene node or \c NULL if no scene node has been set.
	ObjectNode* viewNode() const { return _viewNode; }

	/// \brief Sets the scene node used as camera for the viewport.
	/// \param node The scene node to be used as view point. The scene node must be a camera object and the
	///             viewport type must have been set to ViewportType::VIEW_SCENENODE using setViewType()
	///             to enable camera mode for this viewport.
	void setViewNode(ObjectNode* node) { _viewNode = node; }
#endif

	/// \brief Returns whether a explicitly set center point is used by the orbit navigation mode.
	/// \return \c true if a center point is set.
	bool useOrbitCenter() const { return _useOrbitCenter; }

	/// \brief Sets whether the explicitly set center point is used by the orbit navigation mode.
	void setUseOrbitCenter(bool enable) { _useOrbitCenter = enable; }

	/// \brief Returns the current center point for the orbit navigation mode.
	/// \return The center point in world space.
	///
	/// The center point is only used if it has been activated with a call to setUseOrbitCenter().
	const Point3& orbitCenter() const { return _orbitCenter; }

	/// \brief Sets the current center point to use for the orbit navigation mode.
	/// \param center The center point in world space.
	///
	/// The center point is only used if it is activated with a call to setUseOrbitCenter().
	void setOrbitCenter(const Point3& center) { _orbitCenter = center; }

	/// \brief Returns the caption of the viewport.
	/// \return The title of the viewport.
	const QString& viewportTitle() const { return _viewportTitle; }

protected:

	/// Is called when the value of a property field of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Is called when a RefTarget referenced by this object has generated an event.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Is called when the value of a reference field of this RefMaker changes.
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) override;


protected:

	/// Updates the title text of the viewport based on the current view type.
	void updateViewportTitle();

private:

	/// The viewport these settings belong to.
	Viewport* _viewport;

	/// The type of the viewport (top, left, perspective, etc.)
	PropertyField<ViewType> _viewType;

	/// Shading mode of viewport (shaded, wireframe, etc..)
	PropertyField<ShadingMode> _shadingMode;

	/// Indicates whether the grid is activated.
	PropertyField<bool> _showGrid;

	/// The orientation of the grid.
	PropertyField<AffineTransformation> _gridMatrix;

	/// The zoom or field of view.
	PropertyField<FloatType> _fieldOfView;

	/// World to camera (view) transformation without projection.
	PropertyField<AffineTransformation> _viewMatrix;

	/// Indicates whether the rendering frame is shown.
	PropertyField<bool> _showRenderFrame;

#if 0
	/// The scene node (camera) that has been selected as the view node.
	ReferenceField<ObjectNode> _viewNode;
#endif

	/// The center point in world space used for the orbit navigation mode.
	PropertyField<Point3> _orbitCenter;

	/// Controls whether the explicit center point is used by the orbit navigation mode.
	PropertyField<bool> _useOrbitCenter;

	/// The title of the viewport.
	PropertyField<QString> _viewportTitle;

private:

	Q_OBJECT
	OVITO_OBJECT

#if 0
	DECLARE_REFERENCE_FIELD(_viewNode);
#endif
	DECLARE_PROPERTY_FIELD(_viewType);
	DECLARE_PROPERTY_FIELD(_shadingMode);
	DECLARE_PROPERTY_FIELD(_showGrid);
	DECLARE_PROPERTY_FIELD(_gridMatrix);
	DECLARE_PROPERTY_FIELD(_fieldOfView);
	DECLARE_PROPERTY_FIELD(_viewMatrix);
	DECLARE_PROPERTY_FIELD(_showRenderFrame);
	DECLARE_PROPERTY_FIELD(_orbitCenter);
	DECLARE_PROPERTY_FIELD(_useOrbitCenter);
	DECLARE_PROPERTY_FIELD(_viewportTitle);
};


/**
 * \brief This helper class is used to save the state of the viewports into the scene file.
 *
 * This class holds a collection of ViewportConfiguration objects, one for each viewport.
 */
class ViewportConfigurationSet : public RefTarget
{
public:

	/// Constructor.
	Q_INVOKABLE ViewportConfigurationSet() : _activeViewport(-1), _maximizedViewport(-1) {
		INIT_PROPERTY_FIELD(ViewportConfigurationSet::_viewRecords);
		INIT_PROPERTY_FIELD(ViewportConfigurationSet::_activeViewport);
		INIT_PROPERTY_FIELD(ViewportConfigurationSet::_maximizedViewport);
	}

	/// Returns the records for each of the viewports.
	const QVector<ViewportConfiguration*>& records() const { return _viewRecords; }

	/// Add a record for a new viewport.
	void addViewport(const OORef<ViewportConfiguration>& newRecord) { _viewRecords.push_back(newRecord); }

	/// Return the index of the active viewport or -1 if there is no active viewport.
	int activeViewport() const { return _activeViewport; }

	/// Sets the index of the active viewport.
	void setActiveViewport(int index) { OVITO_ASSERT(index < _viewRecords.size()); _activeViewport = index; }

	/// Return the settings of the active viewport or NULL if there is no active viewport.
	OORef<ViewportConfiguration> activeViewportSettings() const {
		if(_activeViewport < 0 || _activeViewport >= _viewRecords.size()) return NULL;
		return _viewRecords[_activeViewport];
	}

	/// Return the index of the maximized viewport or -1 if there is no maximized viewport.
	int maximizedViewport() const { return _maximizedViewport; }

	/// Sets the maximized viewport.
	void setMaximizedViewport(int index) { OVITO_ASSERT(index < _viewRecords.size()); _maximizedViewport = index; }

	/// The takes the current configuration of the viewports and saves it into this object.
	void saveConfiguration();

	/// This applies the saved configuration to the viewports in the viewport panel.
	void restoreConfiguration();

private:

	/// One record for every viewport.
	VectorReferenceField<ViewportConfiguration> _viewRecords;

	/// The index of the active viewport. Can be -1 if no viewport is active.
	PropertyField<int> _activeViewport;

	/// The index of the maximized viewport or -1.
	PropertyField<int> _maximizedViewport;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_viewRecords)
	DECLARE_PROPERTY_FIELD(_activeViewport)
	DECLARE_PROPERTY_FIELD(_maximizedViewport)
};

};

Q_DECLARE_METATYPE(Ovito::ViewportConfiguration::ViewType)
Q_DECLARE_METATYPE(Ovito::ViewportConfiguration::ShadingMode)
Q_DECLARE_TYPEINFO(Ovito::ViewportConfiguration::ViewType, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ViewportConfiguration::ShadingMode, Q_PRIMITIVE_TYPE);

#endif		// __OVITO_VIEWPORT_CONFIGURATION_H
