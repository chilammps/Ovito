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

#ifndef __OVITO_APPLICATION_H
#define __OVITO_APPLICATION_H

#include <core/Core.h>
#include <core/utilities/Exception.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui)

/**
 * \brief The main application.
 */
class OVITO_CORE_EXPORT Application : public QObject
{
	Q_OBJECT

public:

	/// \brief Returns the one and only instance of this class.
	inline static Application& instance() {
		return _instance;
	}

	/// \brief Constructor.
	Application();

	/// \brief Destructor.
	~Application();

	/// \brief Initializes the application.
	/// \param argc The number of command line arguments.
	/// \param argv The command line arguments.
	/// \return \c true if the application was initialized successfully;
	///         \c false if an error occurred and the program should be terminated.
	///
	/// This is called on program startup. The method creates all other global objects and the main window.
	bool initialize(int& argc, char** argv);

	/// \brief Enters the main event loop.
	/// \return The program exit code.
	///
	/// If the application has been started in console mode then this method does nothing.
	int runApplication();

	/// \brief Releases everything.
	///
	/// This is called before the application exits.
	void shutdown();

	/// \brief Handler method for Qt error messages.
	///
	/// This can be used to set a debugger breakpoint for the OVITO_ASSERT macros.
	static void qtMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg);

	/// \brief Returns whether the application has been started in graphical mode.
	/// \return \c true if the application should use a graphical user interface;
	///         \c false if the application has been started in the non-graphical console mode.
	bool guiMode() const { return !_consoleMode; }

	/// \brief Returns whether the application has been started in console mode.
	/// \return \c true if the application has been started in the non-graphical console mode;
	///         \c false if the application should use a graphical user interface.
	bool consoleMode() const { return _consoleMode; }

	/// \brief Returns whether the application runs in headless mode (without an X server on Linux and no OpenGL support).
	bool headlessMode() const { return _headlessMode; }

	/// \brief When in console mode, this specifies the exit code that will be returned by the application on shutdown.
	void setExitCode(int code) { _exitCode = code; }

	/// \brief Returns a pointer to the main dataset container.
	/// \return The dataset container of the first main window when running in GUI mode;
	///         or the global dataset container when running in console mode.
	DataSetContainer* datasetContainer() const;

	/// \brief Returns the command line options passed to the program.
	const QCommandLineParser& cmdLineParser() const { return _cmdLineParser; }

	/// This registers a functor object to be called after all events in the UI event queue have been processed and
	/// before control returns to the event loop. For a given target object, only one functor can be registered at a
	/// time. Subsequent calls to runOnceLater() with the same target, before control returns to the event loop, will do nothing.
	template<class F>
	void runOnceLater(QObject* target, F&& func) {
		if(_runOnceList.isEmpty())
			metaObject()->invokeMethod(this, "processRunOnceList", Qt::QueuedConnection);
		else if(_runOnceList.contains(target))
			return;
		_runOnceList.insert(target, std::forward<F>(func));
	}

private:

	/// Initializes the graphical user interface of the application.
	void initializeGUI();

	/// Executes the functions registered with the runOnceLater() function.
	/// This method is called after the events in the event queue have been processed.
	Q_INVOKABLE void processRunOnceList();

private:

	/// The Qt application object.
	QScopedPointer<QCoreApplication> _app;

	/// The parser for the command line options passed to the program.
	QCommandLineParser _cmdLineParser;

	/// Indicates that the application is running in console mode.
	bool _consoleMode;

	/// Indicates that the application is running in headless mode (without OpenGL support).
	bool _headlessMode;

	/// In console mode, this is the exit code returned by the application on shutdown.
	int _exitCode;

	/// The list of functor objects registered with runOnceLater(), which are to be executed as soon as control returns to the event loop.
	QMap<QPointer<QObject>,std::function<void()>> _runOnceList;

	/// The main dataset container.
	QPointer<DataSetContainer> _datasetContainer;

	/// The auto-start objects created at application startup.
	std::vector<OORef<AutoStartObject>> _autostartObjects;

	/// The default message handler method of Qt.
	static QtMessageHandler defaultQtMessageHandler;

	/// Handler function for exceptions used in GUI mode.
	static void guiExceptionHandler(const Exception& exception);

	/// Handler function for exceptions used in console mode.
	static void consoleExceptionHandler(const Exception& exception);

	/// The one and only instance of this class.
	static Application _instance;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_APPLICATION_H
