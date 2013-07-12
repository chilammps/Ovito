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

namespace Ovito {

template<class Function>
bool parallelFor(size_t loopCount, FutureInterfaceBase& futureInterface, Function kernel, size_t progressChunkSize = 1024)
{
	futureInterface.setProgressRange(loopCount / progressChunkSize);

	std::vector<std::thread> workers;
	int num_threads = std::max(1, QThread::idealThreadCount());
	size_t chunkSize = loopCount / num_threads;
	size_t startIndex = 0;
	size_t endIndex = chunkSize;
	for(int t = 0; t < num_threads; t++) {
		if(t == num_threads - 1)
			endIndex += loopCount % num_threads;
		workers.push_back(std::thread([&futureInterface, &kernel, startIndex, endIndex, progressChunkSize]() {
			for(size_t i = startIndex; i < endIndex;) {
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

};

#endif // __OVITO_PARALLEL_FOR_H
