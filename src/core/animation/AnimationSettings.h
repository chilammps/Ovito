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

/** 
 * \file AnimationSettings.h 
 * \brief Contains the definition of the Core::AnimationSettings class. 
 */
 
#ifndef __OVITO_ANIMATION_SETTINGS_H
#define __OVITO_ANIMATION_SETTINGS_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include "TimeInterval.h"

namespace Core {

/**
 * \brief This class stores the animation settings for a scene.
 * 
 * Each DataSet has an instance of this class associated with it. It
 * can be accessed via DataSet::animationSettings().
 * 
 * To access the animation settings of the current scene you can also use
 * the methods of AnimManager.
 * 
 * \author Alexander Stukowski
 * \sa AnimManager
 */
class CORE_DLLEXPORT AnimationSettings : public RefTarget
{
public:

	/// \brief Default constructor that initializes the object with default values.
	/// \param isLoading Indicates whether the object is being loaded from a file. 
	///                  This parameter is only used by the object serialization system.
	AnimationSettings(bool isLoading = false);

	/// \brief Gets the current animation time.
	/// \return The current time.
	/// 
	/// The state of the scene at this time is shown in the viewports.
	/// \sa setTime()
	TimeTicks time() const { return _time; }

	/// \brief Sets the current animation time.
	/// \param time The new animation time.
	///
	/// The state of the scene at the given time will be shown in the viewports.
	/// \undoable
	/// \sa time()
	void setTime(TimeTicks time);

	/// \brief Gets the animation interval.
	/// \return The time interval of the animation.
	/// \sa setAnimationInterval() 
	const TimeInterval& animationInterval() const { return _animationInterval; }

	/// \brief Sets the animation interval.
	/// \param interval The new animation interval for the scene.
	/// \undoable
	/// \sa animationInterval()
	void setAnimationInterval(const TimeInterval& interval);
	
	/// \brief Returns the number of frames per second.
	/// \return The number of frames per second.
	/// 
	/// This setting controls the playback speed of the animation.
	///
	/// \sa setFramesPerSecond()
	int framesPerSecond() const { return TICKS_PER_SECOND / _ticksPerFrame; }

	/// \brief Sets the number of frames per second.
	/// \param fps The number of frames per second. Please note that not all
	///            values are allowed here because time is measured in integer ticks units.
	/// \undoable
	/// 
	/// This setting controls the playback speed of the animation.
	///
	/// \sa setFramesPerSecond()
	/// \sa framesPerSecond()
	/// \sa setTicksPerFrame()
	void setFramesPerSecond(int fps) { setTicksPerFrame(TICKS_PER_SECOND / fps); } 	

	/// \brief Returns the number of time ticks per frame.
	/// \return The number of time ticks per animation frame. One tick is 1/4800 of a second.
	/// 
	/// This setting controls the playback speed of the animation.
	///
	/// \sa setTicksPerFrame()
	int ticksPerFrame() const { return _ticksPerFrame; }

	/// \brief Sets the number of time ticks per frame.
	/// \param ticksPerFrame The number of time tick units per animation frame.
	///                      Thsi must be a positive value.
	/// \undoable
	/// 
	/// This setting controls the playback speed of the animation.
	/// \sa ticksPerFrame()
	void setTicksPerFrame(int ticksPerFrame);

	/// \brief Returns the playback speed factor that is used for animation playback in the viewports.
	/// \return The playback speed factor. A value greater than 1 means that the animation is played at a speed higher
	///         than realtime whereas a value smaller than -1 means that the animation is played at a speed lower than realtime.
	/// \sa setPlaybackSpeed()
	int playbackSpeed() const { return _playbackSpeed; }

	/// \brief Sets the playback speed factor that is used for animation playback in the viewport.
	/// \param factor A value greater than 1 means that the animation is played at a speed higher
	///               than realtime. A value smaller than -1 that the animation is played at a speed lower than realtime.
	/// \undoable
	/// \sa playbackSpeed()
	void setPlaybackSpeed(int factor);
	
Q_SIGNALS:

	/// This signal is emmited by this object when its current animation time has changed.
	void timeChanged(TimeTicks newTime);
	
	/// This signal is emmited by this object when its animation interval has changed.
	void intervalChanged(TimeInterval newAnimationInterval);
	
	/// This signal is emmited by this object when its animation speed has changed.
	void speedChanged(int ticksPerFrame);
	
protected:

	/// Saves the class' contents to the given stream. 
	virtual void saveToStream(ObjectSaveStream& stream);
	
	/// Loads the class' contents from the given stream. 
	virtual void loadFromStream(ObjectLoadStream& stream);

	/// Creates a copy of this object. 
	virtual RefTarget::SmartPtr clone(bool deepCopy, CloneHelper& cloneHelper);
	
private:

	/// The current animation working time.
    TimeTicks _time;

	/// The start and end times of the animation.
	TimeInterval _animationInterval;

	/// The number of time ticks per frame.
	/// This controls the animation speed.
	int _ticksPerFrame;
	
	/// The playback speed factor that is used for animation playback in the viewport.
	/// A value greater than 1 means that the animation is played at a speed higher
	/// than realtime.
	/// A value smaller than -1 that the animation is played at a speed lower than realtime.	
	int _playbackSpeed;

	Q_OBJECT
	DECLARE_SERIALIZABLE_PLUGIN_CLASS(AnimationSettings)
};


};

#endif // __OVITO_ANIMATION_SETTINGS_H
