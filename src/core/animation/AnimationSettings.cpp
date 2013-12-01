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
#include "AnimationSettings.h"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, AnimationSettings, RefTarget);
DEFINE_PROPERTY_FIELD(AnimationSettings, _time, "Time");
DEFINE_PROPERTY_FIELD(AnimationSettings, _animationInterval, "AnimationInterval");
DEFINE_PROPERTY_FIELD(AnimationSettings, _ticksPerFrame, "TicksPerFrame");
DEFINE_PROPERTY_FIELD(AnimationSettings, _playbackSpeed, "PlaybackSpeed");

/******************************************************************************
* Default constructor. Assigns default values.
******************************************************************************/
AnimationSettings::AnimationSettings(DataSet* dataset) : RefTarget(dataset),
		_ticksPerFrame(TICKS_PER_SECOND/10), _playbackSpeed(1),
		_animationInterval(0, 0), _time(0), _animSuspendCount(0),  _autoKeyMode(false), _timeIsChanging(0)
{
	INIT_PROPERTY_FIELD(AnimationSettings::_time);
	INIT_PROPERTY_FIELD(AnimationSettings::_animationInterval);
	INIT_PROPERTY_FIELD(AnimationSettings::_ticksPerFrame);
	INIT_PROPERTY_FIELD(AnimationSettings::_playbackSpeed);

	// Call our own listener when the current animation time changes.
	connect(this, SIGNAL(timeChanged(TimePoint)), this, SLOT(onTimeChanged(TimePoint)));
	connect(this, SIGNAL(intervalChanged(TimeInterval)), this, SLOT(onIntervalChanged(TimeInterval)));
}

/******************************************************************************
* Is called when the value of a non-animatable property field of this RefMaker has changed.
******************************************************************************/
void AnimationSettings::propertyChanged(const PropertyFieldDescriptor& field)
{
	if(field == PROPERTY_FIELD(AnimationSettings::_time))
		Q_EMIT timeChanged(time());
	else if(field == PROPERTY_FIELD(AnimationSettings::_animationInterval))
		Q_EMIT intervalChanged(animationInterval());
	else if(field == PROPERTY_FIELD(AnimationSettings::_ticksPerFrame))
		Q_EMIT speedChanged(ticksPerFrame());
}

/******************************************************************************
* Saves the class' contents to an output stream.
******************************************************************************/
void AnimationSettings::saveToStream(ObjectSaveStream& stream)
{
	RefTarget::saveToStream(stream);
	stream.beginChunk(0x01);
	stream << _namedFrames;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from an input stream.
******************************************************************************/
void AnimationSettings::loadFromStream(ObjectLoadStream& stream)
{
	RefTarget::loadFromStream(stream);
	stream.expectChunk(0x01);
	stream >> _namedFrames;
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> AnimationSettings::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<AnimationSettings> clone = static_object_cast<AnimationSettings>(RefTarget::clone(deepCopy, cloneHelper));

	// Copy internal data.
	clone->_namedFrames = this->_namedFrames;

	return clone;
}

/******************************************************************************
* Is called when the current animation time has changed.
******************************************************************************/
void AnimationSettings::onTimeChanged(TimePoint newTime)
{
	// Wait until scene is ready, then repaint viewports.

	_timeIsChanging++;
	dataSet()->runWhenSceneIsReady([this] () {
		_timeIsChanging--;
#if 0
		ViewportManager::instance().updateViewports();
#endif
	});
}

/******************************************************************************
* Is called whenever the active animation interval has changed.
******************************************************************************/
void AnimationSettings::onIntervalChanged(TimeInterval newAnimationInterval)
{
	bool isAnimationInterval = newAnimationInterval.duration() != 0;
#if 0
	ActionManager::instance().getAction(ACTION_GOTO_START_OF_ANIMATION)->setEnabled(isAnimationInterval);
	ActionManager::instance().getAction(ACTION_GOTO_PREVIOUS_FRAME)->setEnabled(isAnimationInterval);
	ActionManager::instance().getAction(ACTION_TOGGLE_ANIMATION_PLAYBACK)->setEnabled(isAnimationInterval);
	ActionManager::instance().getAction(ACTION_GOTO_NEXT_FRAME)->setEnabled(isAnimationInterval);
	ActionManager::instance().getAction(ACTION_GOTO_END_OF_ANIMATION)->setEnabled(isAnimationInterval);
#endif
}

/******************************************************************************
* Converts a time value to its string representation.
******************************************************************************/
QString AnimationSettings::timeToString(TimePoint time)
{
	return QString::number(timeToFrame(time));
}

/******************************************************************************
* Converts a string to a time value.
* Throws an exception when a parsing error occurs.
******************************************************************************/
TimePoint AnimationSettings::stringToTime(const QString& stringValue)
{
	TimePoint value;
	bool ok;
	value = (TimePoint)stringValue.toInt(&ok);
	if(!ok)
		throw Exception(tr("Invalid frame number format: %1").arg(stringValue));
	return frameToTime(value);
}

/******************************************************************************
* Enables or disables auto key generation mode.
******************************************************************************/
void AnimationSettings::setAutoKeyMode(bool on)
{
	if(_autoKeyMode == on)
		return;

	_autoKeyMode = on;
	Q_EMIT autoKeyModeChanged(_autoKeyMode);
}

};
