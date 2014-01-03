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
#include <core/scene/ObjectNode.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/rendering/RenderSettings.h>
#include <core/scene/display/camera/CameraDisplayObject.h>
#include "CameraObject.h"
#include "moc_AbstractCameraObject.cpp"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, AbstractCameraObject, SceneObject)
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, CameraObject, AbstractCameraObject)
IMPLEMENT_OVITO_OBJECT(Core, CameraObjectEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(CameraObject, CameraObjectEditor)
DEFINE_PROPERTY_FIELD(CameraObject, _isPerspective, "IsPerspective")
DEFINE_REFERENCE_FIELD(CameraObject, _fov, "FOV", Controller)
DEFINE_REFERENCE_FIELD(CameraObject, _zoom, "Zoom", Controller)
SET_PROPERTY_FIELD_LABEL(CameraObject, _isPerspective, "Perspective projection")
SET_PROPERTY_FIELD_LABEL(CameraObject, _fov, "View angle")
SET_PROPERTY_FIELD_LABEL(CameraObject, _zoom, "Zoom")
SET_PROPERTY_FIELD_UNITS(CameraObject, _fov, AngleParameterUnit)
SET_PROPERTY_FIELD_UNITS(CameraObject, _zoom, WorldParameterUnit)

/******************************************************************************
* Constructs a camera object.
******************************************************************************/
CameraObject::CameraObject(DataSet* dataset) : AbstractCameraObject(dataset), _isPerspective(true)
{
	INIT_PROPERTY_FIELD(CameraObject::_isPerspective);
	INIT_PROPERTY_FIELD(CameraObject::_fov);
	INIT_PROPERTY_FIELD(CameraObject::_zoom);

	_fov = ControllerManager::instance().createFloatController(dataset);
	_fov->setFloatValue(0, FLOATTYPE_PI/4.0);
	_zoom = ControllerManager::instance().createFloatController(dataset);
	_zoom->setFloatValue(0, 200);

	addDisplayObject(new CameraDisplayObject(dataset));
}

/******************************************************************************
* Asks the object for its validity interval at the given time.
******************************************************************************/
TimeInterval CameraObject::objectValidity(TimePoint time)
{
	TimeInterval interval = TimeInterval::forever();
	if(isPerspective() && _fov) interval.intersect(_fov->validityInterval(time));
	if(!isPerspective() && _zoom) interval.intersect(_zoom->validityInterval(time));
	return interval;
}

/******************************************************************************
* Fills in the missing fields of the camera view descriptor structure.
******************************************************************************/
void CameraObject::projectionParameters(TimePoint time, ViewProjectionParameters& params)
{
	// Transform scene bounding box to camera space.
	Box3 bb = params.boundingBox.transformed(params.viewMatrix).centerScale(1.01);

	// Compute projection matrix.
	params.isPerspective = isPerspective();
	if(params.isPerspective) {
		if(bb.minc.z() < -FLOATTYPE_EPSILON) {
			params.zfar = -bb.minc.z();
			params.znear = std::max(-bb.maxc.z(), params.zfar * 1e-4f);
		}
		else {
			params.zfar = std::max(params.boundingBox.size().length(), FloatType(1));
			params.znear = params.zfar * 1e-4f;
		}
		params.zfar = std::max(params.zfar, params.znear * 1.01f);

		// Get the camera angle.
		params.fieldOfView = _fov->getFloatValue(time, params.validityInterval);
		if(params.fieldOfView < FLOATTYPE_EPSILON) params.fieldOfView = FLOATTYPE_EPSILON;
		if(params.fieldOfView > FLOATTYPE_PI - FLOATTYPE_EPSILON) params.fieldOfView = FLOATTYPE_PI - FLOATTYPE_EPSILON;

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

		// Get the camera zoom.
		params.fieldOfView = _zoom->getFloatValue(time, params.validityInterval);
		if(params.fieldOfView < FLOATTYPE_EPSILON) params.fieldOfView = FLOATTYPE_EPSILON;

		params.projectionMatrix = Matrix4::ortho(-params.fieldOfView / params.aspectRatio, params.fieldOfView / params.aspectRatio,
							-params.fieldOfView, params.fieldOfView, params.znear, params.zfar);
	}
	params.inverseProjectionMatrix = params.projectionMatrix.inverse();
}

/******************************************************************************
* Returns the field of view of the camera.
******************************************************************************/
FloatType CameraObject::fieldOfView(TimePoint time, TimeInterval& validityInterval)
{
	if(isPerspective())
		return _fov->getFloatValue(time, validityInterval);
	else
		return _zoom->getFloatValue(time, validityInterval);
}

/******************************************************************************
* Changes the field of view of the camera.
******************************************************************************/
void CameraObject::setFieldOfView(TimePoint time, FloatType newFOV)
{
	if(isPerspective())
		_fov->setFloatValue(time, newFOV);
	else
		_zoom->setFloatValue(time, newFOV);
}

/******************************************************************************
* Constructor that creates the UI controls for the editor.
******************************************************************************/
void CameraObjectEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the rollout.
	QWidget* rollout = createRollout(tr("Camera"), rolloutParams);

	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(6);

	// Is perspective parameter.
	BooleanParameterUI* isPerspectivePUI = new BooleanParameterUI(this, PROPERTY_FIELD(CameraObject::_isPerspective));
	layout->addWidget(isPerspectivePUI->checkBox());

	QGroupBox* perspectiveProjBox = new QGroupBox(tr("Perspective camera"), rollout);
	perspectiveProjBox->setEnabled(false);
	layout->addWidget(perspectiveProjBox);
	QGridLayout* sublayout = new QGridLayout(perspectiveProjBox);
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setColumnStretch(1, 1);

	// FOV parameter.
	FloatParameterUI* fovPUI = new FloatParameterUI(this, PROPERTY_FIELD(CameraObject::_fov));
	sublayout->addWidget(fovPUI->label(), 0, 0);
	sublayout->addLayout(fovPUI->createFieldLayout(), 0, 1);
	fovPUI->setMinValue(1e-3f);
	fovPUI->setMaxValue(FLOATTYPE_PI - 1e-2f);

	QGroupBox* parallelProjBox = new QGroupBox(tr("Parallel camera"), rollout);
	parallelProjBox->setEnabled(false);
	layout->addWidget(parallelProjBox);
	sublayout = new QGridLayout(parallelProjBox);
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setColumnStretch(1, 1);

	// Zoom parameter.
	FloatParameterUI* zoomPUI = new FloatParameterUI(this, PROPERTY_FIELD(CameraObject::_zoom));
	sublayout->addWidget(zoomPUI->label(), 0, 0);
	sublayout->addLayout(zoomPUI->createFieldLayout(), 0, 1);
	zoomPUI->setMinValue(0);

	connect(isPerspectivePUI->checkBox(), SIGNAL(toggled(bool)), perspectiveProjBox, SLOT(setEnabled(bool)));
	connect(isPerspectivePUI->checkBox(), SIGNAL(toggled(bool)), parallelProjBox, SLOT(setDisabled(bool)));
}

};
