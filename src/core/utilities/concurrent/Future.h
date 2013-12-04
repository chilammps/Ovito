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

namespace Ovito {

class FutureBase {
public:

	bool isCanceled() const { return interface()->isCanceled(); }
	bool isFinished() const { return interface()->isFinished(); }

	void cancel() { interface()->cancel(); }
	void waitForFinished() const {
		interface()->waitForFinished();
	}
	bool isValid() const { return (bool)_interface; }
	void reset() { _interface.reset(); }

    int progressValue() const { return interface()->progressValue(); }
    int progressMaximum() const { return interface()->progressMaximum(); }
    QString progressText() const { return interface()->progressText(); }

protected:

	FutureBase() {}

	explicit FutureBase(const std::shared_ptr<FutureInterfaceBase>& p) : _interface(p) {}

	std::shared_ptr<FutureInterfaceBase>& interface() {
		OVITO_ASSERT(isValid());
		return _interface;
	}
	const std::shared_ptr<FutureInterfaceBase>& interface() const {
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
		auto interface = std::make_shared<Interface>();
		interface->reportStarted();
		if(text.isEmpty() == false)
			interface->setProgressText(text);
		interface->setResult(result);
		interface->reportFinished();
		return Future(interface);
	}

	/// Create a Future without results that is in the canceled state.
	static Future createCanceled() {
		auto interface = std::make_shared<Interface>();
		interface->reportStarted();
		interface->cancel();
		interface->reportFinished();
		return Future(interface);
	}

	const R& result() const {
		interface()->waitForResult();
		return static_cast<FutureInterface<R>*>(interface().get())->_result;
	}
};

template<>
class Future<void> : public FutureBase {
public:
	Future() {}
	explicit Future(const std::shared_ptr<FutureInterface<void>>& p) : FutureBase(p) {}

	void result() const {
		interface()->waitForResult();
	}
};

inline void FutureWatcher::setFuture(const FutureBase& future)
{
	setFutureInterface(future.interface());
}

};

#endif // __OVITO_BACKGROUND_OPERATION_H
