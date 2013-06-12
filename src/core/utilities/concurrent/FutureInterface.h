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

#ifndef __OVITO_FUTURE_INTERFACE_H
#define __OVITO_FUTURE_INTERFACE_H

#include <core/Core.h>
#include "FutureWatcher.h"

namespace Ovito {

template<typename R> class Future;						// Defined in Future.h
template<typename R, typename Function> class Task;		// Defined in Task.h

class FutureInterfaceBase
{
public:

	enum State {
		NoState    = 0,
		Running    = (1<<0),
		Started    = (1<<1),
		Canceled   = (1<<2),
		Finished   = (1<<3),
		ResultSet  = (1<<4)
	};

	bool isRunning() const { return (_state & Running); }
	bool isCanceled() const { return (_state & Canceled); }
	bool isStarted() const { return (_state & Started); }
	bool isFinished() const { return (_state & Finished); }
	bool isResultSet() const { return (_state & ResultSet); }

protected:

	FutureInterfaceBase(State initialState = NoState) : _subTask(nullptr), _state(initialState) {}

	void cancel() {
		QMutexLocker locker(&_mutex);
		if(isCanceled())
			return;

		if(_subTask)
			_subTask->cancel();

		_state = State(_state | Canceled);
		_waitCondition.wakeAll();
		sendCallOut(FutureWatcher::CallOutEvent::Canceled);
	}

    void reportStarted() {
        QMutexLocker locker(&_mutex);
        if(isStarted() || isCanceled() || isFinished())
            return;
        _state = State(Started | Running);
        sendCallOut(FutureWatcher::CallOutEvent::Started);
    }

    void reportFinished() {
        QMutexLocker locker(&_mutex);
        if(!isFinished()) {
            _state = State((_state & ~Running) | Finished);
            _waitCondition.wakeAll();
            sendCallOut(FutureWatcher::CallOutEvent::Finished);
        }
    }

    void reportException() {
    	QMutexLocker locker(&_mutex);
    	if(isCanceled() || isFinished())
    		return;

    	_exceptionStore = std::current_exception();
    	_state = State(_state | Canceled);
    	_waitCondition.wakeAll();
    	sendCallOut(FutureWatcher::CallOutEvent::Canceled);
    }

    void reportResultReady() {
    	if(isCanceled() || isFinished())
    		return;

        _waitCondition.wakeAll();
        sendCallOut(FutureWatcher::CallOutEvent::ResultReady);
    }

    void reportCanceled() {
    	cancel();
    }

    void throwPossibleException() {
    	if(_exceptionStore)
    		std::rethrow_exception(_exceptionStore);
    }

    void waitForResult() {
    	throwPossibleException();

    	QMutexLocker lock(&_mutex);
		if(!isRunning())
			return;
#if 0
		lock.unlock();

		// To avoid deadlocks and reduce the number of threads used, try to
		// run the runnable in the current thread.
		QThreadPool::globalInstance()->d_func()->stealRunnable(d->runnable);

		lock.relock();
		if(!isRunning())
			return;
#endif

		while(isRunning() && isResultSet() == false)
			_waitCondition.wait(&_mutex);

		throwPossibleException();
    }

    void waitForFinished() {
        QMutexLocker lock(&_mutex);
        const bool alreadyFinished = !isRunning();
        lock.unlock();

        if(!alreadyFinished) {
#if 0
            QThreadPool::globalInstance()->d_func()->stealRunnable(d->runnable);
#endif

            lock.relock();
            while(isRunning())
                _waitCondition.wait(&_mutex);
        }

        throwPossibleException();
    }

    void sendCallOut(FutureWatcher::CallOutEvent::CallOutType type) {}

	FutureInterfaceBase* _subTask;
	QList<FutureWatcher*> _watchers;
	QMutex _mutex;
	State _state;
	QWaitCondition _waitCondition;
	std::exception_ptr _exceptionStore;
};

template<typename R>
class FutureInterface : public FutureInterfaceBase
{
public:

	FutureInterface() {}

	template<typename RS>
	bool waitForSubTask(Future<RS>& subFuture) {
		QMutexLocker locker(&_mutex);
		this->_subTask = subFuture.interface().get();
		if(this->isCanceled()) subFuture.cancel();
		locker.unlock();
		try {
			subFuture.waitForFinished();
		}
		catch(...) {
			locker.relock();
			this->_subTask = nullptr;
			throw;
		}
		locker.relock();
		this->_subTask = nullptr;
		if(subFuture.isCanceled()) {
			this->cancel();
			return false;
		}
		return true;
	}

	void setResult(const R& value) {
		QMutexLocker locker(&_mutex);
		if(isCanceled() || isFinished())
			return;
		_result = value;
		_state = State(_state | ResultSet);
		reportResultReady();
	}

	void setResult(R&& value) {
		QMutexLocker locker(&_mutex);
		if(isCanceled() || isFinished())
			return;
		_result = std::move(value);
		_state = State(_state | ResultSet);
		reportResultReady();
	}

private:

	R _result;

	template<typename R2, typename Function> friend class Task;
	template<typename R2> friend class Future;
};

};

#endif // __OVITO_FUTURE_INTERFACE_H
