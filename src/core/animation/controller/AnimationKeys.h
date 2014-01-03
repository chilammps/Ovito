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

namespace Ovito {

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

public:

	Q_PROPERTY(int time READ time);

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
	template<class KeyType, typename KeyInterpolator, Controller::ControllerType ctrlType>
		friend class KeyframeControllerTemplate;
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

public:

	Q_PROPERTY(FloatType value READ value);

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

public:

	Q_PROPERTY(int value READ value);

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_value);
};

/**
 * \brief Animation key class for Vector3 controllers.
 */
class OVITO_CORE_EXPORT Vector3AnimationKey : public TypedAnimationKey<Vector3, Vector3::Zero>
{
public:

	/// Constructor.
	Q_INVOKABLE Vector3AnimationKey(DataSet* dataset, TimePoint time = 0, const Vector3& value = Vector3::Zero()) : TypedAnimationKey<Vector3, Vector3::Zero>(dataset, time, value) {
		INIT_PROPERTY_FIELD(Vector3AnimationKey::_value);
	}

public:

	Q_PROPERTY(Ovito::Vector3 value READ value);

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_value);
};

/**
 * \brief Animation key class for position controllers.
 */
class OVITO_CORE_EXPORT PositionAnimationKey : public TypedAnimationKey<Vector3, Vector3::Zero>
{
public:

	/// Constructor.
	Q_INVOKABLE PositionAnimationKey(DataSet* dataset, TimePoint time = 0, const Vector3& value = Vector3::Zero()) : TypedAnimationKey<Vector3, Vector3::Zero>(dataset, time, value) {
		INIT_PROPERTY_FIELD(PositionAnimationKey::_value);
	}

public:

	Q_PROPERTY(Ovito::Vector3 value READ value);

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_value);
};

/**
 * \brief Animation key class for rotation controllers.
 */
class OVITO_CORE_EXPORT RotationAnimationKey : public TypedAnimationKey<Rotation, Rotation::Identity>
{
public:

	/// Constructor.
	Q_INVOKABLE RotationAnimationKey(DataSet* dataset, TimePoint time = 0, const Rotation& value = Rotation::Identity()) : TypedAnimationKey<Rotation, Rotation::Identity>(dataset, time, value) {
		INIT_PROPERTY_FIELD(RotationAnimationKey::_value);
	}

public:

	Q_PROPERTY(Ovito::Rotation value READ value);

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_value);
};

/**
 * \brief Animation key class for scaling controllers.
 */
class OVITO_CORE_EXPORT ScalingAnimationKey : public TypedAnimationKey<Scaling, Scaling::Identity>
{
public:

	/// Constructor.
	Q_INVOKABLE ScalingAnimationKey(DataSet* dataset, TimePoint time = 0, const Scaling& value = Scaling::Identity()) : TypedAnimationKey<Scaling, Scaling::Identity>(dataset, time, value) {
		INIT_PROPERTY_FIELD(ScalingAnimationKey::_value);
	}

public:

	Q_PROPERTY(Ovito::Scaling value READ value);

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
 * This class is required because the Base::Rotation class does not support the standard
 * addition, scalar multiplication and subtraction operators.
 */
template<>
struct LinearValueInterpolator<Rotation> {
	Rotation operator()(FloatType t, const Rotation& value1, const Rotation& value2) const {
		return Rotation(Rotation::interpolate(value1, value2, t));
	}
};

/**
 * \brief Implementation of the value interpolator concept for scaling values.
 *
 * This class is required because the Base::Scaling class does not support the standard
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

};

#endif // __OVITO_KEYED_CONTROLLERS_H
