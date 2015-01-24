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
#include <core/animation/controller/Controller.h>
#include <core/animation/controller/ConstantControllers.h>
#include <core/animation/controller/LinearInterpolationControllers.h>
#include <core/animation/controller/SplineInterpolationControllers.h>
#include <core/animation/controller/PRSTransformationController.h>
#include <core/animation/AnimationSettings.h>
#include <core/dataset/DataSet.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

/// The singleton instance of the manager class.
ControllerManager* ControllerManager::_instance = nullptr;

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, Controller, RefTarget);

/******************************************************************************
* Returns the float controller's value at the current animation time.
******************************************************************************/
FloatType Controller::currentFloatValue()
{
	TimeInterval iv;
	return getFloatValue(dataset()->animationSettings()->time(), iv);
}

/******************************************************************************
* Returns the integers controller's value at the current animation time.
******************************************************************************/
int Controller::currentIntValue()
{
	TimeInterval iv;
	return getIntValue(dataset()->animationSettings()->time(), iv);
}

/******************************************************************************
* Returns the Vector3 controller's value at the current animation time.
******************************************************************************/
Vector3 Controller::currentVector3Value()
{
	Vector3 v; TimeInterval iv;
	getVector3Value(dataset()->animationSettings()->time(), v, iv);
	return v;
}

/******************************************************************************
* Sets the controller's value at the current animation time.
******************************************************************************/
void Controller::setCurrentFloatValue(FloatType newValue)
{
	setFloatValue(dataset()->animationSettings()->time(), newValue);
}

/******************************************************************************
* Sets the controller's value at the current animation time.
******************************************************************************/
void Controller::setCurrentIntValue(int newValue)
{
	setIntValue(dataset()->animationSettings()->time(), newValue);
}

/******************************************************************************
* Sets the controller's value at the current animation time.
******************************************************************************/
void Controller::setCurrentVector3Value(const Vector3& newValue)
{
	setVector3Value(dataset()->animationSettings()->time(), newValue);
}

/******************************************************************************
* Initializes the controller manager.
******************************************************************************/
ControllerManager::ControllerManager()
{
	OVITO_ASSERT_MSG(!_instance, "ControllerManager constructor", "Multiple instances of this singleton class have been created.");
}

/******************************************************************************
* Creates a new float controller.
******************************************************************************/
OORef<Controller> ControllerManager::createFloatController(DataSet* dataset)
{
	return new LinearFloatController(dataset);
}

/******************************************************************************
* Creates a new integer controller.
******************************************************************************/
OORef<Controller> ControllerManager::createIntController(DataSet* dataset)
{
	return new LinearIntegerController(dataset);
}

/******************************************************************************
* Creates a new Vector3 controller.
******************************************************************************/
OORef<Controller> ControllerManager::createVector3Controller(DataSet* dataset)
{
	return new LinearVectorController(dataset);
}

/******************************************************************************
* Creates a new position controller.
******************************************************************************/
OORef<Controller> ControllerManager::createPositionController(DataSet* dataset)
{
	return new SplinePositionController(dataset);
}

/******************************************************************************
* Creates a new rotation controller.
******************************************************************************/
OORef<Controller> ControllerManager::createRotationController(DataSet* dataset)
{
	return new LinearRotationController(dataset);
}

/******************************************************************************
* Creates a new scaling controller.
******************************************************************************/
OORef<Controller> ControllerManager::createScalingController(DataSet* dataset)
{
	return new LinearScalingController(dataset);
}

/******************************************************************************
* Creates a new transformation controller.
******************************************************************************/
OORef<Controller> ControllerManager::createTransformationController(DataSet* dataset)
{
	return new PRSTransformationController(dataset);
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
