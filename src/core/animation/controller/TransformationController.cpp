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
#include <core/animation/controller/TransformationController.h>
#include <core/utilities/units/UnitsManager.h>
#include <core/animation/TimeInterval.h>
#include <base/linalg/AffineDecomposition.h>

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, TransformationController, Controller)
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, PRSTransformationController, TransformationController)
DEFINE_REFERENCE_FIELD(PRSTransformationController, _position, "Position", PositionController)
DEFINE_REFERENCE_FIELD(PRSTransformationController, _rotation, "Rotation", RotationController)
DEFINE_REFERENCE_FIELD(PRSTransformationController, _scaling, "Scaling", ScalingController)
SET_PROPERTY_FIELD_LABEL(PRSTransformationController, _position, "Position")
SET_PROPERTY_FIELD_LABEL(PRSTransformationController, _rotation, "Rotation")
SET_PROPERTY_FIELD_LABEL(PRSTransformationController, _scaling, "Scaling")
SET_PROPERTY_FIELD_UNITS(PRSTransformationController, _position, WorldParameterUnit)
SET_PROPERTY_FIELD_UNITS(PRSTransformationController, _rotation, AngleParameterUnit)
SET_PROPERTY_FIELD_UNITS(PRSTransformationController, _scaling, PercentParameterUnit)

/******************************************************************************
* Default constructor.
******************************************************************************/
PRSTransformationController::PRSTransformationController()
{
	INIT_PROPERTY_FIELD(PRSTransformationController::_position);
	INIT_PROPERTY_FIELD(PRSTransformationController::_rotation);
	INIT_PROPERTY_FIELD(PRSTransformationController::_scaling);
	_position = ControllerManager::instance().createDefaultController<PositionController>();
	_rotation = ControllerManager::instance().createDefaultController<RotationController>();
	_scaling = ControllerManager::instance().createDefaultController<ScalingController>();
}

/******************************************************************************
* Let the controller apply its value at a certain time to the input value.
******************************************************************************/
void PRSTransformationController::applyValue(TimePoint time, AffineTransformation& result, TimeInterval& validityInterval)
{
	positionController()->applyValue(time, result, validityInterval);
	rotationController()->applyValue(time, result, validityInterval);
	scalingController()->applyValue(time, result, validityInterval);
}

/******************************************************************************
* Sets the controller's value at the specified time.
******************************************************************************/
void PRSTransformationController::setValue(TimePoint time, const AffineTransformation& newValue, bool isAbsoluteValue)
{
	AffineDecomposition decomp(newValue);
	positionController()->setValue(time, decomp.translation, isAbsoluteValue);
	rotationController()->setValue(time, Rotation(decomp.rotation), isAbsoluteValue);
	OVITO_ASSERT_MSG(fabs(decomp.scaling.Q.dot(decomp.scaling.Q) - 1.0) <= FLOATTYPE_EPSILON, "PRSTransformationController::setValue", "Quaternion must be normalized.");
	scalingController()->setValue(time, decomp.scaling, isAbsoluteValue);
}

/******************************************************************************
* This asks the controller to adjust its value after a scene node has got a new
* parent node.
*		oldParentTM - The transformation of the old parent node
*		newParentTM - The transformation of the new parent node
*		contextNode - The node to which this controller is assigned to
******************************************************************************/
void PRSTransformationController::changeParent(TimePoint time, const AffineTransformation& oldParentTM, const AffineTransformation& newParentTM, SceneNode* contextNode)
{
	positionController()->changeParent(time, oldParentTM, newParentTM, contextNode);
	rotationController()->changeParent(time, oldParentTM, newParentTM, contextNode);
	scalingController()->changeParent(time, oldParentTM, newParentTM, contextNode);
}

/******************************************************************************
* Computes the largest time interval containing the given time during which the
* controller's value is constant.
******************************************************************************/
TimeInterval PRSTransformationController::validityInterval(TimePoint time)
{
	TimeInterval iv = TimeInterval::forever();
	iv.intersect(positionController()->validityInterval(time));
	iv.intersect(rotationController()->validityInterval(time));
	iv.intersect(scalingController()->validityInterval(time));
	return iv;
}


};
