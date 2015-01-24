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

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Concurrency)

class FutureBase;
class FutureInterfaceBase;

class OVITO_CORE_EXPORT FutureWatcher : public QObject
{
public:

	FutureWatcher(QObject* parent = nullptr) : QObject(parent), _finished(false) {}

	virtual ~FutureWatcher() {
		setFutureInterface(nullptr, false);
	}

	void setFuture(const FutureBase& future);

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

	void waitForFinished() const;

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

	    CallOutEvent(CallOutType callOutType, FutureInterfaceBase* source) : QEvent((QEvent::Type)callOutType), _source(source) {}
	    CallOutEvent(CallOutType callOutType, int value, FutureInterfaceBase* source) : QEvent((QEvent::Type)callOutType), _value(value), _source(source) {}
	    CallOutEvent(CallOutType callOutType, const QString& text, FutureInterfaceBase* source) : QEvent((QEvent::Type)callOutType), _text(text), _source(source) {}

	    int _value;
	    QString _text;
	    FutureInterfaceBase* _source;
	};

    void postCallOutEvent(CallOutEvent::CallOutType type, FutureInterfaceBase* source) {
    	OVITO_ASSERT(source == _futureInterface.get());
    	QCoreApplication::postEvent(this, new CallOutEvent(type, _futureInterface.get()));
    }

    void postCallOutEvent(CallOutEvent::CallOutType type, int value, FutureInterfaceBase* source) {
    	OVITO_ASSERT(source == _futureInterface.get());
    	QCoreApplication::postEvent(this, new CallOutEvent(type, value, _futureInterface.get()));
    }

    void postCallOutEvent(CallOutEvent::CallOutType type, const QString& text, FutureInterfaceBase* source) {
    	OVITO_ASSERT(source == _futureInterface.get());
    	QCoreApplication::postEvent(this, new CallOutEvent(type, text, _futureInterface.get()));
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

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_FUTURE_WATCHER_H
