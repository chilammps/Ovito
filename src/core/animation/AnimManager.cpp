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
#include <core/animation/AnimManager.h>
#include "AnimationTimeSpinner.h"

namespace Ovito {

/// The singleton instance of the class.
QScopedPointer<AnimManager> AnimManager::_instance;

/******************************************************************************
* Initializes the animation manager.
******************************************************************************/
AnimManager::AnimManager() : _animSuspendCount(0), nullInterval(0,0),  _animationMode(false)
{
	// Reset the animation manager when a new scene has been loaded.
	connect(&DATASET_MANAGER, SIGNAL(dataSetReset(DataSet*)), this, SLOT(reset()));
}

/******************************************************************************
* Resets the animation manager.
******************************************************************************/
void AnimManager::reset()
{
	setAnimationMode(false);
	
	if(_settings) {
		disconnect(_settings.get(), SIGNAL(timeChanged(TimeTicks)), this, SIGNAL(timeChanged(TimeTicks)));
		disconnect(_settings.get(), SIGNAL(intervalChanged(TimeInterval)), this, SIGNAL(intervalChanged(TimeInterval)));
		disconnect(_settings.get(), SIGNAL(speedChanged(int)), this, SIGNAL(speedChanged(int)));
	}
	
	_settings = DATASET_MANAGER.currentSet()->animationSettings();

	if(_settings) {
		connect(_settings.get(), SIGNAL(timeChanged(TimeTicks)), this, SIGNAL(timeChanged(TimeTicks)));
		connect(_settings.get(), SIGNAL(intervalChanged(TimeInterval)), this, SIGNAL(intervalChanged(TimeInterval)));
		connect(_settings.get(), SIGNAL(speedChanged(int)), this, SIGNAL(speedChanged(int)));

		speedChanged(_settings->ticksPerFrame());
		intervalChanged(_settings->animationInterval());
		timeChanged(_settings->time());
	}
}

/******************************************************************************
* Converts a time value to its string representation.
******************************************************************************/
QString AnimManager::timeToString(TimeTicks time)
{
	return QString::number(timeToFrame(time));
}

/******************************************************************************
* Converts a string to a time value.
* Throws an exception when a parsing error occurs.
******************************************************************************/
TimeTicks AnimManager::stringToTime(const QString& stringValue)
{
	TimeTicks value;
	bool ok;
	value = (TimeTicks)stringValue.toInt(&ok);
	if(!ok)
		throw Exception(tr("Invalid frame number: %1").arg(stringValue));
	return frameToTime(value);
}

/******************************************************************************
* Creates a spinner widget that lets the user control the current animation time.
******************************************************************************/
SpinnerWidget* AnimManager::createCurrentTimeSpinner()
{	
	return new AnimationTimeSpinner();	
}

/******************************************************************************
* Enables or disables animation mode.
******************************************************************************/
void AnimManager::setAnimationMode(bool on)
{
	if(_animationMode == on) return;
	
	_animationMode = on;
	animationModeChanged(_animationMode);
}


};
