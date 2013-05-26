///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2008) Alexander Stukowski
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
 * \file TimeInterval.h 
 * \brief Contains definition of the Core::TimeInterval structure. 
 */
 
#ifndef __OVITO_TIME_INTERVAL_H
#define __OVITO_TIME_INTERVAL_H

#include <core/Core.h>
#include <base/io/SaveStream.h>
#include <base/io/LoadStream.h>

namespace Core {

/** 
 * \fn typedef TimeTicks
 * 
 * This is the data type used for time values.
 * One time tick unit is 1/4800 of a second in real time.
 * 
 * Please note that this is an integer data type. Internal times are measured
 * in discrete steps of 1/4800 or a second to avoid rounding errors. There is no finer resolution.
 */
typedef int TimeTicks;

/// This is the number of ticks per second.
#define TICKS_PER_SECOND		4800

/// Special time tick value that specifies an infinite negative value on the time axis.
#define TimeNegativeInfinity (numeric_limits<TimeTicks>::min())

/// Special time tick value that specifies an infinite value on the time axis.
#define TimePositiveInfinity (numeric_limits<TimeTicks>::max())

/// The infinite time interval that contains all time values.
#define TimeForever TimeInterval(TimeNegativeInfinity, TimePositiveInfinity)

/// The empty time interval.
#define TimeNever TimeInterval(TimeNegativeInfinity)

/// \brief This is a function to convert time tick units to real seconds.
/// \param time The time in time ticks units.
/// \return The same time in seconds.
inline FloatType timeToSeconds(TimeTicks time) { return (FloatType)time / (FloatType)TICKS_PER_SECOND; }

// \brief This is a function to convert seconds to time ticks units.
/// \param timeInSeconds The time in seconds.
/// \return The same time in time ticks units.
inline TimeTicks secondsToTime(FloatType timeInSeconds) { return (TimeTicks)ceil(timeInSeconds * (FloatType)TICKS_PER_SECOND + (FloatType)0.5); }

/// \brief This converts a time ticks value to a frame number.
/// \param time The time.
/// \param ticksPerFrame Number of ticks per animation frame. 
/// \return The frame number.
inline int timeToFrame(TimeTicks time, int ticksPerFrame) { return time / ticksPerFrame; }

/// \brief This converts a frame number to a time ticks value.
/// \param frame The frame number. 
/// \param ticksPerFrame Number of ticks per animation frame.
/// \return The time (beginning of frame)
inline TimeTicks frameToTime(int frame, int ticksPerFrame) { return frame * ticksPerFrame; }

/**
 * \brief Represents an interval in time.
 * 
 * \author Alexander Stukowski
 */
class TimeInterval
{
public:

	/// \brief Creates an empty time interval.
	/// 
	/// Both start time and end time are initialized to negative infinity. 
	TimeInterval() : _start(TimeNegativeInfinity), _end(TimeNegativeInfinity) {}
	
	/// \brief Initializes the interval with start and end values.
	/// \param start The start time of the time interval.
	/// \param end The end time (including) of the time interval.
	TimeInterval(const TimeTicks& start, const TimeTicks& end) : _start(start), _end(end) {}
	
	/// \brief Initializes the interval to an instant time.
	/// \param time The time where the interval starts and ends.
	TimeInterval(const TimeTicks& time) : _start(time), _end(time) {}
	
	/// \brief Copy constructor.
	/// \param other Another time interval whose start and end times should be copied.
	TimeInterval(const TimeInterval& other) : _start(other.start()), _end(other.end()) {}

	/// \brief Returns the start time of the interval.
	/// \return The beginning of the time interval.
	TimeTicks start() const { return _start; }
	
	/// \brief Returns the end time of the interval.
	/// \return The time at which the interval end.
	TimeTicks end() const { return _end; }

	/// \brief Sets the start time of the interval.
	/// \param start The new start time.
	void setStart(const TimeTicks& start) { _start = start; }
	
	/// \brief Sets the end time of the interval.
	/// \param end The new end time.
	void setEnd(const TimeTicks& end) { _end = end; }

	/// \brief Checks if this is an empty time interval.
	/// \return \c true if the start time of the interval is behind the end time or if the
	///         end time is negative infinity (TimeNegativeInfinity);
	///         \c false otherwise.
	/// \sa setEmpty()
	bool isEmpty() const { return (end() == TimeNegativeInfinity || start() > end()); }
	
	/// \brief Returns whether this is the infinite time interval.
	/// \return \c true if the start time is negative infinity and the end time of the interval is positive infinity.
	/// \sa setInfinite()
	bool isInfinite() const { return (end() == TimePositiveInfinity && start() == TimeNegativeInfinity); }

	/// \brief Returns the duration of the time interval.
	/// \return The difference between the end and the start time.
	/// \sa setDuration()
	TimeTicks duration() const { return end() - start(); }
	
	/// \brief Sets the duration of the time interval.
	/// \param duration The new duration of the interval.
	///
	/// This method changes the end time of the interval to be 
	/// start() + duration().
	///
	/// \sa duration()
	void setDuration(TimeTicks duration) { setEnd(start() + duration); }

	/// \brief Sets this interval's start time to negative infinity and it's end time to positive infinity.
	/// \sa isInfinite()
	void setInfinite() {
		setStart(TimeNegativeInfinity); 
		setEnd(TimePositiveInfinity); 
	}
	
	/// \brief Sets this interval's start and end time to negative infinity.
	/// \sa isEmpty()
	void setEmpty() { 
		setStart(TimeNegativeInfinity); 
		setEnd(TimeNegativeInfinity);
	}
	
	/// \brief Sets this interval's start and end time to the instant time given.
	/// \param time This value is assigned to both, the start and the end time of the interval.
	void setInstant(TimeTicks time) { 
		setStart(time); 
		setEnd(time);
	}

	/// \brief Compares two intervals for equality.
	/// \param other The interval to compare with.
	/// \return \c true if start and end time of both intervals are equal.
	bool operator==(const TimeInterval& other) const { return (other.start() == start() && other.end() == end()); }
	
	/// \brief Compares two intervals for inequality.
	/// \param other The interval to compare with.
	/// \return \c true if start or end time of both intervals are not equal.
	bool operator!=(const TimeInterval& other) const { return (other.start() != start() || other.end() != end()); }
	
	/// \brief Assignment operator.
	/// \param other The interval to copy.
	/// \return This interval instance.
	TimeInterval& operator=(const TimeInterval& other) { 
		setStart(other.start()); 
		setEnd(other.end()); 
		return *this; 
	}

	/// \brief Returns whether a time lies between start and end time of this interval.
	/// \param time The time to check.
	/// \return \c true if \a time is equal or larger than start() and smaller or equal than end().
	bool contains(TimeTicks time) const { 
		return (start() <= time && time <= end()); 
	}

	/// \brief Intersects this interval with the another one.
	/// \param other Another time interval.
	///
	/// Start and end time of this interval are such that they include the interval \a other as well as \c this interval.
	void intersect(const TimeInterval& other) {
		if(end() < other.start()
			|| start() > other.end()
			|| other.isEmpty()) {
			setEmpty();
		}
		else if(!other.isInfinite()) {
			setStart(max(start(), other.start()));			
			setEnd(min(end(), other.end()));
			OVITO_ASSERT(start() <= end());
		}
	}

private:
	TimeTicks _start, _end;
	
	friend LoadStream& operator>>(LoadStream& stream, TimeInterval& iv);
};

/// \brief Writes a time interval to a binary output stream.
/// \param stream The output stream.
/// \param iv The time interval to write to the output stream \a stream.
/// \return The output stream \a stream.
inline SaveStream& operator<<(SaveStream& stream, const TimeInterval& iv)
{
	return stream << iv.start() << iv.end();
}

/// \brief Reads a time interval from a binary input stream.
/// \param stream The input stream.
/// \param iv Reference to a variable where the parsed data will be stored.
/// \return The input stream \a stream.
inline LoadStream& operator>>(LoadStream& stream, TimeInterval& iv)
{
	stream >> iv._start >> iv._end;
	return stream;
}

};

Q_DECLARE_METATYPE(Core::TimeInterval)
Q_DECLARE_TYPEINFO(Core::TimeInterval, Q_MOVABLE_TYPE);

#endif // __OVITO_TIME_INTERVAL_H
