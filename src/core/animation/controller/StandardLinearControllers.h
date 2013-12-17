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
 * \file StandardLinearControllers.h
 * \brief Contains the definition of the linear interpolation controller classes.
 */

#ifndef __OVITO_STD_LINEAR_CONTROLLERS_H
#define __OVITO_STD_LINEAR_CONTROLLERS_H

#include <core/Core.h>
#include "StandardKeyedControllers.h"

namespace Ovito {

/**
 * \brief Implementation of the key interpolator concept that performs linear interpolation.
 *
 * This class calculates a value in the [0,1] range based on the input time and the times of the two input keys.
 * The returned value specifies the weighting of the two key values.
 *
 * This class is used with the linear interpolation controller.
 */
template<typename ValueType>
struct LinearKeyInterpolator {
	ValueType operator()(TimePoint time,
						 const std::pair<TimePoint, ValueType>& key1,
						 const std::pair<TimePoint, ValueType>& key2) const {
		OVITO_ASSERT(key2.first > key1.first);
		FloatType t = (FloatType)(time - key1.first) / (key2.first - key1.first);
		LinearValueInterpolator<ValueType> valueInterpolator;
		return valueInterpolator(t, key1.second, key2.second);
	}
};

/**
 * \brief A keyed controller that interpolates between float values using
 *        a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearFloatController : public StandardKeyedController<FloatController, FloatType, FloatType, FloatType, LinearKeyInterpolator<FloatType>> {
public:
	Q_INVOKABLE LinearFloatController(DataSet* dataset)
		: StandardKeyedController<FloatController, FloatType, FloatType, FloatType, LinearKeyInterpolator<FloatType>>(dataset) {}
private:
	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A keyed controller that interpolates between integer values using
 *        a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearIntegerController : public StandardKeyedController<IntegerController, int, int, int, LinearKeyInterpolator<int>> {
public:
	Q_INVOKABLE LinearIntegerController(DataSet* dataset)
		: StandardKeyedController<IntegerController, int, int, int, LinearKeyInterpolator<int>>(dataset) {}
private:
	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A keyed controller that interpolates between Vector3 values using
 *        a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearVectorController : public StandardKeyedController<VectorController, Vector3, Vector3, Vector3::Zero, LinearKeyInterpolator<Vector3>> {
public:
	Q_INVOKABLE LinearVectorController(DataSet* dataset)
		: StandardKeyedController<VectorController, Vector3, Vector3, Vector3::Zero, LinearKeyInterpolator<Vector3>>(dataset) {}

private:
	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A keyed controller that interpolates between position values using
 *        a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearPositionController : public KeyedPositionController<Vector3, LinearKeyInterpolator<Vector3>> {
public:
	Q_INVOKABLE LinearPositionController(DataSet* dataset)
		: KeyedPositionController<Vector3, LinearKeyInterpolator<Vector3>>(dataset) {}
private:
	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A keyed controller that interpolates between Base::Rotation values using
 *        a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearRotationController : public KeyedRotationController<Rotation, LinearKeyInterpolator<Rotation>> {
public:
	Q_INVOKABLE LinearRotationController(DataSet* dataset)
		: KeyedRotationController<Rotation, LinearKeyInterpolator<Rotation>>(dataset) {}
private:
	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A keyed controller that interpolates between Base::Scaling values using
 *        a linear interpolation scheme.
 */
class OVITO_CORE_EXPORT LinearScalingController : public KeyedScalingController<Scaling, LinearKeyInterpolator<Scaling> > {
public:
	Q_INVOKABLE LinearScalingController(DataSet* dataset)
		: KeyedScalingController<Scaling, LinearKeyInterpolator<Scaling>>(dataset) {}
private:
	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_STD_LINEAR_CONTROLLERS_H
