///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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
#include <core/dataset/UndoStack.h>
#include <core/dataset/DataSetContainer.h>
#include <core/animation/controller/Controller.h>
#include <core/plugins/PluginManager.h>
#include <core/plugins/autostart/AutoStartObject.h>
#include <core/utilities/io/FileManager.h>

#ifdef Q_OS_MACX
	#include <Carbon/Carbon.h>
#endif

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) 

/// The one and only instance of this class.
Application Application::_instance;

/// Stores a pointer to the original Qt message handler function, which has been replaced with our own handler.
QtMessageHandler Application::defaultQtMessageHandler = nullptr;

/******************************************************************************
* Handler method for Qt error messages.
* This can be used to set a debugger breakpoint for the OVITO_ASSERT macros.
******************************************************************************/
void Application::qtMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
	// Forward message to default handler.
	if(defaultQtMessageHandler) defaultQtMessageHandler(type, context, msg);
	else std::cerr << qPrintable(msg) << std::endl;
}

/******************************************************************************
* Constructor.
******************************************************************************/
Application::Application() : _exitCode(0), _consoleMode(false), _headlessMode(false)
{
}

/******************************************************************************
* Destructor.
******************************************************************************/
Application::~Application()
{
}

/******************************************************************************
* This is called on program startup.
******************************************************************************/
bool Application::initialize(int& argc, char** argv)
{
	// Install custom Qt error message handler to catch fatal errors in debug mode.
	defaultQtMessageHandler = qInstallMessageHandler(qtMessageOutput);

	// Set the application name provided by the active branding class.
	QCoreApplication::setApplicationName(tr("Ovito"));
	QCoreApplication::setOrganizationName(tr("Alexander Stukowski"));
	QCoreApplication::setOrganizationDomain("ovito.org");
	QCoreApplication::setApplicationVersion(QStringLiteral(OVITO_VERSION_STRING));

	// Activate default "C" locale, which will be used to parse numbers in strings.
	std::setlocale(LC_NUMERIC, "C");

	// Register our floating-point data type with the Qt type system.
	qRegisterMetaType<FloatType>("FloatType");

	// Register Qt stream operators for basic types.
	qRegisterMetaTypeStreamOperators<Vector2>("Ovito::Vector2");
	qRegisterMetaTypeStreamOperators<Vector3>("Ovito::Vector3");
	qRegisterMetaTypeStreamOperators<Vector4>("Ovito::Vector4");
	qRegisterMetaTypeStreamOperators<Point2>("Ovito::Point2");
	qRegisterMetaTypeStreamOperators<Point3>("Ovito::Point3");
	qRegisterMetaTypeStreamOperators<AffineTransformation>("Ovito::AffineTransformation");
	qRegisterMetaTypeStreamOperators<Matrix3>("Ovito::Matrix3");
	qRegisterMetaTypeStreamOperators<Matrix4>("Ovito::Matrix4");
	qRegisterMetaTypeStreamOperators<Box2>("Ovito::Box2");
	qRegisterMetaTypeStreamOperators<Box3>("Ovito::Box3");
	qRegisterMetaTypeStreamOperators<Rotation>("Ovito::Rotation");
	qRegisterMetaTypeStreamOperators<Scaling>("Ovito::Scaling");
	qRegisterMetaTypeStreamOperators<Quaternion>("Ovito::Quaternion");
	qRegisterMetaTypeStreamOperators<Color>("Ovito::Color");
	qRegisterMetaTypeStreamOperators<ColorA>("Ovito::ColorA");

	// Register Qt conversion operators for custom types.
	QMetaType::registerConverter<QColor, Color>();
	QMetaType::registerConverter<Color, QColor>();
	QMetaType::registerConverter<QColor, ColorA>();
	QMetaType::registerConverter<ColorA, QColor>();

	// Register command line arguments.
	_cmdLineParser.setApplicationDescription(tr("OVITO - Open Visualization Tool"));
	_cmdLineParser.addOption(QCommandLineOption(QStringList{{"h", "help"}}, tr("Shows this list of program options and exits.")));
	_cmdLineParser.addOption(QCommandLineOption(QStringList{{"v", "version"}}, tr("Prints the program version and exits.")));
	_cmdLineParser.addOption(QCommandLineOption(QStringList{{"nogui"}}, tr("Run in console mode without showing the graphical user interface.")));
	_cmdLineParser.addOption(QCommandLineOption(QStringList{{"glversion"}}, tr("Selects a specific version of the OpenGL standard."), tr("VERSION")));
	_cmdLineParser.addOption(QCommandLineOption(QStringList{{"glcompatprofile"}}, tr("Request the OpenGL compatibility profile instead of the core profile.")));

	// Parse command line arguments.
	// Ignore unknown command line options for now.
	QStringList arguments;
	for(int i = 0; i < argc; i++)
		arguments << QString::fromLocal8Bit(argv[i]);
	
	// Because they may collide with our own options, we should ignore script arguments though. 
	QStringList filteredArguments;
	for(int i = 0; i < argc; i++) {
		if(strcmp(argv[i], "--scriptarg") == 0) {
			i += 1;
			continue;
		}
		filteredArguments.push_back(arguments[i]);
	}
	_cmdLineParser.parse(filteredArguments);

	// Output program version if requested.
	if(_cmdLineParser.isSet("version")) {
		std::cout << qPrintable(QCoreApplication::applicationName()) << " " << qPrintable(QCoreApplication::applicationVersion()) << std::endl;
		_consoleMode = true;
		return true;
	}

	// Check if program was started in console mode.
	if(_cmdLineParser.isSet("nogui")) {
		_consoleMode = true;
#if defined(Q_OS_LINUX)
		// On Unix/Linux, console mode means headless mode if no X server is available.
		if(qEnvironmentVariableIsEmpty("DISPLAY")) {
			_headlessMode = true;
		}
#elif defined(Q_OS_OSX)
		// Don't let Qt move the app to the foreground when running in console mode.
		::setenv("QT_MAC_DISABLE_FOREGROUND_APPLICATION_TRANSFORM", "1", 1);
#endif
	}

	// Create Qt application object.
	if(headlessMode())
		_app.reset(new QCoreApplication(argc, argv));
	else
		_app.reset(new QApplication(argc, argv));

	// Reactivate default "C" locale, which, in the meantime, might have been changed by QCoreApplication.
	std::setlocale(LC_NUMERIC, "C");

	// Install global exception handler.
	// The GUI exception handler shows a message box with the error message.
	// The console mode exception handler prints the error message to stderr.
	if(guiMode())
		Exception::setExceptionHandler(guiExceptionHandler);
	else
		Exception::setExceptionHandler(consoleExceptionHandler);

	try {

		// Initialize global objects in the right order.
		PluginManager::initialize();
		ControllerManager::initialize();
		FileManager::initialize();

		// Load auto-start objects and let them register their custom command line options.
		for(const OvitoObjectType* clazz : PluginManager::instance().listClasses(AutoStartObject::OOType)) {
			OORef<AutoStartObject> obj = static_object_cast<AutoStartObject>(clazz->createInstance(nullptr));
			_autostartObjects.push_back(obj);
			obj->registerCommandLineOptions(_cmdLineParser);
		}

		// Parse the command line parameters again after the plugins have registered their options.
		if(!_cmdLineParser.parse(arguments)) {
	        std::cerr << "Error: " << qPrintable(_cmdLineParser.errorText()) << std::endl;
			_consoleMode = true;
			shutdown();
			return false;
		}

		// Help command line option implicitly activates console mode.
		if(_cmdLineParser.isSet("help"))
			_consoleMode = true;

		if(guiMode()) {
			// Set up graphical user interface.
			initializeGUI();
		}
		else {
			// Create a dataset container.
			_datasetContainer = new DataSetContainer();
			_datasetContainer->setParent(this);
		}

		// Handle --help command line option. Print list of command line options and quit.
		if(_cmdLineParser.isSet("help")) {
			std::cout << qPrintable(_cmdLineParser.helpText()) << std::endl;
			return true;
		}

		// Load scene file specified at the command line.
		if(cmdLineParser().positionalArguments().empty() == false) {
			QString startupFilename = cmdLineParser().positionalArguments().front();
			if(startupFilename.endsWith(".ovito", Qt::CaseInsensitive))
				datasetContainer()->fileLoad(startupFilename);
		}

		// Create an empty dataset if nothing has been loaded.
		if(datasetContainer()->currentSet() == nullptr)
			datasetContainer()->fileNew();

		// Import data file specified at the command line.
		if(cmdLineParser().positionalArguments().empty() == false) {
			QString importFilename = cmdLineParser().positionalArguments().front();
			if(!importFilename.endsWith(".ovito", Qt::CaseInsensitive)) {
				QUrl importURL = FileManager::instance().urlFromUserInput(importFilename);
				datasetContainer()->importFile(importURL);
				datasetContainer()->currentSet()->undoStack().setClean();
			}
		}

		// Invoke auto-start objects.
		for(const auto& obj : _autostartObjects)
			obj->applicationStarted();
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
		return QApplication::exec();
	}
	else {
		// Deliver all events that have been posted during the initialization.
		QCoreApplication::processEvents();
		// Just quit the application after all background tasks have finished.
		if(_datasetContainer)
			_datasetContainer->taskManager().waitForAll();
		return _exitCode;
	}
}

/******************************************************************************
* Initializes the graphical user interface of the application.
******************************************************************************/
void Application::initializeGUI()
{
	// Set the application icon.
	QIcon mainWindowIcon;
	mainWindowIcon.addFile(":/core/mainwin/window_icon_256.png");
	mainWindowIcon.addFile(":/core/mainwin/window_icon_128.png");
	mainWindowIcon.addFile(":/core/mainwin/window_icon_48.png");
	mainWindowIcon.addFile(":/core/mainwin/window_icon_32.png");
	mainWindowIcon.addFile(":/core/mainwin/window_icon_16.png");
	QApplication::setWindowIcon(mainWindowIcon);

	// Create the main window.
	MainWindow* mainWin = new MainWindow();
	_datasetContainer = &mainWin->datasetContainer();

	// Make the application shutdown as soon as the last main window has been closed.
	QGuiApplication::setQuitOnLastWindowClosed(true);

	// Show the main window.
#ifndef OVITO_DEBUG
	mainWin->showMaximized();
#else
	mainWin->show();
#endif
	mainWin->restoreLayout();
}

/******************************************************************************
* This is called on program shutdown.
******************************************************************************/
void Application::shutdown()
{
	// Destroy auto-start objects.
	_autostartObjects.clear();

	// Shutdown global objects in reverse order they were initialized.
	FileManager::shutdown();
	ControllerManager::shutdown();
	PluginManager::shutdown();

	// Destroy Qt application object.
	_app.reset();
}

/******************************************************************************
* Returns a pointer to the main dataset container.
******************************************************************************/
DataSetContainer* Application::datasetContainer() const
{
	OVITO_ASSERT_MSG(!_datasetContainer.isNull(), "Application::datasetContainer()", "There is no global dataset container.");
	return _datasetContainer;
}

/******************************************************************************
* Executes the functions registered with the runOnceLater() function.
* This method is called after the events in the event queue have been processed.
******************************************************************************/
void Application::processRunOnceList()
{
	auto copy = _runOnceList;
	_runOnceList.clear();
	for(auto entry = copy.cbegin(); entry != copy.cend(); ++entry) {
		if(entry.key())
			entry.value()();
	}
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
		std::cerr << "ERROR: " << qPrintable(exception.messages()[i]) << std::endl;
	}
	std::cerr << std::flush;
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
