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
 * \brief Contains the definition of the Ovito::Vector_3 class template.
 */

#ifndef __OVITO_VECTOR3_H
#define __OVITO_VECTOR3_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

/**
 * \brief A vector with three components.
 *
 * Vector_3 represents a vector in three-dimensional space. Note that there exists a corresponding
 * class Point_3, which represents a *point* in three-dimensional space.
 *
 * The template parameter \a T specifies the data type of the vector's components.
 * Two standard instantiations of Vector_3 for floating-point and integer vectors are predefined:
 *
 * \code
 *      typedef Vector_3<FloatType>  Vector3;
 *      typedef Vector_3<int>        Vector3I;
 * \endcode
 *
 * Note that the default constructor does not initialize the components of the vector for performance reasons.
 * The nested type Zero can be used to construct the vector (0,0,0):
 *
 * \code
 *      Vector3 v = Vector3::Zero()
 * \endcode
 *
 * Vector_3 derives from std::array<T,3>. Thus, the vector's components can be accessed via indices, but also via names:
 *
 * \code
 *      v[1]  = 10.0f;
 *      v.y() = 10.0f;
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
 * \sa Point_3, Vector_2, Vector_4
 */
template<typename T>
class Vector_3 : public std::array<T, 3>
{
public:

	/// An empty type that denotes the vector (0,0,0).
	struct Zero {};

	using typename std::array<T, 3>::size_type;
	using typename std::array<T, 3>::difference_type;
	using typename std::array<T, 3>::value_type;
	using typename std::array<T, 3>::iterator;
	using typename std::array<T, 3>::const_iterator;

	/////////////////////////////// Constructors /////////////////////////////////

	/// Constructs a vector without initializing its components. The components will have an undefined value!
	Vector_3() {}

	/// Constructs a vector with all three components initialized to the given value.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR explicit Vector_3(T val) : std::array<T, 3>{{val,val,val}} {}
#else
	explicit Vector_3(T val) { this->fill(val); }
#endif

	/// Initializes the components of the vector with the given values.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Vector_3(T x, T y, T z) : std::array<T, 3>{{x, y, z}} {}
#else
	Vector_3(T x, T y, T z) { this->x() = x; this->y() = y; this->z() = z; }
#endif

	/// Initializes the vector to the null vector. All components are set to zero.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Vector_3(Zero) : std::array<T, 3>{{T(0), T(0), T(0)}} {}
#else
	Vector_3(Zero) { this->fill(T(0)); }
#endif

	/// Initializes the vector from an array.
	Q_DECL_CONSTEXPR explicit Vector_3(const std::array<T, 3>& a) : std::array<T, 3>(a) {}

	/// Casts the vector to another component type \a U.
	template<typename U>
	Q_DECL_CONSTEXPR explicit operator Vector_3<U>() const { return Vector_3<U>(static_cast<U>(x()), static_cast<U>(y()), static_cast<U>(z())); }

    /////////////////////////////// Unary operators //////////////////////////////

	/// Returns the reverse vector (-x(), -y(), -z()).
	Q_DECL_CONSTEXPR Vector_3 operator-() const { return Vector_3(-x(), -y(), -z()); }

	///////////////////////////// Assignment operators ///////////////////////////

	/// Increments the components of this vector by the components of another vector.
	Vector_3& operator+=(const Vector_3& v) { x() += v.x(); y() += v.y(); z() += v.z(); return *this; }

	/// Decrements the components of this vector by the components of another vector.
	Vector_3& operator-=(const Vector_3& v) { x() -= v.x(); y() -= v.y(); z() -= v.z(); return *this; }

	/// Multiplies each component of the vector with a scalar.
	Vector_3& operator*=(T s) { x() *= s; y() *= s; z() *= s; return *this; }

	/// Divides each component of the vector by a scalar.
	Vector_3& operator/=(T s) { x() /= s; y() /= s; z() /= s; return *this; }

	/// Sets all components of the vector to zero.
	Vector_3& operator=(Zero) { setZero(); return *this; }

	/// Sets all components of the vector to zero.
	void setZero() { this->fill(T(0)); }

	//////////////////////////// Component access //////////////////////////

	/// Returns the value of the X component of this vector.
	Q_DECL_CONSTEXPR T x() const { return (*this)[0]; }

	/// Returns the value of the Y component of this vector.
	Q_DECL_CONSTEXPR T y() const { return (*this)[1]; }

	/// Returns the value of the Z component of this vector.
	Q_DECL_CONSTEXPR T z() const { return (*this)[2]; }

	/// Returns a reference to the X component of this vector.
	T& x() { return (*this)[0]; }

	/// Returns a reference to the Y component of this vector.
	T& y() { return (*this)[1]; }

	/// Returns a reference to the Z component of this vector.
	T& z() { return (*this)[2]; }

	////////////////////////////////// Comparison ////////////////////////////////

	/// \brief Compares two vectors for exact equality.
	/// \return \c true if all components are equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const Vector_3& v) const { return (v.x()==x()) && (v.y()==y()) && (v.z()==z()); }

	/// \brief Compares two vectors for inequality.
	/// \return \c true if any of the components are not equal; \c false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const Vector_3& v) const { return (v.x()!=x()) || (v.y()!=y()) || (v.z()!=z()); }

	/// \brief Tests if the vector is the null vector, i.e. if all components are zero.
	/// \return \c true if all components are exactly zero; \c false otherwise
	Q_DECL_CONSTEXPR bool operator==(Zero) const { return (x()==T(0)) && (y()==T(0)) && (z()==T(0)); }

	/// \brief Tests if the vector is not a null vector, i.e. if any of the components is nonzero.
	/// \return \c true if any component is nonzero; \c false if all components are exactly zero.
	Q_DECL_CONSTEXPR bool operator!=(Zero) const { return (x()!=T(0)) || (y()!=T(0)) || (z()!=T(0)); }

	/// \brief Tests if two vectors are equal within a given tolerance.
	/// \param v The vector to compare to this vector.
	/// \param tolerance A non-negative threshold for the equality test. The two vectors are considered equal if
	///        the differences in the three components are all less than this tolerance value.
	/// \return \c true if this vector is equal to \a v within the given tolerance; \c false otherwise.
	Q_DECL_CONSTEXPR bool equals(const Vector_3& v, T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(v.x() - x()) <= tolerance && std::abs(v.y() - y()) <= tolerance && std::abs(v.z() - z()) <= tolerance;
	}

	/// \brief Test if the vector is zero within a given tolerance.
	/// \param tolerance A non-negative threshold.
	/// \return \c true if the absolute vector components are all smaller than \a tolerance.
	Q_DECL_CONSTEXPR bool isZero(T tolerance = T(FLOATTYPE_EPSILON)) const {
		return std::abs(x()) <= tolerance && std::abs(y()) <= tolerance && std::abs(z()) <= tolerance;
	}

	///////////////////////////////// Computations ////////////////////////////////

	/// Computes the inner dot product of this vector with the vector \a b.
	Q_DECL_CONSTEXPR T dot(const Vector_3& b) const { return x()*b.x() + y()*b.y() + z()*b.z(); }

	/// Computes the cross product of this vector with the vector \a b.
	Q_DECL_CONSTEXPR Vector_3 cross(const Vector_3& b) const {
		return Vector_3(y() * b.z() - z() * b.y(),
						z() * b.x() - x() * b.z(),
						x() * b.y() - y() * b.x());
	}

	/// Computes the squared length of the vector.
	Q_DECL_CONSTEXPR T squaredLength() const { return x()*x() + y()*y() + z()*z(); }

	/// Computes the length of the vector.
	Q_DECL_CONSTEXPR T length() const { return static_cast<T>(sqrt(squaredLength())); }

	/// \brief Normalizes this vector by dividing it by its length, making it a unit vector.
	/// \warning Do not call this function if the vector has length zero to avoid division by zero.
	/// In debug builds, a zero vector will be detected and reported. In release builds, the behavior is undefined.
	/// \sa normalized(), normalizeSafely(), resize()
	inline void normalize() {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector3::normalize", "Cannot normalize a vector of length zero.");
		*this /= length();
	}

	/// \brief Returns a normalized version of this vector.
	/// \return The unit vector.
	/// \warning Do not call this function if the vector has length zero to avoid division by zero.
	/// In debug builds, a zero vector will be detected and reported. In release builds, the behavior is undefined.
	/// \sa normalize(), normalizeSafely()
	inline Vector_3 normalized() const {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector3::normalize", "Cannot normalize a vector of length zero.");
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

	/// \brief Rescales this vector to the given length.
	/// \param len The new length of the vector.
	/// \warning Do not call this function if the vector has length zero to avoid division by zero.
	/// In debug builds, a zero vector will be detected and reported. In release builds, the behavior is undefined.
	/// \sa resized(), normalize(), normalized()
	inline void resize(T len) {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector3::resize", "Cannot resize a vector of length zero.");
		*this *= (len / length());
	}

	/// \brief Returns a copy of this vector having the given length.
	/// \param len The length of the vector to return. May be negative.
	/// \return The rescaled vector.
	/// \warning Do not call this function if the vector has length zero to avoid division by zero.
	/// In debug builds, a zero vector will be detected and reported. In release builds, the behavior is undefined.
	/// \sa resize(), normalized()
	inline Vector_3 resized(T len) const {
		OVITO_ASSERT_MSG(*this != Zero(), "Vector3::resized", "Cannot resize a vector of length zero.");
		return *this * (len / length());
	}

	///////////////////////////////// Utilities ////////////////////////////////

	/// \brief Returns the index of the component with the maximum value.
	Q_DECL_CONSTEXPR inline size_type maxComponent() const {
	    return ((x() >= y()) ? ((x() >= z()) ? 0 : 2) : ((y() >= z()) ? 1 : 2));
	}

	/// \brief Returns the index of the component with the minimum value.
	Q_DECL_CONSTEXPR inline size_type minComponent() const {
	    return ((x() <= y()) ? ((x() <= z()) ? 0 : 2) : ((y() <= z()) ? 1 : 2));
	}

	/// \brief Produces a string representation of the vector of the form (x y z).
	QString toString() const {
		return QString("(%1 %2 %3)").arg(x()).arg(y()).arg(z());
	}
};

/// \brief Computes the sum of two vectors.
/// \relates Vector_3
template<typename T>
Q_DECL_CONSTEXPR Vector_3<T> operator+(const Vector_3<T>& a, const Vector_3<T>& b) {
	return Vector_3<T>{ a.x() + b.x(), a.y() + b.y(), a.z() + b.z() };
}

/// \brief Computes the difference of two vectors.
/// \relates Vector_3
template<typename T>
Q_DECL_CONSTEXPR Vector_3<T> operator-(const Vector_3<T>& a, const Vector_3<T>& b) {
	return Vector_3<T>{ a.x() - b.x(), a.y() - b.y(), a.z() - b.z() };
}

/// \brief Computes the product of a vector and a scalar value.
/// \relates Vector_3
template<typename T>
Q_DECL_CONSTEXPR Vector_3<T> operator*(const Vector_3<T>& a, T s) {
	return Vector_3<T>{ a.x() * s, a.y() * s, a.z() * s };
}

/// \brief Computes the product of a scalar value and a vector.
/// \relates Vector_3
template<typename T>
Q_DECL_CONSTEXPR Vector_3<T> operator*(T s, const Vector_3<T>& a) {
	return Vector_3<T>{ a.x() * s, a.y() * s, a.z() * s };
}

/// \brief Computes the division of a vector by a scalar value.
/// \relates Vector_3
template<typename T, typename S>
Q_DECL_CONSTEXPR Vector_3<T> operator/(const Vector_3<T>& a, S s) {
	return Vector_3<T>{ a.x() / s, a.y() / s, a.z() / s };
}

/// \brief Writes a vector to a text output stream.
/// \relates Vector_3
template<typename T>
inline std::ostream& operator<<(std::ostream& os, const Vector_3<T>& v) {
	return os << "(" << v.x() << ", " << v.y()  << ", " << v.z() << ")";
}

/// \brief Writes a vector to a Qt debug stream.
/// \relates Vector_3
template<typename T>
inline QDebug operator<<(QDebug dbg, const Vector_3<T>& v) {
    dbg.nospace() << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
    return dbg.space();
}

/// \brief Writes a vector to a binary output stream.
/// \relates Vector_3
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Vector_3<T>& v) {
	return stream << v.x() << v.y() << v.z();
}

/// \brief Reads a vector from a binary input stream.
/// \relates Vector_3
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Vector_3<T>& v) {
	return stream >> v.x() >> v.y() >> v.z();
}

/// \brief Writes a vector to a Qt data stream.
/// \relates Vector_3
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Vector_3<T>& v) {
	return stream << v.x() << v.y() << v.z();
}

/// \brief Reads a vector from a Qt data stream.
/// \relates Vector_3
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Vector_3<T>& v) {
	return stream >> v.x() >> v.y() >> v.z();
}

/**
 * \brief Instantiation of the Vector_3 class template with the default floating-point type.
 * \relates Vector_3
 */
typedef Vector_3<FloatType>		Vector3;

/**
 * \brief Instantiation of the Vector_3 class template with the default integer type.
 * \relates Vector_3
*/
typedef Vector_3<int>			Vector3I;

inline void glVertex(const Vector_3<GLdouble>& v) { glVertex3dv(v.data()); }
inline void glVertex(const Vector_3<GLfloat>& v) { glVertex3fv(v.data()); }

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Vector3);
Q_DECLARE_METATYPE(Ovito::Vector3I);
Q_DECLARE_METATYPE(Ovito::Vector3*);
Q_DECLARE_METATYPE(Ovito::Vector3I*);
Q_DECLARE_TYPEINFO(Ovito::Vector3, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector3I, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector3*, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Vector3I*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_VECTOR3_H
