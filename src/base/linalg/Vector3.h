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
 * \file Vector3.h
 * \brief Contains definition of the Ovito::Vector_3 template class and operators.
 */

#ifndef __OVITO_VECTOR3_H
#define __OVITO_VECTOR3_H

#include <base/Base.h>
#include <base/io/SaveStream.h>
#include <base/io/LoadStream.h>

namespace Ovito {

/**
 * \brief A vector with three components X, Y, Z.
 *
 * This is one of the basic vector algebra classes. It represents a three
 * dimensional vector in space. There are two instances of this template
 * vector class: \c Vector3 is for floating-point vectors and \c Vector3I is
 * for integer vectors with three components.
 *
 * Note that there is also a class called Point_3 that is used for points
 * in a three dimensional coordinate system.
 */
template<typename T>
class Vector_3 : public std::array<T, 3>
{
public:

	struct Zero {};

public:

	/////////////////////////////// Constructors /////////////////////////////////

	/// \brief Constructs a vector without initializing its components.
	/// \note All components are left uninitialized by this constructor and will therefore have an undefined value!
	Vector_3() {}

	/// \brief Constructs a vector with all three components set to the given value.
	Q_DECL_CONSTEXPR explicit Vector_3(T val) : std::array<T, 3>{{val,val,val}} {}

	/// \brief Initializes the components of the vector with the given component values.
	Q_DECL_CONSTEXPR Vector_3(T x, T y, T z) : std::array<T, 3>{{x, y, z}} {}

	/// \brief Initializes the vector to the null vector. All components are set to zero.
	Q_DECL_CONSTEXPR Vector_3(Zero) : std::array<T, 3>{{T(0), T(0), T(0)}} {}

	/// \brief Initializes the vector from an array.
	Q_DECL_CONSTEXPR explicit Vector_3(const std::array<T, 3>& a) : std::array<T, 3>(a) {}

	/// \brief Casts the vector to a vector with another data type.
	template<typename U>
	Q_DECL_CONSTEXPR explicit operator Vector_3<U>() const { return Vector_3<U>(static_cast<U>(x()), static_cast<U>(y()), static_cast<U>(z())); }

    /////////////////////////////// Unary operators //////////////////////////////

	/// \brief Returns the reverse of the vector.
	/// \return A vector with negated components: (-X, -Y, -Z).
	Q_DECL_CONSTEXPR Vector_3 operator-() const { return {-x(), -y(), -z()}; }

	///////////////////////////// Assignment operators ///////////////////////////

	/// \brief Adds another vector to this vector and stores the result in this vector.
	/// \param v The vector to add to this vector.
	/// \return A reference to \c this vector, which has been changed.
	Vector_3& operator+=(const Vector_3& v) { x() += v.x(); y() += v.y(); z() += v.z(); return *this; }

	/// \brief Subtracts another vector from this vector and stores the result in this vector.
	/// \param v The vector to subtract from this vector.
	/// \return A reference to \c this vector, which has been changed.
	Vector_3& operator-=(const Vector_3& v) { x() -= v.x(); y() -= v.y(); z() -= v.z(); return *this; }

	/// \brief Multiplies each component of the vector with a scalar value and stores the result in this vector.
	/// \param s The scalar value to multiply this vector with.
	/// \return A reference to \c this vector, which has been changed.
	Vector_3& operator*=(T s) { x() *= s; y() *= s; z() *= s; return *this; }

	/// \brief Divides each component of the vector by a scalar value and stores the result in this vector.
	/// \param s The scalar value.
	/// \return A reference to \c this vector, which has been changed.
	Vector_3& operator/=(T s) { x() /= s; y() /= s; z() /= s; return *this; }

	/// \brief Sets all components of the vector to zero.
	Vector_3& operator=(Zero) { return setZero(); }

	/// \brief Sets all components of the vector to zero.
	Vector_3& setZero() { z() = y() = x() = T(0); return *this; }

	//////////////////////////// Component access //////////////////////////

	/// \brief Returns the value of the X component of this vector.
	Q_DECL_CONSTEXPR T x() const { return (*this)[0]; }

	/// \brief Returns the value of the Y component of this vector.
	Q_DECL_CONSTEXPR T y() const { return (*this)[1]; }

	/// \brief Returns the value of the Z component of this vector.
	Q_DECL_CONSTEXPR T z() const { return (*this)[2]; }

	/// \brief Returns a reference to the X component of this vector.
	T& x() { return (*this)[0]; }

	/// \brief Returns a reference to the Y component of this vector.
	T& y() { return (*this)[1]; }

	/// \brief Returns a reference to the Z component of this vector.
	T& z() { return (*this)[2]; }

	////////////////////////////////// Comparison ////////////////////////////////

	/// \brief Compares two vectors for exact equality.
	/// \return true if each of the components are equal; false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const Vector_3& v) const { return (v.x()==x()) && (v.y()==y()) && (v.z()==z()); }

	/// \brief Compares two vectors for inequality.
	/// \return true if any of the components are not equal; false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const Vector_3& v) const { return (v.x()!=x()) || (v.y()!=y()) || (v.z()!=z()); }

	/// \brief Checks whether the vector is the null vector, i.e. all components are zero.
	/// \return true if all of the components are zero; false otherwise
	Q_DECL_CONSTEXPR bool operator==(Zero) const { return (x()==T(0)) && (y()==T(0)) && (z()==T(0)); }

	/// \brief Checks whether the vector is not a null vector, i.e. any of the components is nonzero.
	/// \return true if any of the components is nonzero; false if this is the null vector otherwise
	Q_DECL_CONSTEXPR bool operator!=(Zero) const { return (x()!=T(0)) || (y()!=T(0)) || (z()!=T(0)); }

	/// \brief Checks whether two vectors are equal within a given tolerance.
	/// \param v The vector that should be compared to this vector.
	/// \param tolerance A non-negative threshold for the equality test. The two vectors are considered equal when
	///        the differences in the X, Y, and Z directions are all smaller than this tolerance value.
	/// \return true if this vector is equal to the given vector within the given tolerance.
	Q_DECL_CONSTEXPR bool equals(const Vector_3& v, T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(v.x() - x()) <= tolerance && std::abs(v.y() - y()) <= tolerance && std::abs(v.z() - z()) <= tolerance;
	}

	/// \brief Checks whether the vectors is zero within a given tolerance.
	/// \param tolerance A non-negative threshold.
	/// \return true if the absolute value of all components is smaller than the tolerance.
	Q_DECL_CONSTEXPR bool isZero(T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(x()) <= tolerance && std::abs(y()) <= tolerance && std::abs(z()) <= tolerance;
	}

	///////////////////////////////// Computations ////////////////////////////////

	/// \brief Computes the scalar product of two vectors.
	Q_DECL_CONSTEXPR T dot(const Vector_3& b) const { return x()*b.x() + y()*b.y() + z()*b.z(); }

	/// \brief Computes the cross product of two vectors.
	Q_DECL_CONSTEXPR Vector_3 cross(const Vector_3& b) const {
		return {y() * b.z() - z() * b.y(),
				z() * b.x() - x() * b.z(),
				x() * b.y() - y() * b.x()};
	}

	/// \brief Computes the squared length of the vector.
	Q_DECL_CONSTEXPR T squaredLength() const { return x()*x() + y()*y() + z()*z(); }

	/// \brief Computes the length of the vector.
	Q_DECL_CONSTEXPR T length() const { return static_cast<T>(sqrt(squaredLength())); }

	/// \brief Normalizes this vector to unit length.
	/// \note If this is the null vector then an assertion message is generated in debug builds. In release builds the behavior is undefined.
	inline void normalize() {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector3::normalize", "Cannot normalize a vector with zero length.");
		*this /= length();
	}

	/// \brief Rescales the vector to the given length.
	/// \note If this is the null vector then an assertion message is generated in debug builds. In release builds the behavior is undefined.
	inline void resize(T len) const {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector3::resize", "Cannot resize a vector with zero length.");
		*this *= (len / length());
	}

	/// \brief Returns a normalized copy of this vector.
	/// \note If this is the null vector then an assertion message is generated in debug builds. In release builds the behavior is undefined.
	inline Vector_3 normalized() const {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector3::normalize", "Cannot normalize a vector with zero length.");
		return *this / length();
	}

	/// \brief Normalizes a vector to unit length only if it is non-zero.
	inline void normalizeSafely(T epsilon = T(FLOATTYPE_EPSILON)) {
		T l = length();
		if(l > epsilon)
			*this /= l;
	}

	/// \brief Returns a copy of this vector with the given length.
	/// \note If this is the null vector then an assertion message is generated in debug builds. In release builds the behavior is undefined.
	inline Vector_3 resized(T len) const {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector3::resized", "Cannot resize a vector with zero length.");
		return *this * (len / length());
	}

	///////////////////////////////// Utilities ////////////////////////////////

	/// \brief Returns the index of the component with the maximum value.
	Q_DECL_CONSTEXPR inline std::size_t maxComponent() const {
	    return ((x() >= y()) ? ((x() >= z()) ? 0 : 2) : ((y() >= z()) ? 1 : 2));
	}

	/// \brief Returns the index of the component with the minimum value.
	Q_DECL_CONSTEXPR inline std::size_t minComponent() const {
	    return ((x() <= y()) ? ((x() <= z()) ? 0 : 2) : ((y() <= z()) ? 1 : 2));
	}

	/// \brief Produces a string representation of this vector.
	/// \return A string that contains the components of the vector.
	QString toString() const {
		return QString("(%1 %2 %3)").arg(x()).arg(y()).arg(z());
	}
};

/// \brief Computes the sum of two vectors.
template<typename T>
Q_DECL_CONSTEXPR Vector_3<T> operator+(const Vector_3<T>& a, const Vector_3<T>& b) {
	return { a.x() + b.x(), a.y() + b.y(), a.z() + b.z() };
}

/// \brief Computes the difference of two vectors.
template<typename T>
Q_DECL_CONSTEXPR Vector_3<T> operator-(const Vector_3<T>& a, const Vector_3<T>& b) {
	return { a.x() - b.x(), a.y() - b.y(), a.z() - b.z() };
}

/// \brief Computes the product of a vector and a scalar value.
template<typename T>
Q_DECL_CONSTEXPR Vector_3<T> operator*(const Vector_3<T>& a, T s) {
	return { a.x() * s, a.y() * s, a.z() * s };
}

/// \brief Computes the product of a vector and a scalar value.
template<typename T>
Q_DECL_CONSTEXPR Vector_3<T> operator*(T s, const Vector_3<T>& a) {
	return { a.x() * s, a.y() * s, a.z() * s };
}

/// \brief Computes the division of a vector by a scalar value.
template<typename T, typename S>
Q_DECL_CONSTEXPR Vector_3<T> operator/(const Vector_3<T>& a, S s) {
	return { a.x() / s, a.y() / s, a.z() / s };
}

/// \brief Writes the vector to a text output stream.
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Vector_3<T>& v) {
	return os << "(" << v.x() << ", " << v.y()  << ", " << v.z() << ")";
}

/// \brief Writes the vector to the Qt debug stream.
template<typename T>
inline QDebug operator<<(QDebug dbg, const Vector_3<T>& v) {
    dbg.nospace() << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
    return dbg.space();
}

/// \brief Writes a vector to a binary output stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Vector_3<T>& v) {
	return stream << v.x() << v.y() << v.z();
}

/// \brief Reads a vector from a binary input stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Vector_3<T>& v) {
	return stream >> v.x() >> v.y() >> v.z();
}

/**
 * \fn typedef Vector3
 * \brief Template class instance of the Vector_3 class used for floating-point vectors.
 */
typedef Vector_3<FloatType>		Vector3;

/**
 * \fn typedef Vector3I
 * \brief Template class instance of the Vector_3 class used for integer vectors.
 */
typedef Vector_3<int>			Vector3I;

inline void glVertex(const Vector_3<GLdouble>& v) { glVertex3dv(v.data()); }
inline void glVertex(const Vector_3<GLfloat>& v) { glVertex3fv(v.data()); }

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Vector3);
Q_DECLARE_METATYPE(Ovito::Vector3I);
Q_DECLARE_METATYPE(Ovito::Vector3*);
Q_DECLARE_METATYPE(Ovito::Vector3I*);
Q_DECLARE_TYPEINFO(Ovito::Vector3, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector3I, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector3*, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector3I*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_VECTOR3_H
