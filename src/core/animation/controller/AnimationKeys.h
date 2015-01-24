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

#ifndef __OVITO_ANIMATION_KEYS_H
#define __OVITO_ANIMATION_KEYS_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include "Controller.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

/**
 * \brief Base class for animation keys.
 */
class OVITO_CORE_EXPORT AnimationKey : public RefTarget
{
public:

	/// Constructor.
	AnimationKey(DataSet* dataset, TimePoint time = 0) : RefTarget(dataset), _time(time) {
		INIT_PROPERTY_FIELD(AnimationKey::_time);
	}

	/// Returns the animation time at which the key is set.
	TimePoint time() const { return _time; }

private:

	/// Changes the key's time position.
	void setTime(TimePoint newTime) { _time = newTime; }

	/// The animation time at which the key is positioned.
	PropertyField<TimePoint> _time;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_time);

	friend class KeyframeController;
};

/**
 * \brief Base template class for animation keys.
 */
template<typename ValueType, typename NullValue = ValueType, typename TangentType = ValueType>
class TypedAnimationKey : public AnimationKey
{
public:

	/// The type of value stored by this animation key.
	typedef ValueType value_type;

	/// The type used to initialize default values of this key.
	typedef NullValue nullvalue_type;

	/// The type used for derivatives/tangents.
	typedef TangentType tangent_type;

	/// Constructor.
	TypedAnimationKey(DataSet* dataset, TimePoint time = 0, const value_type& value = nullvalue_type()) : AnimationKey(dataset, time), _value(value) {}

	/// Returns the value of this animation key.
	const value_type& value() const { return _value; }

protected:

	/// Changes the key's value.
	void setValue(const value_type& newValue) { _value = newValue; }

	/// The key's value.
	PropertyField<value_type> _value;

	friend class KeyframeController;
};


/**
 * \brief Animation key class for float controllers.
 */
class OVITO_CORE_EXPORT FloatAnimationKey : public TypedAnimationKey<FloatType>
{
public:

	/// Constructor.
	Q_INVOKABLE FloatAnimationKey(DataSet* dataset, TimePoint time = 0, FloatType value = 0) : TypedAnimationKey<FloatType>(dataset, time, value) {
		INIT_PROPERTY_FIELD(FloatAnimationKey::_value);
	}

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_value);
};

/**
 * \brief Animation key class for integer controllers.
 */
class OVITO_CORE_EXPORT IntegerAnimationKey : public TypedAnimationKey<int>
{
public:

	/// Constructor.
	Q_INVOKABLE IntegerAnimationKey(DataSet* dataset, TimePoint time = 0, int value = 0) : TypedAnimationKey<int>(dataset, time, value) {
		INIT_PROPERTY_FIELD(IntegerAnimationKey::_value);
	}

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_value);
};

/**
 * \brief Animation key class for Vector3 controllers.
 */
class OVITO_CORE_EXPORT Vector3AnimationKey : public TypedAnimationKey<Ovito::Vector3, Ovito::Vector3::Zero>
{
public:

	/// Constructor.
	Q_INVOKABLE Vector3AnimationKey(DataSet* dataset, TimePoint time = 0, const Vector3& value = Vector3::Zero()) : TypedAnimationKey<Vector3, Vector3::Zero>(dataset, time, value) {
		INIT_PROPERTY_FIELD(Vector3AnimationKey::_value);
	}

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_value);
};

/**
 * \brief Animation key class for position controllers.
 */
class OVITO_CORE_EXPORT PositionAnimationKey : public TypedAnimationKey<Ovito::Vector3, Ovito::Vector3::Zero>
{
public:

	/// Constructor.
	Q_INVOKABLE PositionAnimationKey(DataSet* dataset, TimePoint time = 0, const Vector3& value = Vector3::Zero()) : TypedAnimationKey<Vector3, Vector3::Zero>(dataset, time, value) {
		INIT_PROPERTY_FIELD(PositionAnimationKey::_value);
	}

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_value);
};

/**
 * \brief Animation key class for rotation controllers.
 */
class OVITO_CORE_EXPORT RotationAnimationKey : public TypedAnimationKey<Ovito::Rotation, Ovito::Rotation::Identity>
{
public:

	/// Constructor.
	Q_INVOKABLE RotationAnimationKey(DataSet* dataset, TimePoint time = 0, const Rotation& value = Rotation::Identity()) : TypedAnimationKey<Rotation, Rotation::Identity>(dataset, time, value) {
		INIT_PROPERTY_FIELD(RotationAnimationKey::_value);
	}

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_value);
};

/**
 * \brief Animation key class for scaling controllers.
 */
class OVITO_CORE_EXPORT ScalingAnimationKey : public TypedAnimationKey<Ovito::Scaling, Ovito::Scaling::Identity>
{
public:

	/// Constructor.
	Q_INVOKABLE ScalingAnimationKey(DataSet* dataset, TimePoint time = 0, const Scaling& value = Scaling::Identity()) : TypedAnimationKey<Scaling, Scaling::Identity>(dataset, time, value) {
		INIT_PROPERTY_FIELD(ScalingAnimationKey::_value);
	}

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_value);
};

/**
 * \brief Default implementation of the value interpolator concept that does linear interpolation.
 *
 * This template class interpolates linearly between two values of arbitrary types.
 * The value 0.0 of the interpolation parameter \c t is mapped to the first value.
 * The value 1.0 of the interpolation parameter \c t is mapped to the second value.
 */
template<typename ValueType>
struct LinearValueInterpolator {
	ValueType operator()(FloatType t, const ValueType& value1, const ValueType& value2) const {
		return static_cast<ValueType>(value1 + (t * (value2 - value1)));
	}
};

/**
 * \brief Implementation of the value interpolator concept for rotations.
 *
 * This class is required because the Rotation class does not support the standard
 * addition, scalar multiplication and subtraction operators.
 */
template<>
struct LinearValueInterpolator<Rotation> {
	Rotation operator()(FloatType t, const Rotation& value1, const Rotation& value2) const {
#if 1
		return interpolate(value1, value2, t);
#else
		Rotation diff = value2 * value1.inverse();
		return Rotation(diff.axis(), diff.angle() * t) * value1;
#endif
	}

	template<typename T>
    static RotationT<T> interpolate(const RotationT<T>& rot1, const RotationT<T>& rot2, T t) {
    	OVITO_ASSERT(t >= 0 && t <= 1);

    	RotationT<T> _rot2;
    	if(rot1.axis().dot(rot2.axis()) < T(0))
    		_rot2 = RotationT<T>(-rot2.axis(), -rot2.angle(), false);
    	else
    		_rot2 = rot2;

    	// Determine interpolation type, compute extra spins, and adjust angles accordingly.
		if(rot1.axis().equals(_rot2.axis())) {
			return RotationT<T>((T(1) - t) * rot1.axis() + t * _rot2.axis(), (T(1) - t) * rot1.angle() + t * _rot2.angle());
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

			RotationT<T> result = RotationT<T>(slerpExtraSpins(t, q1, q2, extraSpins));
			if(result.axis().dot(interpolateAxis(t, rot1.axis(), _rot2.axis())) < T(0))
				result = RotationT<T>(-result.axis(), -result.angle(), false);
			int nrev = floor((t * _rot2.angle() + (T(1) - t) * rot1.angle() - result.angle())/T(2*M_PI) + T(0.5));
			result.addRevolutions(nrev);
			return result;
		}
		else {
			return RotationT<T>(interpolateAxis(t, rot1.axis(), _rot2.axis()), (T(1) - t) * rot1.angle() + t * _rot2.angle());
		}
    }

    template<typename T>
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

    template<typename T>
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

/**
 * \brief Implementation of the value interpolator concept for scaling values.
 *
 * This class is required because the Scaling class does not support the standard
 * addition, scalar multiplication and subtraction operators.
 */
template<>
struct LinearValueInterpolator<Scaling> {
	Scaling operator()(FloatType t, const Scaling& value1, const Scaling& value2) const {
		return Scaling::interpolate(value1, value2, t);
	}
};

/**
 * \brief Default implementation of the value interpolator concept that does smooth interpolation.
 *
 * This template class interpolates using a cubic spline between two values of arbitrary data type.
 * The value 0.0 of the interpolation parameter \c t is mapped to the first value.
 * The value 1.0 of the interpolation parameter \c t is mapped to the second value.
 */
template<typename ValueType>
struct SplineValueInterpolator {
	ValueType operator()(FloatType t, const ValueType& value1, const ValueType& value2, const ValueType& outPoint1, const ValueType& inPoint2) const {
		FloatType Ti = FloatType(1) - t;
		FloatType U2 = t * t, T2 = Ti * Ti;
		FloatType U3 = U2 * t, T3 = T2 * Ti;
		return value1 * T3 + outPoint1 * (FloatType(3) * t * T2) + inPoint2 * (FloatType(3) * U2 * Ti) + value2 * U3;
	}
};

/**
 * \brief Implementation of the smooth value interpolator concept for rotations.
 *
 * This class is required because the Rotation class does not support the standard
 * addition, scalar multiplication, and subtraction operators.
 */
template<>
struct SplineValueInterpolator<Rotation> {
	Rotation operator()(FloatType t, const Rotation& value1, const Rotation& value2, const Rotation& outPoint1, const Rotation& inPoint2) const {
		return Rotation(Rotation::interpolateQuad(value1, value2, outPoint1, inPoint2, t));
	}
};

/**
 * \brief Implementation of the smooth value interpolator concept for scaling values.
 *
 * This class is required because the Scaling class does not support the standard
 * addition, scalar multiplication, and subtraction operators.
 */
template<>
struct SplineValueInterpolator<Scaling> {
	Scaling operator()(FloatType t, const Scaling& value1, const Scaling& value2, const Scaling& outPoint1, const Scaling& inPoint2) const {
		return Scaling::interpolateQuad(value1, value2, outPoint1, inPoint2, t);
	}
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_KEYED_CONTROLLERS_H
