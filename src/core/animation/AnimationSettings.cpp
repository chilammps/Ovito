///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2014) Alexander Stukowski
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

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, AnimationSettings, RefTarget);
DEFINE_PROPERTY_FIELD(AnimationSettings, _time, "Time");
DEFINE_PROPERTY_FIELD(AnimationSettings, _animationInterval, "AnimationInterval");
DEFINE_PROPERTY_FIELD(AnimationSettings, _ticksPerFrame, "TicksPerFrame");
DEFINE_PROPERTY_FIELD(AnimationSettings, _playbackSpeed, "PlaybackSpeed");

/******************************************************************************
* Constructor.
******************************************************************************/
AnimationSettings::AnimationSettings(DataSet* dataset) : RefTarget(dataset),
		_ticksPerFrame(TICKS_PER_SECOND/10), _playbackSpeed(1),
		_animationInterval(0, 0), _time(0), _animSuspendCount(0),  _autoKeyMode(false), _timeIsChanging(0),
		_isPlaybackActive(false)
{
	INIT_PROPERTY_FIELD(AnimationSettings::_time);
	INIT_PROPERTY_FIELD(AnimationSettings::_animationInterval);
	INIT_PROPERTY_FIELD(AnimationSettings::_ticksPerFrame);
	INIT_PROPERTY_FIELD(AnimationSettings::_playbackSpeed);

	// Call our own listener when the current animation time changes.
	connect(this, &AnimationSettings::timeChanged, this, &AnimationSettings::onTimeChanged);
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
	_timeIsChanging++;
	dataset()->runWhenSceneIsReady([this] () {
		_timeIsChanging--;
		Q_EMIT timeChangeComplete();
	});
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

/******************************************************************************
* Sets the current animation time to the start of the animation interval.
******************************************************************************/
void AnimationSettings::jumpToAnimationStart()
{
	setTime(animationInterval().start());
}

/******************************************************************************
* Sets the current animation time to the end of the animation interval.
******************************************************************************/
void AnimationSettings::jumpToAnimationEnd()
{
	setTime(animationInterval().end());
}

/******************************************************************************
* Jumps to the previous animation frame.
******************************************************************************/
void AnimationSettings::jumpToPreviousFrame()
{
	// Subtract one frame from current time.
	TimePoint newTime = frameToTime(timeToFrame(time()) - 1);
	// Clamp new time
	newTime = std::max(newTime, animationInterval().start());
	// Set new time.
	setTime(newTime);
}

/******************************************************************************
* Jumps to the previous animation frame.
******************************************************************************/
void AnimationSettings::jumpToNextFrame()
{
	// Subtract one frame from current time.
	TimePoint newTime = frameToTime(timeToFrame(time()) + 1);
	// Clamp new time
	newTime = std::min(newTime, animationInterval().end());
	// Set new time.
	setTime(newTime);
}

/******************************************************************************
* Starts playback of the animation in the viewports.
******************************************************************************/
void AnimationSettings::startAnimationPlayback()
{
	if(!_isPlaybackActive) {
		int timerSpeed = 1000;
		if(playbackSpeed() > 1) timerSpeed /= playbackSpeed();
		else if(playbackSpeed() < -1) timerSpeed *= -playbackSpeed();
		_isPlaybackActive = true;
		QTimer::singleShot(timerSpeed / framesPerSecond(), this, SLOT(onPlaybackTimer()));
	}
}

/******************************************************************************
* Stops playback of the animation in the viewports.
******************************************************************************/
void AnimationSettings::stopAnimationPlayback()
{
	_isPlaybackActive = false;
}

/******************************************************************************
* Timer callback used during animation playback.
******************************************************************************/
void AnimationSettings::onPlaybackTimer()
{
	// Check if the animation playback has been deactivated in the meantime.
	if(!_isPlaybackActive)
		return;

	// Add one frame to current time
	int newFrame = timeToFrame(time()) + 1;
	TimePoint newTime = frameToTime(newFrame);

	// Loop back to first frame if end has been reached.
	if(newTime > animationInterval().end())
		newTime = animationInterval().start();

	// Set new time.
	setTime(newTime);

	// Wait until the scene is ready. Then jump to the next frame.
	dataset()->runWhenSceneIsReady([this]() {
		if(_isPlaybackActive) {
			_isPlaybackActive = false;
			startAnimationPlayback();
		}
	});
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
