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

/**
 * \file
 * \brief Contains the definition of the Ovito::Box_2 class template.
 */

#ifndef __OVITO_BOX2_H
#define __OVITO_BOX2_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>
#include "Vector2.h"
#include "Point2.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

/**
 * \brief An axis-aligned box in 2d space.
 *
 * The box is defined by the lower and upper X and Y coordinates (#minc and #maxc fields).
 *
 * The template parameter \a T specifies the data type used for the box coordinates.
 * Two standard instantiations of Box_2 for floating-point and integer coordinates are predefined:
 *
 * \code
 *      typedef Box_2<FloatType>  Box2;
 *      typedef Box_2<int>        Box2I;
 * \endcode
 *
 * A box is considered empty if any of its lower coordinates is greater than the corresponding
 * upper coordinate.
 *
 * \sa Box_3
 * \sa Vector_2, Point_2
 */
template<typename T>
class Box_2
{
public:

	/// The lower XY coordinates of the box.
	Point_2<T> minc;
	/// The upper XY coordinates of the box.
	Point_2<T> maxc;

	/////////////////////////////// Constructors /////////////////////////////////

	/// \brief Constructs an empty box.
	Box_2() : minc(std::numeric_limits<T>::max()), maxc(std::numeric_limits<T>::lowest()) {}

	/// \brief Initializes the box with lower and upper coordinates.
	/// \param lower The corner of the box that specifies the lower boundary coordinates.
	/// \param upper The corner of the box that specifies the upper boundary coordinates.
	Box_2(const Point_2<T>& lower, const Point_2<T>& upper) : minc(lower), maxc(upper) {
		OVITO_ASSERT_MSG(minc.x() <= maxc.x(), "Box_2 constructor", "Lower X coordinate must not be larger than upper X coordinate.");
		OVITO_ASSERT_MSG(minc.y() <= maxc.y(), "Box_2 constructor", "Lower Y coordinate must not be larger than upper Y coordinate.");
	}

	/// \brief Initializes the box with the given coordinates.
	Box_2(T xmin, T ymin, T xmax, T ymax) : minc(Point_2<T>(xmin, ymin)), maxc(Point_2<T>(xmax, ymax)) {
		OVITO_ASSERT(minc.x() <= maxc.x());
		OVITO_ASSERT(minc.y() <= maxc.y());
	}

	/// \brief Crates a square box.
	/// \param center The center of the cubic box.
	/// \param halfEdgeLength One half of the edge length of the square.
	Box_2(const Point_2<T>& center, T halfEdgeLength) {
		OVITO_ASSERT(halfEdgeLength >= 0);
		minc.x() = center.x() - halfEdgeLength;
		minc.y() = center.y() - halfEdgeLength;
		maxc.x() = center.x() + halfEdgeLength;
		maxc.y() = center.y() + halfEdgeLength;
	}

	///////////////////////////////// Attributes /////////////////////////////////

	/// \brief Checks whether this is box is empty.
	///
	/// The box is considered empty if one of the upper boundary coordinates is smaller than
	/// the corresponding lower boundary coordinate.
	Q_DECL_CONSTEXPR bool isEmpty() const {
        return (minc.x() > maxc.x()) || (minc.y() > maxc.y());
	}

	/// \brief Resets the box to the empty state.
	void setEmpty() {
		minc = Point_2<T>(std::numeric_limits<T>::max());
		maxc = Point_2<T>(std::numeric_limits<T>::lowest());
	}

	/// \brief Returns the position of one of the eight corners of the box.
	/// \param i The index of the corner in the range 0 to 3.
	/// \return The coordinates of the i-th corner of the box.
	Point_2<T> operator[](typename Point_2<T>::size_type i) const {
		OVITO_ASSERT_MSG(!isEmpty(), "Box_2::operator[]", "Cannot compute the corners of an empty box.");
		OVITO_ASSERT_MSG(i >= 0 && i < 4, "Box_2::operator[]", "Corner index out of range.");
		const Point_2<T>* const c = &minc;
		OVITO_ASSERT(&c[1] == &maxc);
		return Point_2<T>(c[i&1].x(), c[(i>>1)&1].y());
	}

	/// \brief Computes the width of the box.
	Q_DECL_CONSTEXPR T width() const { return maxc.x() - minc.x(); }

	/// \brief Computes the height of the box.
	Q_DECL_CONSTEXPR T height() const { return maxc.y() - minc.y(); }

	/// \brief Computes the center of the box.
	Q_DECL_CONSTEXPR Point_2<T> center() const {
		return Point_2<T>((minc.x() + maxc.x()) / 2, (minc.y() + maxc.y()) / 2);
	}

	/// \brief Computes the size of the box.
	/// \return The difference between the upper and lower boundary coordinates.
	/// \sa width(), height()
	Q_DECL_CONSTEXPR Vector_2<T> size() const {
		return maxc - minc;
	}

	/// \brief Returns the size of the box in the given dimension.
	/// \param dimension The dimension (0 - 2).
	/// \return The difference between the upper and lower boundary of the box in the given dimension.
	/// \sa size(), width(), height()
	Q_DECL_CONSTEXPR T size(typename Point_2<T>::size_type dimension) const {
		return maxc[dimension] - minc[dimension];
	}

	/////////////////////////////// Classification ///////////////////////////////

	/// \brief Checks whether a point is located inside the box.
	/// \param p The input point.
	/// \return \c true if the point \a p is inside or on the boundaries of the box; \c false if it is outside the box.
	Q_DECL_CONSTEXPR bool contains(const Point_2<T>& p) const {
		return (p.x() >= minc.x() && p.x() <= maxc.x() && p.y() >= minc.y() && p.y() <= maxc.y());
	}

	/// \brief Classifies a point with respect to the box.
	/// \param p The input point.
	/// \param epsilon This threshold is used to test whether the point is on the boundary of the box.
	/// \return -1 if \a p is outside the box; 0 if \a p is on the boundary of the box within the specified tolerance; +1 if inside the box.
	Q_DECL_CONSTEXPR int classifyPoint(const Point_2<T>& p, T epsilon = T(FLOATTYPE_EPSILON)) const {
		return
				(p.x() > maxc.x() + epsilon || p.y() > maxc.y() + epsilon) ||
				(p.x() < minc.x() - epsilon || p.y() < minc.y() - epsilon)
						? -1 :
				((p.x() < maxc.x() - epsilon && p.x() > minc.x() + epsilon && p.y() < maxc.y() - epsilon && p.y() > minc.y() + epsilon)
						? 1 : 0);
	}

	/// \brief Tests if another box is contained in this box.
	/// \param b The other box.
	/// \return \c true if the box \a b is completely inside this box.
	Q_DECL_CONSTEXPR bool containsBox(const Box_2<T>& b) const {
		return (b.minc.x() >= minc.x() && b.maxc.x() <= maxc.x()) &&
			(b.minc.y() >= minc.y() && b.maxc.y() <= maxc.y());
	}

	/// \brief Tests whether the intersection of two boxes is not empty.
	/// \param b The other box.
	/// \return \c true if the box \a b is not completely outside of this box;
	///         \c false if the two boxes do not overlap or are empty.
	Q_DECL_CONSTEXPR bool intersects(const Box_2<T>& b) const {
		return (maxc.x() > b.minc.x() && minc.x() < b.maxc.x() &&
				maxc.y() > b.minc.y() && minc.y() < b.maxc.y() &&
				!isEmpty() && !b.isEmpty());
	}

    //////////////////////////////// Modification ////////////////////////////////

	/// \brief Extends this box to include the given point.
	/// \param p The point which should be included in this box after the method returns.
	/// \sa addPoints(), addBox()
	void addPoint(const Point_2<T>& p) {
		minc.x() = std::min(minc.x(), p.x()); maxc.x() = std::max(maxc.x(), p.x());
		minc.y() = std::min(minc.y(), p.y()); maxc.y() = std::max(maxc.y(), p.y());
	}

	/// \brief Extends this box to include the given point.
	/// \sa addPoints(), addBox()
	void addPoint(T x, T y) {
		minc.x() = std::min(minc.x(), x); maxc.x() = std::max(maxc.x(), x);
		minc.y() = std::min(minc.y(), y); maxc.y() = std::max(maxc.y(), y);
	}

	/// \brief Extends the box to include the given set of points.
	/// \param points Pointer to the first element of an array of points.
	/// \param count The number of points in the array.
	/// \sa addPoint()
	void addPoints(const Point_2<T>* points, std::size_t count) {
		for(; count != 0; count--, points++) {
			minc.x() = std::min(minc.x(), points->X); maxc.x() = std::max(maxc.x(), points->X);
			minc.y() = std::min(minc.y(), points->Y); maxc.y() = std::max(maxc.y(), points->Y);
		}
	}

	/// \brief Extends this box to include the given box.
	/// \param b The other box.
	/// \sa addPoint()
	void addBox(const Box_2& b) {
		minc.x() = std::min(minc.x(), b.minc.x()); maxc.x() = std::max(maxc.x(), b.maxc.x());
		minc.y() = std::min(minc.y(), b.minc.y()); maxc.y() = std::max(maxc.y(), b.maxc.y());
	}

	/// \brief Extends the box to include the given x coordinate.
    void includeX(T x) {
        minc.x() = std::min(minc.x(), x); maxc.x() = std::max(maxc.x(), x);
	}

	/// \brief Extends the box to include the given y coordinate.
    void includeY(T y) {
        minc.y() = std::min(minc.y(), y); maxc.y() = std::max(maxc.y(), y);
	}

    ////////////////////////////////// Utilities /////////////////////////////////

	/// \brief Generates a string representation of this box.
	QString toString() const {
		return "[Min: " + minc.toString() + " Max: " + maxc.toString() + "]";
	}
};

/// \brief Writes a 2d box to a binary output stream.
/// \relates Box_2
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Box_2<T>& b)
{
	return stream << b.minc << b.maxc;
}

/// \brief Reads a 2d box from a binary input stream.
/// \relates Box_2
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Box_2<T>& b)
{
	return stream >> b.minc >> b.maxc;
}

/// \brief Prints a box to a Qt debug stream.
/// \relates Box_2
template<typename T>
inline QDebug operator<<(QDebug dbg, const Box_2<T>& b) {
    dbg.nospace() << "[" << b.minc << "] - [" << b.maxc << "]";
    return dbg.space();
}

/// \brief Prints a box to a text output stream.
/// \relates Box_2
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Box_2<T> &b) {
	return os << '[' << b.minc << "] - [" << b.maxc << ']';
}

/// \brief Writes a box to a Qt data stream.
/// \relates Box_2
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Box_2<T>& b) {
	return stream << b.minc << b.maxc;
}

/// \brief Reads a box from a Qt data stream.
/// \relates Box_2
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Box_2<T>& b) {
	return stream >> b.minc >> b.maxc;
}

/**
 * \brief Template class instance of the Box_2 class used for floating-point calculations based on Point2.
 * \relates Box_2
 */
typedef Box_2<FloatType>	Box2;

/**
 * \brief Template class instance of the Box_2 class used for integer calculations based on Point2I.
 * \relates Box_2
 */
typedef Box_2<int>			Box2I;

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Box2);
Q_DECLARE_METATYPE(Ovito::Box2I);
Q_DECLARE_METATYPE(Ovito::Box2*);
Q_DECLARE_METATYPE(Ovito::Box2I*);
Q_DECLARE_TYPEINFO(Ovito::Box2, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Box2I, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Box2*, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Box2I*, Q_MOVABLE_TYPE);

#endif // __OVITO_BOX2_H
