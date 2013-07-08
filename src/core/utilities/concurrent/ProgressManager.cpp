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
#include <core/utilities/concurrent/ProgressManager.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/app/Application.h>

namespace Ovito {

/// The singleton instance of the class.
ProgressManager* ProgressManager::_instance = nullptr;

/******************************************************************************
* Initializes the progress manager.
******************************************************************************/
ProgressManager::ProgressManager()
{
	OVITO_ASSERT_MSG(!_instance, "ProgressManager constructor", "Multiple instances of this singleton class have been created.");
	qRegisterMetaType<FutureInterfacePointer>("FutureInterfacePointer");

	if(Application::instance().guiMode()) {
		// Create progress display widget.
		_progressTextDisplay = new QLabel();
		_progressWidget = new QWidget();
		QHBoxLayout* progressWidgetLayout = new QHBoxLayout(_progressWidget);
		progressWidgetLayout->setContentsMargins(QMargins());
		progressWidgetLayout->setSpacing(0);
		_progressBar = new QProgressBar(_progressWidget);
		_cancelTaskButton = new QPushButton(tr("Cancel"), _progressWidget);
		progressWidgetLayout->addWidget(_progressBar);
		progressWidgetLayout->addWidget(_cancelTaskButton);
		_progressWidget->setMaximumWidth(_progressWidget->minimumSizeHint().width());
		_widgetCleanupHandler.add(_progressWidget);
		_widgetCleanupHandler.add(_progressTextDisplay);
	}
	_indicatorVisible = false;

	connect(&_taskStartedSignalMapper, SIGNAL(mapped(QObject*)), this, SLOT(taskStarted(QObject*)));
	connect(&_taskFinishedSignalMapper, SIGNAL(mapped(QObject*)), this, SLOT(taskFinished(QObject*)));
	connect(&_taskProgressValueChangedSignalMapper, SIGNAL(mapped(QObject*)), this, SLOT(taskProgressValueChanged(QObject*)));
	connect(&_taskProgressTextChangedSignalMapper, SIGNAL(mapped(QObject*)), this, SLOT(taskProgressTextChanged(QObject*)));
	connect(_cancelTaskButton, SIGNAL(clicked(bool)), this, SLOT(cancelAll()));
}

/******************************************************************************
* Registers a future with the progress manager.
******************************************************************************/
void ProgressManager::addTaskInternal(FutureInterfacePointer futureInterface)
{
	FutureWatcher* watcher = new FutureWatcher(this);
	connect(watcher, SIGNAL(started()), &_taskStartedSignalMapper, SLOT(map()));
	connect(watcher, SIGNAL(finished()), &_taskFinishedSignalMapper, SLOT(map()));
	connect(watcher, SIGNAL(progressRangeChanged(int)), &_taskProgressValueChangedSignalMapper, SLOT(map()));
	connect(watcher, SIGNAL(progressValueChanged(int)), &_taskProgressValueChangedSignalMapper, SLOT(map()));
	connect(watcher, SIGNAL(progressTextChanged(const QString&)), &_taskProgressTextChangedSignalMapper, SLOT(map()));
	_taskStartedSignalMapper.setMapping(watcher, watcher);
	_taskFinishedSignalMapper.setMapping(watcher, watcher);
	_taskProgressValueChangedSignalMapper.setMapping(watcher, watcher);
	_taskProgressTextChangedSignalMapper.setMapping(watcher, watcher);

	// Activate the future watcher.
	watcher->setFutureInterface(futureInterface);
}

/******************************************************************************
* Is called when a task has started to run.
******************************************************************************/
void ProgressManager::taskStarted(QObject* object)
{
	FutureWatcher* watcher = static_cast<FutureWatcher*>(object);

	// Show progress indicator only if the task doesn't finish within 300 milliseconds.
	if(_taskStack.isEmpty() && Application::instance().guiMode())
		QTimer::singleShot(200, this, SLOT(showIndicator()));

	_taskStack.push(watcher);
}

/******************************************************************************
* Is called when a task has finished.
******************************************************************************/
void ProgressManager::taskFinished(QObject* object)
{
	FutureWatcher* watcher = static_cast<FutureWatcher*>(object);
	OVITO_ASSERT(_taskStack.contains(watcher));
	_taskStack.remove(_taskStack.indexOf(watcher));
	watcher->deleteLater();
	updateIndicator();
}

/******************************************************************************
* Is called when the progress of a task has changed
******************************************************************************/
void ProgressManager::taskProgressValueChanged(QObject* object)
{
	OVITO_ASSERT(_taskStack.contains(static_cast<FutureWatcher*>(object)));
	if(_taskStack.top() == object)
		updateIndicator();
}

/******************************************************************************
* Is called when the status text of a task has changed.
******************************************************************************/
void ProgressManager::taskProgressTextChanged(QObject* object)
{
	OVITO_ASSERT(_taskStack.contains(static_cast<FutureWatcher*>(object)));
	if(_taskStack.top() == object)
		updateIndicator();
}

/******************************************************************************
* Shows the progress indicator widget.
******************************************************************************/
void ProgressManager::showIndicator()
{
	if(_indicatorVisible == false && _taskStack.isEmpty() == false) {
		QStatusBar* statusBar = MainWindow::instance().statusBar();
		statusBar->addWidget(_progressTextDisplay, 1);
		statusBar->addPermanentWidget(_progressWidget);
		_progressTextDisplay->show();
		_progressWidget->show();
		_indicatorVisible = true;
		updateIndicator();
	}
}

/******************************************************************************
* Shows or hides the progress indicator widgets and updates the displayed information.
******************************************************************************/
void ProgressManager::updateIndicator()
{
	if(_indicatorVisible == false)
		return;

	if(_taskStack.isEmpty() == true) {
		QStatusBar* statusBar = MainWindow::instance().statusBar();
		statusBar->removeWidget(_progressWidget);
		statusBar->removeWidget(_progressTextDisplay);
		_indicatorVisible = false;
	}
	else {
		FutureWatcher* watcher = _taskStack.top();
		_progressBar->setRange(0, watcher->progressMaximum());
		_progressBar->setValue(watcher->progressValue());
		_progressTextDisplay->setText(watcher->progressText());
	}
}

/******************************************************************************
* Cancels all running background tasks.
******************************************************************************/
void ProgressManager::cancelAll()
{
	for(FutureWatcher* watcher : _taskStack)
		watcher->cancel();
}

/******************************************************************************
* Cancels all running background tasks and waits for them to finish.
******************************************************************************/
void ProgressManager::cancelAllAndWait()
{
	cancelAll();
	for(FutureWatcher* watcher : _taskStack) {
		try {
			watcher->waitForFinished();
		}
		catch(...) {}
	}
}

/******************************************************************************
* Waits for the given task to finish and displays a modal progress dialog
* to show the task's progress.
******************************************************************************/
bool ProgressManager::waitForTask(const FutureInterfacePointer& futureInterface)
{
	OVITO_ASSERT_MSG(QThread::currentThread() == QApplication::instance()->thread(), "ProgressManager::waitForTask", "Function can only be called from the GUI thread.");

	// Before showing any progress dialog, check if task has already finished.
	if(futureInterface->isFinished())
		return !futureInterface->isCanceled();

	// Show progress dialog.
	QProgressDialog progressDialog(&MainWindow::instance());
	progressDialog.setWindowModality(Qt::WindowModal);
	progressDialog.setAutoClose(false);
	progressDialog.setAutoReset(false);
	progressDialog.setMinimumDuration(0);

	FutureWatcher watcher;
	connect(&watcher, SIGNAL(progressRangeChanged(int)), &progressDialog, SLOT(setMaximum(int)));
	connect(&watcher, SIGNAL(progressValueChanged(int)), &progressDialog, SLOT(setValue(int)));
	connect(&watcher, SIGNAL(progressTextChanged(const QString&)), &progressDialog, SLOT(setLabelText(const QString&)));
	connect(&progressDialog, SIGNAL(canceled()), &watcher, SLOT(cancel()));
	watcher.setFutureInterface(futureInterface);

	progressDialog.setLabelText(futureInterface->progressText());
	progressDialog.setMaximum(futureInterface->progressMaximum());
	progressDialog.setValue(futureInterface->progressValue());

	progressDialog.open();
	while(!watcher.isFinished()) {
		QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 100);
	}

	return !futureInterface->isCanceled();
}

};
