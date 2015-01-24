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
 * \brief Contains the definition of the Ovito::BoundedPriorityQueue class template.
 */

#ifndef __OVITO_BOUNDED_PRIORITY_QUEUE_H
#define __OVITO_BOUNDED_PRIORITY_QUEUE_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util)

/**
 * \brief A container class implementing a priority queue with a fixed compile-time maximum capacity and a variable run-time capacity.
 *
 * \tparam T The type of the queue's elements.
 * \tparam Compare The functor type defining the priority order of elements in the queue.
 * \tparam QUEUE_SIZE_LIMIT The compile-time maximum capacity of the queue.
 *
 * While the queue has not reached its maximum capacity, elements are
 * inserted as they will be in a heap, the root (top()) being such that
 * Compare(top(),x)==false for any x in the queue.
 * Once the queue is full, trying to insert x in the queue will have no effect if
 * Compare(x,top())==false. Otherwise, the element at the root of the heap is removed
 * and x is inserted so as to keep the heap property.
 */
template<typename T, typename Compare = std::less<T>, int QUEUE_SIZE_LIMIT = 32>
class BoundedPriorityQueue
{
public:
	typedef T value_type;
	typedef const value_type* const_iterator;

	/// Constructor.
	BoundedPriorityQueue(int size, const Compare& comp = Compare()) : _count(0), _maxSize(size), _comp(comp) {
		OVITO_ASSERT(size <= QUEUE_SIZE_LIMIT);
	}

	/// Returns the current number of elements in the queue.
	int size() const { return _count; }

	/// Removes all elements of the queue. The max size remains unchanged.
	void clear() { _count = 0; }

	/// Returns whether the maximum queue size has been reached.
	bool full() const { return _count == _maxSize; }

	/// Returns whether the current queue size is zero.
	bool empty() const { return _count == 0; }

	/// Returns the greatest element.
	const value_type& top() const { return _data[0]; }

	/// Inserts a new element into the priority queue.
	void insert(const value_type& x) {
		value_type* data1 = (&_data[0]-1);
		if(full()) {
			if(_comp(x, top())) {
				int j = 1, k = 2;
				while(k <= _count) {
					value_type* z = &(data1[k]);
					if(k < _count && _comp(*z, data1[k+1]))
						z = &(data1[++k]);

					if(_comp(*z, x)) break;
					data1[j] = *z;
					j = k;
					k = j << 1;
				}
				data1[j] = x;
			}
		}
		else {
			int i = ++_count, j;
			while(i >= 2) {
				j = i >> 1;
				value_type& y = data1[j];
				if(_comp(x, y)) break;
				data1[i] = y;
				i = j;
			}
			data1[i] = x;
		}
	}

	/// Returns an iterator pointing to the first element in the queue.
	const_iterator begin() const { return _data; }

	/// Returns an iterator pointing to the element after the last element in the queue.
	const_iterator end() const { return _data + _count; }

	/// Returns the i-th entry in the queue.
	const value_type& operator[](int i) const { OVITO_ASSERT(i < _count); return _data[i]; }

	/// Sort the entries of the queue.
	void sort() { std::sort(_data, _data + _count, _comp); }

protected:

	int _count;
	int _maxSize;
	value_type _data[QUEUE_SIZE_LIMIT];
	Compare _comp;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_BOUNDED_PRIORITY_QUEUE_H

