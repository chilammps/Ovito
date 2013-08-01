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
 * \file Box2.h
 * \brief Contains the definition of the Ovito::Box_2 template class.
 */

#ifndef __OVITO_BOX2_H
#define __OVITO_BOX2_H

#include <base/Base.h>
#include "Vector2.h"
#include "Point2.h"

namespace Ovito {

/**
 * \brief A bounding box in 2D space.
 *
 * This class stores an axis-aligned box in 2D space.
 * It is defined by minimum and maximum coordinates in X and Y direction.
 */
template<typename T>
class Box_2
{
public:
	/// The coordinates of the lower left corner.
	Point_2<T> minc;
	/// The coordinates of the upper right corner.
	Point_2<T> maxc;

	/////////////////////////////// Constructors /////////////////////////////////

	/// \brief Creates an empty box.
	Box_2() : minc(std::numeric_limits<T>::max()), maxc(-std::numeric_limits<T>::max()) {}

	/// \brief Initializes the box with the minimum and maximum coordinates.
	/// \param minCorner A point that specifies the corner with minimum coordinates of the box.
	/// \param maxCorner A point that specifies the corner with maximum coordinates of the box.
	Box_2(const Point_2<T>& minCorner, const Point_2<T>& maxCorner) : minc(minCorner), maxc(maxCorner) {
		OVITO_ASSERT_MSG(minc.x() <= maxc.x(), "Box_2 constructor", "X component of the minimum corner point must not be larger than the maximum corner point.");
		OVITO_ASSERT_MSG(minc.y() <= maxc.y(), "Box_2 constructor", "Y component of the minimum corner point must not be larger than the maximum corner point.");
	}

	/// \brief Initializes the box with the given coordinates.
	Box_2(T xmin, T ymin, T xmax, T ymax) : minc(Point_2<T>(xmin, ymin)), maxc(Point_2<T>(xmax, ymax)) {
		OVITO_ASSERT(minc.x() <= maxc.x());
		OVITO_ASSERT(minc.y() <= maxc.y());
	}

	///////////////////////////////// Attributes /////////////////////////////////

	/// \brief Checks whether this is an empty box.
	///
	/// The box is considered empty when one of the maximum corner coordinates is less
	/// then the minimum corner coordinate.
	/// \return true if this box is empty; false otherwise.
	Q_DECL_CONSTEXPR bool isEmpty() const {
        return (minc.x() > maxc.x()) || (minc.y() > maxc.y());
	}

	/// \brief Resets the box to the empty state.
	void setEmpty() {
		minc = Point_2<T>( std::numeric_limits<T>::max());
		maxc = Point_2<T>(-std::numeric_limits<T>::max());
	}

	/// \brief Returns the position of one of the four corners of the box corner.
	/// \param i The index of the corner (0 - 3).
	/// \return The coordinate of the i-th corner of the box.
	Point_2<T> operator[](int i) const {
		switch(i) {
			case 0: return Point_2<T>(minc.x(), minc.y());
			case 1: return Point_2<T>(maxc.x(), minc.y());
			case 2: return Point_2<T>(maxc.x(), maxc.y());
			case 3: return Point_2<T>(minc.x(), maxc.y());
			default:
				OVITO_ASSERT_MSG(false, "Box2::operator[]", "Corner index out of range.");
				throw std::invalid_argument("Corner index out of range.");
				return typename Point_2<T>::Origin();
		}
	}

	/// \brief Computes the width of the box.
	Q_DECL_CONSTEXPR T width() const { return maxc.x() - minc.x(); }

	/// \brief Computes the height of the box.
	Q_DECL_CONSTEXPR T height() const { return maxc.y() - minc.y(); }

	/// \brief Computes the center of the box.
	/// \return The center of the box.
	Q_DECL_CONSTEXPR Point_2<T> center() const {
		return Point_2<T>((minc.x() + maxc.x()) / 2, (minc.y() + maxc.y()) / 2);
	}

	/// \brief Computes the size of the box.
	/// \return The difference between the maximum and minimum corner.
	/// \sa width(), height()
	Q_DECL_CONSTEXPR Vector_2<T> size() const {
		return maxc - minc;
	}

	/// \brief Returns the size of the box in the given dimension.
	/// \param dimension The index of the dimension (0 or 1).
	/// \sa size(), width(), height()
	Q_DECL_CONSTEXPR T size(typename Point_2<T>::size_type dimension) const {
		return maxc[dimension] - minc[dimension];
	}

	/////////////////////////////// Classification ///////////////////////////////

	/// \brief Checks whether a point is inside the box.
	/// \param p The point to test.
	/// \return true if the given point is inside or on the edge of the bounding box; false if it is completely outside the box.
	Q_DECL_CONSTEXPR bool contains(const Point_2<T>& p) const {
		return (p.x() >= minc.x() && p.x() <= maxc.x() && p.y() >= minc.y() && p.y() <= maxc.y());
	}

	/// \brief Classifies the given point with respect to the box.
	///
	/// Returns -1 if the point is outside of the box.
	/// Returns 0 if the point is on the boundary of the box within the given tolerance.
	/// Returns +1 if the point is inside of the box.
	Q_DECL_CONSTEXPR int classifyPoint(const Point_2<T>& p, T epsilon = T(FLOATTYPE_EPSILON)) const {
		return
				(p.x() > maxc.x() + epsilon || p.y() > maxc.y() + epsilon) ||
				(p.x() < minc.x() - epsilon || p.y() < minc.y() - epsilon)
						? -1 :
				((p.x() < maxc.x() - epsilon && p.x() > minc.x() + epsilon && p.y() < maxc.y() - epsilon && p.y() > minc.y() + epsilon)
						? 1 : 0);
	}

	/// \brief Checks whether another box is contained in this box.
	/// \return true if the given box is completely inside the bounding box.
	Q_DECL_CONSTEXPR bool containsBox(const Box_2<T>& b) const {
		return (b.minc.x() >= minc.x() && b.maxc.x() <= maxc.x()) &&
			(b.minc.y() >= minc.y() && b.maxc.y() <= maxc.y());
	}

	/// \brief Checks whether the intersection of two boxes is not empty.
	/// \return true if the given box is not completely outside of this box.
	Q_DECL_CONSTEXPR bool intersects(const Box_2<T>& b) const {
		return (maxc.x() > b.minc.x() && minc.x() < b.maxc.x() &&
				maxc.y() > b.minc.y() && minc.y() < b.maxc.y() &&
				!isEmpty() && !b.isEmpty());
	}

    //////////////////////////////// Modification ////////////////////////////////

	/// \brief Enlarges the box to include the given point.
	/// \sa addPoints(), addBox()
	void addPoint(const Point_2<T>& p) {
		minc.x() = std::min(minc.x(), p.x()); maxc.x() = std::max(maxc.x(), p.x());
		minc.y() = std::min(minc.y(), p.y()); maxc.y() = std::max(maxc.y(), p.y());
	}

	/// \brief Enlarges the box to include the given point.
	/// \sa addPoints(), addBox()
	void addPoint(T x, T y) {
		minc.x() = std::min(minc.x(), x); maxc.x() = std::max(maxc.x(), x);
		minc.y() = std::min(minc.y(), y); maxc.y() = std::max(maxc.y(), y);
	}

	/// \brief Enlarges the box to include the given points.
	/// \param points Pointer to the first element of an array of points.
	/// \param count The number of points in the array.
	/// \sa addPoint()
	void addPoints(const Point_2<T>* points, std::size_t count) {
		for(; count != 0; count--, points++) {
			minc.x() = std::min(minc.x(), points->X); maxc.x() = std::max(maxc.x(), points->X);
			minc.y() = std::min(minc.y(), points->Y); maxc.y() = std::max(maxc.y(), points->Y);
		}
	}

	/// \brief Enlarges this box to include the given box.
	/// \sa addPoint()
	void addBox(const Box_2& b) {
		minc.x() = std::min(minc.x(), b.minc.x()); maxc.x() = std::max(maxc.x(), b.maxc.x());
		minc.y() = std::min(minc.y(), b.minc.y()); maxc.y() = std::max(maxc.y(), b.maxc.y());
	}

	/// \brief Enlarges the box to include the given x coordinate.
    void includeX(T x) {
        minc.x() = std::min(minc.x(), x); maxc.x() = std::max(maxc.x(), x);
	}

	/// \brief Enlarges the box to include the given y coordinate.
    void includeY(T y) {
        minc.y() = std::min(minc.y(), y); maxc.y() = std::max(maxc.y(), y);
	}

    ////////////////////////////////// Utilities /////////////////////////////////

	/// \brief Returns a string representation of this box.
	QString toString() const {
		return "[Min: " + minc.toString() + " Max: " + maxc.toString() + "]";
	}
};

/// \brief Writes a 2d box to a binary output stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Box_2<T>& b)
{
	return stream << b.minc << b.maxc;
}

/// \brief Reads a 2d box from a binary input stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Box_2<T>& b)
{
	return stream >> b.minc >> b.maxc;
}

/// \brief Writes the box to the Qt debug stream.
template<typename T>
inline QDebug operator<<(QDebug dbg, const Box_2<T>& b) {
    dbg.nospace() << "[" << b.minc << "] - [" << b.maxc << "]";
    return dbg.space();
}

/// \brief Prints the box to a text output stream.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Box_2<T> &b) {
	return os << '[' << b.minc << "] - [" << b.maxc << ']';
}

/**
 * \fn typedef Box2
 * \brief Template class instance of the Box_2 class used for floating-point calculations based on Point2.
 */
typedef Box_2<FloatType>	Box2;

/**
 * \fn typedef Box2I
 * \brief Template class instance of the Box_2 class used for integer calculations based on Point2I.
 */
typedef Box_2<int>			Box2I;

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Box2)
Q_DECLARE_METATYPE(Ovito::Box2I)
Q_DECLARE_TYPEINFO(Ovito::Box2, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Box2I, Q_MOVABLE_TYPE);

#endif // __OVITO_BOX2_H
