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
#include "UnitsManager.h"

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(ParameterUnit, OvitoObject);
IMPLEMENT_OVITO_OBJECT(FloatParameterUnit, ParameterUnit);
IMPLEMENT_OVITO_OBJECT(IntegerParameterUnit, ParameterUnit);
IMPLEMENT_OVITO_OBJECT(WorldParameterUnit, FloatParameterUnit);
IMPLEMENT_OVITO_OBJECT(AngleParameterUnit, FloatParameterUnit);
IMPLEMENT_OVITO_OBJECT(PercentParameterUnit, FloatParameterUnit);
IMPLEMENT_OVITO_OBJECT(TimeParameterUnit, IntegerParameterUnit);

/// The singleton instance of the class.
QScopedPointer<UnitsManager> UnitsManager::_instance;

/******************************************************************************
* Constructor.
******************************************************************************/
UnitsManager::UnitsManager()
{
	OVITO_ASSERT_MSG(!_instance, "UnitsManager constructor", "Multiple instances of this singleton class have been created.");

	// Create standard unit objects.
	_units[&FloatParameterUnit::OOType] = _floatIdentityUnit = new FloatParameterUnit();
	_units[&IntegerParameterUnit::OOType] = _integerIdentityUnit = new IntegerParameterUnit();
	_units[&TimeParameterUnit::OOType] = _timeUnit = new TimeParameterUnit();
	_units[&PercentParameterUnit::OOType] = _percentUnit = new PercentParameterUnit();
	_units[&AngleParameterUnit::OOType] = _angleUnit = new AngleParameterUnit();
	_units[&WorldParameterUnit::OOType] = _worldUnit = new WorldParameterUnit();
}

/******************************************************************************
* Returns the global instance of the given parameter unit service.
******************************************************************************/
ParameterUnit* UnitsManager::getUnit(const OvitoObjectType& parameterUnitClass)
{
	OVITO_CHECK_POINTER(&parameterUnitClass);
	OVITO_ASSERT_MSG(parameterUnitClass.isDerivedFrom(ParameterUnit::OOType), "UnitsManager::getUnit()", "A ParameterUnit derived must be specified.");

	auto iter = _units.find(&parameterUnitClass);
	if(iter != _units.end())
		return iter->second.get();

	// Create an instance of this class.
	OORef<ParameterUnit> unit = static_object_cast<ParameterUnit>(parameterUnitClass.createInstance());
	_units.insert(std::make_pair(&parameterUnitClass, unit));

	return unit.get();
}

};
