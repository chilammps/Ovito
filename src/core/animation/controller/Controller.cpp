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
#include <core/animation/controller/StandardControllers.h>
#include <core/animation/controller/TransformationController.h>
#include <core/gui/undo/UndoManager.h>

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Controller, RefTarget)
IMPLEMENT_OVITO_OBJECT(FloatController, Controller)
IMPLEMENT_OVITO_OBJECT(IntegerController, Controller)
IMPLEMENT_OVITO_OBJECT(BooleanController, Controller)
IMPLEMENT_OVITO_OBJECT(VectorController, Controller)
IMPLEMENT_OVITO_OBJECT(PositionController, Controller)
IMPLEMENT_OVITO_OBJECT(RotationController, Controller)
IMPLEMENT_OVITO_OBJECT(ScalingController, Controller)

///////////////////////////// SINGLETON CLASS METHODS ///////////////////////////////

/// The singleton instance of the class.
QScopedPointer<ControllerManager> ControllerManager::_instance;

/******************************************************************************
* Initializes the controller manager.
******************************************************************************/
ControllerManager::ControllerManager()
{
	OVITO_ASSERT_MSG(!_instance, "ControllerManager constructor", "Multiple instances of this singleton class have been created.");

	_defaultMap[&FloatController::OOType] = &LinearFloatController::OOType;
	_defaultMap[&IntegerController::OOType] = &ConstIntegerController::OOType;
	_defaultMap[&BooleanController::OOType] = &ConstBooleanController::OOType;
	_defaultMap[&VectorController::OOType] = &LinearVectorController::OOType;
	_defaultMap[&PositionController::OOType] = &LinearPositionController::OOType;
	_defaultMap[&RotationController::OOType] = &LinearRotationController::OOType;
	_defaultMap[&ScalingController::OOType] = &LinearScalingController::OOType;
	_defaultMap[&TransformationController::OOType] = &PRSTransformationController::OOType;
}

/// Creates a new instance of the default implementation for the given base controller type.
OORef<Controller> ControllerManager::createDefaultController(const OvitoObjectType& controllerBaseClass)
{
	auto iter = _defaultMap.find(&controllerBaseClass);
	if(iter == _defaultMap.end()) {
		OVITO_ASSERT_MSG(false, "ControllerManager::createDefaultController", "Unknown controller base class.");
		return nullptr;
	}
	OVITO_ASSERT_MSG(iter->second != NULL, "ControllerManager::createDefaultController", "No default controller implementation available.");
	return static_object_cast<Controller>(iter->second->createInstance());
}

};
