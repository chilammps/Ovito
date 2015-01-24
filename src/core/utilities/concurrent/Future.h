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

#ifndef __OVITO_FUTURE_H
#define __OVITO_FUTURE_H

#include <core/Core.h>
#include "FutureInterface.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Concurrency)

class FutureBase {
public:

	bool isCanceled() const { return getinterface()->isCanceled(); }
	bool isFinished() const { return getinterface()->isFinished(); }

	void cancel() { getinterface()->cancel(); }
	void waitForFinished() const {
		getinterface()->waitForFinished();
	}
	bool isValid() const { return (bool)_interface; }
	void reset() { _interface.reset(); }

    int progressValue() const { return getinterface()->progressValue(); }
    int progressMaximum() const { return getinterface()->progressMaximum(); }
    QString progressText() const { return getinterface()->progressText(); }

protected:

	FutureBase() {}

	explicit FutureBase(const std::shared_ptr<FutureInterfaceBase>& p) : _interface(p) {}
	
	std::shared_ptr<FutureInterfaceBase>& getinterface() {
		OVITO_ASSERT(isValid());
		return _interface;
	}
	const std::shared_ptr<FutureInterfaceBase>& getinterface() const {
		OVITO_ASSERT(isValid());
		return _interface;
	}
	std::shared_ptr<FutureInterfaceBase> _interface;

	template<typename R2, typename Function> friend class Task;
	template<typename R2> friend class FutureInterface;
	friend class FutureInterfaceBase;
	friend class FutureWatcher;
	friend class TaskManager;
};

template<typename R>
class Future : public FutureBase {
public:
	typedef FutureInterface<R> Interface;

	Future() {}

	explicit Future(const std::shared_ptr<Interface>& p) : FutureBase(p) {}

	/// Create a Future with immediate results.
	static Future createImmediate(const R& result, const QString& text = QString()) {
		auto iface = std::make_shared<Interface>();
		iface->reportStarted();
		if(text.isEmpty() == false)
			iface->setProgressText(text);
		iface->setResult(result);
		iface->reportFinished();
		return Future(iface);
	}

	/// Create a Future with a thrown exception.
	static Future createFailed(const Exception& ex) {
		auto iface = std::make_shared<Interface>();
		iface->reportStarted();
		iface->reportException(std::make_exception_ptr(ex));
		iface->reportFinished();
		return Future(iface);
	}

	/// Create a Future without results that is in the canceled state.
	static Future createCanceled() {
		auto iface = std::make_shared<Interface>();
		iface->reportStarted();
		iface->cancel();
		iface->reportFinished();
		return Future(iface);
	}

	const R& result() const {
		getinterface()->waitForResult();
		return static_cast<FutureInterface<R>*>(getinterface().get())->_result;
	}
};

template<>
class Future<void> : public FutureBase {
public:
	Future() {}
	explicit Future(const std::shared_ptr<FutureInterface<void>>& p) : FutureBase(p) {}

	void result() const {
		getinterface()->waitForResult();
	}
};

inline void FutureWatcher::setFuture(const FutureBase& future)
{
	setFutureInterface(future.getinterface());
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_BACKGROUND_OPERATION_H
