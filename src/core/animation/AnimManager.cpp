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
#include <core/dataset/DataSetManager.h>
#include <core/viewport/ViewportManager.h>

namespace Ovito {

/// The singleton instance of the class.
AnimManager* AnimManager::_instance = nullptr;

/******************************************************************************
* Initializes the animation manager.
******************************************************************************/
AnimManager::AnimManager() : _animSuspendCount(0),  _animationMode(false), _timeIsChanging(0)
{
	OVITO_ASSERT_MSG(!_instance, "AnimManager constructor", "Multiple instances of this singleton class have been created.");

	// Reset the animation manager when a new scene has been loaded.
	connect(&DataSetManager::instance(), SIGNAL(dataSetReset(DataSet*)), this, SLOT(reset()));

	// Call our own listener when the current animation time changes.
	connect(this, SIGNAL(timeChanged(TimePoint)), this, SLOT(onTimeChanged(TimePoint)));
}

/******************************************************************************
* Resets the animation manager.
******************************************************************************/
void AnimManager::reset()
{
	setAnimationMode(false);
	
	if(_settings) {
		disconnect(_settings.get(), SIGNAL(timeChanged(TimePoint)), this, SIGNAL(timeChanged(TimePoint)));
		disconnect(_settings.get(), SIGNAL(intervalChanged(TimeInterval)), this, SIGNAL(intervalChanged(TimeInterval)));
		disconnect(_settings.get(), SIGNAL(speedChanged(int)), this, SIGNAL(speedChanged(int)));
	}

	_settings = DataSetManager::instance().currentSet()->animationSettings();

	if(_settings) {
		connect(_settings.get(), SIGNAL(timeChanged(TimePoint)), SIGNAL(timeChanged(TimePoint)));
		connect(_settings.get(), SIGNAL(intervalChanged(TimeInterval)), SIGNAL(intervalChanged(TimeInterval)));
		connect(_settings.get(), SIGNAL(speedChanged(int)), SIGNAL(speedChanged(int)));

		Q_EMIT speedChanged(_settings->ticksPerFrame());
		Q_EMIT intervalChanged(_settings->animationInterval());
		Q_EMIT timeChanged(_settings->time());
	}
}

/******************************************************************************
* Is called when the current animation time has changed.
******************************************************************************/
void AnimManager::onTimeChanged(TimePoint newTime)
{
	// Wait until scene is ready, then repaint viewports.

	_timeIsChanging++;
	qDebug() << "onTimeChanged" << newTime << " _timeIsChanging=" << _timeIsChanging;
	DataSetManager::instance().runWhenSceneIsReady([this] () {
		_timeIsChanging--;
		qDebug() << "onTimeChanged scene is ready handler: current time= " << time() << " _timeIsChanging=" << _timeIsChanging;
		ViewportManager::instance().updateViewports();
	});
}

/******************************************************************************
* Converts a time value to its string representation.
******************************************************************************/
QString AnimManager::timeToString(TimePoint time)
{
	return QString::number(timeToFrame(time));
}

/******************************************************************************
* Converts a string to a time value.
* Throws an exception when a parsing error occurs.
******************************************************************************/
TimePoint AnimManager::stringToTime(const QString& stringValue)
{
	TimePoint value;
	bool ok;
	value = (TimePoint)stringValue.toInt(&ok);
	if(!ok)
		throw Exception(tr("Invalid frame number format: %1").arg(stringValue));
	return frameToTime(value);
}

/******************************************************************************
* Enables or disables animation mode.
******************************************************************************/
void AnimManager::setAnimationMode(bool on)
{
	if(_animationMode == on)
		return;
	
	_animationMode = on;
	animationModeChanged(_animationMode);
}

};
