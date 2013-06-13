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

#ifndef __OVITO_TASK_H
#define __OVITO_TASK_H

#include <core/Core.h>
#include "FutureInterface.h"
#include "Future.h"

namespace Ovito {

template<typename R, typename Function>
class Task : public QRunnable
{
public:

	Task(Function fn) : _p(std::make_shared<FutureInterface<R>>()), _function(fn) {}

	virtual void run() override {
		try {
			_function(*_p.get());
		}
		catch(...) {
			_p->reportException();
		}
		_p->reportFinished();
	}

	Future<R> start() {
		_p->_runnable = this;
		_p->reportStarted();
		std::shared_ptr<FutureInterface<R>> p2(_p);
		QThreadPool::globalInstance()->start(this);
		return Future<R>(p2);
	}

	Future<R> future() const { return Future<R>(_p); }

	void abort() {
		Future<R>(_p).abort();
	}

private:
	Function _function;
	std::shared_ptr<FutureInterface<R>> _p;
};

template<typename R, typename Function>
Future<R> runInBackground(Function f)
{
	return (new Task<R,Function>(f))->start();
}

};

#endif // __OVITO_BACKGROUND_OPERATION_H
