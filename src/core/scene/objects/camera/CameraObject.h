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

#ifndef __OVITO_CAMERA_OBJECT_H
#define __OVITO_CAMERA_OBJECT_H

#include <core/Core.h>
#include <core/scene/objects/camera/AbstractCameraObject.h>
#include <core/scene/objects/WeakVersionedObjectReference.h>
#include <core/animation/controller/Controller.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/scene/objects/DisplayObject.h>
#include <core/rendering/LinePrimitive.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene) OVITO_BEGIN_INLINE_NAMESPACE(StdObj)

/**
 * The default camera object.
 */
class OVITO_CORE_EXPORT CameraObject : public AbstractCameraObject
{
public:

	/// Constructor.
	Q_INVOKABLE CameraObject(DataSet* dataset);

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Camera"); }

	/// Returns whether this camera uses a perspective or an orthogonal projection.
	bool isPerspective() const { return _isPerspective; }

	/// Sets whether this camera uses a perspective or an orthogonal projection.
	void setIsPerspective(bool perspective) { _isPerspective = perspective; }

	/// Returns the controller that controls the field-of-view angle of the camera with perspective projection.
	Controller* fovController() const { return _fov; }

	/// Returns the controller that controls the zoom of the camera with orthogonal projection.
	Controller* zoomController() const { return _zoom; }

	/// Returns whether this camera is a target camera directory at a target object.
	bool isTargetCamera() const;

	/// Changes the type of the camera to a target camera or a free camera.
	void setIsTargetCamera(bool enable);

	/// With a target camera, indicates the distance between the camera and its target.
	FloatType targetDistance() const;

	/// \brief Returns a structure describing the camera's projection.
	/// \param[in] time The animation time for which the camera's projection parameters should be determined.
	/// \param[in,out] projParams The structure that is to be filled with the projection parameters.
	///     The following fields of the ViewProjectionParameters structure are already filled in when the method is called:
	///   - ViewProjectionParameters::aspectRatio (The aspect ratio (height/width) of the viewport)
	///   - ViewProjectionParameters::viewMatrix (The world to view space transformation)
	///   - ViewProjectionParameters::boundingBox (The bounding box of the scene in world space coordinates)
	virtual void projectionParameters(TimePoint time, ViewProjectionParameters& projParams) override;

	/// \brief Returns the field of view of the camera.
	virtual FloatType fieldOfView(TimePoint time, TimeInterval& validityInterval) override;

	/// \brief Changes the field of view of the camera.
	virtual void setFieldOfView(TimePoint time, FloatType newFOV) override;

	/// Asks the object for its validity interval at the given time.
	virtual TimeInterval objectValidity(TimePoint time) override;

public:

	Q_PROPERTY(bool isTargetCamera READ isTargetCamera WRITE setIsTargetCamera);
	Q_PROPERTY(bool isPerspective READ isPerspective WRITE setIsPerspective);

private:

	/// Determines if this camera uses a perspective projection.
	PropertyField<bool> _isPerspective;

	/// This controller stores the field of view of the camera if it uses a perspective projection.
	ReferenceField<Controller> _fov;

	/// This controller stores the field of view of the camera if it uses an orthogonal projection.
	ReferenceField<Controller> _zoom;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_isPerspective);
	DECLARE_REFERENCE_FIELD(_fov);
	DECLARE_REFERENCE_FIELD(_zoom);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A properties editor for the CameraObject class.
 */
class CameraObjectEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE CameraObjectEditor() {}

protected:
	
	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;
	
private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A scene display object for camera objects.
 */
class OVITO_CORE_EXPORT CameraDisplayObject : public DisplayObject
{
public:

	/// \brief Constructor.
	Q_INVOKABLE CameraDisplayObject(DataSet* dataset) : DisplayObject(dataset) {}

	/// \brief Lets the display object render a camera object.
	virtual void render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Computes the view-dependent bounding box of the data object for interactive rendering in the viewports.
	virtual Box3 viewDependentBoundingBox(TimePoint time, Viewport* viewport, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Camera icon"); }

protected:

	/// The buffered geometry used to render the icon.
	std::shared_ptr<LinePrimitive> _cameraIcon;

	/// The icon geometry to be rendered in object picking mode.
	std::shared_ptr<LinePrimitive> _pickingCameraIcon;

	/// The geometry for the camera's viewing cone and target line.
	std::shared_ptr<LinePrimitive> _cameraCone;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffer.
	SceneObjectCacheHelper<
		WeakVersionedOORef<DataObject>,		// Camera object + revision number
		Color									// Display color
		> _geometryCacheHelper;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffer.
	SceneObjectCacheHelper<
		Color,						// Display color
		FloatType,					// Camera target distance
		bool,						// Target line visible
		FloatType,					// Cone aspect ratio
		FloatType					// Cone angle
		> _coneCacheHelper;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_CAMERA_OBJECT_H
