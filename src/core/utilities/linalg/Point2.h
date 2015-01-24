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
 * \brief Contains the definition of the Ovito::Point_2 class template.
 */

#ifndef __OVITO_POINT2_H
#define __OVITO_POINT2_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>
#include "Vector3.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

/**
 * \brief A point in 2d space.
 *
 * Point_2 represents a point in two-dimensional space with coordinates x and y.
 * Note that there exists a corresponding class Vector_2, which represents a *vector* in two-dimensional space.
 *
 * The template parameter \a T specifies the data type of the points's components.
 * Two standard instantiations of Point_2 for floating-point and integer coordinates are predefined:
 *
 * \code
 *      typedef Point_2<FloatType>  Point2;
 *      typedef Point_2<int>        Point2I;
 * \endcode
 *
 * Note that the default constructor does not initialize the components of the point for performance reasons.
 * The nested type Origin can be used to construct the point (0,0):
 *
 * \code
 *      Point2 p = Point2::Origin()
 * \endcode
 *
 * Origin can also be used to convert between points and vectors:
 *
 * \code
 *      Vector2 v0(0.5, 1.5);
 *      Point2 p1 = Point2::Origin() + v0;  // Vector to point conversion
 *      Point2 p2 = p1 + v0;                // Adding a vector to a point
 *      Vector2 delta = p2 - p1;            // Vector connecting two points
 *      Vector2 v1 = p2 - Point2::Origin(); // Point to vector conversion
 * \endcode
 *
 * Point_2 derives from std::array<T,2>. Thus, the point coordinates can be accessed via indices, but also via names:
 *
 * \code
 *      p[1]  = 10.0f;
 *      p.y() = 10.0f;
 * \endcode
 *
 * \sa Vector_2, Point_3
 */
template<typename T>
class Point_2 : public std::array<T, 2>
{
public:

	/// An empty type that denotes the point (0,0).
	struct Origin {};

	using typename std::array<T, 2>::size_type;
	using typename std::array<T, 2>::difference_type;
	using typename std::array<T, 2>::value_type;
	using typename std::array<T, 2>::iterator;
	using typename std::array<T, 2>::const_iterator;

	/////////////////////////////// Constructors /////////////////////////////////

	/// Constructs a point without initializing its components. The components will have an undefined value!
	Point_2() {}

	/// Constructs a point with \c x and \c y components initialized to the given value.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR explicit Point_2(T val) : std::array<T, 2>{{val,val}} {}
#else
	explicit Point_2(T val) { this->fill(val); }
#endif

	/// Initializes the coordinates of the point with the given values.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Point_2(T x, T y) : std::array<T, 2>{{x, y}} {}
#else
	Point_2(T x, T y) { this->x() = x; this->y() = y; }
#endif

	/// Initializes the point to the origin. All coordinates are set to zero.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Point_2(Origin) : std::array<T, 2>{{T(0), T(0)}} {}
#else
	Point_2(Origin) { this->fill(T(0)); }
#endif

	/// Initializes the point from an array coordinates.
	Q_DECL_CONSTEXPR explicit Point_2(const std::array<T, 2>& a) : std::array<T, 2>(a) {}

	/// Casts the point to another coordinate type \a U.
	template<typename U>
	Q_DECL_CONSTEXPR explicit operator Point_2<U>() const { return Point_2<U>(static_cast<U>(x()), static_cast<U>(y())); }

	///////////////////////////// Assignment operators ///////////////////////////

	/// Adds a vector to this point.
	Point_2& operator+=(const Vector_2<T>& v) { x() += v.x(); y() += v.y(); return *this; }

	/// Subtracts a vector from this point.
	Point_2& operator-=(const Vector_2<T>& v) { x() -= v.x(); y() -= v.y(); return *this; }

	/// Multiplies all coordinates of the point with a scalar value.
	Point_2& operator*=(T s) { x() *= s; y() *= s; return *this; }

	/// Divides all coordinates of the point by a scalar value.
	Point_2& operator/=(T s) { x() /= s; y() /= s; return *this; }

	/// Sets all coordinates of the point to zero.
	Point_2& operator=(Origin) { this->fill(T(0)); return *this; }

	/// Converts a point to a vector.
	Vector_2<T> operator-(Origin) const {
		return Vector_2<T>(*this);
	}

	//////////////////////////// Component access //////////////////////////

	/// \brief Returns the value of the X coordinate of this point.
	Q_DECL_CONSTEXPR T x() const { return (*this)[0]; }

	/// \brief Returns the value of the Y coordinate of this point.
	Q_DECL_CONSTEXPR T y() const { return (*this)[1]; }

	/// \brief Returns a reference to the X coordinate of this point.
	T& x() { return (*this)[0]; }

	/// \brief Returns a reference to the Y coordinate of this point.
	T& y() { return (*this)[1]; }

	////////////////////////////////// Comparison ////////////////////////////////

	/// \brief Compares two points for exact equality.
	/// \return \c true if all coordinates are equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const Point_2& p) const { return (p.x()==x()) && (p.y()==y()); }

	/// \brief Compares two points for inequality.
	/// \return \c true if any of the coordinates is not equal; \c false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const Point_2& p) const { return (p.x()!=x()) || (p.y()!=y()); }

	/// \brief Tests whether this point is at the origin, i.e. all of its coordinates are zero.
	/// \return \c true if all of the coordinates are exactly zero; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(Origin) const { return (x()==T(0)) && (y()==T(0)); }

	/// \brief Tests whether the point is not at the origin, i.e. any of the coordinates is nonzero.
	/// \return \c true if any of the coordinates is nonzero; \c false if this is the origin point.
	Q_DECL_CONSTEXPR bool operator!=(Origin) const { return (x()!=T(0)) || (y()!=T(0)); }

	/// \brief Tests if two points are equal within a specified tolerance.
	/// \param p The second point.
	/// \param tolerance A non-negative threshold for the equality test. The two points are considered equal if
	///        the absolute differences in their X and Y coordinates are all smaller than this tolerance.
	/// \return \c true if this point is equal to the second point within the specified tolerance; \c false otherwise.
	Q_DECL_CONSTEXPR bool equals(const Point_2& p, T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(p.x() - x()) <= tolerance && std::abs(p.y() - y()) <= tolerance;
	}

	/// \brief Tests whether this point is at the origin within a specified tolerance.
	/// \param tolerance A non-negative threshold.
	/// \return \c true if the absolute values of the point's coordinates are all below \a tolerance.
	Q_DECL_CONSTEXPR bool isOrigin(T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(x()) <= tolerance && std::abs(y()) <= tolerance;
	}

	///////////////////////////////// Utilities ////////////////////////////////

	/// \brief Returns the index of the coordinate with the maximum value.
	Q_DECL_CONSTEXPR inline std::size_t maxComponent() const {
	    return (x() >= y()) ? 0 : 1;
	}

	/// \brief Returns the index of the coordinate with the minimum value.
	Q_DECL_CONSTEXPR inline std::size_t minComponent() const {
	    return (x() <= y()) ? 0 : 1;
	}

	/// \brief Produces a string representation of this point.
	/// \return A string that contains the coordinate of the point.
	QString toString() const {
		return QString("(%1 %2)").arg(x()).arg(y());
	}
};

/// \brief Computes the sum of a point and a vector.
/// \relates Point_2
template<typename T>
Q_DECL_CONSTEXPR Point_2<T> operator+(const Point_2<T>& a, const Vector_2<T>& b) {
	return Point_2<T>{ a.x() + b.x(), a.y() + b.y() };
}

/// \brief Computes the sum of a vector and a point.
/// \relates Point_2
template<typename T>
Q_DECL_CONSTEXPR Point_2<T> operator+(const Vector_2<T>& a, const Point_2<T>& b) {
	return b + a;
}

/// \brief Computes the vector connecting two points.
/// \relates Point_2
template<typename T>
Q_DECL_CONSTEXPR Vector_2<T> operator-(const Point_2<T>& a, const Point_2<T>& b) {
	return Vector_2<T>{ a.x() - b.x(), a.y() - b.y() };
}

/// \brief Subtracts a vector from a point.
/// \relates Point_2
template<typename T>
Q_DECL_CONSTEXPR Point_2<T> operator-(const Point_2<T>& a, const Vector_2<T>& b) {
	return Point_2<T>{ a.x() - b.x(), a.y() - b.y() };
}

/// \brief Computes the component-wise product of a point and a scalar value.
/// \relates Point_2
template<typename T>
Q_DECL_CONSTEXPR Point_2<T> operator*(const Point_2<T>& a, T s) {
	return Point_2<T>{ a.x() * s, a.y() * s };
}

/// \brief Computes the component-wise product of a point and a scalar value.
/// \relates Point_2
template<typename T>
Q_DECL_CONSTEXPR Point_2<T> operator*(T s, const Point_2<T>& a) {
	return Point_2<T>{ a.x() * s, a.y() * s };
}

/// \brief Computes the component-wise division of a vector by a scalar value.
/// \relates Point_2
template<typename T>
Q_DECL_CONSTEXPR Point_2<T> operator/(const Point_2<T>& a, T s) {
	return Point_2<T>{ a.x() / s, a.y() / s };
}

/// \brief Writes a point to a text output stream.
/// \relates Point_2
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Point_2<T>& v) {
	return os << "(" << v.x() << ", " << v.y() << ")";
}

/// \brief Writes a point to a Qt debug stream.
/// \relates Point_2
template<typename T>
inline QDebug operator<<(QDebug dbg, const Point_2<T>& v) {
    dbg.nospace() << "(" << v.x() << ", " << v.y() << ")";
    return dbg.space();
}

/// \brief Writes a point to a binary output stream.
/// \relates Point_2
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Point_2<T>& v) {
	return stream << v.x() << v.y();
}

/// \brief Reads a point from a binary input stream.
/// \relates Point_2
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Point_2<T>& v) {
	return stream >> v.x() >> v.y();
}

/// \brief Writes a point to a Qt data stream.
/// \relates Point_2
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Point_2<T>& v) {
	return stream << v.x() << v.y();
}

/// \brief Reads a point from a Qt data stream.
/// \relates Point_2
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Point_2<T>& v) {
	return stream >> v.x() >> v.y();
}


/**
 * \brief Instantiation of the Point_2 class template with the default floating-point type.
 * \relates Point_2
 */
typedef Point_2<FloatType>		Point2;

/**
 * \brief Instantiation of the Point_2 class template with the default integer type.
 * \relates Point_2
 */
typedef Point_2<int>			Point2I;

// Type-specific OpenGL functions:
inline void glVertex(const Point_2<GLdouble>& v) { glVertex2dv(v.data()); }
inline void glVertex(const Point_2<GLfloat>& v) { glVertex2fv(v.data()); }

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Point2);
Q_DECLARE_METATYPE(Ovito::Point2I);
Q_DECLARE_METATYPE(Ovito::Point2*);
Q_DECLARE_METATYPE(Ovito::Point2I*);
Q_DECLARE_TYPEINFO(Ovito::Point2, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Point2I, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Point2*, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Point2I*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_POINT2_H
