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

#ifndef __OVITO_LINEAR_INTERPOLATION_CONTROLLERS_H
#define __OVITO_LINEAR_INTERPOLATION_CONTROLLERS_H

#include <core/Core.h>
#include "KeyframeController.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

/**
 * \brief Implementation of the key interpolator concept that performs linear interpolation.
 *
 * This class is used with the linear interpolation controllers.
 */
template<typename KeyType>
struct LinearKeyInterpolator {
	typename KeyType::value_type operator()(TimePoint time, KeyType* key1, KeyType* key2) const {
		OVITO_ASSERT(key2->time() > key1->time());
		FloatType t = (FloatType)(time - key1->time()) / (key2->time() - key1->time());
		LinearValueInterpolator<typename KeyType::value_type> valueInterpolator;
		return valueInterpolator(t, key1->value(), key2->value());
	}
};

/**
 * \brief A keyframe controller that interpolates between float values using a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearFloatController
	: public KeyframeControllerTemplate<FloatAnimationKey,
	  	  	  	  	  	  	  	  	  	LinearKeyInterpolator<FloatAnimationKey>,
	  	  	  	  	  	  	  	  	  	Controller::ControllerTypeFloat>
{
public:

	/// Constructor.
	Q_INVOKABLE LinearFloatController(DataSet* dataset)
		: KeyframeControllerTemplate<FloatAnimationKey, LinearKeyInterpolator<FloatAnimationKey>, Controller::ControllerTypeFloat>(dataset) {}

	/// \brief Gets the controller's value at a certain animation time.
	virtual FloatType getFloatValue(TimePoint time, TimeInterval& validityInterval) override {
		FloatType val;
		getInterpolatedValue(time, val, validityInterval);
		return val;
	}

	/// \brief Sets the controller's value at the given animation time.
	virtual void setFloatValue(TimePoint time, FloatType newValue) override {
		setAbsoluteValue(time, newValue);
	}

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A keyframe controller that interpolates between integer values using a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearIntegerController
	: public KeyframeControllerTemplate<IntegerAnimationKey,
	  	  	  	  	  	  	  	  	  	LinearKeyInterpolator<IntegerAnimationKey>,
	  	  	  	  	  	  	  	  	  	Controller::ControllerTypeInt>
{
public:

	/// Constructor.
	Q_INVOKABLE LinearIntegerController(DataSet* dataset)
		: KeyframeControllerTemplate<IntegerAnimationKey, LinearKeyInterpolator<IntegerAnimationKey>, Controller::ControllerTypeInt>(dataset) {}

	/// \brief Gets the controller's value at a certain animation time.
	virtual int getIntValue(TimePoint time, TimeInterval& validityInterval) override {
		int val;
		getInterpolatedValue(time, val, validityInterval);
		return val;
	}

	/// \brief Sets the controller's value at the given animation time.
	virtual void setIntValue(TimePoint time, int newValue) override {
		setAbsoluteValue(time, newValue);
	}

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A keyframe controller that interpolates between Vector3 values using a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearVectorController
	: public KeyframeControllerTemplate<Vector3AnimationKey,
	  	  	  	  	  	  	  	  	  	LinearKeyInterpolator<Vector3AnimationKey>,
	  	  	  	  	  	  	  	  	  	Controller::ControllerTypeVector3>
{
public:

	/// Constructor.
	Q_INVOKABLE LinearVectorController(DataSet* dataset)
		: KeyframeControllerTemplate<Vector3AnimationKey, LinearKeyInterpolator<Vector3AnimationKey>, Controller::ControllerTypeVector3>(dataset) {}

	/// \brief Gets the controller's value at a certain animation time.
	virtual void getVector3Value(TimePoint time, Vector3& value, TimeInterval& validityInterval) override {
		getInterpolatedValue(time, value, validityInterval);
	}

	/// \brief Sets the controller's value at the given animation time.
	virtual void setVector3Value(TimePoint time, const Vector3& newValue) override {
		setAbsoluteValue(time, newValue);
	}

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A keyframe controller that interpolates between position values using a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearPositionController
	: public KeyframeControllerTemplate<PositionAnimationKey,
	  	  	  	  	  	  	  	  	  	LinearKeyInterpolator<PositionAnimationKey>,
	  	  	  	  	  	  	  	  	  	Controller::ControllerTypePosition>
{
public:

	/// Constructor.
	Q_INVOKABLE LinearPositionController(DataSet* dataset)
		: KeyframeControllerTemplate<PositionAnimationKey, LinearKeyInterpolator<PositionAnimationKey>, Controller::ControllerTypePosition>(dataset) {}

	/// \brief Gets the controller's value at a certain animation time.
	virtual void getPositionValue(TimePoint time, Vector3& value, TimeInterval& validityInterval) override {
		getInterpolatedValue(time, value, validityInterval);
	}

	/// \brief Sets the controller's value at the given animation time.
	virtual void setPositionValue(TimePoint time, const Vector3& newValue, bool isAbsolute) override {
		if(isAbsolute)
			setAbsoluteValue(time, newValue);
		else
			setRelativeValue(time, newValue);
	}

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A keyframe controller that interpolates between rotation values using a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearRotationController
	: public KeyframeControllerTemplate<RotationAnimationKey,
	  	  	  	  	  	  	  	  	  	LinearKeyInterpolator<RotationAnimationKey>,
	  	  	  	  	  	  	  	  	  	Controller::ControllerTypeRotation>
{
public:

	/// Constructor.
	Q_INVOKABLE LinearRotationController(DataSet* dataset)
		: KeyframeControllerTemplate<RotationAnimationKey, LinearKeyInterpolator<RotationAnimationKey>, Controller::ControllerTypeRotation>(dataset) {}

	/// \brief Gets the controller's value at a certain animation time.
	virtual void getRotationValue(TimePoint time, Rotation& value, TimeInterval& validityInterval) override {
		getInterpolatedValue(time, value, validityInterval);
	}

	/// \brief Sets the controller's value at the given animation time.
	virtual void setRotationValue(TimePoint time, const Rotation& newValue, bool isAbsolute) override {
		if(isAbsolute)
			setAbsoluteValue(time, newValue);
		else
			setRelativeValue(time, newValue);
	}

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A keyframe controller that interpolates between scaling values using a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearScalingController
	: public KeyframeControllerTemplate<ScalingAnimationKey,
	  	  	  	  	  	  	  	  	  	LinearKeyInterpolator<ScalingAnimationKey>,
	  	  	  	  	  	  	  	  	  	Controller::ControllerTypeScaling>
{
public:

	/// Constructor.
	Q_INVOKABLE LinearScalingController(DataSet* dataset)
		: KeyframeControllerTemplate<ScalingAnimationKey, LinearKeyInterpolator<ScalingAnimationKey>, Controller::ControllerTypeScaling>(dataset) {}

	/// \brief Gets the controller's value at a certain animation time.
	virtual void getScalingValue(TimePoint time, Scaling& value, TimeInterval& validityInterval) override {
		getInterpolatedValue(time, value, validityInterval);
	}

	/// \brief Sets the controller's value at the given animation time.
	virtual void setScalingValue(TimePoint time, const Scaling& newValue, bool isAbsolute) override {
		if(isAbsolute)
			setAbsoluteValue(time, newValue);
		else
			setRelativeValue(time, newValue);
	}

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_LINEAR_INTERPOLATION_CONTROLLERS_H
