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

#include <core/Core.h>
#include "FutureWatcher.h"
#include "FutureInterface.h"
#include "Future.h"
#include "moc_FutureWatcher.cpp"

namespace Ovito {

enum {
    MaxProgressEmitsPerSecond = 10
};

void FutureInterfaceBase::setProgressRange(int maximum)
{
    QMutexLocker locker(&_mutex);
    _progressMaximum = maximum;
    sendCallOut(FutureWatcher::CallOutEvent::ProgressRange, maximum);
}

void FutureInterfaceBase::setProgressValue(int value)
{
    QMutexLocker locker(&_mutex);

    if(value == _progressValue)
    	return;

    if(isCanceled() || isFinished())
        return;

    _progressValue = value;
    if(_progressTime.isValid() && _progressValue != _progressMaximum)
    	if(_progressTime.elapsed() < (1000 / MaxProgressEmitsPerSecond))
            return;

    _progressTime.start();
    sendCallOut(FutureWatcher::CallOutEvent::ProgressValue, value);
}

void FutureInterfaceBase::setProgressText(const QString& progressText)
{
    QMutexLocker locker(&_mutex);

    if(isCanceled() || isFinished())
        return;

    _progressText = progressText;
    sendCallOut(FutureWatcher::CallOutEvent::ProgressText, progressText);
}

bool FutureInterfaceBase::isProgressUpdateNeeded()
{
    QMutexLocker locker(&_mutex);
    return !_progressTime.isValid() || (_progressTime.elapsed() > (1000 / MaxProgressEmitsPerSecond));
}

void FutureWatcher::setFutureInterface(const std::shared_ptr<FutureInterfaceBase>& futureInterface, bool pendingAssignment)
{
	if(futureInterface == _futureInterface)
		return;

	if(_futureInterface) {
		if(pendingAssignment) {
	        _finished = false;
	        QCoreApplication::removePostedEvents(this);
		}
		_futureInterface->unregisterWatcher(this);
	}
	_futureInterface = futureInterface;
	if(_futureInterface)
		_futureInterface->registerWatcher(this);
}

void FutureWatcher::customEvent(QEvent* event)
{
	if(_futureInterface) {
		if(event->type() == (QEvent::Type)CallOutEvent::Started)
			Q_EMIT started();
		else if(event->type() == (QEvent::Type)CallOutEvent::Finished) {
			_finished = true;
			Q_EMIT finished();
		}
		else if(event->type() == (QEvent::Type)CallOutEvent::Canceled)
			Q_EMIT canceled();
		else if(event->type() == (QEvent::Type)CallOutEvent::ResultReady) {
			if(!_futureInterface->isCanceled()) {
				Q_EMIT resultReady();
			}
		}
		else if(event->type() == (QEvent::Type)CallOutEvent::ProgressValue) {
			if(!_futureInterface->isCanceled())
				Q_EMIT progressValueChanged(static_cast<CallOutEvent*>(event)->_value);
		}
		else if(event->type() == (QEvent::Type)CallOutEvent::ProgressText) {
			if(!_futureInterface->isCanceled())
				Q_EMIT progressTextChanged(static_cast<CallOutEvent*>(event)->_text);
		}
		else if(event->type() == (QEvent::Type)CallOutEvent::ProgressRange) {
			Q_EMIT progressRangeChanged(static_cast<CallOutEvent*>(event)->_value);
		}
	}
	QObject::customEvent(event);
}

void FutureWatcher::cancel()
{
	if(_futureInterface)
		_futureInterface->cancel();
}

bool FutureWatcher::isCanceled() const
{
	return _futureInterface->isCanceled();
}

bool FutureWatcher::isFinished() const
{
	return _finished;
}

int FutureWatcher::progressMaximum() const
{
	return _futureInterface->progressMaximum();
}

int FutureWatcher::progressValue() const
{
	return _futureInterface->progressValue();
}

QString FutureWatcher::progressText() const
{
	return _futureInterface->progressText();
}


};
