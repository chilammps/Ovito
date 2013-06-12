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

template<typename R> class Future;
template<typename R, typename Function> class Task;

class FutureWatcher : public QObject
{
public:

	FutureWatcher() {}

	template<typename R>
	void setFuture(const Future<R>& future);

protected:

	class FutureCallOutEvent : public QEvent
	{
	public:
	    enum CallOutType {
	        Canceled,
	        ResultReady,
	    };

	    FutureCallOutEvent(CallOutType callOutType) : QEvent(QEvent::User), _callOutType(callOutType) {}

	    CallOutType _callOutType;
	};

	void emitCanceled() {
		QCoreApplication::postEvent(this, new FutureCallOutEvent(FutureCallOutEvent::Canceled));
	}
	void emitResultReady() {
		QCoreApplication::postEvent(this, new FutureCallOutEvent(FutureCallOutEvent::ResultReady));
	}

	virtual void customEvent(QEvent* event) override {
		if(event->type() == QEvent::User) {
			if(static_cast<FutureCallOutEvent*>(event)->_callOutType == FutureCallOutEvent::Canceled)
				Q_EMIT canceled();
			else if(static_cast<FutureCallOutEvent*>(event)->_callOutType == FutureCallOutEvent::ResultReady)
				Q_EMIT resultReady();
		}
		QObject::customEvent(event);
	}

Q_SIGNALS:

	void canceled();
	void resultReady();

private:

	friend class FutureInterfaceBase;
};

class FutureInterfaceBase
{
public:
	bool isCanceled() const { return _isCanceled; }
protected:

	FutureInterfaceBase() : _isCanceled(false), _hasResultBeenSet(false), _subTask(nullptr) {}

	void cancel() {
		_isCanceled = true;
	}

	void emitCanceled() {
		QMutexLocker locker(&_mutex);
		if(!_signalObject.isNull())
			_signalObject->emitCanceled();
	}
	void emitResultReady() {
		QMutexLocker locker(&_mutex);
		if(!_signalObject.isNull())
			_signalObject->emitResultReady();
	}

	volatile bool _isCanceled;
	bool _hasResultBeenSet;
	volatile FutureInterfaceBase* _subTask;
	QList<FutureWatcher*> _watchers;
	QMutex _mutex;
};

template<typename R>
class FutureInterface : public FutureInterfaceBase
{
public:

	FutureInterface() : _future(_promise.get_future()) {}

	template<typename RS>
	bool waitForSubTask(Future<RS>& subFuture) {
		this->_subTask = subFuture._p.get();
		if(this->isCanceled()) subFuture.cancel();
		subFuture.waitForFinished();
		this->_subTask = nullptr;
		if(subFuture.isCanceled()) {
			this->cancel();
			return false;
		}
		return true;
	}

	void setResult(const R& value) {
		OVITO_ASSERT(_hasResultBeenSet == false);
		_hasResultBeenSet = true;
		_promise.set_value(value);
	}

	void setResult(R&& value) {
		OVITO_ASSERT(_hasResultBeenSet == false);
		_hasResultBeenSet = true;
		_promise.set_value(std::move(value));
	}

	void setResult(R& value) {
		OVITO_ASSERT(_hasResultBeenSet == false);
		_hasResultBeenSet = true;
		_promise.set_value(value);
	}

private:

	std::promise<R> _promise;
	std::future<R> _future;

	template<typename R2, typename Function> friend class Task;
	template<typename R2> friend class Future;
};

template<typename R>
class Future {
public:
	typedef FutureInterface<R> Interface;

	Future() {}
	Future(const R& result) : _p(std::make_shared<Interface>()) { _p->_promise.set_value(result); }
	Future(R&& result) : _p(std::make_shared<Interface>()) { _p->_promise.set_value(std::move(result)); }
	bool isCanceled() const { OVITO_ASSERT(isValid()); return _p->isCanceled(); }
	void cancel() { OVITO_ASSERT(isValid()); _p->cancel(); }
	R result() const {
		OVITO_ASSERT(isValid());
		OVITO_ASSERT(!isCanceled());
		OVITO_ASSERT(_p->_future.valid());
		return _p->_future.get();
	}
	void waitForFinished() const {
		OVITO_ASSERT(isValid());
		OVITO_ASSERT(_p->_future.valid());
		_p->_future.wait();
	}
	void abort() {
		cancel();
		waitForFinished();
	}
	bool isValid() const { return (bool)_p; }
private:
	Future(const std::shared_ptr<Interface>& p) : _p(p) {}
	std::shared_ptr<Interface> _p;

	template<typename R2, typename Function> friend class Task;
	template<typename R2> friend class FutureInterface;
};


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
			_p->_promise.set_exception(std::current_exception());
			_p->emitResultReady();
			return;
		}

		if(!_p->_hasResultBeenSet) {
			OVITO_ASSERT_MSG(_p->isCanceled(), "Task::run", "Promise has not been satisfied by the worker function.");
			_p->_promise.set_value(R());
			_p->emitCanceled();
		}
		else {
			if(_p->isCanceled())
				_p->emitCanceled();
			else
				_p->emitResultReady();
		}
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
	Task<R,Function>* task = new Task<R,Function>(f);
	Future<R> future = task->future();
	QThreadPool::globalInstance()->start(task);
	return future;
}

};

#endif // __OVITO_BACKGROUND_OPERATION_H
