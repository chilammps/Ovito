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

namespace Ovito {

/**
 * \brief A reference (a.k.a. smart pointer) to a particular revision of an object.
 *
 * This class stores a reference-counted pointer (OORef) to an object and, in addition,
 * a revision number, which refers to a particular version (or state in time) of the referenced object.
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

template<class T, class U> VersionedOORef<T> static_pointer_cast(const VersionedOORef<U>& p) {
	return VersionedOORef<T>(static_cast<T*>(p.get()), p.revisionNumber());
}

template<class T, class U> VersionedOORef<T> const_pointer_cast(const VersionedOORef<U>& p) {
	return VersionedOORef<T>(const_cast<T*>(p.get()), p.revisionNumber());
}

template<class T, class U> VersionedOORef<T> dynamic_pointer_cast(const VersionedOORef<U>& p) {
	T* obj = qobject_cast<T*>(p.get());
	return VersionedOORef<T>(obj, obj ? p.revisionNumber() : 0);
}

template<class T> QDebug operator<<(QDebug debug, const VersionedOORef<T>& p) {
	return debug << p.get();
}

}
;
// End of namespace Ovito

#endif // __OVITO_OBJECT_REFERENCE_H
