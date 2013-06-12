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

class FutureWatcher : public QObject
{
public:

	FutureWatcher() {}

	//template<typename R>
	//void setFuture(const Future<R>& future);

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

#if 0
	virtual void customEvent(QEvent* event) override {
		if(event->type() == QEvent::User) {
			if(static_cast<FutureCallOutEvent*>(event)->_callOutType == FutureCallOutEvent::Canceled)
				Q_EMIT canceled();
			else if(static_cast<FutureCallOutEvent*>(event)->_callOutType == FutureCallOutEvent::ResultReady)
				Q_EMIT resultReady();
		}
		QObject::customEvent(event);
	}
#endif

Q_SIGNALS:

	void canceled();
	void finished();
	void started();
	void resultReady();

private:

	friend class FutureInterfaceBase;
	Q_OBJECT
};

};

#endif // __OVITO_FUTURE_WATCHER_H
