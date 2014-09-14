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

/** 
 * \file TaskManager.h
 * \brief Contains the definition of the Ovito::TaskManager class.
 */
 
#ifndef __OVITO_PROGRESS_MANAGER_H
#define __OVITO_PROGRESS_MANAGER_H

#include <core/Core.h>
#include "Future.h"
#include "Task.h"

namespace Ovito {

typedef std::shared_ptr<Ovito::FutureInterfaceBase> FutureInterfacePointer;

/**
 * \brief Manages the background tasks.
 */
class OVITO_CORE_EXPORT TaskManager : public QObject
{
	Q_OBJECT

public:

	/// \brief Constructor.
	TaskManager(MainWindow* mainWindow);

	/// Destructor.
	~TaskManager() {
		cancelAllAndWait();
	}

	/// \brief Runs a function in a background thread.
	///
	/// This function is thread-safe.
	template<typename R, typename Function>
	Future<R> runInBackground(Function f) {
		Future<R> future = (new Task<R,Function>(f))->start();
		addTask(future);
		return future;
	}

	/// \brief Registers a future with the progress manager, which will display the progress of the background task
	///        in the main window.
	///
	/// This function is thread-safe.
	template<typename R>
	void addTask(const Future<R>& future) {
		// Execute the function call in the GUI thread.
		QMetaObject::invokeMethod(this, "addTaskInternal", Q_ARG(FutureInterfacePointer, future.interface()));
	}

	/// \brief Waits for the given task to finish and displays a modal progress dialog
	///        to show the task's progress.
	/// \return False if the task has been canceled by the user.
	///
	/// This function must be called from the GUI thread.
	template<typename R>
	bool waitForTask(const Future<R>& future) {
		return waitForTask(future.interface());
	}

private:

	/// \brief Registers a future with the progress manager.
	Q_INVOKABLE void addTaskInternal(FutureInterfacePointer futureInterface);

	/// \brief Waits for the given task to finish and displays a modal progress dialog
	///        to show the task's progress.
	bool waitForTask(const FutureInterfacePointer& futureInterface);

public Q_SLOTS:

	/// \brief Cancels all running background tasks.
	void cancelAll();

	/// \brief Cancels all running background tasks and waits for them to finish.
	void cancelAllAndWait();

	/// \brief Waits for all tasks to finish.
	void waitForAll();

private Q_SLOTS:

	/// \brief Is called when a task has started to run.
	void taskStarted(QObject* object);

	/// \brief Is called when a task has finished.
	void taskFinished(QObject* object);

	/// \brief Is called when the progress of a task has changed
	void taskProgressValueChanged(QObject* object);

	/// \brief Is called when the status text of a task has changed.
	void taskProgressTextChanged(QObject* object);

	/// \brief Shows the progress indicator widgets.
	void showIndicator();

	/// \brief Updates the displayed information in the indicator widget.
	void updateIndicator();

private:
	
	QSignalMapper _taskStartedSignalMapper;
	QSignalMapper _taskFinishedSignalMapper;
	QSignalMapper _taskProgressValueChangedSignalMapper;
	QSignalMapper _taskProgressTextChangedSignalMapper;

	QStack<FutureWatcher*> _taskStack;

	/// The window this progress manager is associated with.
	MainWindow* _mainWindow;

	/// The progress bar widget.
	QProgressBar* _progressBar;

	/// The button that lets the user cancel running tasks.
	QAbstractButton* _cancelTaskButton;

	/// The parent widget of the progress bar and the cancel button.
	QWidget* _progressWidget;

	/// The label that displays the current progress text.
	QLabel* _progressTextDisplay;

	/// This is used to automatically destroy the progress indicator widget on application shutdown.
	QObjectCleanupHandler _widgetCleanupHandler;

	/// True if the indicator widget is currently visible.
	bool _indicatorVisible;
};

};

Q_DECLARE_METATYPE(Ovito::FutureInterfacePointer);

#endif // __OVITO_PROGRESS_MANAGER_H
