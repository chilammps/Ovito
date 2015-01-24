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

#ifndef __OVITO_TIME_INTERVAL_H
#define __OVITO_TIME_INTERVAL_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

/** 
 * \brief A point in animation time.
 *
 * One animation time unit is 1/4800 of a second in real time.
 * 
 * Note that this is an integer data type. Times are measured
 * in discrete steps of 1/4800 of a second to avoid rounding errors.
 */
typedef int TimePoint;

/// The number of time ticks per second.
enum { TICKS_PER_SECOND = 4800 };

/// Returns the smallest possible time value.
Q_DECL_CONSTEXPR inline TimePoint TimeNegativeInfinity() {
	return std::numeric_limits<TimePoint>::lowest();
}

/// Returns the largest possible time value.
Q_DECL_CONSTEXPR inline TimePoint TimePositiveInfinity() {
	return std::numeric_limits<TimePoint>::max();
}

/// Converts time tick units to seconds.
Q_DECL_CONSTEXPR inline FloatType TimeToSeconds(TimePoint t) {
	return (FloatType)t / TICKS_PER_SECOND;
}

/// Converts seconds to internal time ticks.
inline TimePoint TimeFromSeconds(FloatType timeInSeconds) {
	return (TimePoint)std::ceil(timeInSeconds * TICKS_PER_SECOND + FloatType(0.5));
}

/**
 * \brief An interval in (animation) time, which is defined by a start and an end time.
 */
class TimeInterval
{
public:

	/// \brief Creates an empty time interval.
	/// 
	/// Both start time and end time are initialized to negative infinity. 
	Q_DECL_CONSTEXPR TimeInterval() : _start(TimeNegativeInfinity()), _end(TimeNegativeInfinity()) {}
	
	/// \brief Initializes the interval with start and end values.
	/// \param start The start time of the time interval.
	/// \param end The end time (including) of the time interval.
	Q_DECL_CONSTEXPR TimeInterval(TimePoint start, TimePoint end) : _start(start), _end(end) {}
	
	/// \brief Initializes the interval to an instant time.
	/// \param time The time where the interval starts and ends.
	Q_DECL_CONSTEXPR TimeInterval(TimePoint time) : _start(time), _end(time) {}

	/// \brief Returns the start time of the interval.
	/// \return The beginning of the time interval.
	Q_DECL_CONSTEXPR TimePoint start() const { return _start; }
	
	/// \brief Returns the end time of the interval.
	/// \return The time at which the interval end.
	Q_DECL_CONSTEXPR TimePoint end() const { return _end; }

	/// \brief Sets the start time of the interval.
	/// \param start The new start time.
	void setStart(TimePoint start) { _start = start; }
	
	/// \brief Sets the end time of the interval.
	/// \param end The new end time.
	void setEnd(TimePoint end) { _end = end; }

	/// \brief Checks if this is an empty time interval.
	/// \return \c true if the start time of the interval is behind the end time or if the
	///         end time is negative infinity (TimeNegativeInfinity);
	///         \c false otherwise.
	/// \sa setEmpty()
	Q_DECL_CONSTEXPR bool isEmpty() const { return (end() == TimeNegativeInfinity() || start() > end()); }
	
	/// \brief Returns whether this is the infinite time interval.
	/// \return \c true if the start time is negative infinity and the end time of the interval is positive infinity.
	/// \sa setInfinite()
	Q_DECL_CONSTEXPR bool isInfinite() const { return (end() == TimePositiveInfinity() && start() == TimeNegativeInfinity()); }

	/// \brief Returns the duration of the time interval.
	/// \return The difference between the end and the start time.
	/// \sa setDuration()
	Q_DECL_CONSTEXPR TimePoint duration() const { return end() - start(); }
	
	/// \brief Sets the duration of the time interval.
	/// \param duration The new duration of the interval.
	///
	/// This method changes the end time of the interval to be 
	/// start() + duration().
	///
	/// \sa duration()
	void setDuration(TimePoint duration) { setEnd(start() + duration); }

	/// \brief Sets this interval's start time to negative infinity and it's end time to positive infinity.
	/// \sa isInfinite()
	void setInfinite() {
		setStart(TimeNegativeInfinity());
		setEnd(TimePositiveInfinity());
	}
	
	/// \brief Sets this interval's start and end time to negative infinity.
	/// \sa isEmpty()
	void setEmpty() { 
		setStart(TimeNegativeInfinity());
		setEnd(TimeNegativeInfinity());
	}
	
	/// \brief Sets this interval's start and end time to the instant time given.
	/// \param time This value is assigned to both, the start and the end time of the interval.
	void setInstant(TimePoint time) {
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
	Q_DECL_CONSTEXPR bool contains(TimePoint time) const {
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
			setStart(std::max(start(), other.start()));
			setEnd(std::min(end(), other.end()));
			OVITO_ASSERT(start() <= end());
		}
	}

	/// Return the infinite time interval that contains all time values.
	static Q_DECL_CONSTEXPR TimeInterval infinite() { return TimeInterval(TimeNegativeInfinity(), TimePositiveInfinity()); }

	/// Return the empty time interval that contains no time values.
	static Q_DECL_CONSTEXPR TimeInterval empty() { return TimeInterval(TimeNegativeInfinity()); }

private:

	TimePoint _start, _end;

	friend LoadStream& operator>>(LoadStream& stream, TimeInterval& iv);
};

/// \brief Writes a time interval to a binary output stream.
/// \param stream The output stream.
/// \param iv The time interval to write to the output stream \a stream.
/// \return The output stream \a stream.
/// \relates TimeInterval
inline SaveStream& operator<<(SaveStream& stream, const TimeInterval& iv)
{
	return stream << iv.start() << iv.end();
}

/// \brief Reads a time interval from a binary input stream.
/// \param stream The input stream.
/// \param iv Reference to a variable where the parsed data will be stored.
/// \return The input stream \a stream.
/// \relates TimeInterval
inline LoadStream& operator>>(LoadStream& stream, TimeInterval& iv)
{
	stream >> iv._start >> iv._end;
	return stream;
}

/// \brief Writes a time interval to the debug stream.
/// \relates TimeInterval
inline QDebug operator<<(QDebug stream, const TimeInterval& iv)
{
	stream.nospace() << "[" << iv.start() << ", " << iv.end() << "]";
	return stream.space();
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::TimeInterval);
Q_DECLARE_TYPEINFO(Ovito::TimeInterval, Q_MOVABLE_TYPE);

#endif // __OVITO_TIME_INTERVAL_H
