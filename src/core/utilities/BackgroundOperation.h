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

#ifndef __OVITO_BACKGROUND_OPERATION_H
#define __OVITO_BACKGROUND_OPERATION_H

#include <core/Core.h>

namespace Ovito {

class TaskBase : public QRunnable
{
public:

	class FutureInterfaceBase {
	public:
		bool isCanceled() const { return _isCanceled; }
	private:
		FutureInterfaceBase() : _isCanceled(false) {}
		void cancel() { _isCanceled = true; }
		volatile bool _isCanceled;
	};

public:

	TaskBase() : _p(std::make_shared<FutureInterface>()) {}

	virtual void run() override {
		try {
			_p->_promise.set_value(_function(*_p.get()));
		}
		catch(...) {
			_p->_promise.set_exception(std::current_exception());
		}
	}

	Future future() const { return Future(_p); }

	void abort() {
		Future future(_p);
		future.cancel();
		future.waitForFinished();
	}

private:
	Function _function;
	std::shared_ptr<FutureInterface> _p;
};


template<typename R, typename Function>
class Task : public QRunnable
{
public:

	class FutureInterface {
	public:
		bool isCanceled() const { return _isCanceled; }
		template<typename RS, typename Function>
		bool waitForSubTask(Task<RS>::Future& subFuture) {
			subFuture.waitForFinished();
			return isCanceled();
		}
	private:
		FutureInterface() : _isCanceled(false), _future(_promise.get_future()) {}
		void cancel() { _isCanceled = true; }
		std::promise<R> _promise;
		std::future<R> _future;
		volatile bool _isCanceled;
	};

	class Future {
	public:
		Future(const R& result) : _p(std::make_shared<FutureInterface>()) { _p->_promise.set_value(result); }
		Future(R&& result) : _p(std::make_shared<FutureInterface>()) { _p->_promise.set_value(std::move(result)); }
		bool isCanceled() const { return _p->isCanceled(); }
		void cancel() { _p->cancel(); }
		const R& result() const {
			OVITO_ASSERT(!_p->_isCanceled);
			OVITO_ASSERT(_p->_future.valid());
			return _p->_future.get();
		}
		void waitForFinished() const {
			OVITO_ASSERT(_p->_future.valid());
			_p->_future.wait();
		}
	private:
		Future(const std::shared_ptr<FutureInterface>& p) : _p(p) {}
		std::shared_ptr<FutureInterface> _p;
	};

public:

	Task() : _p(std::make_shared<FutureInterface>()) {}

	virtual void run() override {
		try {
			_p->_promise.set_value(_function(*_p.get()));
		}
		catch(...) {
			_p->_promise.set_exception(std::current_exception());
		}
	}

	Future future() const { return Future(_p); }

	void abort() {
		Future future(_p);
		future.cancel();
		future.waitForFinished();
	}

private:
	Function _function;
	std::shared_ptr<FutureInterface> _p;
};

template<typename TM, typename TS>
bool waitForSlaveFuture(Task<TM>::FutureInterface& masterInterface, Task<TS>::Future& slaveFuture)
{
	{
		QFutureWatcher<TM> watcher;
		watcher.moveToThread(QApplication::instance()->thread());
		QObject::connect(&watcher, &QFutureWatcher<TM>::canceled, [&slaveFuture]() { slaveFuture.cancel(); } );
		watcher.setFuture(masterInterface.future());
		slaveFuture.waitForFinished();
	}
	if(slaveFuture.isCanceled()) {
		masterInterface.cancel();
		return false;
	}
	return true;
}

};

#endif // __OVITO_BACKGROUND_OPERATION_H
