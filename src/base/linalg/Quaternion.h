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
 * \file Quaternion.h
 * \brief Contains definition of the Ovito::Quaternion class and operators.
 */

#ifndef __OVITO_QUATERNION_H
#define __OVITO_QUATERNION_H

#include <base/Base.h>

namespace Ovito {

template<typename T> class Matrix_34;

/**
 * \brief Represents a rotation in 3d space.
 *
 * Rotations can also be represented by the Rotation and the AffineTransformation
 * class. Please note that only the Rotation class is able to represent rotations with more than
 * one revolution, i.e. a rotation angle larger than 360 degrees.
 */
template<typename T>
class QuaternionT : public std::array<T, 4>
{
public:

	struct Identity {};

public:

	/////////////////////////////// Constructors /////////////////////////////////

	/// \brief Constructs a quaternion without initializing its components.
	/// \note All components are left uninitialized by this constructor and will therefore have an undefined value!
	QuaternionT() {}

	/// \brief Initializes the quaternion with the given values.
	/// \param _x The first quaternion component.
	/// \param _y The second quaternion component.
	/// \param _z The third quaternion component.
	/// \param _w The fourth quaternion component.
	Q_DECL_CONSTEXPR QuaternionT(T x, T y, T z, T w) : std::array<T, 4>{{x,y,z,w}} {}

	/// \brief Constructs an identity quaternion.
	/// The new quaternion represents the null transformation, i.e. no rotation at all.
	explicit Q_DECL_CONSTEXPR QuaternionT(Identity) : std::array<T, 4>{{ T(0), T(0), T(0), T(1) }} {}

	/// \brief Initializes the quaternion from rotational part of a transformation matrix.
	/// \param tm A rotation matrix.
	///
	/// It is assumed that \a tm is a pure rotation matrix.
	explicit QuaternionT(const Matrix_34<T>& tm);

	/// \brief Sets the quaternion to the identity quaternion.
	QuaternionT& setIdentity() {
		z() = y() = x() = T(0);
		w() = T(1);
		return *this;
	}

	/// \brief Sets the quaternion to the identity quaternion.
	QuaternionT& operator=(Identity) { return setIdentity(); }

    ///////////////////////////// Component access ///////////////////////////////

	/// \brief Returns the value of the X component of this quaternion.
	Q_DECL_CONSTEXPR const T& x() const { return (*this)[0]; }

	/// \brief Returns the value of the Y component of this quaternion.
	Q_DECL_CONSTEXPR const T& y() const { return (*this)[1]; }

	/// \brief Returns the value of the Z component of this quaternion.
	Q_DECL_CONSTEXPR const T& z() const { return (*this)[2]; }

	/// \brief Returns the value of the W component of this quaternion.
	Q_DECL_CONSTEXPR const T& w() const { return (*this)[3]; }

	/// \brief Returns a reference to the X component of this quaternion.
	T& x() { return (*this)[0]; }

	/// \brief Returns a reference to the Y component of this quaternion.
	T& y() { return (*this)[1]; }

	/// \brief Returns a reference to the Z component of this quaternion.
	T& z() { return (*this)[2]; }

	/// \brief Returns a reference to the W component of this quaternion.
	T& w() { return (*this)[3]; }

    /////////////////////////////// Unary operators //////////////////////////////

	/// \brief Negates all components of the quaternion.
	/// \return A new quaternion with all components negated.
	/// \note The returned quaternion does not represent the inverse rotation!
	/// \sa inverse()
	Q_DECL_CONSTEXPR QuaternionT operator-() const { return {-x(), -y(), -z(), -w()}; }

	/// \brief Returns the inverse (or conjugate) of this rotation.
	/// \return A new quaternion representing the inverse rotation of this quaternion.
	Q_DECL_CONSTEXPR QuaternionT  inverse() const { return { -x(), -y(), -z(), w() }; }

	////////////////////////////////// Comparison ////////////////////////////////

	/// \brief Compares two quaternions for equality.
	/// \param q The quaternion to compare with.
	/// \return \c true if each of the components are equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const QuaternionT& q) const { return (q.x() == x() && q.y() == y() && q.z() == z() && q.w() == w()); }

	/// \brief Compares two quaternions for inequality.
	/// \param q The quaternion to compare with.
	/// \return \c true if any of the components are not equal; \c false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const QuaternionT& q) const { return !(q == *this); }

	///////////////////////////////// Computations ////////////////////////////////

	/// \brief Multiplies each component of the quaternion with a scalar value and stores the result in this quaternion.
	/// \param s The scalar value to multiply this quaternion with.
	/// \return A reference to \c this quaternion, which has been changed.
	QuaternionT& operator*=(T s) { x() *= s; y() *= s; z() *= s; w() *= s; return *this; }

	/// \brief Divides each component of the quaternion by a scalar value and stores the result in this quaternion.
	/// \param s The scalar value.
	/// \return A reference to \c this quaternion, which has been changed.
	QuaternionT& operator/=(T s) { x() /= s; y() /= s; z() /= s; w() /= s; return *this; }

	/// \brief Computes the scalar product of two quaternions.
	Q_DECL_CONSTEXPR T dot(const QuaternionT& b) const { return x()*b.y() + y()*b.y() + z()*b.z() + w()*b.w(); }

	/// \brief Normalizes this quaternion to unit length.
	inline void normalize() {
		T c = sqrt(dot(*this));
		OVITO_ASSERT_MSG(c > 0, "Quaternion::normalize", "Cannot normalize the null quaternion.");
		x() /= c; y() /= c; z() /= c; w() /= c;
	}

	/// \brief Returns a Normalized version of this quaternion.
	inline QuaternionT normalized() const {
		T c = sqrt(dot(*this));
		OVITO_ASSERT_MSG(c > 0, "Quaternion::normalized", "Cannot normalize the null quaternion.");
		return { x() / c, y() / c, z() / c, w() / c };
	}

	///////////////////////////////// Interpolation //////////////////////////////

	/// \brief Interpolates between two quaternions using spherical linear interpolation.
	/// \param q1 The first rotation.
	/// \param q2 The second rotation.
	/// \param alpha The parameter for the linear interpolation in the range [0,1].
	/// \return A linear interpolation between \a q1 and \a q2.
    static QuaternionT interpolate(const QuaternionT& q1, const QuaternionT& q2, T alpha) {
    	OVITO_ASSERT_MSG(std::abs(q1.dot(q1) - T(1)) <= T(FLOATTYPE_EPSILON), "Quaternion::interpolate", "Quaternions must be normalized.");
    	OVITO_ASSERT_MSG(std::abs(q2.dot(q2) - T(1)) <= T(FLOATTYPE_EPSILON), "Quaternion::interpolate", "Quaternions must be normalized.");

    	T cos_t = q1.dot(q2);

    	// Same quaternion? (avoid domain error)
    	if(T(1) <= std::abs(cos_t))
    		return q1;

    	// t is now theta
    	T theta = std::acos(cos_t);
    	T sin_t = std::sin(theta);

    	// Same quaternion? (avoid zero-div)
    	if(sin_t == 0)
    		return q1;

    	T s = std::sin((T(1)-alpha)*theta)/sin_t;
    	T t = std::sin(alpha*theta)/sin_t;

    	QuaternionT res(s*q1.x() + t*q2.x(), s*q1.y() + t*q2.y(), s*q1.z() + t*q2.z(), s*q1.w() + t*q2.w());
    	res.normalize();
    	return res;
    }

	/// \brief Interpolates between two quaternions using spherical quadratic interpolation.
	/// \param q1 The first rotation (at t==0.0).
	/// \param q2 The second rotation (at t==1.0).
	/// \param out Controls the tangential direction at \a q1.
	/// \param in Controls the tangential direction at \a q2.
	/// \param alpha The interpolation parameter in the range [0,1].
	/// \return The interpolated quaternion between \a q1 and \a q2.
	static QuaternionT interpolateQuad(const QuaternionT& q1, const QuaternionT& q2, const QuaternionT& out, const QuaternionT& in, T alpha) {
		QuaternionT slerpP = interpolate(q1, q2, alpha);
		QuaternionT slerpQ = interpolate(out, in, alpha);
		T Ti = 2 * alpha * (1 - alpha);
		return interpolate(slerpP, slerpQ, Ti);
	}

	////////////////////////////////// Utilities /////////////////////////////////

	/// \brief Returns a string representation of this quaternion.
	/// \return A string that contains the four components of the quaternion.
	QString toString() const {
		return QString("[%1 %2 %3 %4]").arg(x()).arg(y()).arg(z()).arg(w());
	}
};

};

#include "Vector3.h"
#include "Vector4.h"
#include "AffineTransformation.h"
#include "Matrix3.h"

namespace Ovito {

/// \brief Initializes the quaternion from rotational part of a transformation matrix.
/// \param tm A rotation matrix.
///
/// It is assumed that \a tm is a pure rotation matrix.
template<typename T>
inline QuaternionT<T>::QuaternionT(const Matrix_34<T>& tm)
{
	// Make sure this is a pure rotation matrix.
    OVITO_ASSERT_MSG(tm.isRotationMatrix(), "Quaternion constructor" , "Quaternion::Quaternion(const AffineTransformation& tm) accepts only pure rotation matrices.");

	// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".
    T trace = tm(0,0) + tm(1,1) + tm(2,2);
	if(trace > 0) {
		T root = sqrt(trace + 1.0);
		w() = 0.5 * root;
		root = 0.5 / root;
		x() = (tm(2,1) - tm(1,2)) * root;
		y() = (tm(0,2) - tm(2,0)) * root;
		z() = (tm(1,0) - tm(0,1)) * root;
	}
	else {
		static const typename Matrix_34<T>::size_type next[] = { 1, 2, 0 };
		typename Matrix_34<T>::size_type i = 0;
		if(tm(1,1) > tm(0,0)) i = 1;
		if(tm(2,2) > tm(i,i)) i = 2;
		typename Matrix_34<T>::size_type j = next[i];
		typename Matrix_34<T>::size_type k = next[j];
		T root = sqrt(tm(i,i) - tm(j,j) - tm(k,k) + 1.0);
		(*this)[i] = 0.5 * root;
		root = 0.5 / root;
		w() = (tm(k,j) - tm(j,k)) * root;
		(*this)[j] = (tm(j,i) + tm(i,j)) * root;
		(*this)[k] = (tm(k,i) + tm(i,k)) * root;
	}

	// Since we represent a rotation, make sure we are unit length.
	OVITO_ASSERT(std::abs(dot(*this)-T(1)) <= T(FLOATTYPE_EPSILON));
}

/// \brief Multiplies two quaternions.
/// \param a The first rotation.
/// \param b The second rotation.
/// \return A new rotation that is equal to first applying rotation \a b and then applying rotation \a a.
template<typename T>
inline QuaternionT<T> operator*(const QuaternionT<T>& a, const QuaternionT<T>& b)
{
	return {
		a.w()*b.x() + a.x()*b.w() + a.y()*b.z() - a.z()*b.y(),
		a.w()*b.y() + a.y()*b.w() + a.z()*b.x() - a.x()*b.z(),
		a.w()*b.z() + a.z()*b.w() + a.x()*b.y() - a.y()*b.x(),
		a.w()*b.w() - a.x()*b.x() - a.y()*b.y() - a.z()*b.z() };
}

/// \brief Rotates a vector.
/// \param q The rotation.
/// \param v The vector.
/// \return The rotated vector v.
template<typename T>
inline Vector_3<T> operator*(const QuaternionT<T>& q, const Vector_3<T>& v)
{
	OVITO_ASSERT_MSG(std::fabs(q.dot(q) - T(1)) <= T(FLOATTYPE_EPSILON), "Vector rotation", "Quaternion must be normalized.");
	return Matrix3(1.0 - 2.0*(q.y()*q.y() + q.z()*q.z()),       2.0*(q.x()*q.y() - q.w()*q.z()),       2.0*(q.x()*q.z() + q.w()*q.y()),
			         2.0*(q.x()*q.y() + q.w()*q.z()), 1.0 - 2.0*(q.x()*q.x() + q.z()*q.z()),       2.0*(q.y()*q.z() - q.w()*q.x()),
		             2.0*(q.x()*q.z() - q.w()*q.y()),       2.0*(q.y()*q.z() + q.w()*q.x()), 1.0 - 2.0*(q.x()*q.x() + q.y()*q.y())) * v;
}

/// \brief Writes a quaternion to a text output stream.
/// \param os The output stream.
/// \param q The quaternion to write to the output stream \a os.
/// \return The output stream \a os.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const QuaternionT<T>& q) {
	return os << '[' << q.x() << ' ' << q.y() << ' ' << q.z() << ' ' << q.w() << ']';
}


/// \brief Writes the quaternion to the Qt debug stream.
template<typename T>
inline QDebug operator<<(QDebug dbg, const QuaternionT<T>& q) {
    dbg.nospace() << "[" << q.x() << ", " << q.y() << ", " << q.z() << ", " << q.w() << "]";
    return dbg.space();
}

/// \brief Writes a Quaternion to a binary output stream.
/// \param stream The output stream.
/// \param q The quaternion to write to the output stream \a stream.
/// \return The output stream \a stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const QuaternionT<T>& q) {
	return stream << q.x() << q.y() << q.z() << q.w();
}

/// \brief Reads a Quaternion from a binary input stream.
/// \param stream The input stream.
/// \param q Reference to a quaternion variable where the parsed data will be stored.
/// \return The input stream \a stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, QuaternionT<T>& q) {
	return stream >> q.x() >> q.y() >> q.z() >> q.w();
}

/**
 * \fn typedef Quaternion
 * \brief Template class instance of the QuaternionT class used for floating-point quaternions.
 */
typedef QuaternionT<FloatType>		Quaternion;

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Quaternion)
Q_DECLARE_TYPEINFO(Ovito::Quaternion, Q_PRIMITIVE_TYPE);

#endif // __OVITO_QUATERNION_H
