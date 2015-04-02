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

#ifndef __OVITO_DATA_OBJECT_WITH_SHARED_STORAGE_H
#define __OVITO_DATA_OBJECT_WITH_SHARED_STORAGE_H

#include <core/Core.h>
#include "DataObject.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

/**
 * \brief Abstract base class data objects that store their data in a shared storage.
 */
template<class T>
class DataObjectWithSharedStorage : public DataObject
{
public:

	/// Constructor.
	DataObjectWithSharedStorage(DataSet* dataset, T* storage) : DataObject(dataset), _storage(storage) {}

	/// \brief Replaces the internal data storage with a new one.
	void setStorage(T* storage) {
		OVITO_CHECK_POINTER(storage);
		_storage = storage;
		changed();
	}

	/// \brief Returns the data storage encapsulated by this data object.
	///
	/// The returned storage might be shared by multiple data objects and may not be modified.
	/// \sa modifiableStorage()
	T* storage() const { return _storage.data(); }

	/// \brief Returns the data encapsulated by this object after making sure it is not shared by other owners.
	///
	/// Note that changed() must be called when done modifying the data.
	T* modifiableStorage() {
		_storage.detach();
		return _storage.data();
	}

	/// \brief This method must be called every time the data of the object has been changed.
	///
	/// Generates a ReferenceEvent::TargetChanged event and increments the revision counter of the object.
	void changed() {
		notifyDependents(ReferenceEvent::TargetChanged);
	}

protected:

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override {
		// Let the base class create an instance of this class.
		OORef<DataObjectWithSharedStorage<T>> clone = static_object_cast<DataObjectWithSharedStorage<T>>(DataObject::clone(deepCopy, cloneHelper));

		// Shallow copy data storage.
		clone->_storage = this->_storage;

		return clone;
	}

private:

	/// The internal data storage. May be shared by multiple owners.
	QExplicitlySharedDataPointer<T> _storage;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_DATA_OBJECT_WITH_SHARED_STORAGE_H
