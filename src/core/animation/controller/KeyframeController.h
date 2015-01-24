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

#ifndef __OVITO_KEYFRAME_CONTROLLER_H
#define __OVITO_KEYFRAME_CONTROLLER_H

#include <core/Core.h>
#include <core/dataset/DataSet.h>
#include <core/animation/AnimationSettings.h>
#include "Controller.h"
#include "AnimationKeys.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

/**
 * \brief Base class for animation controllers that are based on animation keys.
 */
class OVITO_CORE_EXPORT KeyframeController : public Controller
{
public:

	/// Constructor.
	KeyframeController(DataSet* dataset) : Controller(dataset) {
		INIT_PROPERTY_FIELD(KeyframeController::_keys);
	}

	/// Returns the controller's list of animation keys.
	const QVector<AnimationKey*>& keys() const { return _keys; }

	/// Maps all keys from the old animation interval to the new interval.
	virtual void rescaleTime(const TimeInterval& oldAnimationInterval, const TimeInterval& newAnimationInterval) override;

	/// Calculates the largest time interval containing the given time during which the controller's value does not change.
	virtual TimeInterval validityInterval(TimePoint time) override;

	/// Determines whether the animation keys of this controller are sorted with respect to time.
	bool areKeysSorted() const;

	/// Moves the keys in the given set by the given time shift.
	void moveKeys(const QVector<AnimationKey*> keysToMove, TimePoint shift);

	/// Deletes the given set of keys from the controller.
	void deleteKeys(const QVector<AnimationKey*> keysToDelete);

protected:

	/// Inserts a new animation key into this controller's list of keys.
	int insertKey(AnimationKey* key, int insertionPos = -1);

	/// This updates the keys after their times or values have changed.
	virtual void updateKeys() {}

	/// Changes a key's value.
	/// This is a wrapper for the protected function TypedAnimationKey::setValue().
	template<typename KeyType>
	static void setKeyValueInternal(KeyType* key, const typename KeyType::value_type& newValue) { key->setValue(newValue); }

private:

	/// Stores the list of animation keys.
	VectorReferenceField<AnimationKey> _keys;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_keys);
};

/**
 * \brief Base class for animation controllers that are based on animation keys.
 */
template<class KeyType, typename KeyInterpolator, Controller::ControllerType ctrlType>
class KeyframeControllerTemplate : public KeyframeController
{
public:

	/// The value type stored by this controller.
	typedef typename KeyType::value_type value_type;

	/// The type used to construct default key values.
	typedef typename KeyType::nullvalue_type nullvalue_type;

	/// Constructor.
	KeyframeControllerTemplate(DataSet* dataset) : KeyframeController(dataset) {}

	/// Returns the value type of the controller.
	virtual ControllerType controllerType() const override { return ctrlType; }

	/// Returns the list of keys of this animation controller.
	const QVector<KeyType*>& typedKeys() const { return reinterpret_cast<const QVector<KeyType*>&>(keys()); }

protected:

	/// Queries the controller for its value at a certain time.
	void getInterpolatedValue(TimePoint time, value_type& result, TimeInterval& validityInterval) const {
		const QVector<KeyType*>& keys = typedKeys();
		if(keys.empty()) {
			result = nullvalue_type();
			return;
		}
		OVITO_ASSERT(areKeysSorted());

		// Handle out of range cases.
		if(time <= keys.front()->time()) {
			result = keys.front()->value();
			if(keys.size() != 1)
				validityInterval.intersect(TimeInterval(TimeNegativeInfinity(), keys.front()->time()));
		}
		else if(time >= keys.back()->time()) {
			result = keys.back()->value();
			if(keys.size() != 1)
				validityInterval.intersect(TimeInterval(keys.back()->time(), TimePositiveInfinity()));
		}
		else {
			// Intersect validity interval.
			validityInterval.intersect(TimeInterval(time));

			for(auto key = keys.begin() + 1; key != keys.end(); ++key) {
				if((*key)->time() == time) {
					// No interpolation necessary.
					result = (*key)->value();
					return;
				}
				else if((*key)->time() > time) {
					// Interpolate between two keys.
					result = KeyInterpolator()(time, *(key - 1), *key);
					return;
				}
			}

			// This should never happen.
			OVITO_ASSERT_MSG(false, "KeyframeControllerTemplate::getInterpolatedValue", "Invalid controller keys.");
			result = nullvalue_type();
		}
	}

	/// Creates a new animation key at the specified time or replaces the value of an existing key.
	void setKeyValue(TimePoint time, const value_type& newValue) {
		const QVector<KeyType*>& keys = typedKeys();
		int index;
		for(index = 0; index < keys.size(); index++) {
			if(keys[index]->time() == time) {
				setKeyValueInternal(keys[index], newValue);
				return;
			}
			else if(keys[index]->time() > time) {
				break;
			}
		}
		insertKey(new KeyType(dataset(), time, newValue), index);
	}

	/// Sets the controller's value at the specified time.
	void setAbsoluteValue(TimePoint time, const value_type& newValue) {
		if(keys().empty()) {
			// Create an additional key at time 0 if the controller doesn't have any keys yet.
			if(time != 0 && dataset()->animationSettings()->isAnimating() && newValue != nullvalue_type()) {
				insertKey(new KeyType(dataset()), 0);
				insertKey(new KeyType(dataset(), time, newValue), time > 0 ? 1 : 0);
			}
			else {
				insertKey(new KeyType(dataset(), 0, newValue), 0);
			}
		}
		else if(!dataset()->animationSettings()->isAnimating()) {
			if(keys().size() == 1) {
				setKeyValueInternal(typedKeys().front(), newValue);
			}
			else {
				value_type deltaValue(newValue);
				value_type oldValue;
				// Get delta from new absolute value.
				TimeInterval iv;
				getInterpolatedValue(time, oldValue, iv);
				if(newValue == oldValue) return;
				deltaValue -= oldValue;
				// Apply delta value to all keys.
				for(KeyType* key : typedKeys()) {
					value_type v = key->value();
					v += deltaValue;
					setKeyValueInternal(key, v);
				}
			}
		}
		else {
			setKeyValue(time, newValue);
		}
		updateKeys();
	}

	/// Changes the controller's value at the specified time.
	void setRelativeValue(TimePoint time, const value_type& deltaValue) {
		if(deltaValue == nullvalue_type())
			return;
		if(keys().empty()) {
			// Create an additional key at time 0 if the controller doesn't have any keys yet.
			if(time != 0 && dataset()->animationSettings()->isAnimating()) {
				insertKey(new KeyType(dataset()), 0);
				insertKey(new KeyType(dataset(), time, deltaValue), time > 0 ? 1 : 0);
			}
			else {
				insertKey(new KeyType(dataset(), 0, deltaValue), 0);
			}
		}
		else if(!dataset()->animationSettings()->isAnimating()) {
			// Apply delta value to all keys.
			for(KeyType* key : typedKeys()) {
				value_type v = key->value();
				v += deltaValue;
				setKeyValueInternal(key, v);
			}
		}
		else {
			TimeInterval iv;
			value_type oldValue;
			getInterpolatedValue(time, oldValue, iv);
			oldValue += deltaValue;
			setKeyValue(time, oldValue);
		}
		updateKeys();
	}

	/// Loads the object from a file stream.
	/// This method is here to support reading old files written by Ovito 2.3.x or older.
	virtual void loadFromStream(ObjectLoadStream& stream) override {
		KeyframeController::loadFromStream(stream);
		if(stream.formatVersion() < 20400) {
			stream.expectChunk(0x01);
			quint32 nkeys;
			stream >> nkeys;
			for(quint32 i = 0; i < nkeys; i++) {
				TimePoint time;
				value_type value;
				stream >> time >> value;
				setAbsoluteValue(time, value);
			}
			stream.closeChunk();
		}
	}
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_KEYFRAME_CONTROLLER_H
