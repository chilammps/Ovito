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
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>

namespace Ovito {

/// Stores a pointer to the original Qt message handler function, which has been replaced with our own handler.
QtMessageHandler Application::defaultQtMessageHandler = NULL;

/******************************************************************************
* Handler method for Qt error messages.
* This can be used to set a debugger breakpoint for the OVITO_ASSERT macros.
******************************************************************************/
void Application::qtMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	if(type == QtFatalMsg) {
		// This code line can be used to set a breakpoint to debug OVITO_ASSERT() macro exceptions.
		int a = 1;
	}

	// Pass message on to default handler.
	if(defaultQtMessageHandler) defaultQtMessageHandler(type, context, msg);
	else std::cerr << msg.toLocal8Bit().constData() << std::endl;
}

/******************************************************************************
* This is called on program startup.
******************************************************************************/
bool Application::initialize()
{
	// Install custom Qt error message handler to catch fatal errors in debug mode.
	defaultQtMessageHandler = qInstallMessageHandler(qtMessageOutput);

	// Install global exception handler.
	// The GUI exception handler shows a message box with the error message.
	// The console mode exception handlers just prints the error message to stderr.
	if(guiMode())
		Exception::setExceptionHandler(guiExceptionHandler);
	else
		Exception::setExceptionHandler(consoleExceptionHandler);

	try {
		// Parse command line arguments.
		if(!parseCommandLine())
			return false;
	}
	catch(const Exception& ex) {
		ex.showError();
		return false;
	}

	try {

		// Set the application name provided by the active branding class.
		setApplicationName(tr("Ovito"));
		setOrganizationName(tr("Alexander Stukowski"));
		setApplicationVersion(tr("Version %1.%2.%3")
				.arg(OVITO_VERSION_MAJOR)
				.arg(OVITO_VERSION_MINOR)
				.arg(OVITO_VERSION_REVISION));

		// Create the main application window.
		if(guiMode()) {

			// Set the application icon.
			QIcon mainWindowIcon;
			mainWindowIcon.addFile(":/core/mainwin/window_icon_256.png");
			mainWindowIcon.addFile(":/core/mainwin/window_icon_128.png");
			mainWindowIcon.addFile(":/core/mainwin/window_icon_48.png");
			mainWindowIcon.addFile(":/core/mainwin/window_icon_32.png");
			mainWindowIcon.addFile(":/core/mainwin/window_icon_16.png");
			setWindowIcon(mainWindowIcon);

			// Create the main window.
			MainWindow* mainWin = new MainWindow(tr("Ovito (Open Visualization Tool)"));

			// Show the main window.
			mainWin->showMaximized();
			mainWin->restoreLayout();

			// Quit application when main window is closed.
			connect(mainWin, SIGNAL(destroyed(QObject*)), this, SLOT(quit()));
		}
	}
	catch(const Exception& ex) {
		ex.showError();
		shutdown();
		return false;
	}
	return true;
}

/******************************************************************************
* Starts the main event loop.
******************************************************************************/
int Application::runApplication()
{
	if(guiMode()) {
		// Enter the main event loop.
		return exec();
	}
	else {
		// No event processing needed in console mode.
		// Just exit the application.
		return _exitCode;
	}
}

/******************************************************************************
* This is called on program shutdown.
******************************************************************************/
void Application::shutdown()
{
}

/******************************************************************************
* Parses the command line parameters.
******************************************************************************/
bool Application::parseCommandLine()
{
	QStringList params = QCoreApplication::arguments();
	return true;
}

/******************************************************************************
* Handler function for exceptions used in GUI mode.
******************************************************************************/
void Application::guiExceptionHandler(const Exception& exception)
{
	exception.logError();
	QMessageBox msgbox;
	msgbox.setWindowTitle(tr("Error - %1").arg(QCoreApplication::applicationName()));
	msgbox.setStandardButtons(QMessageBox::Ok);
	msgbox.setText(exception.message());
	msgbox.setIcon(QMessageBox::Critical);
	if(exception.messages().size() > 1) {
		QString detailText;
		for(int i = 1; i < exception.messages().size(); i++)
			detailText += exception.messages()[i] + "\n";
		msgbox.setDetailedText(detailText);
	}
	msgbox.exec();
}

/******************************************************************************
* Handler function for exceptions used in console mode.
******************************************************************************/
void Application::consoleExceptionHandler(const Exception& exception)
{
	for(int i = exception.messages().size() - 1; i >= 0; i--) {
		std::cerr << "ERROR: ";
		std::cerr << exception.messages()[i].toLocal8Bit().constData() << std::endl;
	}
	std::cerr << std::flush;
}

};
