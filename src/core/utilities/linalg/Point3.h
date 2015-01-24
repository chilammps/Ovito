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
 * \brief Contains the definition of the Ovito::Point_3 class template.
 */

#ifndef __OVITO_POINT3_H
#define __OVITO_POINT3_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>
#include "Vector3.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

/**
 * \brief A point in 3d space.
 *
 * Point_3 represents a point in three-dimensional space with three coordinates x,y, and z.
 * Note that there exists a corresponding class Vector_3, which represents a *vector* in three-dimensional space.
 *
 * The template parameter \a T specifies the data type of the points's components.
 * Two standard instantiations of Point_3 for floating-point and integer coordinates are predefined:
 *
 * \code
 *      typedef Point_3<FloatType>  Point3;
 *      typedef Point_3<int>        Point3I;
 * \endcode
 *
 * Point_3 derives from std::array<T,3>. Thus, the point coordinates can be accessed via indices, but also via names:
 *
 * \code
 *      p[1]  = 10.0f;
 *      p.y() = 10.0f;
 * \endcode
 *
 * Note that the default constructor does not initialize the components of the point for performance reasons.
 * The nested type Origin can be used to construct the point (0,0,0):
 *
 * \code
 *      Point3 p = Point3::Origin()
 * \endcode
 *
 * Origin can also be used to convert between points and vectors:
 *
 * \code
 *      Vector3 v0(1, 2, 3);
 *      Point3 p1 = Point3::Origin() + v0;   // Vector to point conversion
 *      Point3 p2 = p1 + v0;                 // Adding a vector to a point
 *      Vector3 delta = p2 - p1;             // Vector connecting two points
 *      Vector3 v1 = p2 - Point3::Origin();  // Point to vector conversion
 *      FloatType r = (p2 - p1).length();    // Distance between two points
 * \endcode
 *
 * Note that points and vectors behave differently under affine transformations:
 *
 * \code
 *      AffineTransformation tm = AffineTransformation::rotationZ(angle) * AffineTransformation::translation(t);
 *      Point3  p = tm *  Point3(1,2,3);    // Translates and rotates the point (1,2,3).
 *      Vector3 v = tm * Vector3(1,2,3);    // Only rotates the vector (1,2,3). Translation doesn't change a vector.
 * \endcode
 *
 * \sa Vector_3, Point_2
 */
template<typename T>
class Point_3 : public std::array<T, 3>
{
public:

	/// An empty type that denotes the point (0,0,0).
	struct Origin {};

	using typename std::array<T, 3>::size_type;
	using typename std::array<T, 3>::difference_type;
	using typename std::array<T, 3>::value_type;
	using typename std::array<T, 3>::iterator;
	using typename std::array<T, 3>::const_iterator;

	/////////////////////////////// Constructors /////////////////////////////////

	/// Constructs a point without initializing its components. The components will have an undefined value!
	Point_3() {}

	/// Constructs a point with all three components initialized to the given value.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR explicit Point_3(T val) : std::array<T, 3>{{val,val,val}} {}
#else
	explicit Point_3(T val) { this->fill(val); }
#endif

	/// Initializes the coordinates of the point with the given values.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Point_3(T x, T y, T z) : std::array<T, 3>{{x, y, z}} {}
#else
	Point_3(T x, T y, T z) { this->x() = x; this->y() = y; this->z() = z; }
#endif

	/// Initializes the point to the origin. All coordinates are set to zero.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Point_3(Origin) : std::array<T, 3>{{T(0), T(0), T(0)}} {}
#else
	Point_3(Origin) { this->fill(T(0)); }
#endif

	/// Initializes the point from an array of three coordinates.
	Q_DECL_CONSTEXPR explicit Point_3(const std::array<T, 3>& a) : std::array<T, 3>(a) {}

	/// Casts the point to another coordinate type \a U.
	template<typename U>
	Q_DECL_CONSTEXPR explicit operator Point_3<U>() const { return Point_3<U>(static_cast<U>(x()), static_cast<U>(y()), static_cast<U>(z())); }

	///////////////////////////// Assignment operators ///////////////////////////

	/// Adds a vector to this point.
	Point_3& operator+=(const Vector_3<T>& v) { x() += v.x(); y() += v.y(); z() += v.z(); return *this; }

	/// Subtracts a vector from this point.
	Point_3& operator-=(const Vector_3<T>& v) { x() -= v.x(); y() -= v.y(); z() -= v.z(); return *this; }

	/// Multiplies all coordinates of the point with a scalar value.
	Point_3& operator*=(T s) { x() *= s; y() *= s; z() *= s; return *this; }

	/// Divides all coordinates of the point by a scalar value.
	Point_3& operator/=(T s) { x() /= s; y() /= s; z() /= s; return *this; }

	/// Sets all coordinates of the point to zero.
	Point_3& operator=(Origin) { z() = y() = x() = T(0); return *this; }

	/// Converts a point to a vector.
	const Vector_3<T>& operator-(Origin) const {
		// Implement this as a simple cast to Vector3 for best performance.
		OVITO_STATIC_ASSERT(sizeof(Vector_3<T>) == sizeof(Point_3<T>));
		return reinterpret_cast<const Vector_3<T>&>(*this);
	}

	//////////////////////////// Component access //////////////////////////

	/// \brief Returns the value of the X coordinate of this point.
	Q_DECL_CONSTEXPR T x() const { return (*this)[0]; }

	/// \brief Returns the value of the Y coordinate of this point.
	Q_DECL_CONSTEXPR T y() const { return (*this)[1]; }

	/// \brief Returns the value of the Z coordinate of this point.
	Q_DECL_CONSTEXPR T z() const { return (*this)[2]; }

	/// \brief Returns a reference to the X coordinate of this point.
	T& x() { return (*this)[0]; }

	/// \brief Returns a reference to the Y coordinate of this point.
	T& y() { return (*this)[1]; }

	/// \brief Returns a reference to the Z coordinate of this point.
	T& z() { return (*this)[2]; }

	////////////////////////////////// Comparison ////////////////////////////////

	/// \brief Compares two points for exact equality.
	/// \return \c true if all coordinates are equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const Point_3& p) const { return (p.x()==x()) && (p.y()==y()) && (p.z()==z()); }

	/// \brief Compares two points for inequality.
	/// \return \c true if any of the coordinates is not equal; \c false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const Point_3& p) const { return (p.x()!=x()) || (p.y()!=y()) || (p.z()!=z()); }

	/// \brief Tests whether this point is at the origin, i.e. all of its coordinates are zero.
	/// \return \c true if all of the coordinates are exactly zero; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(Origin) const { return (x()==T(0)) && (y()==T(0)) && (z()==T(0)); }

	/// \brief Tests whether the point is not at the origin, i.e. any of the coordinates is nonzero.
	/// \return \c true if any of the coordinates is nonzero; \c false if this is the origin point.
	Q_DECL_CONSTEXPR bool operator!=(Origin) const { return (x()!=T(0)) || (y()!=T(0)) || (z()!=T(0)); }

	/// \brief Tests if two points are equal within a specified tolerance.
	/// \param p The second point.
	/// \param tolerance A non-negative threshold for the equality test. The two points are considered equal if
	///        the absolute differences in their X, Y, and Z coordinates are all smaller than this tolerance.
	/// \return \c true if this point is equal to the second point within the specified tolerance; \c false otherwise.
	Q_DECL_CONSTEXPR bool equals(const Point_3& p, T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(p.x() - x()) <= tolerance && std::abs(p.y() - y()) <= tolerance && std::abs(p.z() - z()) <= tolerance;
	}

	/// \brief Tests whether this point is at the origin within a specified tolerance.
	/// \param tolerance A non-negative threshold.
	/// \return \c true if the absolute values of the point's coordinates are all below \a tolerance.
	Q_DECL_CONSTEXPR bool isOrigin(T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(x()) <= tolerance && std::abs(y()) <= tolerance && std::abs(z()) <= tolerance;
	}

	///////////////////////////////// Utilities ////////////////////////////////

	/// \brief Returns the index of the coordinate with the maximum value.
	Q_DECL_CONSTEXPR inline size_type maxComponent() const {
	    return ((x() >= y()) ? ((x() >= z()) ? 0 : 2) : ((y() >= z()) ? 1 : 2));
	}

	/// \brief Returns the index of the coordinate with the minimum value.
	Q_DECL_CONSTEXPR inline size_type minComponent() const {
	    return ((x() <= y()) ? ((x() <= z()) ? 0 : 2) : ((y() <= z()) ? 1 : 2));
	}

	/// \brief Generates a string representation of this point of the form (x y z).
	QString toString() const {
		return QString("(%1 %2 %3)").arg(x()).arg(y()).arg(z());
	}
};

/// \brief Computes the sum of a point and a vector.
/// \relates Point_3
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator+(const Point_3<T>& a, const Vector_3<T>& b) {
	return Point_3<T>{ a.x() + b.x(), a.y() + b.y(), a.z() + b.z() };
}

/// \brief Converts a vector to a point.
/// \relates Point_3
template<typename T>
Q_DECL_CONSTEXPR const Point_3<T>& operator+(typename Point_3<T>::Origin, const Vector_3<T>& b) {
	// Use cast for best performance.
	OVITO_STATIC_ASSERT(sizeof(Vector_3<T>) == sizeof(Point_3<T>));
	return reinterpret_cast<const Point_3<T>&>(b);
}

/// \brief Computes the sum of a vector and a point.
/// \relates Point_3
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator+(const Vector_3<T>& a, const Point_3<T>& b) {
	return b + a;
}

/// \brief Subtracts a vector from a point.
/// \relates Point_3
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator-(const Point_3<T>& a, const Vector_3<T>& b) {
	return Point_3<T>{ a.x() - b.x(), a.y() - b.y(), a.z() - b.z() };
}

/// \brief Computes the vector connecting to two points.
/// \relates Point_3
template<typename T>
Q_DECL_CONSTEXPR Vector_3<T> operator-(const Point_3<T>& a, const Point_3<T>& b) {
	return Vector_3<T>{ a.x() - b.x(), a.y() - b.y(), a.z() - b.z() };
}

/// \brief Computes the component-wise product of a point and a scalar value.
/// \relates Point_3
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator*(const Point_3<T>& a, T s) {
	return Point_3<T>{ a.x() * s, a.y() * s, a.z() * s };
}

/// \brief Computes the component-wise product of a point and a scalar value.
/// \relates Point_3
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator*(T s, const Point_3<T>& a) {
	return Point_3<T>{ a.x() * s, a.y() * s, a.z() * s };
}

/// \brief Computes the component-wise division of a point by a scalar value.
/// \relates Point_3
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator/(const Point_3<T>& a, T s) {
	return Point_3<T>{ a.x() / s, a.y() / s, a.z() / s };
}

/// \brief Writes a point to a text output stream.
/// \relates Point_3
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Point_3<T>& v) {
	return os << "(" << v.x() << ", " << v.y()  << ", " << v.z() << ")";
}

/// \brief Writes a point to a Qt debug stream.
/// \relates Point_3
template<typename T>
inline QDebug operator<<(QDebug dbg, const Point_3<T>& v) {
    dbg.nospace() << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
    return dbg.space();
}

/// \brief Writes a point to a binary output stream.
/// \relates Point_3
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Point_3<T>& v) {
	return stream << v.x() << v.y() << v.z();
}

/// \brief Reads a point from a binary input stream.
/// \relates Point_3
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Point_3<T>& v) {
	return stream >> v.x() >> v.y() >> v.z();
}

/// \brief Writes a point to a Qt data stream.
/// \relates Point_3
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Point_3<T>& v) {
	return stream << v.x() << v.y() << v.z();
}

/// \brief Reads a point from a Qt data stream.
/// \relates Point_3
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Point_3<T>& v) {
	return stream >> v.x() >> v.y() >> v.z();
}

/**
 * \brief Instantiation of the Point_3 class template with the default floating-point type.
 * \relates Point_3
 */
typedef Point_3<FloatType>		Point3;

/**
 * \brief Instantiation of the Point_3 class template with the default integer type.
 * \relates Point_3
 */
typedef Point_3<int>			Point3I;

inline void glVertex(const Point_3<GLdouble>& v) { glVertex3dv(v.data()); }
inline void glVertex(const Point_3<GLfloat>& v) { glVertex3fv(v.data()); }

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Point3);
Q_DECLARE_METATYPE(Ovito::Point3I);
Q_DECLARE_METATYPE(Ovito::Point3*);
Q_DECLARE_METATYPE(Ovito::Point3I*);
Q_DECLARE_TYPEINFO(Ovito::Point3, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Point3I, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Point3*, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Point3I*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_POINT3_H
