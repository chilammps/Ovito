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
#include <core/utilities/units/UnitsManager.h>
#include <core/animation/controller/LookAtController.h>
#include <core/scene/SceneNode.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LookAtController, Controller);
DEFINE_REFERENCE_FIELD(LookAtController, _rollCtrl, "Roll", Controller);
DEFINE_FLAGS_REFERENCE_FIELD(LookAtController, _targetNode, "Target", SceneNode, PROPERTY_FIELD_NEVER_CLONE_TARGET | PROPERTY_FIELD_NO_SUB_ANIM);
SET_PROPERTY_FIELD_LABEL(LookAtController, _rollCtrl, "Roll");
SET_PROPERTY_FIELD_LABEL(LookAtController, _targetNode, "Target");
SET_PROPERTY_FIELD_UNITS(LookAtController, _rollCtrl, AngleParameterUnit);

/******************************************************************************
* Constructor.
******************************************************************************/
LookAtController::LookAtController(DataSet* dataset) : Controller(dataset)
{
	INIT_PROPERTY_FIELD(LookAtController::_rollCtrl);
	INIT_PROPERTY_FIELD(LookAtController::_targetNode);

	// Create sub-controller.
	_rollCtrl = ControllerManager::instance().createFloatController(dataset);
}

/******************************************************************************
* Queries the controller for its absolute value at a certain time.
******************************************************************************/
void LookAtController::getRotationValue(TimePoint time, Rotation& result, TimeInterval& validityInterval)
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

	// Get rolling angle.
	FloatType rollAngle = 0.0;
	if(rollController())
		rollAngle = rollController()->getFloatValue(time, validityInterval);

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
void LookAtController::setRotationValue(TimePoint time, const Rotation& newValue, bool isAbsoluteValue)
{
	// Cannot set value for this controller type.
}

/******************************************************************************
* Lets a rotation controller apply its value to an existing transformation matrix.
******************************************************************************/
void LookAtController::applyRotation(TimePoint time, AffineTransformation& result, TimeInterval& validityInterval)
{
	// Save source position for later use.
	_sourcePos = result.translation();
	_sourcePosValidity = validityInterval;

	Controller::applyRotation(time, result, validityInterval);
}

/******************************************************************************
* Computes the largest time interval containing the given time during which the
* controller's value is constant.
******************************************************************************/
TimeInterval LookAtController::validityInterval(TimePoint time)
{
	TimeInterval iv(TimeInterval::infinite());
	if(rollController())
		iv.intersect(rollController()->validityInterval(time));
	if(targetNode())
		targetNode()->getWorldTransform(time, iv);
	return iv;
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
