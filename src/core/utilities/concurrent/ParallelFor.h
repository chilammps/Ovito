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

#ifndef __OVITO_PARALLEL_FOR_H
#define __OVITO_PARALLEL_FOR_H

#include <core/Core.h>
#include "FutureInterface.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Concurrency)

template<class Function, typename T>
bool parallelFor(
		T loopCount,
		FutureInterfaceBase& futureInterface,
		Function kernel,
		T progressChunkSize = 1024)
{
	futureInterface.setProgressRange(loopCount / progressChunkSize);
	futureInterface.setProgressValue(0);

	std::vector<std::thread> workers;
	int num_threads = std::max(1, QThread::idealThreadCount());
	T chunkSize = loopCount / num_threads;
	T startIndex = 0;
	T endIndex = chunkSize;
	for(int t = 0; t < num_threads; t++) {
		if(t == num_threads - 1)
			endIndex += loopCount % num_threads;
		workers.push_back(std::thread([&futureInterface, &kernel, startIndex, endIndex, progressChunkSize]() {
			for(T i = startIndex; i < endIndex;) {
				// Execute kernel.
				kernel(i);

				i++;

				// Update progress indicator.
				if((i % progressChunkSize) == 0) {
					if(i != 0)
						futureInterface.incrementProgressValue();
					if(futureInterface.isCanceled())
						return;
				}
			}
		}));
		startIndex = endIndex;
		endIndex += chunkSize;
	}

	for(auto& t : workers)
		t.join();

	futureInterface.incrementProgressValue(loopCount % progressChunkSize);
	return !futureInterface.isCanceled();
}

template<class Function, typename T>
void parallelFor(T loopCount, Function kernel)
{
	std::vector<std::thread> workers;
	int num_threads = QThread::idealThreadCount();
	if(num_threads < 1) num_threads = 1;
	else if(num_threads > loopCount) {
		if(loopCount <= 0) return;
		num_threads = loopCount;
	}
	T chunkSize = loopCount / num_threads;
	T startIndex = 0;
	T endIndex = chunkSize;
	for(int t = 0; t < num_threads; t++) {
		if(t == num_threads - 1) {
			OVITO_ASSERT(endIndex + (loopCount % num_threads) == loopCount);
			endIndex = loopCount;
			for(T i = startIndex; i < endIndex; ++i) {
				kernel(i);
			}
		}
		else {
			OVITO_ASSERT(endIndex <= loopCount);
			workers.push_back(std::thread([&kernel, startIndex, endIndex]() {
				for(T i = startIndex; i < endIndex; ++i) {
					kernel(i);
				}
			}));
		}
		startIndex = endIndex;
		endIndex += chunkSize;
	}

	for(auto& t : workers) {
		t.join();
	}
}

template<class Function>
void parallelForChunks(size_t loopCount, Function kernel)
{
	std::vector<std::thread> workers;
	size_t num_threads = QThread::idealThreadCount();
	if(num_threads < 1) num_threads = 1;
	else if(num_threads > loopCount) {
		if(loopCount <= 0) return;
		num_threads = loopCount;
	}
	size_t chunkSize = loopCount / num_threads;
	size_t startIndex = 0;
	for(size_t t = 0; t < num_threads; t++) {
		if(t == num_threads - 1) {
			chunkSize += loopCount % num_threads;
			OVITO_ASSERT(startIndex + chunkSize == loopCount);
			kernel(startIndex, chunkSize);
		}
		else {
			workers.push_back(std::thread([&kernel, startIndex, chunkSize]() {
				kernel(startIndex, chunkSize);
			}));
		}
		startIndex += chunkSize;
	}
	for(auto& t : workers)
		t.join();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_PARALLEL_FOR_H
