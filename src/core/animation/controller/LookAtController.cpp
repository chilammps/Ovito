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
#include <core/utilities/units/UnitsManager.h>
#include <core/animation/controller/LookAtController.h>
#include <core/scene/SceneNode.h>

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LookAtController, RotationController)
DEFINE_REFERENCE_FIELD(LookAtController, _rollCtrl, "Roll", FloatController)
DEFINE_FLAGS_REFERENCE_FIELD(LookAtController, _targetNode, "Target", SceneNode, PROPERTY_FIELD_NEVER_CLONE_TARGET)
SET_PROPERTY_FIELD_LABEL(LookAtController, _rollCtrl, "Roll")
SET_PROPERTY_FIELD_LABEL(LookAtController, _targetNode, "Target")
SET_PROPERTY_FIELD_UNITS(LookAtController, _rollCtrl, AngleParameterUnit)

/******************************************************************************
* Default constructor.
******************************************************************************/
LookAtController::LookAtController()
{
	INIT_PROPERTY_FIELD(LookAtController::_rollCtrl);
	INIT_PROPERTY_FIELD(LookAtController::_targetNode);

	// Create sub-controller.
	_rollCtrl = ControllerManager::instance().createDefaultController<FloatController>();
}

/******************************************************************************
* Queries the controller for its absolute value at a certain time.
******************************************************************************/
void LookAtController::getValue(TimePoint time, Rotation& result, TimeInterval& validityInterval)
{
	// Get position of target node.
	Vector3 targetPos = Vector3::Zero();
	if(targetNode()) {
		const AffineTransformation& targetTM = targetNode()->getWorldTransform(time, validityInterval);
		targetPos = targetTM.translation();
	}
	
	if(!_sourcePosValidity.isEmpty())
		validityInterval.intersect(_sourcePosValidity);
	else
		validityInterval.intersect(TimeInterval(time));

	// Get rolling.
	FloatType rollAngle = 0.0;
	if(rollController())
		rollController()->getValue(time, rollAngle, validityInterval);

	if(targetPos == _sourcePos) {
		result.setIdentity();
		return;
	}
	
	AffineTransformation tm = AffineTransformation::lookAt(Point3::Origin() + _sourcePos, Point3::Origin() + targetPos, Vector3(0,0,1));
	tm.translation() = Vector3::Zero();
	result = Rotation(tm).inverse();

	if(rollAngle != 0.0)
		result = result * Rotation(Vector3(0,0,1), rollAngle);

	// Reset source validity.
	_sourcePosValidity.setEmpty();
}

/******************************************************************************
* Sets the controller's value at the specified time.
******************************************************************************/
void LookAtController::setValue(TimePoint time, const Rotation& newValue, bool isAbsoluteValue)
{
	// Cannot set value for this controller type.
}

/******************************************************************************
* Let the controller add its value at a certain time to the input value.
******************************************************************************/
void LookAtController::applyValue(TimePoint time, AffineTransformation& result, TimeInterval& validityInterval)
{
	// Save source position for later use.
	_sourcePos = result.translation();
	_sourcePosValidity = validityInterval;

	RotationController::applyValue(time, result, validityInterval);            			
}

/******************************************************************************
* Computes the largest time interval containing the given time during which the
* controller's value is constant.
******************************************************************************/
TimeInterval LookAtController::validityInterval(TimePoint time)
{
	TimeInterval iv(TimeInterval::forever());
	if(rollController())
		iv.intersect(rollController()->validityInterval(time));
	if(targetNode())
		targetNode()->getWorldTransform(time, iv);
	return iv;
}

};
