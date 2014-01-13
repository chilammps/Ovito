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
 * \file Rotation.h
 * \brief Contains definition of the Ovito::Rotation class and operators.
 */

#ifndef __OVITO_ROTATION_H
#define __OVITO_ROTATION_H

#include <base/Base.h>
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix3.h"

namespace Ovito {

/**
 * \brief This describes a three-dimensional rotation in space using an axis and an angle.
 *
 * The rotation is defined by an unit vector that specifies the axis of rotation
 * and an angle.
 *
 * Rotations can also be described by the Quaternion and the AffineTransformation
 * class but only the Rotation class is able to represent rotations with more than
 * one revolution, i.e. rotation angles larger than 360 degrees.
 */
template<typename T>
class RotationT
{
private:

	/// \brief The axis of rotation. This is a unit vector.
	Vector_3<T> _axis;

	/// \brief The rotation angle in radians.
	T _angle;

public:

	struct Identity {};

public:

	/////////////////////////////// Constructors /////////////////////////////////

	/// \brief Constructs a Rotation object without initializing its components.
	/// \note The axis and the angle are left uninitialized by this constructor and will therefore have an undefined value!
	RotationT() {}

	/// \brief Constructor that builds up a rotation from an axis and an angle.
	/// \param _axis The vector that specifies the rotation axis. It does not have to be a unit vector.
	///              It is automatically normalized by the Rotation constructor.
	/// \param _angle The rotation angle in radians.
	Q_DECL_CONSTEXPR RotationT(const Vector_3<T>& axis, T angle, bool normalize = true) : _axis(normalize ? axis.normalized() : axis), _angle(angle) {}

	/// \brief Initializes the object to the null rotation.
	/// The axis is initialized with the (0,0,1) vector and the angle is set to zero.
	Q_DECL_CONSTEXPR RotationT(Identity) : _axis{T(0),T(0),T(1)}, _angle(T(0)) {}

	/// \brief Initializes the object from rotational part of the matrix.
	/// \param tm A rotation matrix.
	///
	/// It is assumed that \a tm is a pure rotation matrix.
	/// The calculated rotation angle will be in the range [-pi, +pi].
    explicit RotationT(const Matrix_34<T>& tm) {
    	_axis.x() = tm(2,1) - tm(1,2);
    	_axis.y() = tm(0,2) - tm(2,0);
    	_axis.z() = tm(1,0) - tm(0,1);
    	if(_axis == typename Vector_3<T>::Zero()) {
    		_angle = T(0);
    		_axis = Vector_3<T>(0, 0, 1);
    	}
    	else {
    		T trace = tm(0,0) + tm(1,1) + tm(2,2) - T(1);
    		T s = _axis.length();
    		_axis /= s;
    		_angle = atan2(s, trace);
    	}
    }

	/// \brief Initializes the object from a quaternion.
	/// \param q The input rotation.
	///
	/// The calculated rotation angle will be in the range [0, 2*pi].
	explicit RotationT(const QuaternionT<T>& q) {
		T scaleSquared = q.x()*q.x() + q.y()*q.y() + q.z()*q.z();
		if(scaleSquared <= T(FLOATTYPE_EPSILON)) {
			_angle = T(0);
			_axis = Vector_3<T>(0, 0, 1);
		}
		else {
			if(q.w() < T(-1))
				_angle = T(M_PI) * T(2);
			else if(q.w() > T(1))
				_angle = T(0);
			else
				_angle = acos(q.w()) * T(2);
			_axis = Vector_3<T>(q.x(), q.y(), q.z()) / (T)sqrt(scaleSquared);
			OVITO_ASSERT(std::abs(_axis.squaredLength() - T(1)) <= T(FLOATTYPE_EPSILON));
		}
	}

	/// \brief Constructs a rotation that rotates one vector into a second vector.
	/// \param a The vector to be rotated. Can be of any length but must not be the null vector.
	/// \param b The target vector. Can be of any length but must not be the null vector.
	RotationT(const Vector_3<T>& a, const Vector_3<T>& b) {
		Vector_3<T> an = a.normalized();
		Vector_3<T> bn = b.normalized();
		T cos = an.dot(bn);
		if(cos > T(1) - T(FLOATTYPE_EPSILON)) {
			_angle = 0;
			_axis = Vector_3<T>(0,0,1);
		}
		else if(cos < T(-1) + T(FLOATTYPE_EPSILON)) {
			_angle = T(M_PI);
			_axis = Vector_3<T>(0,0,1);
		}
		else {
			_angle = acos(cos);
			_axis = a.cross(b).normalized();
		}
	}

	/////////////////////////////// Component access //////////////////////////////

	/// \brief Returns the axis of rotation.
	Q_DECL_CONSTEXPR const Vector_3<T>& axis() const { return _axis; }

	/// \brief Returns the angle of rotation.
	Q_DECL_CONSTEXPR T angle() const { return _angle; }

	/// \brief Changes the axis of rotation.
	void setAxis(const Vector_3<T>& axis) { _axis = axis; }

	/// \brief Changes the angle of rotation.
	void setAngle(T angle) { _angle = angle; }

	/////////////////////////////// Unary operators //////////////////////////////

	/// \brief Returns the inverse of this rotation.
	/// \return A rotation with the same axis but negative rotation angle.
	Q_DECL_CONSTEXPR RotationT inverse() const  { return RotationT(_axis, -_angle, false); }

	/// \brief Converts the rotation to a Quaternion.
	/// \return A quaternion that represents the same rotation.
	///
	/// Please note that any extra revolutions are lost by this conversion.
	explicit operator QuaternionT<T>() const {
		T omega = _angle * T(0.5);
		T s = sin(omega);
		return QuaternionT<T>(_axis.x() * s, _axis.y() * s, _axis.z() * s, cos(omega)).normalized();
	}

	/////////////////////////////// Binary operators /////////////////////////////

	/// \brief Adds the given rotation to this rotation.
	/// \param r2 The rotation to add to this rotation.
	/// \return This resulting rotation which is equal to \c r2*(*this).
	RotationT& operator+=(const RotationT& r2) { *this = r2 * (*this); return *this; }

	/// \brief Adds the inverse of another rotation to this rotation.
	/// \param r2 The rotation to subtract from this rotation.
	/// \return This resulting rotation which is equal to \c (*this)*r2.inverse().
	RotationT& operator-=(const RotationT& r2) { *this = (*this) * r2.inverse(); return *this; }

	/// \brief Sets the rotation to the identity rotation.
	RotationT& setIdentity() {
		_axis = Vector_3<T>(T(0),T(0),T(1));
		_angle = T(0);
		return *this;
	}

	/// \brief Sets the rotation to the identity rotation.
	RotationT& operator=(Identity) { return setIdentity(); }

	////////////////////////////////// Comparison ////////////////////////////////

	/// \brief Returns whether two rotations are the same.
	/// \param r The rotation to compare with.
	/// \return \c true if the axis and the angle of both rotations are either equal or opposite;
	///         \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const RotationT& r) const { return ((r._axis==_axis) && (r._angle==_angle)) || ((r._axis==-_axis) && (r._angle==-_angle)); }

	/// \brief Returns whether two rotations are the not same.
	/// \param r The rotation to compare with.
	/// \return \c true if the axis or the angle of both rotations are neither equal or opposite;
	///         \c false otherwise.
	Q_DECL_CONSTEXPR bool operator!=(const RotationT& r) const { return ((r._axis!=_axis) || (r._angle!=_angle)) && ((r._axis!=-_axis) || (r._angle!=-_angle)); }

	/// \brief Returns whether the rotation angle is zero.
	/// \return \c true if the rotation angle is zero; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(Identity) const { return (_angle == T(0)); }

	/// \brief Returns whether the rotation angle is not zero.
	/// \return \c true if the rotation angle is not zero; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator!=(Identity) const { return (_angle != T(0)); }

	/// \brief Checks whether two rotations are equal within a given tolerance.
	/// \param r The rotation that should be compared to this rotation.
	/// \param tolerance A non-negative threshold for the equality test. The two rotations are considered equal when
	///        the differences in the X, Y, and Z components of the rotation axis and the angle are all smaller than this tolerance value.
	///        Two rotations with equal but opposite axis and angle are considered equal.
	/// \return true if this rotation is equal to the given rotation within the given tolerance.
	Q_DECL_CONSTEXPR bool equals(const RotationT& r, T tolerance = T(FLOATTYPE_EPSILON)) const {
		return (std::abs(angle() - r.angle()) <= tolerance && axis().equals( r.axis(), tolerance)) ||
			   (std::abs(angle() + r.angle()) <= tolerance && axis().equals(-r.axis(), tolerance));
	}

	///////////////////////////////// Interpolation //////////////////////////////

	/// \brief Interpolates between the two rotations using spherical linear interpolation and handles multiple revolutions.
	/// \param rot1 The first rotation.
	/// \param rot2 The second rotation.
	/// \param t The parameter for the linear interpolation in the range [0,1].
	/// \return A linear interpolation between \a rot1 and \a rot2.
    static RotationT interpolate(const RotationT& rot1, const RotationT& rot2, T t) {
    	OVITO_ASSERT(t >= 0 && t <= 1);

    	RotationT _rot2;
    	if(rot1.axis().dot(rot2.axis()) < T(0))
    		_rot2 = RotationT(-rot2.axis(), -rot2.angle(), false);
    	else
    		_rot2 = rot2;

    	// Determine interpolation type, compute extra spins, and adjust angles accordingly.
		if(rot1.axis().equals(_rot2.axis())) {
			return RotationT((T(1) - t) * rot1.axis() + t * _rot2.axis(), (T(1) - t) * rot1.angle() + t * _rot2.angle());
		}
		else if(rot1.angle() != T(0)) {
			T fDiff = _rot2.angle() - rot1.angle();
			T fDiffUnit = fDiff/T(2*M_PI);
			int extraSpins = (int)floor(fDiffUnit + T(0.5));
			if(extraSpins * fDiffUnit * (fDiffUnit - extraSpins) < 0)
				extraSpins = -extraSpins;

	    	QuaternionT<T> q1 = (QuaternionT<T>)rot1;
	    	QuaternionT<T> q2 = (QuaternionT<T>)_rot2;

	    	// Eliminate any non-acute angles between quaternions. This
	    	// is done to prevent potential discontinuities that are the result of
	    	// invalid intermediate value quaternions.
	    	if(q1.dot(q2) < T(0))
	    		q2 = -q2;

	    	// Clamp identity quaternions so that |w| <= 1 (avoids problems with
	    	// call to acos() in slerpExtraSpins).
	    	if(q1.w() < T(-1)) q1.w() = T(-1); else if(q1.w() > T(1)) q1.w() = T(1);
	    	if(q2.w() < T(-1)) q2.w() = T(-1); else if(q2.w() > T(1)) q2.w() = T(1);

			RotationT result = RotationT(slerpExtraSpins(t, q1, q2, extraSpins));
			if(result.axis().dot(interpolateAxis(t, rot1.axis(), _rot2.axis())) < T(0))
				result = RotationT(-result.axis(), -result.angle(), false);
			int nrev = floor((t * _rot2.angle() + (T(1) - t) * rot1.angle() - result.angle())/T(2*M_PI) + T(0.5));
			result.addRevolutions(nrev);
			return result;
		}
		else {
			return RotationT(interpolateAxis(t, rot1.axis(), _rot2.axis()), (T(1) - t) * rot1.angle() + t * _rot2.angle());
		}
    }

	/// \brief Interpolates between the two rotations using spherical quadratic interpolation.
	/// \param rot1 The first rotation (at t == 0).
	/// \param rot2 The second rotation (at t == 1).
	/// \param out Controls the tangential direction at \a rot1.
	/// \param in Controls the tangential direction at \a rot2.
	/// \param t The interpolation parameter in the range [0,1].
	/// \return The interpolated rotation between \a rot1 and \a rot2.
    static RotationT interpolateQuad(const RotationT& rot1, const RotationT& rot2, const RotationT& out, const RotationT& in, T t) {
    	RotationT slerpP = interpolate(rot1, rot2, t);
    	RotationT slerpQ = interpolate(out, in, t);
    	T Ti = T(2) * t * (T(1) - t);
    	return interpolate(slerpP, slerpQ, Ti);
    }

	/// \brief Constructs a rotation from three Euler angles.
	static RotationT fromEuler(const Vector_3<T>& eulerAngles, typename Matrix_3<T>::EulerAxisSequence axisSequence) {
		OVITO_ASSERT(axisSequence == Matrix_3<T>::szyx);
		return RotationT(Vector3(1,0,0), eulerAngles[2]) * RotationT(Vector3(0,1,0), eulerAngles[1]) * RotationT(Vector3(0,0,1), eulerAngles[0]);
	}

	/// \brief Converts the rotation to three Euler angles.
	Vector_3<T> toEuler(typename Matrix_3<T>::EulerAxisSequence axisSequence) const {
		if(*this == Identity()) return typename Vector_3<T>::Zero();
		Vector_3<T> euler = Matrix_3<T>::rotation(*this).toEuler(axisSequence);

		// Handles rotations with multiple revolutions.
		// Since the Euler-angle decomposition routine cannot handle this case directly,
		// we have to determine the correct revolution number for each Euler axis in a trial-and-error
		// fashion. To this end, we test all possible combinations of revolutions until
		// we the one that yields the original axis-angle rotation. Multiple equivalent decompositions
		// are ranked, because we prefer Euler decompositions that rotate only about a single axis.
		int maxRevolutions = (int)std::floor(std::abs(angle()) / T(M_PI*2) + T(0.5 + FLOATTYPE_EPSILON));
		if(maxRevolutions == 0) return euler;
		Vector_3<T> bestDecomposition = euler;
		int bestDecompositionRanking = -1;
		for(int xr = -maxRevolutions; xr <= maxRevolutions; xr++) {
			Vector_3<T> euler2;
			euler2.x() = euler.x() + T(M_PI*2) * xr;
			int maxRevolutionsY = maxRevolutions - std::abs(xr);
			for(int yr = -maxRevolutionsY; yr <= maxRevolutionsY; yr++) {
				euler2.y() = euler.y() + T(M_PI*2) * yr;
				int maxRevolutionsZ = maxRevolutionsY - std::abs(yr);
				for(int zr = -maxRevolutionsZ; zr <= maxRevolutionsZ; zr++) {
					euler2.z() = euler.z() + T(M_PI*2) * zr;
					if(equals(fromEuler(euler2, axisSequence))) {
						int ranking = int(std::abs(euler2.x()) <= T(FLOATTYPE_EPSILON)) + int(std::abs(euler2.y()) <= T(FLOATTYPE_EPSILON)) + int(std::abs(euler2.z()) <= T(FLOATTYPE_EPSILON));
						if(ranking > bestDecompositionRanking) {
							bestDecomposition = euler2;
							bestDecompositionRanking = ranking;
						}
					}
				}
			}
		}
		return bestDecomposition;
	}

    ////////////////////////////////// Utilities /////////////////////////////////

	/// \brief Returns the number of revolutions.
	/// \return The rounded value of \c angle divided by 2*pi.
	/// \sa setRevolutions()
	/// \sa addRevolutions()
	Q_DECL_CONSTEXPR int revolutions() const { return (int)(_angle/T(M_PI*2)); }

	/// \brief Sets the number of revolutions.
	/// \param n The new number of revolutions. This can be negative.
	/// \sa revolutions()
	/// \sa addRevolutions()
	void setRevolutions(int n) { _angle = std::fmod(_angle, T(2*M_PI)) + (T(2*M_PI)*n); }

	/// \brief Adds the given number of revolutions.
	/// \param n The number of revolutions to add to the angle. This can be negative.
	///
	/// The rotation angle is increased by \c n*2*pi.
	/// \sa revolutions()
	void addRevolutions(int n) { _angle += T(2*M_PI) * n; }

	/// \brief Returns a string representation of this rotation.
	/// \return A string that contains the components of the rotation structure.
	QString toString() const {
		return QStringLiteral("[Axis: %1 Angle: %2]").arg(axis().toString()).arg(angle());
	}

private:

	static inline Vector_3<T> interpolateAxis(T time, const Vector_3<T>& axis0, const Vector_3<T>& axis1) {
		// assert:  axis0 and axis1 are unit length
		// assert:  axis0.dot(axis1) >= 0
		// assert:  0 <= time <= 1

		T cos = axis0.dot(axis1);  // >= 0 by assertion
		OVITO_ASSERT(cos >= T(0));
		if(cos > T(1)) cos = T(1); // round-off error might create problems in acos call

		T angle = acos(cos);
		T invSin = T(1) / sin(angle);
		T timeAngle = time * angle;
		T coeff0 = sin(angle - timeAngle) * invSin;
		T coeff1 = sin(timeAngle) * invSin;

		return (coeff0 * axis0 + coeff1 * axis1);
	}

	static inline QuaternionT<T> slerpExtraSpins(T t, const QuaternionT<T>& p, const QuaternionT<T>& q, int iExtraSpins) {
		T fCos = p.dot(q);
		OVITO_ASSERT(fCos >= T(0));

		// Numerical round-off error could create problems in call to acos.
		if(fCos < T(-1)) fCos = T(-1);
		else if(fCos > T(1)) fCos = T(1);

		T fAngle = acos(fCos);
		T fSin = sin(fAngle);  // fSin >= 0 since fCos >= 0

		if(fSin < T(1e-3)) {
			return p;
		}
		else {
			T fPhase = T(M_PI) * (T)iExtraSpins * t;
			T fInvSin = T(1) / fSin;
			T fCoeff0 = sin((T(1) - t) * fAngle - fPhase) * fInvSin;
			T fCoeff1 = sin(t * fAngle + fPhase) * fInvSin;
			return QuaternionT<T>(fCoeff0*p.x() + fCoeff1*q.x(), fCoeff0*p.y() + fCoeff1*q.y(),
			                        fCoeff0*p.z() + fCoeff1*q.z(), fCoeff0*p.w() + fCoeff1*q.w());
		}
	}
};

/// \brief Performs the multiplication of two rotations.
/// \param r1 The first rotation.
/// \param r2 The second rotation.
/// \return A new rotation that is equal to first applying \a r2 and then applying \a r1.
template<typename T>
inline RotationT<T> operator*(const RotationT<T>& r1, const RotationT<T>& r2) {
	if(r1 == typename RotationT<T>::Identity()) return r2;
	if(r2 == typename RotationT<T>::Identity()) return r1;
	QuaternionT<T> q1 = (QuaternionT<T>)r1;
	QuaternionT<T> q2 = (QuaternionT<T>)r2;
	QuaternionT<T> q = q1 * q2;
	RotationT<T> result(q);
	int rev;
	if(r1.axis().dot(r2.axis()) >= T(0))
		rev = (int)floor(((r1.angle()+r2.angle()) / T(M_PI*2)));
	else
		rev = (int)floor(((r1.angle()-r2.angle()) / T(M_PI*2)));
	if((rev & 1) != 0) {
		result.setAngle(-result.angle());
		rev++;
		result.setAxis(-result.axis());
	}
	result.addRevolutions(rev);
	return result;
}

/// \brief Writes the Rotation to a text output stream.
/// \param os The output stream.
/// \param r The rotation to write to the output stream \a os.
/// \return The output stream \a os.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const RotationT<T>& r) {
	return os << '[' << r.axis().x() << ' ' << r.axis().y()  << ' ' << r.axis().z() << "], " << r.angle();
}

/// \brief Writes the rotation to the Qt debug stream.
template<typename T>
inline QDebug operator<<(QDebug dbg, const RotationT<T>& r) {
    dbg.nospace() << "[" << r.axis().x() << ", " << r.axis().y() << ", " << r.axis().z() << "], " << r.angle();
    return dbg.space();
}


/// \brief Writes a Rotation to a binary output stream.
/// \param stream The output stream.
/// \param r The rotation to write to the output stream \a stream.
/// \return The output stream \a stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const RotationT<T>& r)
{
	return stream << r.axis() << r.angle();
}

/// \brief Reads a Rotation from a binary input stream.
/// \param stream The input stream.
/// \param r Reference to a rotation variable where the parsed data will be stored.
/// \return The input stream \a stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, RotationT<T>& r)
{
	Vector_3<T> axis;
	T angle;
	stream >> axis >> angle;
	r.setAxis(axis);
	r.setAngle(angle);
	return stream;
}

/**
 * \fn typedef Rotation
 * \brief Template class instance of the RotationT template.
 */
typedef RotationT<FloatType>		Rotation;

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Rotation)
Q_DECLARE_TYPEINFO(Ovito::Rotation, Q_PRIMITIVE_TYPE);

#endif // __OVITO_ROTATION_H
