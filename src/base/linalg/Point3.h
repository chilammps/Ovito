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
 * \file Point3.h
 * \brief Contains the definition of the Ovito::Point_3 class template.
 */

#ifndef __OVITO_POINT3_H
#define __OVITO_POINT3_H

#include <base/Base.h>
#include <base/io/SaveStream.h>
#include <base/io/LoadStream.h>
#include "Vector3.h"

namespace Ovito {

/**
 * \brief A point in 3D space.
 */
template<typename T>
class Point_3 : public std::array<T, 3>
{
public:

	struct Origin {};

public:

	/////////////////////////////// Constructors /////////////////////////////////

	/// \brief Constructs a point without initializing its coordinates.
	/// \note All coordinates are left uninitialized by this constructor and will therefore have an undefined value!
	Point_3() {}

	/// \brief Constructs a point with all three coordinates set to the given value.
	Q_DECL_CONSTEXPR explicit Point_3(T val) : std::array<T, 3>{{val,val,val}} {}

	/// \brief Initializes the coordinates of the point with the given component values.
	Q_DECL_CONSTEXPR Point_3(T x, T y, T z) : std::array<T, 3>{{x, y, z}} {}

	/// \brief Initializes the point to the origin. All coordinates are set to zero.
	Q_DECL_CONSTEXPR Point_3(Origin) : std::array<T, 3>{{T(0), T(0), T(0)}} {}

	/// \brief Initializes the point from an array.
	Q_DECL_CONSTEXPR explicit Point_3(const std::array<T, 3>& a) : std::array<T, 3>(a) {}

	/// \brief Casts the point to a point with another data type.
	template<typename U>
	Q_DECL_CONSTEXPR explicit operator Point_3<U>() const { return Point_3<U>(static_cast<U>(x()), static_cast<U>(y()), static_cast<U>(z())); }

	///////////////////////////// Assignment operators ///////////////////////////

	/// \brief Adds a vector to this point and stores the result in this object.
	/// \param v The vector to add to this point.
	/// \return A reference to \c this point, which has been changed.
	Point_3& operator+=(const Vector_3<T>& v) { x() += v.x(); y() += v.y(); z() += v.z(); return *this; }

	/// \brief Subtracts a vector from this point and stores the result in this object.
	/// \param v The vector to subtract from this point.
	/// \return A reference to \c this point, which has been changed.
	Point_3& operator-=(const Vector_3<T>& v) { x() -= v.x(); y() -= v.y(); z() -= v.z(); return *this; }

	/// \brief Multplies all coordinates of the point with a scalar value and stores the result in this object.
	/// \param s The scalar value to multiply this point with.
	/// \return A reference to \c this point, which has been changed.
	Point_3& operator*=(T s) { x() *= s; y() *= s; z() *= s; return *this; }

	/// \brief Divides all coordinates of the point by a scalar value and stores the result in this object.
	/// \param s The scalar value.
	/// \return A reference to \c this object, which has been changed.
	Point_3& operator/=(T s) { x() /= s; y() /= s; z() /= s; return *this; }

	/// \brief Sets all coordinates of the point to zero.
	Point_3& operator=(Origin) { z() = y() = x() = T(0); return *this; }

	/// \brief Converts a point to a vector.
	Vector_3<T> operator-(Origin) const {
		return Vector_3<T>(*this);
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
	/// \return true if all coordinates are equal; false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const Point_3& p) const { return (p.x()==x()) && (p.y()==y()) && (p.z()==z()); }

	/// \brief Compares two vectors for inequality.
	/// \return true if any of the coordinates is not equal; false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const Point_3& p) const { return (p.x()!=x()) || (p.y()!=y()) || (p.z()!=z()); }

	/// \brief Checks whether the point is origin, i.e. all coordinates are zero.
	/// \return true if all of the coordinates are exactly zero; false otherwise.
	Q_DECL_CONSTEXPR bool operator==(Origin) const { return (x()==T(0)) && (y()==T(0)) && (z()==T(0)); }

	/// \brief Checks whether the point is not the origin, i.e. any of the coordinates is nonzero.
	/// \return true if any of the coordinates is nonzero; false if this is the origin point otherwise.
	Q_DECL_CONSTEXPR bool operator!=(Origin) const { return (x()!=T(0)) || (y()!=T(0)) || (z()!=T(0)); }

	/// \brief Checks whether two points are equal within a given tolerance.
	/// \param p The point that should be compared to this object.
	/// \param tolerance A non-negative threshold for the equality test. The two points are considered equal when
	///        the differences in the X, Y, and Z directions are all smaller than this tolerance value.
	/// \return true if this point is equal to the given point within the given tolerance.
	Q_DECL_CONSTEXPR bool equals(const Point_3& p, T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(p.x() - x()) <= tolerance && std::abs(p.y() - y()) <= tolerance && std::abs(p.z() - z()) <= tolerance;
	}

	/// \brief Checks whether the point is the origin within a given tolerance.
	/// \param tolerance A non-negative threshold.
	/// \return true if the absolute value of all components is smaller than the tolerance.
	Q_DECL_CONSTEXPR bool isOrigin(T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(x()) <= tolerance && std::abs(y()) <= tolerance && std::abs(z()) <= tolerance;
	}

	///////////////////////////////// Utilities ////////////////////////////////

	/// \brief Returns the index of the coordinate with the maximum value.
	Q_DECL_CONSTEXPR inline std::size_t maxComponent() const {
	    return ((x() >= y()) ? ((x() >= z()) ? 0 : 2) : ((y() >= z()) ? 1 : 2));
	}

	/// \brief Returns the index of the coordinate with the minimum value.
	Q_DECL_CONSTEXPR inline std::size_t minComponent() const {
	    return ((x() <= y()) ? ((x() <= z()) ? 0 : 2) : ((y() <= z()) ? 1 : 2));
	}

	/// \brief Produces a string representation of this point.
	/// \return A string that contains the coordinate of the point.
	QString toString() const {
		return QString("(%1 %2 %3)").arg(x()).arg(y()).arg(z());
	}
};

/// \brief Computes the sum of a point and a vector.
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator+(const Point_3<T>& a, const Vector_3<T>& b) {
	return Point_3<T>{ a.x() + b.x(), a.y() + b.y(), a.z() + b.z() };
}

/// \brief Converts a vector to a point.
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator+(typename Point_3<T>::Origin, const Vector_3<T>& b) {
	return Point_3<T>(b);
}

/// \brief Computes the sum of a vector and a point.
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator+(const Vector_3<T>& a, const Point_3<T>& b) {
	return b + a;
}

/// \brief Computes the difference of a point and a vector.
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator-(const Point_3<T>& a, const Vector_3<T>& b) {
	return Point_3<T>{ a.x() - b.x(), a.y() - b.y(), a.z() - b.z() };
}

/// \brief Computes the difference vector of two points.
template<typename T>
Q_DECL_CONSTEXPR Vector_3<T> operator-(const Point_3<T>& a, const Point_3<T>& b) {
	return Vector_3<T>{ a.x() - b.x(), a.y() - b.y(), a.z() - b.z() };
}

/// \brief Computes the product of a point and a scalar value.
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator*(const Point_3<T>& a, T s) {
	return Point_3<T>{ a.x() * s, a.y() * s, a.z() * s };
}

/// \brief Computes the product of a point and a scalar value.
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator*(T s, const Point_3<T>& a) {
	return Point_3<T>{ a.x() * s, a.y() * s, a.z() * s };
}

/// \brief Computes the division of a vector by a scalar value.
template<typename T>
Q_DECL_CONSTEXPR Point_3<T> operator/(const Point_3<T>& a, T s) {
	return Point_3<T>{ a.x() / s, a.y() / s, a.z() / s };
}

/// \brief Writes the point to a text output stream.
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Point_3<T>& v) {
	return os << "(" << v.x() << ", " << v.y()  << ", " << v.z() << ")";
}

/// \brief Writes the point to the Qt debug stream.
template<typename T>
inline QDebug operator<<(QDebug dbg, const Point_3<T>& v) {
    dbg.nospace() << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
    return dbg.space();
}

/// \brief Writes a point to a binary output stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Point_3<T>& v) {
	return stream << v.x() << v.y() << v.z();
}

/// \brief Reads a point from a binary input stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Point_3<T>& v) {
	return stream >> v.x() >> v.y() >> v.z();
}

/// \brief Writes a point to a Qt data stream.
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Point_3<T>& v) {
	return stream << v.x() << v.y() << v.z();
}

/// \brief Reads a point from a Qt data stream.
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Point_3<T>& v) {
	return stream >> v.x() >> v.y() >> v.z();
}

/**
 * \fn typedef Point3
 * \brief Template class instance of the Point_3 class used for floating-point points.
 */
typedef Point_3<FloatType>		Point3;

/**
 * \fn typedef Point3I
 * \brief Template class instance of the Point_3 class used for integer points.
 */
typedef Point_3<int>			Point3I;

inline void glVertex(const Point_3<GLdouble>& v) { glVertex3dv(v.data()); }
inline void glVertex(const Point_3<GLfloat>& v) { glVertex3fv(v.data()); }

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Point3);
Q_DECLARE_METATYPE(Ovito::Point3I);
Q_DECLARE_METATYPE(Ovito::Point3*);
Q_DECLARE_METATYPE(Ovito::Point3I*);
Q_DECLARE_TYPEINFO(Ovito::Point3, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Point3I, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Point3*, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Point3I*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_POINT3_H
