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
 * \file StandardKeyedControllers.h
 * \brief Contains the definition of the Ovito::StandardKeyedController class and
 *        its derived classes.
 */

#ifndef __OVITO_STD_KEYED_CONTROLLERS_H
#define __OVITO_STD_KEYED_CONTROLLERS_H

#include <core/Core.h>
#include "Controller.h"
#include <core/dataset/UndoStack.h>
#include <core/animation/AnimationSettings.h>
#include <core/utilities/io/ObjectLoadStream.h>
#include <core/utilities/io/ObjectSaveStream.h>

namespace Ovito {

template<class BaseControllerClass,
		 typename _ValueType,
		 typename _KeyType,
		 typename _NullValue,
		 class KeyInterpolator>
class StandardKeyedController : public BaseControllerClass
{
protected:
	typedef _NullValue NullValue;
	typedef _ValueType ValueType;
	typedef _KeyType KeyType;
	typedef std::map<TimePoint, KeyType> KeyArray;
	typedef typename KeyArray::iterator Key;
	typedef typename KeyArray::const_iterator ConstKey;

	// This class stores the old key values of the controller so that they can be restored on undo.
	class KeyChangeOperation : public UndoableOperation {
	public:
		KeyChangeOperation(StandardKeyedController* ctrl) : _controller(ctrl), _storedKeys(ctrl->_keys) {}
		virtual void undo() override {
			_storedKeys.swap(_controller->_keys);
			_controller->notifyDependents(ReferenceEvent::TargetChanged);
		}
	private:
		OORef<StandardKeyedController> _controller;
		KeyArray _storedKeys;
	};

public:

    /// constructor.
	StandardKeyedController(DataSet* dataset) : BaseControllerClass(dataset) {}

	/// Queries the controller for its absolute value at a certain time.
	virtual void getValue(TimePoint time, ValueType& result, TimeInterval& validityInterval) override {
		if(_keys.empty()) {
			result = NullValue();
			return;
		}

		// Handle out of range cases.
		if(time <= _keys.begin()->first) {
			result = _keys.begin()->second;
			if(_keys.size() != 1)
				validityInterval.intersect(TimeInterval(TimeNegativeInfinity(), _keys.begin()->first));
		}
		else if(time >= _keys.rbegin()->first) {
			result = _keys.rbegin()->second;
			if(_keys.size() != 1)
				validityInterval.intersect(TimeInterval(_keys.rbegin()->first, TimePositiveInfinity()));
		}
		else {
			// Intersect validity interval.
			validityInterval.intersect(TimeInterval(time));

			ConstKey lastKey = _keys.begin();
			ConstKey key = lastKey;
			while((++key) != _keys.end()) {
				if(key->first == time) {
					// No interpolation necessary.
					result = key->second;
					return;
				}
				else if(key->first > time) {
					// Interpolate between two keys.
					KeyInterpolator keyInterpolator;
					result = keyInterpolator(time, *lastKey, *key);
					return;
				}
				lastKey = key;
			}

			// Should not happen.
			OVITO_ASSERT_MSG(false, "StandardKeyedController::getValue", "Invalid controller keys.");
			result = NullValue();
		}
	}

	/// Sets the controller's value at the specified time.
	virtual void setValue(TimePoint time, const ValueType& newValue, bool isAbsoluteValue) override {
		if(_keys.empty()) {
			// Handle undo.
			if(this->dataset()->undoStack().isRecording())
				this->dataset()->undoStack().push(new KeyChangeOperation(this));
			// Automatically create a key at time 0 if the controller still has its default value (i.e. no keys).
			if(time != 0 && this->dataset()->animationSettings()->isAnimating() && newValue != NullValue())
				_keys[0] = NullValue();
			// Set initial value.
			_keys[time] = newValue;
			this->updateKeys();
			this->notifyDependents(ReferenceEvent::TargetChanged);
		}

		ValueType deltaValue(newValue);
		ValueType oldValue;
		// Get delta from new absolute value.
		if(isAbsoluteValue) {
			TimeInterval iv;
			getValue(time, oldValue, iv);
			if(newValue == oldValue) return;
		}
		else if(deltaValue == NullValue()) return;	// Nothing to do.

		// Handle undo.
		if(this->dataset()->undoStack().isRecording())
			this->dataset()->undoStack().push(new KeyChangeOperation(this));

		if(!this->dataset()->animationSettings()->isAnimating()) {
			if(_keys.size() == 1 && isAbsoluteValue) {
				_keys.begin()->second = newValue;
			}
			else {
				if(isAbsoluteValue) deltaValue -= oldValue;
				// Apply delta value to all keys.
				for(auto key = _keys.begin(); key != _keys.end(); ++key)
					key->second += deltaValue;
			}
		}
		else {
			if(isAbsoluteValue) deltaValue -= oldValue;
			Key key = insertKey(time);
			key->second += deltaValue;
		}
		this->updateKeys();

		// Send change message.
		this->notifyDependents(ReferenceEvent::TargetChanged);
	}

	/// This will rescale the time values of all keys from the old animation interval to the new interval.
	virtual void rescaleTime(const TimeInterval& oldAnimationInterval, const TimeInterval& newAnimationInterval) override {
		OVITO_ASSERT(!oldAnimationInterval.isInfinite());
		OVITO_ASSERT(!newAnimationInterval.isInfinite());
		if(oldAnimationInterval.duration() == 0 && oldAnimationInterval.start() == newAnimationInterval.start()) return;
		// Handle undo.
		if(this->dataset()->undoStack().isRecording())
			this->dataset()->undoStack().push(new KeyChangeOperation(this));
		KeyArray newKeys;
		if(oldAnimationInterval.duration() != 0) {
			for(auto key = _keys.begin(); key != _keys.end(); ++key) {
				TimePoint newTime = (qint64)(key->first - oldAnimationInterval.start()) * newAnimationInterval.duration()
								/ oldAnimationInterval.duration() + newAnimationInterval.start();
				newKeys.insert(std::make_pair(newTime, key->second));
			}
		}
		else {
			for(auto key = _keys.begin(); key != _keys.end(); ++key) {
				TimePoint newTime = key->first - oldAnimationInterval.start() + newAnimationInterval.start();
				newKeys.insert(std::make_pair(newTime, key->second));
			}
		}
		_keys = newKeys;
		updateKeys();
		// Send change message.
		this->notifyDependents(ReferenceEvent::TargetChanged);
	}

	/// \brief Creates a new key at the given time with the specified value.
	/// \param time The animation where a key should be created.
	/// \param value The absolute value of the new animation key.
	///
	/// If there is an existing key at that same animation time, it is replaced with the new key.
	virtual void createKey(TimePoint time, const ValueType& value) override {
		// Get existing key if there is one.
		Key key = _keys.find(time);
		if(key != _keys.end()) {
			// Check if existing key already has the same value.
			if(value == (ValueType)key->second) return;	// nothing to do.
		}

		// Handle undo.
		if(this->dataset()->undoStack().isRecording())
			this->dataset()->undoStack().push(new KeyChangeOperation(this));

		if(key == _keys.end())	// Create a new key.
			key = _keys.insert(std::make_pair(time, value)).first;
		else
			key->second = value;
		updateKeys();

		// Send change message.
		this->notifyDependents(ReferenceEvent::TargetChanged);
	}

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override {
		BaseControllerClass::saveToStream(stream);
		stream.beginChunk(0x01);
		stream << (quint32)_keys.size();
		for(auto key = _keys.begin(); key != _keys.end(); ++key) {
			stream << key->first;
			stream << key->second;
		}
		stream.endChunk();
	}

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override {
		BaseControllerClass::loadFromStream(stream);
		stream.expectChunk(0x01);
		quint32 nkeys;
		stream >> nkeys;
		_keys.clear();
		for(quint32 i = 0; i < nkeys; i++) {
			TimePoint time;
			stream >> time;
			stream >> _keys[time];
		}
		OVITO_ASSERT(_keys.size() == nkeys);
		stream.closeChunk();
	}

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override {
		// Let the base class create an instance of this class.
		OORef<StandardKeyedController> clone = static_object_cast<StandardKeyedController>(BaseControllerClass::clone(deepCopy, cloneHelper));
		clone->_keys = this->_keys;
		return clone;
	}

	/// Inserts a new key at the specified time.
	virtual Key insertKey(const TimePoint& time) {
		// Get existing key if there is one.
		Key key = _keys.find(time);
		if(key != _keys.end()) return key;

		// Get initial value of new key.
		ValueType v = this->getValueAtTime(time);

		// Create new key.
		return _keys.insert(std::make_pair(time, v)).first;
	}

	/// This is called each time the value or time positions of one or more keys has been changed.
	virtual void updateKeys() {}

	/// The list of controller keys.
    KeyArray _keys;
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
 * This template class interpolates using a cubic spline between two values of abitrary types.
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
 * This class is required because the Base::Rotation class does not support the standard
 * addition, scalar multiplication and subtraction operators.
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
 * This class is required because the Base::Scaling class does not support the standard
 * addition, scalar multiplication and subtraction operators.
 */
template<>
struct SplineValueInterpolator<Scaling> {
	Scaling operator()(FloatType t, const Scaling& value1, const Scaling& value2, const Scaling& outPoint1, const Scaling& inPoint2) const {
		return Scaling::interpolateQuad(value1, value2, outPoint1, inPoint2, t);
	}
};

/**
 * \brief Base template class for keyed float controllers.
 */
template<typename KeyType, class KeyInterpolator>
class KeyedFloatController : public StandardKeyedController<FloatController, FloatType, KeyType, FloatType, KeyInterpolator>
{
public:

	/// Constructor.
	KeyedFloatController(DataSet* dataset) : StandardKeyedController<FloatController, FloatType, KeyType, FloatType, KeyInterpolator>(dataset) {}
};

/**
 * \brief Base template class for keyed vector controllers.
 */
template<typename KeyType, class KeyInterpolator>
class KeyedVectorController : public StandardKeyedController<VectorController, Vector3, KeyType, Vector3::Zero, KeyInterpolator>
{
public:

	/// Constructor.
	KeyedVectorController(DataSet* dataset) : StandardKeyedController<VectorController, Vector3, KeyType, Vector3::Zero, KeyInterpolator>(dataset) {}
};

/**
 * \brief Base template class for keyed position controllers.
 */
template<typename KeyType, class KeyInterpolator>
class KeyedPositionController : public StandardKeyedController<PositionController, Vector3, KeyType, Vector3::Zero, KeyInterpolator>
{
public:

	/// Constructor.
	KeyedPositionController(DataSet* dataset) : StandardKeyedController<PositionController, Vector3, KeyType, Vector3::Zero, KeyInterpolator>(dataset) {}

	virtual void changeParent(TimePoint time, const AffineTransformation& oldParentTM, const AffineTransformation& newParentTM, SceneNode* contextNode) override {
		if(this->_keys.empty()) return;
		AffineTransformation rel = oldParentTM * newParentTM.inverse();

		// Handle undo.
		typedef StandardKeyedController<PositionController, Vector3, KeyType, Vector3::Zero, KeyInterpolator> BaseClassType;
		if(this->dataset()->undoStack().isRecording())
			this->dataset()->undoStack().push(new typename BaseClassType::KeyChangeOperation(this));

		if(!this->dataset()->animationSettings()->isAnimating()) {
			// Apply delta value to all keys.
			for(auto key = this->_keys.begin(); key != this->_keys.end(); ++key)
				key->second = (rel * (Point3::Origin() + (Vector3)key->second)) - Point3::Origin();
		}
		else {
			auto key = this->insertKey(time);
			key->second = (rel * (Point3::Origin() + (Vector3)key->second)) - Point3::Origin();
		}

		// Send change message.
		this->notifyDependents(ReferenceEvent::TargetChanged);
	}
};

/**
 * \brief Base template class for keyed rotation controllers.
 */
template<typename KeyType, class KeyInterpolator>
class KeyedRotationController : public StandardKeyedController<RotationController, Rotation, KeyType, Rotation::Identity, KeyInterpolator>
{
public:

	/// Constructor.
	KeyedRotationController(DataSet* dataset) : StandardKeyedController<RotationController, Rotation, KeyType, Rotation::Identity, KeyInterpolator>(dataset) {}

	virtual void changeParent(TimePoint time, const AffineTransformation& oldParentTM, const AffineTransformation& newParentTM, SceneNode* contextNode) override {
		// to be implemented.
		OVITO_ASSERT_MSG(false, "KeyedRotationController::changeParent", "Method not implemented.");
	}
};

/**
 * \brief Base template class for keyed scaling controllers.
 * \author Alexander Stukowski
 */
template<typename KeyType, class KeyInterpolator>
class KeyedScalingController : public StandardKeyedController<ScalingController, Scaling, KeyType, Scaling::Identity, KeyInterpolator>
{
public:

	/// Constructor.
	KeyedScalingController(DataSet* dataset) : StandardKeyedController<ScalingController, Scaling, KeyType, Scaling::Identity, KeyInterpolator>(dataset) {}

	virtual void changeParent(TimePoint time, const AffineTransformation& oldParentTM, const AffineTransformation& newParentTM, SceneNode* contextNode) override {
		// to be implemented.
		OVITO_ASSERT_MSG(false, "KeyedScalingController::changeParent", "Method not implemented.");
	}
};

};	// End of namespace

#endif // __OVITO_STD_KEYED_CONTROLLERS_H
