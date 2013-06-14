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

	FutureWatcher(QObject* parent = nullptr) : QObject(parent), _finished(false) {}

	virtual ~FutureWatcher() {
		setFutureInterface(nullptr, false);
	}

	template<typename R>
	void setFuture(const Future<R>& future);

	void setFutureInterface(const std::shared_ptr<FutureInterfaceBase>& futureInterface) {
		setFutureInterface(futureInterface, true);
	}

	void unsetFuture() {
		setFutureInterface(nullptr, true);
	}

	bool isCanceled() const;
	bool isFinished() const;
    int progressMaximum() const;
    int progressValue() const;
    QString progressText() const;

protected:

	class CallOutEvent : public QEvent
	{
	public:
	    enum CallOutType {
	    	Started = QEvent::User,
	    	Finished,
	        Canceled,
	        ResultReady,
	        ProgressValue,
	        ProgressRange,
	        ProgressText,
	    };

	    CallOutEvent(CallOutType callOutType) : QEvent((QEvent::Type)callOutType) {}
	    CallOutEvent(CallOutType callOutType, int value) : QEvent((QEvent::Type)callOutType), _value(value) {}
	    CallOutEvent(CallOutType callOutType, const QString& text) : QEvent((QEvent::Type)callOutType), _text(text) {}

	    int _value;
	    QString _text;
	};

    void postCallOutEvent(CallOutEvent::CallOutType type) {
    	QCoreApplication::postEvent(this, new CallOutEvent(type));
    }

    void postCallOutEvent(CallOutEvent::CallOutType type, int value) {
    	QCoreApplication::postEvent(this, new CallOutEvent(type, value));
    }

    void postCallOutEvent(CallOutEvent::CallOutType type, const QString& text) {
    	QCoreApplication::postEvent(this, new CallOutEvent(type, text));
    }

	virtual void customEvent(QEvent* event) override;

	void setFutureInterface(const std::shared_ptr<FutureInterfaceBase>& futureInterface, bool pendingAssignment);

Q_SIGNALS:

	void canceled();
	void finished();
	void started();
	void resultReady();
	void progressRangeChanged(int maximum);
	void progressValueChanged(int progressValue);
	void progressTextChanged(const QString& progressText);

public Q_SLOTS:

	void cancel();

private:

	std::shared_ptr<FutureInterfaceBase> _futureInterface;
    bool _finished;

	friend class FutureInterfaceBase;
	Q_OBJECT
};

};

#endif // __OVITO_FUTURE_WATCHER_H
