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
#include <core/dataset/DataSet.h>
#include <core/animation/AnimationSettings.h>
#include "UnitsManager.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Units)

/******************************************************************************
* Constructor.
******************************************************************************/
UnitsManager::UnitsManager(DataSet* dataset) : _dataset(dataset)
{
	// Create standard unit objects.
	_units[&FloatParameterUnit::staticMetaObject] = _floatIdentityUnit = new FloatParameterUnit(this, dataset);
	_units[&IntegerParameterUnit::staticMetaObject] = _integerIdentityUnit = new IntegerParameterUnit(this, dataset);
	_units[&TimeParameterUnit::staticMetaObject] = _timeUnit = new TimeParameterUnit(this, dataset);
	_units[&PercentParameterUnit::staticMetaObject] = _percentUnit = new PercentParameterUnit(this, dataset);
	_units[&AngleParameterUnit::staticMetaObject] = _angleUnit = new AngleParameterUnit(this, dataset);
	_units[&WorldParameterUnit::staticMetaObject] = _worldUnit = new WorldParameterUnit(this, dataset);
}

/******************************************************************************
* Returns the global instance of the given parameter unit service.
******************************************************************************/
ParameterUnit* UnitsManager::getUnit(const QMetaObject* parameterUnitClass)
{
	OVITO_CHECK_POINTER(parameterUnitClass);

	auto iter = _units.find(parameterUnitClass);
	if(iter != _units.end())
		return iter->second;

	// Create an instance of this class.
	ParameterUnit* unit = qobject_cast<ParameterUnit*>(parameterUnitClass->newInstance(Q_ARG(QObject*, this), Q_ARG(DataSet*, _dataset)));
	OVITO_ASSERT_MSG(unit != nullptr, "UnitsManager::getUnit()", "Failed to create instance of requested parameter unit type.");
	_units.insert({ parameterUnitClass, unit });

	return unit;
}

/******************************************************************************
* Constructor.
******************************************************************************/
TimeParameterUnit::TimeParameterUnit(QObject* parent, DataSet* dataset) : IntegerParameterUnit(parent, dataset)
{
	connect(dataset, &DataSet::animationSettingsReplaced, this, &TimeParameterUnit::onAnimationSettingsReplaced);
	_animSettings = dataset->animationSettings();
}

/******************************************************************************
* Converts the given string to a time value.
******************************************************************************/
FloatType TimeParameterUnit::parseString(const QString& valueString)
{
	if(!_animSettings) return 0;
	return _animSettings->stringToTime(valueString);
}

/******************************************************************************
* Converts a time value to a string.
******************************************************************************/
QString TimeParameterUnit::formatValue(FloatType value)
{
	if(!_animSettings) return QString();
	return _animSettings->timeToString((TimePoint)value);
}

/******************************************************************************
* Returns the (positive) step size used by spinner widgets for this
* parameter unit type.
******************************************************************************/
FloatType TimeParameterUnit::stepSize(FloatType currentValue, bool upDirection)
{
	if(!_animSettings) return 0;
	if(upDirection)
		return ceil((currentValue + FloatType(1)) / _animSettings->ticksPerFrame()) * _animSettings->ticksPerFrame() - currentValue;
	else
		return currentValue - floor((currentValue - FloatType(1)) / _animSettings->ticksPerFrame()) * _animSettings->ticksPerFrame();
}

/******************************************************************************
* Given an arbitrary value, which is potentially invalid, rounds it to the
* closest valid value.
******************************************************************************/
FloatType TimeParameterUnit::roundValue(FloatType value)
{
	if(!_animSettings) return value;
	return floor(value / _animSettings->ticksPerFrame() + FloatType(0.5)) * _animSettings->ticksPerFrame();
}

/******************************************************************************
* This is called whenever the current animation settings of the dataset have
* been replaced by new ones.
******************************************************************************/
void TimeParameterUnit::onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings)
{
	disconnect(_speedChangedConnection);
	disconnect(_timeFormatChangedConnection);
	_animSettings = newAnimationSettings;
	if(newAnimationSettings) {
		_speedChangedConnection = connect(newAnimationSettings, &AnimationSettings::speedChanged, this, &TimeParameterUnit::formatChanged);
		_timeFormatChangedConnection = connect(newAnimationSettings, &AnimationSettings::timeFormatChanged, this, &TimeParameterUnit::formatChanged);
	}
	Q_EMIT formatChanged();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
