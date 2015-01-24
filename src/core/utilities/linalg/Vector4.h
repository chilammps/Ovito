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
 * \brief Contains the definition of the Ovito::Vector_4 class template.
 */

#ifndef __OVITO_VECTOR4_H
#define __OVITO_VECTOR4_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>
#include "Vector3.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

/**
 * \brief A vector with four components.
 *
 * The template parameter \a T specifies the data type of the vector's components.
 * Two standard instantiations of Vector_4 for floating-point and integer vectors are predefined:
 *
 * \code
 *      typedef Vector_4<FloatType>  Vector4;
 *      typedef Vector_4<int>        Vector4I;
 * \endcode
 *
 * Note that the default constructor does not initialize the components of the vector for performance reasons.
 * The nested type Zero can be used to construct the vector (0,0,0,0):
 *
 * \code
 *      Vector4 v = Vector4::Zero()
 * \endcode
 *
 * Vector_4 derives from std::array<T,4>. Thus, the vector's components can be accessed via indices, but also via names:
 *
 * \code
 *      v[3]  = 10.0f;
 *      v.w() = 10.0f;
 * \endcode
 *
 * \sa Vector_2, Vector_3, Matrix_4
 */
template<typename T>
class Vector_4 : public std::array<T, 4>
{
public:

	/// An empty type that denotes the vector (0,0,0,0).
	struct Zero {};

	using typename std::array<T, 4>::size_type;
	using typename std::array<T, 4>::difference_type;
	using typename std::array<T, 4>::value_type;
	using typename std::array<T, 4>::iterator;
	using typename std::array<T, 4>::const_iterator;

	/////////////////////////////// Constructors /////////////////////////////////

	/// Constructs a vector without initializing its components. The components will have an undefined value!
	Vector_4() {}

	/// Constructs a vector with all four components initialized to the given value.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR explicit Vector_4(T val) : std::array<T, 4>{{val,val,val,val}} {}
#else
	explicit Vector_4(T val) { this->fill(val); }
#endif

		/// Initializes the components of the vector with the given values.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Vector_4(T x, T y, T z, T w) : std::array<T, 4>{{x, y, z, w}} {}
#else
	Vector_4(T x, T y, T z, T w) { this->x() = x; this->y() = y; this->z() = z; this->w() = w; }
#endif

		/// Initializes the vector to the null vector. All components are set to zero.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Vector_4(Zero) : std::array<T, 4>{{T(0), T(0), T(0), T(0)}} {}
#else
	Vector_4(Zero) { this->fill(T(0)); }
#endif

	/// Initializes the vector from an array.
	Q_DECL_CONSTEXPR explicit Vector_4(const std::array<T, 4>& a) : std::array<T, 4>(a) {}

	/// Initializes the 4-vector from a 3-vector.
	/// \param v Specifies the xyz components of the new vector.
	/// \param w The w component of the new vector.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR explicit Vector_4(const Vector_3<T>& v, T w) : std::array<T, 4>{{v.x(), v.y(), v.z(), w}} {}
#else
	explicit Vector_4(const Vector_3<T>& v, T w) { this->x() = v.x; this->y() = v.y; this->z() = v.z; this->w() = w; }
#endif

    /////////////////////////////// Unary operators //////////////////////////////

	/// Returns the reverse vector (-x(), -y(), -z(), -w()).
	Q_DECL_CONSTEXPR Vector_4 operator-() const { return Vector_4{-x(), -y(), -z(), -w()}; }

	///////////////////////////// Assignment operators ///////////////////////////

	/// Increments the components of this vector by the components of another vector.
	Vector_4& operator+=(const Vector_4& v) { x() += v.x(); y() += v.y(); z() += v.z(); w() += v.w(); return *this; }

	/// Decrements the components of this vector by the components of another vector.
	Vector_4& operator-=(const Vector_4& v) { x() -= v.x(); y() -= v.y(); z() -= v.z(); w() -= v.w(); return *this; }

	/// Multiplies each component of the vector with a scalar.
	Vector_4& operator*=(T s) { x() *= s; y() *= s; z() *= s; w() *= s; return *this; }

	/// Divides each component of the vector by a scalar.
	Vector_4& operator/=(T s) { x() /= s; y() /= s; z() /= s; w() /= s; return *this; }

	/// Sets all components of the vector to zero.
	Vector_4& operator=(Zero) { setZero(); return *this; }

	/// Sets all components of the vector to zero.
	void setZero() { this->fill(T(0)); }

	//////////////////////////// Component access //////////////////////////

	/// Returns the value of the X component of this vector.
	Q_DECL_CONSTEXPR T x() const { return (*this)[0]; }

	/// Returns the value of the Y component of this vector.
	Q_DECL_CONSTEXPR T y() const { return (*this)[1]; }

	/// Returns the value of the Z component of this vector.
	Q_DECL_CONSTEXPR T z() const { return (*this)[2]; }

	/// Returns the value of the W component of this vector.
	Q_DECL_CONSTEXPR T w() const { return (*this)[3]; }

	/// Returns a reference to the X component of this vector.
	T& x() { return (*this)[0]; }

	/// Returns a reference to the Y component of this vector.
	T& y() { return (*this)[1]; }

	/// Returns a reference to the Z component of this vector.
	T& z() { return (*this)[2]; }

	/// Returns a reference to the W component of this vector.
	T& w() { return (*this)[3]; }

	////////////////////////////////// Comparison ////////////////////////////////

	/// \brief Compares two vectors for exact equality.
	/// \return \c true if all components are equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const Vector_4& v) const { return (v.x()==x()) && (v.y()==y()) && (v.z()==z()) && (v.w()==w()); }

	/// \brief Compares two vectors for inequality.
	/// \return \c true if any of the components are not equal; \c false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const Vector_4& v) const { return (v.x()!=x()) || (v.y()!=y()) || (v.z()!=z()) || (v.w()!=w()); }

	/// \brief Tests if the vector is the null vector, i.e. if all components are zero.
	/// \return \c true if all components are exactly zero; \c false otherwise
	Q_DECL_CONSTEXPR bool operator==(Zero) const { return (x()==T(0)) && (y()==T(0)) && (z()==T(0)) && (w()==T(0)); }

	/// \brief Tests if the vector is not a null vector, i.e. if any of the components is nonzero.
	/// \return \c true if any component is nonzero; \c false if all components are exactly zero.
	Q_DECL_CONSTEXPR bool operator!=(Zero) const { return (x()!=T(0)) || (y()!=T(0)) || (z()!=T(0)) || (w()!=T(0)); }

	/// \brief Tests if two vectors are equal within a given tolerance.
	/// \param v The vector to compare to this vector.
	/// \param tolerance A non-negative threshold for the equality test. The two vectors are considered equal if
	///        the differences in the four components are all less than this tolerance value.
	/// \return \c true if this vector is equal to \a v within the given tolerance; \c false otherwise.
	Q_DECL_CONSTEXPR bool equals(const Vector_4& v, T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(v.x() - x()) <= tolerance && std::abs(v.y() - y()) <= tolerance && std::abs(v.z() - z()) <= tolerance && std::abs(v.w() - w()) <= tolerance;
	}

	/// \brief Test if the vector is zero within a given tolerance.
	/// \param tolerance A non-negative threshold.
	/// \return \c true if the absolute vector components are all smaller than \a tolerance.
	Q_DECL_CONSTEXPR bool isZero(T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(x()) <= tolerance && std::abs(y()) <= tolerance && std::abs(z()) <= tolerance && std::abs(w()) <= tolerance;
	}

	///////////////////////////////// Computations ////////////////////////////////

	/// Computes the inner dot product of this vector with the vector \a b.
	Q_DECL_CONSTEXPR T dot(const Vector_4& b) const { return x()*b.x() + y()*b.y() + z()*b.z() + w()*b.w(); }

	/// Computes the squared length of the vector.
	Q_DECL_CONSTEXPR T squaredLength() const { return x()*x() + y()*y() + z()*z() + w()*w(); }

	/// Computes the length of the vector.
	Q_DECL_CONSTEXPR T length() const { return static_cast<T>(sqrt(squaredLength())); }

	/// \brief Normalizes this vector by dividing it by its length, making it a unit vector.
	/// \warning Do not call this function if the vector has length zero to avoid division by zero.
	/// In debug builds, a zero vector will be detected and reported. In release builds, the behavior is undefined.
	/// \sa normalized(), normalizeSafely()
	inline void normalize() {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector4::normalize", "Cannot normalize a vector with zero length.");
		*this /= length();
	}

	/// \brief Returns a normalized version of this vector.
	/// \return The unit vector.
	/// \warning Do not call this function if the vector has length zero to avoid division by zero.
	/// In debug builds, a zero vector will be detected and reported. In release builds, the behavior is undefined.
	/// \sa normalize(), normalizeSafely()
	inline Vector_4 normalized() const {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector4::normalize", "Cannot normalize a vector with zero length.");
		return *this / length();
	}

	/// \brief Normalizes this vector to make it a unit vector (only if it is non-zero).
	/// \param epsilon The epsilon used to test if this vector is zero.
	/// This method rescales this vector to unit length if its original length is greater than \a epsilon.
	/// Otherwise it does nothing.
	/// \sa normalize(), normalized()
	inline void normalizeSafely(T epsilon = T(FLOATTYPE_EPSILON)) {
		T l = length();
		if(l > epsilon)
			*this /= l;
	}

	///////////////////////////////// Utilities ////////////////////////////////

	/// \brief Returns the index of the component with the maximum value.
	Q_DECL_CONSTEXPR inline size_type maxComponent() const {
	    return std::distance(std::array<T, 4>::begin(), std::max_element(std::array<T, 4>::begin(), std::array<T, 4>::end()));
	}

	/// \brief Returns the index of the component with the minimum value.
	Q_DECL_CONSTEXPR inline size_type minComponent() const {
	    return std::distance(std::array<T, 4>::begin(), std::min_element(std::array<T, 4>::begin(), std::array<T, 4>::end()));
	}

	/// \brief Produces a string representation of the vector of the form (x y z w).
	QString toString() const {
		return QString("(%1 %2 %3 %4)").arg(x()).arg(y()).arg(z()).arg(w());
	}
};

/// \brief Computes the sum of two vectors.
/// \relates Vector_4
template<typename T>
Q_DECL_CONSTEXPR Vector_4<T> operator+(const Vector_4<T>& a, const Vector_4<T>& b) {
	return { a.x() + b.x(), a.y() + b.y(), a.z() + b.z(), a.w() + b.w() };
}

/// \brief Computes the difference of two vectors.
/// \relates Vector_4
template<typename T>
Q_DECL_CONSTEXPR Vector_4<T> operator-(const Vector_4<T>& a, const Vector_4<T>& b) {
	return Vector_4<T>{ a.x() - b.x(), a.y() - b.y(), a.z() - b.z(), a.w() - b.w() };
}

/// \brief Computes the product of a vector and a scalar value.
/// \relates Vector_4
template<typename T>
Q_DECL_CONSTEXPR Vector_4<T> operator*(const Vector_4<T>& a, T s) {
	return Vector_4<T>{ a.x() * s, a.y() * s, a.z() * s, a.w() * s };
}

/// \brief Computes the product of a scalar value and a vector.
/// \relates Vector_4
template<typename T>
Q_DECL_CONSTEXPR Vector_4<T> operator*(T s, const Vector_4<T>& a) {
	return Vector_4<T>{ a.x() * s, a.y() * s, a.z() * s, a.w() * s };
}

/// \brief Computes the division of a vector by a scalar value.
/// \relates Vector_4
template<typename T>
Q_DECL_CONSTEXPR Vector_4<T> operator/(const Vector_4<T>& a, T s) {
	return Vector_4<T>{ a.x() / s, a.y() / s, a.z() / s, a.w() / s };
}

/// \brief Writes a vector to a text output stream.
/// \relates Vector_4
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Vector_4<T>& v) {
	return os << "(" << v.x() << ", " << v.y()  << ", " << v.z() << ", " << v.w() << ")";
}

/// \brief Writes a vector to a Qt debug stream.
/// \relates Vector_4
template<typename T>
inline QDebug operator<<(QDebug dbg, const Vector_4<T>& v) {
    dbg.nospace() << "(" << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << ")";
    return dbg.space();
}

/// \brief Writes a vector to a binary output stream.
/// \relates Vector_4
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Vector_4<T>& v) {
	return stream << v.x() << v.y() << v.z() << v.w();
}

/// \brief Reads a vector from a binary input stream.
/// \relates Vector_4
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Vector_4<T>& v) {
	return stream >> v.x() >> v.y() >> v.z() >> v.w();
}

/// \brief Writes a vector to a Qt data stream.
/// \relates Vector_4
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Vector_4<T>& v) {
	return stream << v.x() << v.y() << v.z() << v.w();
}

/// \brief Reads a vector from a Qt data stream.
/// \relates Vector_4
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Vector_4<T>& v) {
	return stream >> v.x() >> v.y() >> v.z() >> v.w();
}

/**
 * \brief Instantiation of the Vector_4 class template with the default floating-point type.
 * \relates Vector_4
 */
typedef Vector_4<FloatType>		Vector4;

/**
 * \brief Instantiation of the Vector_4 class template with the default integer type.
 * \relates Vector_4
 */
typedef Vector_4<int>			Vector4I;

inline void glVertex(const Vector_4<GLdouble>& v) { glVertex4dv(v.data()); }
inline void glVertex(const Vector_4<GLfloat>& v) { glVertex4fv(v.data()); }

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Vector4);
Q_DECLARE_METATYPE(Ovito::Vector4I);
Q_DECLARE_METATYPE(Ovito::Vector4*);
Q_DECLARE_METATYPE(Ovito::Vector4I*);
Q_DECLARE_TYPEINFO(Ovito::Vector4, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector4I, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector4*, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector4I*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_VECTOR4_H
