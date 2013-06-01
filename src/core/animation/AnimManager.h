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
 * \file AnimManager.h 
 * \brief Contains the definition of the Ovito::AnimManager class.
 */
 
#ifndef __OVITO_ANIM_MANAGER_H
#define __OVITO_ANIM_MANAGER_H

#include <core/Core.h>
#include "AnimationSettings.h"
#include "TimeInterval.h"

namespace Ovito {
	
/**
 * \brief Manages the global animation settings.
 */
class AnimManager : public QObject
{
	Q_OBJECT

public:

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the AnimManager singleton class.
	inline static AnimManager& instance() {
		if(!_instance) _instance.reset(new AnimManager());
		return *_instance.data();
	}

	/// \brief Returns whether animation recording is active and animation keys should be automatically generated.
	/// \return \c true if animating is currently turned on and not suspended; \c false otherwise.
	/// 
	/// When animating is turned on, controllers should automatically set keys when their value is changed.
	bool isAnimating() const { return _animationMode && _animSuspendCount == 0; }
	
	/// \brief Returns whether animation mode has been activated.
	/// \return \c true if the automatic generation of keys has been enabled. 
	/// \note The automatic generation of animation keys may be suspended by a call to suspendAnim().
	///       This overrides the animation mode. Controllers should use isAnimating() to check whether
	///       have to generate a key on value change.
	bool animationMode() const { return _animationMode; }

	/// \brief Suspends the animation mode temporarily.
	/// 
	/// Automatic generation of animation keys is suspended by this method until a call to resumeAnim().
	/// If suspendAnim() is called multiple times then resumeAnim() must be called the same number of
	/// times until animation mode is enabled again.
	///
	/// It is recommended to use the AnimationSuspender helper class to suspend animation mode because
	/// this is more exception save than the suspendAnim()/resumeAnim() combination.
	void suspendAnim() { _animSuspendCount++; }

	/// \brief Resumes the automatic generation of animation keys.
	/// 
	/// This re-enables animation mode after it had 
	/// been suspended by a call to suspendAnim().
	void resumeAnim() { 
		OVITO_ASSERT_MSG(_animSuspendCount > 0, "AnimManager::resumeAnim()", "resumeAnim() has been called more often than suspendAnim()."); 
		_animSuspendCount--; 
	}

	/// \brief Returns the current animation time.
	/// \return The current time.
	/// 
	/// The state of the scene at this time is shown in the viewports.
	TimePoint time() const {
		if(!_settings) return 0; 
		return _settings->time(); 
	}

	/// \brief Sets the current animation time.
	/// \param time The new animation time.
	///
	/// The state of the scene at the given time will be shown in the viewports.
	/// \undoable
	void setTime(TimePoint time) {
		if(_settings) _settings->setTime(time); 
	}

	/// \brief Gets the animation interval.
	/// \return The time interval of the animation.
	const TimeInterval& animationInterval() const { 
		if(!_settings) {
			static const TimeInterval nullInterval;
			return nullInterval;
		}
		return _settings->animationInterval(); 
	}

	/// \brief Sets the animation interval.
	/// \param interval The new animation interval for the current scene.
	/// \undoable
	void setAnimationInterval(const TimeInterval& interval) { 
		if(_settings) _settings->setAnimationInterval(interval); 
	}
	
	/// \brief Returns the number of frames per second.
	/// \return The number of frames per second.
	/// 
	/// This setting controls the playback speed of the animation.
	int framesPerSecond() const { 
		if(!_settings) return 1;
		return _settings->framesPerSecond(); 
	}

	/// \brief Sets the number of frames per second.
	/// \param fps The number of frames per second. Please note that not all
	///            values are allowed here because time is measured in integer ticks units.
	/// \undoable
	/// 
	/// This setting controls the playback speed of the animation.
	void setFramesPerSecond(int fps) { 
		if(_settings) _settings->setFramesPerSecond(fps); 
	}

	/// \brief Returns the number of time ticks per frame.
	/// \return The number of time ticks per animation frame. One tick is 1/4800 of a second.
	/// 
	/// This setting controls the playback speed of the animation.
	int ticksPerFrame() const { 
		if(!_settings) return TICKS_PER_SECOND;
		return _settings->ticksPerFrame(); 
	}

	/// \brief Sets the number of time ticks per frame.
	/// \param ticksPerFrame The number of time tick units per animation frame.
	///                      Thsi must be a positive value.
	/// \undoable
	/// 
	/// This setting controls the playback speed of the animation.
	void setTicksPerFrame(int ticksPerFrame) {
		if(_settings) _settings->setTicksPerFrame(ticksPerFrame);
	}

	/// \brief Returns the playback speed factor that is used for animation playback in the viewports.
	/// \return The playback speed factor. A value greater than 1 means that the animation is played at a speed higher
	///         than realtime whereas a value smaller than -1 means that the animation is played at a speed lower than realtime.
	int playbackSpeed() const { 
		if(!_settings) return 1;
		return _settings->playbackSpeed(); 
	}

	/// \brief Sets the playback speed factor that is used for animation playback in the viewport.
	/// \param factor A value greater than 1 means that the animation is played at a speed higher
	///               than realtime. A value smaller than -1 that the animation is played at a speed lower than realtime.
	/// \undoable
	void setPlaybackSpeed(int factor) {
		if(_settings) _settings->setPlaybackSpeed(factor);
	}
	
	/// \brief Converts an animation frame number to a time value.
	/// \param frame A frame number starting at 0.
	/// \return The animation time at which the animation frame begins.
	TimePoint frameToTime(int frame) const { return frame * ticksPerFrame(); }
	
	/// \brief Converts a time value to an animation frame number.
	/// \param time A time in ticks units.
	/// \return The animation frame that corresponds to the given time.
	int timeToFrame(TimePoint time) const { return time / ticksPerFrame(); }

	/// \brief Converts a time value to its string representation.
	/// \param time Some time value.
	/// \return A human-readable representation of the time value.
	QString timeToString(TimePoint time);

	/// \brief Converts a string to a time value.
	/// \param stringValue The  human-readable representation of a time value.
	/// \return The parsed time value.
	/// \throw Exception when a parsing error occurs.
	TimePoint stringToTime(const QString& stringValue);

public Q_SLOTS:

	/// \brief Enables or disables animation mode.
	/// \param on Controls the state of the animation mode.
	/// \note The automatic generation of animation keys may be suspended by a call to suspendAnim().
	///       This overrides the animation mode. Controllers should use isAnimating() to check whether
	///       have to generate a key on value change.
	void setAnimationMode(bool on);

	/// \brief Resets the animation manager.
	/// 
	/// This turns animation mode off.
	void reset();

Q_SIGNALS:

	/// This signal is emitted by the AnimManager when the current animation time has changed.
	void timeChanged(TimePoint newTime);

	/// This signal is emitted by the AnimManager when the animation interval has changed.
	void intervalChanged(TimeInterval newAnimationInterval);

	/// This signal is emitted by the AnimManager when the animation speed has changed.
	void speedChanged(int ticksPerFrame);

	/// This signal is emitted by the AnimManager when the time to string conversion format has changed.
	void timeFormatChanged();
	
	/// This signal is emitted when the animation mode has been activated or deactivated.
	void animationModeChanged(bool active);
	
private:

	/// Counts the number of times the animation modes has been suspended.
	int _animSuspendCount;

	/// Points to the animation settings of the current DataSet.
	OORef<AnimationSettings> _settings;
	
	/// Indicates whether animation recording mode is active.
	bool _animationMode;

private:
    
	/// Private constructor.
	/// This is a singleton class; no public instances are allowed.
	AnimManager();

	/// The singleton instance of this class.
	static QScopedPointer<AnimManager> _instance;
};

/**
 * \brief A small helper object that suspends generation of animation keys while it
 *        exists. It can be used to make your code exception-safe.
 * 
 * The constructor of this class calls AnimManager::suspendAnim() and
 * the destructor calls AnimManager::resumeAnim().
 * 
 * Just create an instance of this class on the stack to suspend the automatic generation of animation keys on the undo stack
 * during the lifetime of the class instance.
 */
struct AnimationSuspender {
	AnimationSuspender() { AnimManager::instance().suspendAnim(); }
	~AnimationSuspender() { AnimManager::instance().resumeAnim(); }
};

};

#endif // __OVITO_ANIM_MANAGER_H
