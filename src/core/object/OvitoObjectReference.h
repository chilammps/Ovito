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

#ifndef __OVITO_OBJECT_REFERENCE_H
#define __OVITO_OBJECT_REFERENCE_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

/**
 * \brief A smart pointer holding a reference to an OvitoObject.
 *
 * This smart pointer class takes care of incrementing and decrementing
 * the reference counter of the object it is pointing to. As soon as no
 * OORef pointer to an object instance is left, the object is automatically
 * deleted.
 */
template<class T>
class OORef
{
private:

	typedef OORef this_type;

public:

    typedef T element_type;

    /// Default constructor.
    OORef() Q_DECL_NOTHROW : px(nullptr) {}

    /// Initialization constructor.
    OORef(T* p) : px(p) {
    	if(px) px->incrementReferenceCount();
    }

    /// Copy constructor.
    OORef(const OORef& rhs) : px(rhs.get()) {
    	if(px) px->incrementReferenceCount();
    }

    /// Copy and conversion constructor.
    template<class U>
    OORef(const OORef<U>& rhs) : px(rhs.get()) {
    	if(px) px->incrementReferenceCount();
    }

    /// Move constructor.
    OORef(OORef&& rhs) Q_DECL_NOTHROW : px(rhs.px) {
    	rhs.px = nullptr;
    }

    /// Destructor.
    ~OORef() {
    	if(px) px->decrementReferenceCount();
    }

    template<class U>
    OORef& operator=(const OORef<U>& rhs) {
    	this_type(rhs).swap(*this);
    	return *this;
    }

    OORef& operator=(const OORef& rhs) {
    	this_type(rhs).swap(*this);
    	return *this;
    }

    OORef& operator=(OORef&& rhs) Q_DECL_NOTHROW {
    	this_type(static_cast<OORef&&>(rhs)).swap(*this);
    	return *this;
    }

    OORef& operator=(T* rhs) {
    	this_type(rhs).swap(*this);
    	return *this;
    }

    void reset() Q_DECL_NOTHROW {
    	this_type().swap(*this);
    }

    void reset(T* rhs) {
    	this_type(rhs).swap(*this);
    }

    inline T* get() const Q_DECL_NOTHROW {
    	return px;
    }

    inline operator T*() const Q_DECL_NOTHROW {
    	return px;
    }

    inline T& operator*() const {
    	OVITO_ASSERT(px != nullptr);
    	return *px;
    }

    inline T* operator->() const {
    	OVITO_ASSERT(px != nullptr);
    	return px;
    }

    inline void swap(OORef& rhs) Q_DECL_NOTHROW {
    	std::swap(px,rhs.px);
    }

private:

    T* px;
};

template<class T, class U> inline bool operator==(const OORef<T>& a, const OORef<U>& b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(const OORef<T>& a, const OORef<U>& b)
{
    return a.get() != b.get();
}

template<class T, class U> inline bool operator==(const OORef<T>& a, U* b)
{
    return a.get() == b;
}

template<class T, class U> inline bool operator!=(const OORef<T>& a, U* b)
{
    return a.get() != b;
}

template<class T, class U> inline bool operator==(T* a, const OORef<U>& b)
{
    return a == b.get();
}

template<class T, class U> inline bool operator!=(T* a, const OORef<U>& b)
{
    return a != b.get();
}

template<class T> inline bool operator==(const OORef<T>& p, std::nullptr_t) Q_DECL_NOTHROW
{
    return p.get() == nullptr;
}

template<class T> inline bool operator==(std::nullptr_t, const OORef<T>& p) Q_DECL_NOTHROW
{
    return p.get() == nullptr;
}

template<class T> inline bool operator!=(const OORef<T>& p, std::nullptr_t) Q_DECL_NOTHROW
{
    return p.get() != nullptr;
}

template<class T> inline bool operator!=(std::nullptr_t, const OORef<T>& p) Q_DECL_NOTHROW
{
    return p.get() != nullptr;
}

template<class T> inline bool operator<(const OORef<T>& a, const OORef<T>& b)
{
    return std::less<T*>()(a.get(), b.get());
}

template<class T> void swap(OORef<T>& lhs, OORef<T>& rhs) Q_DECL_NOTHROW
{
	lhs.swap(rhs);
}

template<class T> T* get_pointer(const OORef<T>& p)
{
    return p.get();
}

template<class T, class U> OORef<T> static_pointer_cast(const OORef<U>& p)
{
    return static_cast<T*>(p.get());
}

template<class T, class U> OORef<T> const_pointer_cast(const OORef<U>& p)
{
    return const_cast<T*>(p.get());
}

template<class T, class U> OORef<T> dynamic_pointer_cast(const OORef<U>& p)
{
    return qobject_cast<T*>(p.get());
}

template<class T> QDebug operator<<(QDebug debug, const OORef<T>& p)
{
	return debug << p.get();
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_OBJECT_REFERENCE_H
