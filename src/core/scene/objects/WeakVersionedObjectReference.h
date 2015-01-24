///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_WEAK_VERSIONED_OBJECT_REFERENCE_H
#define __OVITO_WEAK_VERSIONED_OBJECT_REFERENCE_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

/**
 * \brief A weak reference (a.k.a. guarded pointer) that refers to a particular revision of an object.
 *
 * Data objects undergo changes when the user alters a parameter or if the external file containing the
 * source data has been modified. To keep track of such changes, and to manage the automatic recalculation
 * of modifiers when the input of a data flow pipeline changes, OVITO uses the system of object revision numbers.
 *
 * Each C++ instance of a data container class has an internal revision counter that is incremented each time
 * the object is being modified in some way. This allows to detect changes made to an object without
 * looking at the stored data. In particular, it avoid saving a complete copy of the old data to detect changes.
 *
 * The WeakVersionedOORef class stores an ordinary guarded pointer (QPointer) to a data object and,
 * in addition, a revision number, which refers to a particular version (or state in time) of that object.
 *
 * Two WeakVersionedOORef instances compare equal only if both the raw C++ pointers match as well as the
 * object revision numbers they refer to.
 */
template<class T>
class WeakVersionedOORef {
private:

	typedef WeakVersionedOORef this_type;

public:

	typedef T* element_type;

	/// Default constructor.
	WeakVersionedOORef() Q_DECL_NOTHROW : _revision(0) {}

	/// Initialization constructor.
	WeakVersionedOORef(T* p) : _ref(p), _revision(p ? p->revisionNumber() : 0) {}

	/// Initialization constructor with explicit revision number.
	WeakVersionedOORef(T* p, unsigned int revision) : _ref(p), _revision(revision) {}

	WeakVersionedOORef& operator=(T* rhs) {
		_ref = rhs;
		_revision = rhs ? rhs->revisionNumber() : 0;
		return *this;
	}

	void reset() Q_DECL_NOTHROW {
		_ref.clear();
		_revision = 0;
	}

	void reset(T* rhs) {
		_ref = rhs;
		_revision = rhs ? rhs->revisionNumber() : 0;
	}

	inline T* get() const Q_DECL_NOTHROW {
		return _ref.data();
	}

	inline operator T*() const Q_DECL_NOTHROW {
		return _ref.data();
	}

	inline T& operator*() const {
		return *_ref;
	}

	inline T* operator->() const {
		return _ref.data();
	}

	inline void swap(WeakVersionedOORef& rhs) Q_DECL_NOTHROW {
		std::swap(_ref, rhs._ref);
		std::swap(_revision, rhs._revision);
	}

	inline unsigned int revisionNumber() const { return _revision; }

	inline void updateRevisionNumber() {
		if(_ref) _revision = _ref->revisionNumber();
	}

private:

	// The internal guarded pointer.
	QPointer<T> _ref;

	// The referenced revision of the object.
	unsigned int _revision;
};

template<class T, class U> inline bool operator==(const WeakVersionedOORef<T>& a, const WeakVersionedOORef<U>& b) {
	return a.get() == b.get() && a.revisionNumber() == b.revisionNumber();
}

template<class T, class U> inline bool operator!=(const WeakVersionedOORef<T>& a, const WeakVersionedOORef<U>& b) {
	return a.get() != b.get() || a.revisionNumber() != b.revisionNumber();
}

template<class T, class U> inline bool operator==(const WeakVersionedOORef<T>& a, U* b) {
	return a.get() == b && (b == nullptr || a.revisionNumber() == b->revisionNumber());
}

template<class T, class U> inline bool operator!=(const WeakVersionedOORef<T>& a, U* b) {
	return a.get() != b || (b != nullptr && a.revisionNumber() != b->revisionNumber());
}

template<class T, class U> inline bool operator==(T* a, const WeakVersionedOORef<U>& b) {
	return a == b.get() && (a == nullptr || a->revisionNumber() == b.revisionNumber());
}

template<class T, class U> inline bool operator!=(T* a, const WeakVersionedOORef<U>& b) {
	return a != b.get() || (a != nullptr && a->revisionNumber() != b.revisionNumber());
}

template<class T> inline bool operator==(const WeakVersionedOORef<T>& p, std::nullptr_t)
Q_DECL_NOTHROW
{
	return p.get() == nullptr;
}

template<class T> inline bool operator==(std::nullptr_t, const WeakVersionedOORef<T>& p)
Q_DECL_NOTHROW
{
	return p.get() == nullptr;
}

template<class T> inline bool operator!=(const WeakVersionedOORef<T>& p, std::nullptr_t)
Q_DECL_NOTHROW
{
	return p.get() != nullptr;
}

template<class T> inline bool operator!=(std::nullptr_t, const WeakVersionedOORef<T>& p)
Q_DECL_NOTHROW
{
	return p.get() != nullptr;
}

template<class T> void swap(WeakVersionedOORef<T>& lhs, WeakVersionedOORef<T>& rhs)
Q_DECL_NOTHROW
{
	lhs.swap(rhs);
}

template<class T> T* get_pointer(const WeakVersionedOORef<T>& p) {
	return p.get();
}

template<class T, class U> T* static_object_cast(const WeakVersionedOORef<U>& p) {
	return static_cast<T*>(p.get());
}

template<class T, class U> T* dynamic_object_cast(const WeakVersionedOORef<U>& p) {
	return dynamic_object_cast<T>(p.get());
}

template<class T> QDebug operator<<(QDebug debug, const WeakVersionedOORef<T>& p) {
	return debug << p.get();
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_WEAK_VERSIONED_OBJECT_REFERENCE_H
