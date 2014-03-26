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
 * \file Vector4.h
 * \brief Contains definition of the Ovito::Vector_4 template class and operators.
 */

#ifndef __OVITO_VECTOR4_H
#define __OVITO_VECTOR4_H

#include <base/Base.h>
#include <base/io/SaveStream.h>
#include <base/io/LoadStream.h>
#include "Vector3.h"

namespace Ovito {

/**
 * \brief A vector with four components X, Y, Z, W.
 */
template<typename T>
class Vector_4 : public std::array<T, 4>
{
public:

	struct Zero {};

public:

	/////////////////////////////// Constructors /////////////////////////////////

	/// \brief Constructs a vector without initializing its components.
	/// \note All components are left uninitialized by this constructor and will therefore have an undefined value!
	Vector_4() {}

	/// \brief Constructs a vector with all four components set to the given value.
	Q_DECL_CONSTEXPR explicit Vector_4(T val) : std::array<T, 4>{{val,val,val,val}} {}

	/// \brief Initializes the components of the vector with the given component values.
	Q_DECL_CONSTEXPR Vector_4(T x, T y, T z, T w) : std::array<T, 4>{{x, y, z, w}} {}

	/// \brief Initializes the vector to the null vector. All components are set to zero.
	Q_DECL_CONSTEXPR Vector_4(Zero) : std::array<T, 4>{{T(0), T(0), T(0), T(0)}} {}

	/// \brief Initializes the vector from an array.
	Q_DECL_CONSTEXPR explicit Vector_4(const std::array<T, 4>& a) : std::array<T, 4>(a) {}

	/// \brief Initializes the 4-vector from a 3-vector.
	Q_DECL_CONSTEXPR explicit Vector_4(const Vector_3<T>& v, T w) : std::array<T, 4>{{v.x(), v.y(), v.z(), w}} {}

    /////////////////////////////// Unary operators //////////////////////////////

	/// \brief Returns the reverse of the vector.
	/// \return A vector with negated components: (-X, -Y, -Z, -W).
	Q_DECL_CONSTEXPR Vector_4 operator-() const { return {-x(), -y(), -z(), -w()}; }

	///////////////////////////// Assignment operators ///////////////////////////

	/// \brief Adds another vector to this vector and stores the result in this vector.
	/// \param v The vector to add to this vector.
	/// \return A reference to \c this vector, which has been changed.
	Vector_4& operator+=(const Vector_4& v) { x() += v.x(); y() += v.y(); z() += v.z(); w() += v.w(); return *this; }

	/// \brief Subtracts another vector from this vector and stores the result in this vector.
	/// \param v The vector to subtract from this vector.
	/// \return A reference to \c this vector, which has been changed.
	Vector_4& operator-=(const Vector_4& v) { x() -= v.x(); y() -= v.y(); z() -= v.z(); w() -= v.w(); return *this; }

	/// \brief Multiplies each component of the vector with a scalar value and stores the result in this vector.
	/// \param s The scalar value to multiply this vector with.
	/// \return A reference to \c this vector, which has been changed.
	Vector_4& operator*=(T s) { x() *= s; y() *= s; z() *= s; w() *= s; return *this; }

	/// \brief Divides each component of the vector by a scalar value and stores the result in this vector.
	/// \param s The scalar value.
	/// \return A reference to \c this vector, which has been changed.
	Vector_4& operator/=(T s) { x() /= s; y() /= s; z() /= s; w() /= s; return *this; }

	/// \brief Sets all components of the vector to zero.
	Vector_4& operator=(Zero) { w() = z() = y() = x() = T(0); return *this; }

	//////////////////////////// Component access //////////////////////////

	/// \brief Returns the value of the X component of this vector.
	Q_DECL_CONSTEXPR T x() const { return (*this)[0]; }

	/// \brief Returns the value of the Y component of this vector.
	Q_DECL_CONSTEXPR T y() const { return (*this)[1]; }

	/// \brief Returns the value of the Z component of this vector.
	Q_DECL_CONSTEXPR T z() const { return (*this)[2]; }

	/// \brief Returns the value of the W component of this vector.
	Q_DECL_CONSTEXPR T w() const { return (*this)[3]; }

	/// \brief Returns a reference to the X component of this vector.
	T& x() { return (*this)[0]; }

	/// \brief Returns a reference to the Y component of this vector.
	T& y() { return (*this)[1]; }

	/// \brief Returns a reference to the Z component of this vector.
	T& z() { return (*this)[2]; }

	/// \brief Returns a reference to the W component of this vector.
	T& w() { return (*this)[2]; }

	////////////////////////////////// Comparison ////////////////////////////////

	/// \brief Compares two vectors for exact equality.
	/// \return true if each of the components are equal; false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const Vector_4& v) const { return (v.x()==x()) && (v.y()==y()) && (v.z()==z()) && (v.w()==w()); }

	/// \brief Compares two vectors for inequality.
	/// \return true if any of the components are not equal; false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const Vector_4& v) const { return (v.x()!=x()) || (v.y()!=y()) || (v.z()!=z()) || (v.w()!=w()); }

	/// \brief Checks whether the vector is the null vector, i.e. all components are zero.
	/// \return true if all of the components are zero; false otherwise
	Q_DECL_CONSTEXPR bool operator==(Zero) const { return (x()==T(0)) && (y()==T(0)) && (z()==T(0)) && (w()==T(0)); }

	/// \brief Checks whether the vector is not a null vector, i.e. any of the components is nonzero.
	/// \return true if any of the components is nonzero; false if this is the null vector otherwise
	Q_DECL_CONSTEXPR bool operator!=(Zero) const { return (x()!=T(0)) || (y()!=T(0)) || (z()!=T(0)) || (w()!=T(0)); }

	/// \brief Checks whether two vectors are equal within a given tolerance.
	/// \param v The vector that should be compared to this vector.
	/// \param tolerance A non-negative threshold for the equality test. The two vectors are considered equal when
	///        the differences in the X, Y, Z, and W directions are all smaller than this tolerance value.
	/// \return true if this vector is equal to the given vector within the given tolerance.
	Q_DECL_CONSTEXPR bool equals(const Vector_4& v, T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(v.x() - x()) <= tolerance && std::abs(v.y() - y()) <= tolerance && std::abs(v.z() - z()) <= tolerance && std::abs(v.w() - w()) <= tolerance;
	}

	/// \brief Checks whether the vectors is zero within a given tolerance.
	/// \param tolerance A non-negative threshold.
	/// \return true if the absolute value of all components is smaller than the tolerance.
	Q_DECL_CONSTEXPR bool isZero(T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(x()) <= tolerance && std::abs(y()) <= tolerance && std::abs(z()) <= tolerance && std::abs(w()) <= tolerance;
	}

	///////////////////////////////// Computations ////////////////////////////////

	/// \brief Computes the scalar product of two vectors.
	Q_DECL_CONSTEXPR T dot(const Vector_4& b) const { return x()*b.x() + y()*b.y() + z()*b.z() + w()*b.w(); }

	/// \brief Computes the squared length of the vector.
	Q_DECL_CONSTEXPR T squaredLength() const { return x()*x() + y()*y() + z()*z() + w()*w(); }

	/// \brief Computes the length of the vector.
	Q_DECL_CONSTEXPR T length() const { return static_cast<T>(sqrt(squaredLength())); }

	/// \brief Normalizes this vector to unit length.
	/// \note If this is the null vector then an assertion message is generated in debug builds. In release builds the behavior is undefined.
	inline void normalize() {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector4::normalize", "Cannot normalize a vector with zero length.");
		*this /= length();
	}

	/// \brief Returns a normalized copy of this vector.
	/// \note If this is the null vector then an assertion message is generated in debug builds. In release builds the behavior is undefined.
	inline Vector_4 normalized() const {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector4::normalize", "Cannot normalize a vector with zero length.");
		return *this / length();
	}

	/// \brief Normalizes a vector to unit length only if it is non-zero.
	inline void normalizeSafely(T epsilon = T(FLOATTYPE_EPSILON)) {
		T l = length();
		if(l > epsilon)
			*this /= l;
	}

	///////////////////////////////// Utilities ////////////////////////////////

	/// \brief Returns the index of the component with the maximum value.
	inline std::size_t maxComponent() const {
	    return std::distance(std::array<T, 4>::begin(), std::max_element(std::array<T, 4>::begin(), std::array<T, 4>::end()));
	}

	/// \brief Returns the index of the component with the minimum value.
	Q_DECL_CONSTEXPR inline std::size_t minComponent() const {
	    return std::distance(std::array<T, 4>::begin(), std::min_element(std::array<T, 4>::begin(), std::array<T, 4>::end()));
	}

	/// \brief Produces a string representation of this vector.
	/// \return A string that contains the components of the vector.
	QString toString() const {
		return QString("(%1 %2 %3 %4)").arg(x()).arg(y()).arg(z()).arg(w());
	}
};

/// \brief Computes the sum of two vectors.
template<typename T>
Q_DECL_CONSTEXPR Vector_4<T> operator+(const Vector_4<T>& a, const Vector_4<T>& b) {
	return { a.x() + b.x(), a.y() + b.y(), a.z() + b.z(), a.w() + b.w() };
}

/// \brief Computes the difference of two vectors.
template<typename T>
Q_DECL_CONSTEXPR Vector_4<T> operator-(const Vector_4<T>& a, const Vector_4<T>& b) {
	return { a.x() - b.x(), a.y() - b.y(), a.z() - b.z(), a.w() - b.w() };
}

/// \brief Computes the product of a vector and a scalar value.
template<typename T>
Q_DECL_CONSTEXPR Vector_4<T> operator*(const Vector_4<T>& a, T s) {
	return { a.x() * s, a.y() * s, a.z() * s, a.w() * s };
}

/// \brief Computes the product of a vector and a scalar value.
template<typename T>
Q_DECL_CONSTEXPR Vector_4<T> operator*(T s, const Vector_4<T>& a) {
	return { a.x() * s, a.y() * s, a.z() * s, a.w() * s };
}

/// \brief Computes the division of a vector by a scalar value.
template<typename T>
Q_DECL_CONSTEXPR Vector_4<T> operator/(const Vector_4<T>& a, T s) {
	return { a.x() / s, a.y() / s, a.z() / s, a.w() / s };
}

/// \brief Writes the vector to a text output stream.
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Vector_4<T>& v) {
	return os << "(" << v.x() << ", " << v.y()  << ", " << v.z() << ", " << v.w() << ")";
}

/// \brief Writes the vector to the Qt debug stream.
template<typename T>
inline QDebug operator<<(QDebug dbg, const Vector_4<T>& v) {
    dbg.nospace() << "(" << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << ")";
    return dbg.space();
}

/// \brief Writes a vector to a binary output stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Vector_4<T>& v) {
	return stream << v.x() << v.y() << v.z() << v.w();
}

/// \brief Reads a vector from a binary input stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Vector_4<T>& v) {
	return stream >> v.x() >> v.y() >> v.z() >> v.w();
}

/// \brief Writes a vector to a Qt data stream.
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Vector_4<T>& v) {
	return stream << v.x() << v.y() << v.z() << v.w();
}

/// \brief Reads a vector from a Qt data stream.
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Vector_4<T>& v) {
	return stream >> v.x() >> v.y() >> v.z() >> v.w();
}

/**
 * \fn typedef Vector4
 * \brief Template class instance of the Vector_4 class used for floating-point vectors.
 */
typedef Vector_4<FloatType>		Vector4;

/**
 * \fn typedef Vector4I
 * \brief Template class instance of the Vector_4 class used for integer vectors.
 */
typedef Vector_4<int>			Vector4I;

inline void glVertex(const Vector_4<GLdouble>& v) { glVertex4dv(v.data()); }
inline void glVertex(const Vector_4<GLfloat>& v) { glVertex4fv(v.data()); }

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Vector4);
Q_DECLARE_METATYPE(Ovito::Vector4I);
Q_DECLARE_METATYPE(Ovito::Vector4*);
Q_DECLARE_METATYPE(Ovito::Vector4I*);
Q_DECLARE_TYPEINFO(Ovito::Vector4, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector4I, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector4*, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector4I*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_VECTOR4_H
