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
 * \file OvitoObjectReference.h
 * \brief Contains the definition of the Ovito::OORef class.
 */

#ifndef __OVITO_OBJECT_REFERENCE_H
#define __OVITO_OBJECT_REFERENCE_H

#include <core/Core.h>

namespace Ovito {

/**
 * \brief A smart pointer to an OvitoObject that uses reference counting system.
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
    	if(px != 0) px->decrementReferenceCount();
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

    typedef element_type* this_type::*unspecified_bool_type;
    operator unspecified_bool_type() const Q_DECL_NOTHROW {
    	return px == 0 ? 0 : &this_type::px;
    }

    // operator! is redundant, but some compilers need it
    bool operator!() const Q_DECL_NOTHROW {
    	return px == 0;
    }

    void reset() Q_DECL_NOTHROW {
    	this_type().swap( *this );
    }

    void reset(T* rhs) {
    	this_type( rhs ).swap( *this );
    }

    T * get() const Q_DECL_NOTHROW {
    	return px;
    }

    T& operator*() const {
    	OVITO_ASSERT(px != nullptr);
    	return *px;
    }

    T* operator->() const {
    	OVITO_ASSERT(px != nullptr);
    	return px;
    }

    void swap(OORef& rhs) Q_DECL_NOTHROW {
    	T* tmp = px;
    	px = rhs.px;
    	rhs.px = tmp;
    }

private:

    T* px;
};

template<class T, class U> inline bool operator==(OORef<T> const & a, OORef<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(OORef<T> const & a, OORef<U> const & b)
{
    return a.get() != b.get();
}

template<class T, class U> inline bool operator==(OORef<T> const & a, U * b)
{
    return a.get() == b;
}

template<class T, class U> inline bool operator!=(OORef<T> const & a, U * b)
{
    return a.get() != b;
}

template<class T, class U> inline bool operator==(T * a, OORef<U> const & b)
{
    return a == b.get();
}

template<class T, class U> inline bool operator!=(T * a, OORef<U> const & b)
{
    return a != b.get();
}


template<class T> inline bool operator==(OORef<T> const & p, std::nullptr_t) Q_DECL_NOTHROW
{
    return p.get() == nullptr;
}

template<class T> inline bool operator==(std::nullptr_t, OORef<T> const & p) Q_DECL_NOTHROW
{
    return p.get() == nullptr;
}

template<class T> inline bool operator!=(OORef<T> const & p, std::nullptr_t) Q_DECL_NOTHROW
{
    return p.get() != nullptr;
}

template<class T> inline bool operator!=(std::nullptr_t, OORef<T> const & p) Q_DECL_NOTHROW
{
    return p.get() != nullptr;
}

template<class T> inline bool operator<(OORef<T> const & a, OORef<T> const & b)
{
    return std::less<T*>()(a.get(), b.get());
}

template<class T> void swap(OORef<T> & lhs, OORef<T> & rhs)
{
    lhs.swap(rhs);
}

template<class T> T* get_pointer(OORef<T> const & p)
{
    return p.get();
}

template<class T, class U> OORef<T> static_pointer_cast(OORef<U> const & p)
{
    return static_cast<T*>(p.get());
}

template<class T, class U> OORef<T> const_pointer_cast(OORef<U> const & p)
{
    return const_cast<T*>(p.get());
}

template<class T, class U> OORef<T> dynamic_pointer_cast(OORef<U> const & p)
{
    return dynamic_cast<T*>(p.get());
}

};	// End of namespace Ovito

#endif // __OVITO_OBJECT_REFERENCE_H
