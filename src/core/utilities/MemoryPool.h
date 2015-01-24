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
 * \file
 * \brief Contains the definition of the Ovito::MemoryPool class template.
 */

#ifndef __OVITO_MEMORY_POOL_H
#define __OVITO_MEMORY_POOL_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util)

/**
 * \brief A simple memory pool for the efficient allocation of a large number of object instances.
 *
 * \tparam T The type of object to be allocated by the memory pool.
 *
 * New object instances can be dynamically allocated via #construct().
 * A restriction is that all instances belonging to the memory can only be destroyed at once using the
 * #clear() method. This memory pool provides no way to free the memory of individual object instances.
 *
 */
template<typename T>
class MemoryPool
{
public:

	/// Constructs a new memory pool.
	/// \param pageSize Controls the number of objects per memory page allocated by this pool.
	MemoryPool(size_t pageSize = 1024) : _lastPageNumber(pageSize), _pageSize(pageSize) {}

	/// Releases the memory reserved by this pool and destroys all allocated object instances.
	~MemoryPool() { clear(); }

	/// Allocates, constructs, and returns a new object instance.
	/// Any arguments passed to this method are forwarded to the class constructor.
	template<class... Args>
	inline T* construct(Args&&... args) {
		T* p = malloc();
		_alloc.construct(p, std::forward<Args>(args)...);
		return p;
	}

	/// Destroys all object instances belonging to the memory pool
	/// and releases the memory pages allocated by the pool.
	inline void clear(bool keepPageReserved = false) {
		for(auto i = _pages.cbegin(); i != _pages.cend(); ++i) {
			T* p = *i;
			T* pend = p + _pageSize;
			if(i+1 == _pages.end())
				pend = p + _lastPageNumber;
			for(; p != pend; ++p)
				_alloc.destroy(p);
			if(!keepPageReserved || i != _pages.begin())
				_alloc.deallocate(*i, _pageSize);
		}
		if(!keepPageReserved) {
			_pages.clear();
			_lastPageNumber = _pageSize;
		}
		else if(!_pages.empty()) {
			_pages.resize(1);
			_lastPageNumber = 0;
		}
	}

	/// Returns the number of bytes currently reserved by this memory pool.
	size_t memoryUsage() const {
		return _pages.size() * _pageSize * sizeof(T);
	}

	/// Swaps this memory pool with another pool instance.
	void swap(MemoryPool<T>& other) {
		_pages.swap(other._pages);
		std::swap(_lastPageNumber, other._lastPageNumber);
		std::swap(_pageSize, other._pageSize);
		std::swap(_alloc, other._alloc);
	}

private:

	/// Allocates memory for a new object instance.
	T* malloc() {
		T* p;
		if(_lastPageNumber == _pageSize) {
			_pages.push_back(p = _alloc.allocate(_pageSize));
			_lastPageNumber = 1;
		}
		else {
			p = _pages.back() + _lastPageNumber;
			_lastPageNumber++;
		}
		return p;
	}

	std::vector<T*> _pages;
	size_t _lastPageNumber;
	size_t _pageSize;
	std::allocator<T> _alloc;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_MEMORY_POOL_H

