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

#ifndef __OVITO_FUTURE_WATCHER_H
#define __OVITO_FUTURE_WATCHER_H

#include <core/Core.h>

namespace Ovito {

template<typename R> class Future;
class FutureInterfaceBase;

class FutureWatcher : public QObject
{
public:

	FutureWatcher() : _finished(false) {}
	virtual ~FutureWatcher() {
		setFutureInterface(nullptr, false);
	}

	template<typename R>
	void setFuture(const Future<R>& future);

	void unsetFuture() {
		setFutureInterface(nullptr, true);
	}

protected:

	class CallOutEvent : public QEvent
	{
	public:
	    enum CallOutType {
	    	Started = QEvent::User,
	    	Finished,
	        Canceled,
	        ResultReady,
	    };

	    CallOutEvent(CallOutType callOutType) : QEvent((QEvent::Type)callOutType) {}
	};

    void postCallOutEvent(CallOutEvent::CallOutType type) {
    	QCoreApplication::postEvent(this, new CallOutEvent(type));
    }

	virtual void customEvent(QEvent* event) override;

	void setFutureInterface(const std::shared_ptr<FutureInterfaceBase>& futureInterface, bool pendingAssignment);

Q_SIGNALS:

	void canceled();
	void finished();
	void started();
	void resultReady();

private:

	std::shared_ptr<FutureInterfaceBase> _futureInterface;
    bool _finished;

	friend class FutureInterfaceBase;
	Q_OBJECT
};

};

#endif // __OVITO_FUTURE_WATCHER_H
