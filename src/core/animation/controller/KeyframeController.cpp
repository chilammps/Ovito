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

#include <core/Core.h>
#include "KeyframeController.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, KeyframeController, Controller);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(KeyframeController, _keys, "Keys", AnimationKey, PROPERTY_FIELD_ALWAYS_CLONE | PROPERTY_FIELD_NO_SUB_ANIM);
SET_PROPERTY_FIELD_LABEL(KeyframeController, _keys, "Keys");

/******************************************************************************
* Maps all keys from the old animation interval to the new interval.
******************************************************************************/
void KeyframeController::rescaleTime(const TimeInterval& oldAnimationInterval, const TimeInterval& newAnimationInterval)
{
	OVITO_ASSERT(!oldAnimationInterval.isInfinite());
	OVITO_ASSERT(!newAnimationInterval.isInfinite());
	if(oldAnimationInterval.duration() == 0 && oldAnimationInterval.start() == newAnimationInterval.start())
		return;

	for(AnimationKey* key : keys()) {
		TimePoint newTime;
		if(oldAnimationInterval.duration() != 0)
			newTime = (qint64)(key->time() - oldAnimationInterval.start()) * newAnimationInterval.duration()
						/ oldAnimationInterval.duration() + newAnimationInterval.start();
		else
			newTime = key->time() - oldAnimationInterval.start() + newAnimationInterval.start();
		key->_time = newTime;
	}
	OVITO_ASSERT(areKeysSorted());
	updateKeys();
}

/******************************************************************************
* Calculates the largest time interval containing the given time during
* which the controller's value does not change.
******************************************************************************/
TimeInterval KeyframeController::validityInterval(TimePoint time)
{
	OVITO_ASSERT(areKeysSorted());
	if(keys().empty())
		return TimeInterval::infinite();
	else if(time <= keys().front()->time())
		return TimeInterval(TimeNegativeInfinity(), keys().front()->time());
	else if(time >= keys().back()->time())
		return TimeInterval(keys().back()->time(), TimePositiveInfinity());
	else
		return TimeInterval(time);
}

/******************************************************************************
* Inserts a new animation key into this controller's list of keys.
******************************************************************************/
int KeyframeController::insertKey(AnimationKey* key, int insertionPos)
{
	OVITO_CHECK_OBJECT_POINTER(key);
	OVITO_ASSERT(keys().contains(key) == false);

	// Determine the list position at which to insert the new key.
	if(insertionPos == -1) {
		for(int index = 0; index < keys().size(); index++) {
			if(keys()[index]->time() >= key->time()) {
				if(keys()[index]->time() == key->time()) {
					// Replace existing key.
					_keys.set(index, key);
				}
				else {
					// Insert new key.
					_keys.insert(index, key);
				}
				OVITO_ASSERT(areKeysSorted());
				return index;
			}
		}

		// Insert new key at the end.
		_keys.push_back(key);
		return _keys.size() - 1;
	}
	else {
		_keys.insert(insertionPos, key);
		OVITO_ASSERT(areKeysSorted());
		return insertionPos;
	}
}

/******************************************************************************
* Determines whether the animation keys of this controller are sorted with respect to time.
******************************************************************************/
bool KeyframeController::areKeysSorted() const
{
	for(int index = 1; index < keys().size(); index++) {
		if(keys()[index]->time() < keys()[index-1]->time())
			return false;
	}
	return true;
}

/******************************************************************************
* Moves the keys in the given set by the given time shift.
******************************************************************************/
void KeyframeController::moveKeys(const QVector<AnimationKey*> keysToMove, TimePoint shift)
{
	if(shift == 0)
		return;

	// First, remove the selected keys from the controller.
	QVector<OORef<AnimationKey>> removedKeys;
	for(AnimationKey* key : keysToMove) {
		int index = keys().indexOf(key);
		if(index >= 0) {
			removedKeys.push_back(key);
			_keys.remove(index);
		}
	}

	// Change times and re-insert keys into the controller.
	for(const OORef<AnimationKey>& key : removedKeys) {
		key->setTime(key->time() + shift);
		insertKey(key.get());
	}
	updateKeys();
}

/******************************************************************************
* Deletes the given set of keys from the controller.
******************************************************************************/
void KeyframeController::deleteKeys(const QVector<AnimationKey*> keysToDelete)
{
	for(AnimationKey* key : keysToDelete)
		key->deleteReferenceObject();
	updateKeys();
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
