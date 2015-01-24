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

#ifndef __OVITO_VERSIONED_OBJECT_REFERENCE_H
#define __OVITO_VERSIONED_OBJECT_REFERENCE_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

/**
 * \brief A reference (a.k.a. smart pointer) that refers to a particular revision of an object.
 *
 * Data objects undergo changes when the user alters a parameter or if the external file containing the
 * source data has been modified. To keep track of such changes, and to manage the automatic recalculation
 * of modifiers when the input of a data flow pipeline changes, OVITO uses the system of object revision numbers.
 *
 * Each C++ instance of a data container class has an internal revision counter that is incremented each time
 * the object is being modified in some way. This allows to detect changes made to an object without
 * looking at the stored data. In particular, it avoid saving a complete copy of the old data to detect changes.
 *
 * This VersionedOORef smart pointer class stores an ordinary reference-counted pointer (OORef) to a data object and,
 * in addition, a revision number, which refers to a particular version (or state in time) of that object.
 *
 * Two VersionedOORef instances compare equal only if both the raw C++ pointers match as well as the
 * object revision numbers they refer to.
 */
template<class T>
class VersionedOORef {
private:

	typedef VersionedOORef this_type;

public:

	typedef typename OORef<T>::element_type element_type;

	/// Default constructor.
	VersionedOORef() Q_DECL_NOTHROW : _revision(0) {}

	/// Initialization constructor.
	VersionedOORef(T* p) : _ref(p), _revision(p ? p->revisionNumber() : 0) {}

	/// Initialization constructor with explicit revision number.
	VersionedOORef(T* p, unsigned int revision) : _ref(p), _revision(revision) {}

	VersionedOORef& operator=(T* rhs) {
		_ref = rhs;
		_revision = rhs ? rhs->revisionNumber() : 0;
		return *this;
	}

	void reset() Q_DECL_NOTHROW {
		_ref.reset();
		_revision = 0;
	}

	void reset(T* rhs) {
		_ref.reset(rhs);
		_revision = rhs ? rhs->revisionNumber() : 0;
	}

	inline T* get() const Q_DECL_NOTHROW {
		return _ref.get();
	}

	inline operator T*() const Q_DECL_NOTHROW {
		return _ref.get();
	}

	inline T& operator*() const {
		return *_ref;
	}

	inline T* operator->() const {
		return _ref.get();
	}

	inline void swap(VersionedOORef& rhs) Q_DECL_NOTHROW {
		_ref.swap(rhs._ref);
		std::swap(_revision, rhs._revision);
	}

	inline unsigned int revisionNumber() const { return _revision; }

	inline void updateRevisionNumber() {
		if(_ref) _revision = _ref->revisionNumber();
	}

private:

	// The internal OORef pointer.
	OORef<T> _ref;

	// The referenced revision of the object.
	unsigned int _revision;
};

template<class T, class U> inline bool operator==(const VersionedOORef<T>& a, const VersionedOORef<U>& b) {
	return a.get() == b.get() && a.revisionNumber() == b.revisionNumber();
}

template<class T, class U> inline bool operator!=(const VersionedOORef<T>& a, const VersionedOORef<U>& b) {
	return a.get() != b.get() || a.revisionNumber() != b.revisionNumber();
}

template<class T, class U> inline bool operator==(const VersionedOORef<T>& a, U* b) {
	return a.get() == b && (b == nullptr || a.revisionNumber() == b->revisionNumber());
}

template<class T, class U> inline bool operator!=(const VersionedOORef<T>& a, U* b) {
	return a.get() != b || (b != nullptr && a.revisionNumber() != b->revisionNumber());
}

template<class T, class U> inline bool operator==(T* a, const VersionedOORef<U>& b) {
	return a == b.get() && (a == nullptr || a->revisionNumber() == b.revisionNumber());
}

template<class T, class U> inline bool operator!=(T* a, const VersionedOORef<U>& b) {
	return a != b.get() || (a != nullptr && a->revisionNumber() != b.revisionNumber());
}

template<class T> inline bool operator==(const VersionedOORef<T>& p, std::nullptr_t)
Q_DECL_NOTHROW
{
	return p.get() == nullptr;
}

template<class T> inline bool operator==(std::nullptr_t, const VersionedOORef<T>& p)
Q_DECL_NOTHROW
{
	return p.get() == nullptr;
}

template<class T> inline bool operator!=(const VersionedOORef<T>& p, std::nullptr_t)
Q_DECL_NOTHROW
{
	return p.get() != nullptr;
}

template<class T> inline bool operator!=(std::nullptr_t, const VersionedOORef<T>& p)
Q_DECL_NOTHROW
{
	return p.get() != nullptr;
}

template<class T> void swap(VersionedOORef<T>& lhs, VersionedOORef<T>& rhs)
Q_DECL_NOTHROW
{
	lhs.swap(rhs);
}

template<class T> T* get_pointer(const VersionedOORef<T>& p) {
	return p.get();
}

template<class T, class U> T* static_object_cast(const VersionedOORef<U>& p) {
	return static_cast<T*>(p.get());
}

template<class T, class U> T* dynamic_object_cast(const VersionedOORef<U>& p) {
	return dynamic_object_cast<T>(p.get());
}

template<class T> QDebug operator<<(QDebug debug, const VersionedOORef<T>& p) {
	return debug << p.get();
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_VERSIONED_OBJECT_REFERENCE_H
