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

#ifndef __OVITO_SPLINE_INTERPOLATION_CONTROLLERS_H
#define __OVITO_SPLINE_INTERPOLATION_CONTROLLERS_H

#include <core/Core.h>
#include "KeyframeController.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

/**
 * \brief Base template class for animation keys used by spline interpolation controllers.
 */
template<class BaseKeyClass>
class SplineAnimationKey : public BaseKeyClass
{
public:

	using typename BaseKeyClass::value_type;
	using typename BaseKeyClass::nullvalue_type;
	using typename BaseKeyClass::tangent_type;

	/// Constructor.
	SplineAnimationKey(DataSet* dataset, TimePoint time, const value_type& value)
		: BaseKeyClass(dataset, time, value), _inTangent(nullvalue_type()), _outTangent(nullvalue_type()) {}

	/// \brief Returns the tangent that defines the left derivative at the key point.
	const tangent_type& inTangent() const { return _inTangent; }

	/// \brief Returns the tangent that defines the right derivative at the key point.
	const tangent_type& outTangent() const { return _outTangent; }

	/// \brief Sets the tangent that defines the left derivative at the key point.
	void setInTangent(const tangent_type& t) { _inTangent = t; }

	/// \brief Sets the tangent that defines the right derivative at the key point.
	void setOutTangent(const tangent_type& t) { _outTangent = t; }

	/// \brief Returns the point that defines the incoming tangent.
	value_type inPoint() const { return this->value() + inTangent(); }

	/// \brief Returns the point that defines the outgoing direction.
	value_type outPoint() const { return this->value() + outTangent(); }

protected:

	/// The tangent that defines the left derivative at the key point.
	PropertyField<tangent_type> _inTangent;

	/// The tangent that defines the right derivative at the key point.
	PropertyField<tangent_type> _outTangent;
};


/**
 * \brief Animation key class for spline interpolation of float values.
 */
class OVITO_CORE_EXPORT FloatSplineAnimationKey : public SplineAnimationKey<FloatAnimationKey>
{
public:

	/// Constructor.
	Q_INVOKABLE FloatSplineAnimationKey(DataSet* dataset, TimePoint time = 0, FloatType value = 0) : SplineAnimationKey<FloatAnimationKey>(dataset, time, value) {
		INIT_PROPERTY_FIELD(FloatSplineAnimationKey::_inTangent);
		INIT_PROPERTY_FIELD(FloatSplineAnimationKey::_outTangent);
	}

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_inTangent);
	DECLARE_PROPERTY_FIELD(_outTangent);
};

/**
 * \brief Animation key class for spline interpolation of position values.
 */
class OVITO_CORE_EXPORT PositionSplineAnimationKey : public SplineAnimationKey<PositionAnimationKey>
{
public:

	/// Constructor.
	Q_INVOKABLE PositionSplineAnimationKey(DataSet* dataset, TimePoint time = 0, const Vector3& value = Vector3::Zero()) : SplineAnimationKey<PositionAnimationKey>(dataset, time, value) {
		INIT_PROPERTY_FIELD(PositionSplineAnimationKey::_inTangent);
		INIT_PROPERTY_FIELD(PositionSplineAnimationKey::_outTangent);
	}

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_inTangent);
	DECLARE_PROPERTY_FIELD(_outTangent);
};

/**
 * \brief Implementation of the key interpolator concept that performs cubic spline interpolation.
 *
 * This class is used with the spline interpolation controllers.
 */
template<typename KeyType>
struct SplineKeyInterpolator {
	typename KeyType::value_type operator()(TimePoint time, KeyType* key1, KeyType* key2) const {
		OVITO_ASSERT(key2->time() > key1->time());
		FloatType t = (FloatType)(time - key1->time()) / (key2->time() - key1->time());
		SplineValueInterpolator<typename KeyType::value_type> valueInterpolator;
		return valueInterpolator(t, key1->value(), key2->value(), key1->outPoint(), key2->inPoint());
	}
};

/**
 * \brief Base class for spline interpolation controllers.
 */
template<class KeyType, Controller::ControllerType ctrlType>
class SplineControllerBase : public KeyframeControllerTemplate<KeyType, SplineKeyInterpolator<KeyType>, ctrlType>
{
public:

	/// Constructor.
	SplineControllerBase(DataSet* dataset)
		: KeyframeControllerTemplate<KeyType, SplineKeyInterpolator<KeyType>, ctrlType>(dataset) {}

protected:

	/// This updates the keys after their times or values have changed.
	virtual void updateKeys() override {
		// Call base implementation.
		KeyframeControllerTemplate<KeyType, SplineKeyInterpolator<KeyType>, ctrlType>::updateKeys();

		if(this->keys().size() >= 2) {
			auto key1 = this->typedKeys().begin();
			auto key2 = key1 + 1;

			// Update the tangent vector of the first key.
			(*key1)->setOutTangent(((*key2)->value() - (*key1)->value()) / FloatType(3));

			// Update the tangent vectors for inner keys.
			auto key3 = key2 + 1;
			while(key3 != this->typedKeys().end()) {
				typename KeyType::tangent_type tangentL = (*key2)->value() - (*key1)->value();
				typename KeyType::tangent_type tangentR = (*key3)->value() - (*key2)->value();
				typename KeyType::tangent_type avgTangent = (*key3)->value() - (*key1)->value();
				(*key2)->setOutTangent(avgTangent * (tangentR.length() / avgTangent.length() / FloatType(6)));
				(*key2)->setInTangent(-avgTangent * (tangentL.length() / avgTangent.length() / FloatType(6)));
				key1 = key2;
				key2 = key3;
				++key3;
			}

			// Update the tangent vector of the last key.
			(*key2)->setInTangent(((*key1)->value() - (*key2)->value()) / FloatType(3));
		}
	}
};

/**
 * \brief A keyframe controller that interpolates between position values using a cubic-spline interpolation scheme.
 */
class OVITO_CORE_EXPORT SplinePositionController
	: public SplineControllerBase<PositionSplineAnimationKey, Controller::ControllerTypePosition>
{
public:

	/// Constructor.
	Q_INVOKABLE SplinePositionController(DataSet* dataset)
		: SplineControllerBase<PositionSplineAnimationKey, Controller::ControllerTypePosition>(dataset) {}

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

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_SPLINE_INTERPOLATION_CONTROLLERS_H
