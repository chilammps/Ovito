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
 * \file ProgressManager.h
 * \brief Contains the definition of the Ovito::ProgressManager class.
 */
 
#ifndef __OVITO_PROGRESS_MANAGER_H
#define __OVITO_PROGRESS_MANAGER_H

#include <core/Core.h>
#include "Future.h"

namespace Ovito {

typedef std::shared_ptr<Ovito::FutureInterfaceBase> FutureInterfacePointer;

/**
 * \brief Manages the running background tasks.
 */
class ProgressManager : public QObject
{
	Q_OBJECT

public:

	/// Destructor.
	~ProgressManager() { OVITO_ASSERT(_taskStack.isEmpty()); }

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the ProgressManager singleton class.
	inline static ProgressManager& instance() {
		OVITO_ASSERT_MSG(_instance != nullptr, "ProgressManager::instance", "Singleton object is not initialized yet.");
		return *_instance;
	}

	/// \brief Registers a future with the progress manager, who will display the progress of the background task
	///        in the main window.
	///
	/// This function is thread-safe.
	template<typename R>
	void addTask(const Future<R>& future) {
		// Execute the function call in the GUI thread.
		QMetaObject::invokeMethod(this, "addTaskInternal", Q_ARG(FutureInterfacePointer, future.interface()));
	}

private:

	/// \brief Registers a future with the progress manager.
	Q_INVOKABLE void addTaskInternal(FutureInterfacePointer futureInterface);

public Q_SLOTS:

	/// \brief Cancels all running background tasks and waits for them to finish.
	void cancelAll();

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

private:
    
	/// Private constructor.
	/// This is a singleton class; no public instances are allowed.
	ProgressManager();

	/// Create the singleton instance of this class.
	static void initialize() { _instance = new ProgressManager(); }

	/// Deletes the singleton instance of this class.
	static void shutdown() { delete _instance; _instance = nullptr; }

	/// The singleton instance of this class.
	static ProgressManager* _instance;

	friend class Application;
};

};

Q_DECLARE_METATYPE(Ovito::FutureInterfacePointer);

#endif // __OVITO_PROGRESS_MANAGER_H
