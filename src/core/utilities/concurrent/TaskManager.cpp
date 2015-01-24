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
#include <core/utilities/concurrent/TaskManager.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/app/Application.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Concurrency)

/******************************************************************************
* Initializes the progress manager.
******************************************************************************/
TaskManager::TaskManager(MainWindow* mainWindow) : QObject(mainWindow), _mainWindow(mainWindow),
		_progressBar(nullptr), _cancelTaskButton(nullptr), _progressWidget(nullptr),
		_progressTextDisplay(nullptr)
{
	qRegisterMetaType<std::shared_ptr<FutureInterfaceBase>>("FutureInterfacePointer");

	_indicatorVisible = false;

	connect(&_taskStartedSignalMapper, (void (QSignalMapper::*)(QObject*))&QSignalMapper::mapped, this, &TaskManager::taskStarted);
	connect(&_taskFinishedSignalMapper, (void (QSignalMapper::*)(QObject*))&QSignalMapper::mapped, this, &TaskManager::taskFinished);
	connect(&_taskProgressValueChangedSignalMapper, (void (QSignalMapper::*)(QObject*))&QSignalMapper::mapped, this, &TaskManager::taskProgressValueChanged);
	connect(&_taskProgressTextChangedSignalMapper, (void (QSignalMapper::*)(QObject*))&QSignalMapper::mapped, this, &TaskManager::taskProgressTextChanged);
}

/******************************************************************************
* Registers a future with the progress manager.
******************************************************************************/
void TaskManager::addTaskInternal(std::shared_ptr<FutureInterfaceBase> futureInterface)
{
	FutureWatcher* watcher = new FutureWatcher(this);
	connect(watcher, &FutureWatcher::started, &_taskStartedSignalMapper, (void (QSignalMapper::*)())&QSignalMapper::map);
	connect(watcher, &FutureWatcher::finished, &_taskFinishedSignalMapper, (void (QSignalMapper::*)())&QSignalMapper::map);
	connect(watcher, &FutureWatcher::progressRangeChanged, &_taskProgressValueChangedSignalMapper, (void (QSignalMapper::*)())&QSignalMapper::map);
	connect(watcher, &FutureWatcher::progressValueChanged, &_taskProgressValueChangedSignalMapper, (void (QSignalMapper::*)())&QSignalMapper::map);
	connect(watcher, &FutureWatcher::progressTextChanged, &_taskProgressTextChangedSignalMapper, (void (QSignalMapper::*)())&QSignalMapper::map);
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
void TaskManager::taskStarted(QObject* object)
{
	FutureWatcher* watcher = static_cast<FutureWatcher*>(object);

	// Show progress indicator only if the task doesn't finish within 300 milliseconds.
	if(_taskStack.isEmpty() && _mainWindow)
		QTimer::singleShot(200, this, SLOT(showIndicator()));

	_taskStack.push(watcher);
}

/******************************************************************************
* Is called when a task has finished.
******************************************************************************/
void TaskManager::taskFinished(QObject* object)
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
void TaskManager::taskProgressValueChanged(QObject* object)
{
	OVITO_ASSERT(_taskStack.contains(static_cast<FutureWatcher*>(object)));
	if(_taskStack.top() == object)
		updateIndicator();
}

/******************************************************************************
* Is called when the status text of a task has changed.
******************************************************************************/
void TaskManager::taskProgressTextChanged(QObject* object)
{
	OVITO_ASSERT(_taskStack.contains(static_cast<FutureWatcher*>(object)));
	if(_taskStack.top() == object)
		updateIndicator();
}

/******************************************************************************
* Shows the progress indicator widget.
******************************************************************************/
void TaskManager::showIndicator()
{
	if(_indicatorVisible == false && _taskStack.isEmpty() == false) {

		// Create progress display widget.
		if(_mainWindow && _progressWidget == nullptr) {
			_progressWidget = new QWidget();
			QHBoxLayout* progressWidgetLayout = new QHBoxLayout(_progressWidget);
			progressWidgetLayout->setContentsMargins(0,0,0,0);
			progressWidgetLayout->setSpacing(0);
			_progressTextDisplay = new QLabel();
			_progressTextDisplay->setLineWidth(0);
			_progressTextDisplay->setAlignment(Qt::Alignment(Qt::AlignRight | Qt::AlignVCenter));
			_progressTextDisplay->setAutoFillBackground(true);
			_progressTextDisplay->setMargin(2);
			_progressBar = new QProgressBar(_progressWidget);
			_cancelTaskButton = new QToolButton(_progressWidget);
			_cancelTaskButton->setText(tr("Cancel"));
			QIcon cancelIcon(":/core/mainwin/process-stop-16.png");
			cancelIcon.addFile(":/core/mainwin/process-stop-22.png");
			_cancelTaskButton->setIcon(cancelIcon);
			progressWidgetLayout->addWidget(_progressBar);
			progressWidgetLayout->addWidget(_cancelTaskButton);
			_progressWidget->setMinimumHeight(_progressTextDisplay->minimumSizeHint().height());
			_widgetCleanupHandler.add(_progressTextDisplay);
			_widgetCleanupHandler.add(_progressWidget);
			connect(_cancelTaskButton, &QAbstractButton::clicked, this, &TaskManager::cancelAll);
			_mainWindow->statusBarLayout()->insertWidget(1, _progressWidget);
		}

		_mainWindow->statusBar()->addWidget(_progressTextDisplay, 1);
		_progressWidget->show();
		_progressTextDisplay->show();
		_indicatorVisible = true;
		updateIndicator();
	}
}

/******************************************************************************
* Shows or hides the progress indicator widgets and updates the displayed information.
******************************************************************************/
void TaskManager::updateIndicator()
{
	if(_indicatorVisible == false)
		return;

	if(_taskStack.isEmpty() == true) {
		_progressWidget->hide();
		_mainWindow->statusBar()->removeWidget(_progressTextDisplay);
		_indicatorVisible = false;
	}
	else {
		FutureWatcher* watcher = _taskStack.top();
		_progressBar->setRange(0, watcher->progressMaximum());
		_progressBar->setValue(watcher->progressValue());
		_progressTextDisplay->setText(watcher->progressText());
		_progressWidget->show();
	}
}

/******************************************************************************
* Cancels all running background tasks.
******************************************************************************/
void TaskManager::cancelAll()
{
	for(FutureWatcher* watcher : _taskStack)
		watcher->cancel();
}

/******************************************************************************
* Cancels all running background tasks and waits for them to finish.
******************************************************************************/
void TaskManager::cancelAllAndWait()
{
	cancelAll();
	waitForAll();
}

/******************************************************************************
* Waits for all tasks to finish.
******************************************************************************/
void TaskManager::waitForAll()
{
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
bool TaskManager::waitForTask(const std::shared_ptr<FutureInterfaceBase>& futureInterface)
{
	OVITO_ASSERT_MSG(QThread::currentThread() == QApplication::instance()->thread(), "TaskManager::waitForTask", "Function can only be called from the GUI thread.");

	// Before showing any progress dialog, check if task has already finished.
	if(futureInterface->isFinished())
		return !futureInterface->isCanceled();

	if(Application::instance().guiMode()) {
		OVITO_ASSERT(_mainWindow != nullptr);

		// Poll the task for a few milliseconds to give it a chance to finish
		// before showing the progress dialog.
		for(int i = 0; i < 10; i++) {
			QThread::msleep(10);
			QThread::yieldCurrentThread();
			if(futureInterface->isFinished())
				return !futureInterface->isCanceled();
		}

		// Show progress dialog.
		QProgressDialog progressDialog(_mainWindow);
		progressDialog.setWindowModality(Qt::WindowModal);
		progressDialog.setAutoClose(false);
		progressDialog.setAutoReset(false);
		progressDialog.setMinimumDuration(0);
		QLabel* label = new QLabel();
		label->setWordWrap(true);
		label->setMinimumWidth(500);
		progressDialog.setLabel(label);

		FutureWatcher watcher;
		connect(&watcher, &FutureWatcher::progressRangeChanged, &progressDialog, &QProgressDialog::setMaximum);
		connect(&watcher, &FutureWatcher::progressValueChanged, &progressDialog, &QProgressDialog::setValue);
		connect(&watcher, &FutureWatcher::progressTextChanged, &progressDialog, &QProgressDialog::setLabelText);
		connect(&progressDialog, &QProgressDialog::canceled, &watcher, &FutureWatcher::cancel);
		watcher.setFutureInterface(futureInterface);

		progressDialog.setLabelText(futureInterface->progressText());
		progressDialog.setMaximum(futureInterface->progressMaximum());
		progressDialog.setValue(futureInterface->progressValue());

		progressDialog.open();
		while(!watcher.isFinished()) {
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 100);
		}

	}
	else {
		FutureWatcher watcher;
		watcher.setFutureInterface(futureInterface);
		while(!watcher.isFinished()) {
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 100);
		}
	}
	return !futureInterface->isCanceled();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
