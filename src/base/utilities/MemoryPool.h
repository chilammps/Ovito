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
 * \file MemoryPool.h
 * \brief Contains the definition of the Ovito::MemoryPool template class.
 */

#ifndef __OVITO_MEMORY_POOL_H
#define __OVITO_MEMORY_POOL_H

#include <base/Base.h>

namespace Ovito {

/**
 * Template class that provides a memory pool for efficient allocation of object instances.
 *
 * Instances of a certain class/struct can be allocated by this class very efficiently.
 * A restriction is that all instances can only be destroyed at once.
 */
template<typename T>
class MemoryPool
{
public:

	/// Constructor.
	MemoryPool(size_t _pageSize = 1024) : lastPageNumber(_pageSize), pageSize(_pageSize) {}

	/// Destructor.
	~MemoryPool() { clear(); }

	/// Allocates and constructs a new object instance.
	template<class... Types>
	inline T* construct(const Types&... args) {
		T* p = malloc();
		alloc.construct(p, args...);
		return p;
	}

	/// Destroys all object instances.
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

	size_t memoryUsage() const {
		return pages.size() * pageSize * sizeof(T);
	}

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
	size_t pageSize;
	std::allocator<T> alloc;
};

}; // End of namespace

#endif // __OVITO_MEMORY_POOL_H

