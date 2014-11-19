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
 * \brief Contains the definition of the Ovito::Util::MemoryPool class template.
 */

#ifndef __OVITO_MEMORY_POOL_H
#define __OVITO_MEMORY_POOL_H

#include <base/Base.h>

namespace Ovito { namespace Util {

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
	MemoryPool(size_t pageSize = 1024) : lastPageNumber(pageSize), _pageSize(pageSize) {}

	/// Releases the memory reserved by this pool and destroys all allocated object instances.
	~MemoryPool() { clear(); }

	/// Allocates, constructs, and returns a new object instance.
	/// Any arguments passed to this method are forwarded to the class constructor.
	template<class... Args>
	inline T* construct(Args&&... args) {
		T* p = malloc();
		alloc.construct(p, std::forward<Args>(args)...);
		return p;
	}

	/// Destroys all object instances belonging to the memory pool
	/// and releases the memory pages allocated by the pool.
	inline void clear(bool keepPageReserved = false) {
		for(auto i = pages.cbegin(); i != pages.cend(); ++i) {
			T* p = *i;
			T* pend = p + pageSize;
			if(i+1 == pages.end())
				pend = p + lastPageNumber;
			for(; p != pend; ++p)
				alloc.destroy(p);
			if(!keepPageReserved || i != pages.begin())
				alloc.deallocate(*i, pageSize);
		}
		if(!keepPageReserved) {
			pages.clear();
			lastPageNumber = pageSize;
		}
		else if(!pages.empty()) {
			pages.resize(1);
			lastPageNumber = 0;
		}
	}

	/// Returns the number of bytes currently reserved by this memory pool.
	size_t memoryUsage() const {
		return pages.size() * pageSize * sizeof(T);
	}

	/// Swaps this memory pool with another pool instance.
	void swap(MemoryPool<T>& other) {
		pages.swap(other.pages);
		std::swap(lastPageNumber, other.lastPageNumber);
		std::swap(pageSize, other.pageSize);
		std::swap(alloc, other.alloc);
	}

private:

	/// Allocates memory for a new object instance.
	T* malloc() {
		T* p;
		if(lastPageNumber == pageSize) {
			pages.push_back(p = alloc.allocate(pageSize));
			lastPageNumber = 1;
		}
		else {
			p = pages.back() + lastPageNumber;
			lastPageNumber++;
		}
		return p;
	}

	std::vector<T*> pages;
	size_t lastPageNumber;
	size_t _pageSize;
	std::allocator<T> alloc;
};

}}	// End of namespace

#endif // __OVITO_MEMORY_POOL_H

