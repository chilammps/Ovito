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

#include <plugins/pyscript/PyScript.h>
#include <plugins/pyscript/binding/PythonBinding.h>
#include <core/plugins/PluginManager.h>
#include <core/gui/app/Application.h>
#include "ScriptEngine.h"

namespace PyScript {

using namespace boost::python;

/// Flag that indicates whether the global Python interpreter has been initialized.
bool ScriptEngine::_isInterpreterInitialized = false;

/// The prototype namespace that is used to initialize new script engines with.
dict ScriptEngine::_prototypeMainNamespace;

/// The script engine that is currently active (i.e. which is executing a script).
QAtomicPointer<ScriptEngine> ScriptEngine::_activeEngine;

/// Head of linked list that contains all initXXX functions.
PythonPluginRegistration* PythonPluginRegistration::linkedlist = nullptr;

/******************************************************************************
* Initializes the scripting engine and sets up the environment.
******************************************************************************/
ScriptEngine::ScriptEngine(DataSet* dataset, QObject* parent, bool redirectOutputToConsole)
	: QObject(parent), _dataset(dataset)
{
	// Initialize the underlying Python interpreter if it hasn't been already.
	initializeInterpreter();

	// Initialize state of the script engine.
	try {
		// Make a local copy of the global main namespace for this engine.
		// The original namespace dictionary is not touched.
		_mainNamespace = _prototypeMainNamespace.copy();

		// Add a reference to the current dataset to the namespace.
		import("ovito").attr("dataset") = ptr(dataset);
	}
	catch(const error_already_set&) {
		PyErr_Print();
		throw Exception(tr("Failed to initialize Python interpreter. See console output for error details."));
	}

	// Install default signal handlers for Python script output, which forward the script output to the host application's stdout/stderr.
	if(redirectOutputToConsole) {
		connect(this, &ScriptEngine::scriptOutput, [](const QString& str) { std::cout << str.toLocal8Bit().constData(); });
		connect(this, &ScriptEngine::scriptError, [](const QString& str) { std::cerr << str.toLocal8Bit().constData(); });
	}
}

/******************************************************************************
* Initializes the Python interpreter and sets up the global namespace.
******************************************************************************/
void ScriptEngine::initializeInterpreter()
{
	if(_isInterpreterInitialized)
		return;	// Interpreter is already initialized.
	try {

		// Call Py_SetProgramName() because the Python interpreter uses the path of the main executable to determine the 
		// location of Python standard library, which gets shipped with the static build of OVITO.
		static QByteArray programName = QCoreApplication::applicationFilePath().toLocal8Bit();
		Py_SetProgramName(programName.data());

		// Make our internal script modules available by registering their initXXX functions with the Python interpreter.
		// This is always required for static builds where all Ovito plugins are linked into the main executable file.
		// On Windows this pre-registration is also needed, because OVITO plugin dynamic libraries have an .dll extension and the Python interpreter 
		// can only find modules that have a .pyd extension.
		for(PythonPluginRegistration* r = PythonPluginRegistration::linkedlist; r != nullptr; r = r->_next) {
			PyImport_AppendInittab(r->_moduleName, r->_initFunc);
		}

		// Initialize the Python interpreter.
		Py_Initialize();

		// Import the main module and get a reference to the main namespace.
		object main_module = import("__main__");
		_prototypeMainNamespace = extract<dict>(main_module.attr("__dict__"));

		// Install automatic QString to Python string conversion.
		struct QString_to_python_str {
			static PyObject* convert(const QString& s) {
				return incref(object(s.toLocal8Bit().constData()).ptr());
			}
		};
		to_python_converter<QString, QString_to_python_str>();

		// Install automatic Python string to QString conversion.
		auto convertible = [](PyObject* obj_ptr) -> void* {
			if(!PyString_Check(obj_ptr) && !PyUnicode_Check(obj_ptr)) return nullptr;
			return obj_ptr;
		};
		auto construct = [](PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data) {
			void* storage = ((boost::python::converter::rvalue_from_python_storage<QString>*)data)->storage.bytes;
			if(PyString_Check(obj_ptr)) {
				const char* value = PyString_AsString(obj_ptr);
				if(!value) throw_error_already_set();
				new (storage) QString(value);
				data->convertible = storage;
			}
			else if(PyUnicode_Check(obj_ptr)) {
				const Py_UNICODE* value = PyUnicode_AS_UNICODE(obj_ptr);
				if(!value) throw_error_already_set();
				if(sizeof(Py_UNICODE) == sizeof(wchar_t))
					new (storage) QString(QString::fromWCharArray(reinterpret_cast<const wchar_t*>(value)));
				else if(sizeof(Py_UNICODE) == sizeof(uint))
					new (storage) QString(QString::fromUcs4(reinterpret_cast<const uint*>(value)));
				else if(sizeof(Py_UNICODE) == sizeof(QChar))
					new (storage) QString(reinterpret_cast<const QChar*>(value));
				else
					throw Exception(tr("The Unicode character size used by Python has an unsupported size: %1 bytes").arg(sizeof(Py_UNICODE)));
				data->convertible = storage;
			}
		};
		converter::registry::push_back(convertible, construct, boost::python::type_id<QString>());

		object sys_module = import("sys");

		// Install output redirection (don't do this in console mode as it interferes with the interactive interpreter).
		if(Application::instance().guiMode()) {
			// Register the output redirector class.
			class_<InterpreterStdOutputRedirector, std::auto_ptr<InterpreterStdOutputRedirector>, boost::noncopyable>("__StdOutStreamRedirectorHelper", no_init).def("write", &InterpreterStdOutputRedirector::write);
			class_<InterpreterStdErrorRedirector, std::auto_ptr<InterpreterStdErrorRedirector>, boost::noncopyable>("__StdErrStreamRedirectorHelper", no_init).def("write", &InterpreterStdErrorRedirector::write);
			// Replace stdout and stderr streams.
			sys_module.attr("stdout") = ptr(new InterpreterStdOutputRedirector());
			sys_module.attr("stderr") = ptr(new InterpreterStdErrorRedirector());
		}

		// Install Ovito to Python exception translator.
		auto toPythonExceptionTranslator = [](const Exception& ex) {
			PyErr_SetString(PyExc_RuntimeError, ex.messages().join(QChar('\n')).toLocal8Bit().constData());
		};
		register_exception_translator<Exception>(toPythonExceptionTranslator);

		// Add directories containing OVITO's Python modules to sys.path.
		list sys_path = extract<list>(sys_module.attr("path"));
		for(const QDir& pluginDir : PluginManager::instance().pluginDirs()) {
#ifndef Q_OS_WIN
			sys_path.insert(0, QDir::toNativeSeparators(pluginDir.absolutePath() + "/python"));
#else
			object path2(QDir::toNativeSeparators(pluginDir.absolutePath() + "/python"));
			PyList_Insert(sys_path.ptr(), 0, path2.ptr());
#endif
		}
	}
	catch(const Exception&) {
		throw;
	}
	catch(const error_already_set&) {
		PyErr_Print();
		throw Exception(tr("Python interpreter has exited with an error."));
	}
	catch(...) {
		throw Exception(tr("Unhandled exception thrown by Python interpreter."));
	}

	_isInterpreterInitialized = true;
	OVITO_ASSERT(!_prototypeMainNamespace.is_none());
}

/******************************************************************************
* Executes one or more Python statements.
******************************************************************************/
int ScriptEngine::execute(const QString& commands)
{
	if(_mainNamespace.is_none())
		throw Exception("Script engine is not initialized.");

	if(!_activeEngine.testAndSetAcquire(nullptr, this))
		throw Exception("There is already another script engine being active.");

	try {
		exec(commands.toLocal8Bit().constData(), _mainNamespace, _mainNamespace);
	    _activeEngine.storeRelease(nullptr);
		return 0;
	}
	catch(const error_already_set&) {
		// Handle call to sys.exit()
		if(PyErr_Occurred() && PyErr_ExceptionMatches(PyExc_SystemExit)) {
		    _activeEngine.storeRelease(nullptr);
			return handleSystemExit();
		}
		PyErr_Print();
	    _activeEngine.storeRelease(nullptr);
		throw Exception(tr("Python interpreter has exited with an error. See interpreter output for details."));
	}
	catch(const Exception&) {
	    _activeEngine.storeRelease(nullptr);
		throw;
	}
	catch(const std::exception& ex) {
	    _activeEngine.storeRelease(nullptr);
		throw Exception(tr("Script execution error: %1").arg(QString::fromLocal8Bit(ex.what())));
	}
	catch(...) {
	    _activeEngine.storeRelease(nullptr);
		throw Exception(tr("Unhandled exception thrown by Python interpreter."));
	}
}

/******************************************************************************
* Executes a Python program.
******************************************************************************/
int ScriptEngine::executeFile(const QString& file)
{
	if(_mainNamespace.is_none())
		throw Exception("Script engine is not initialized.");

	if(!_activeEngine.testAndSetAcquire(nullptr, this))
		throw Exception("There is already another script engine being active.");

	try {
		// Pass command line parameters to the script.
		list argList;
		argList.append(file);
		QStringList scriptArguments = Application::instance().cmdLineParser().values("scriptarg");
		for(const QString& a : scriptArguments)
			argList.append(a);
		import("sys").attr("argv") = argList;

	    exec_file(QDir::toNativeSeparators(file).toLatin1().constData(), _mainNamespace, _mainNamespace);
	    _activeEngine.storeRelease(nullptr);
	    return 0;
	}
	catch(const error_already_set&) {
		// Handle call to sys.exit()
		if(PyErr_Occurred() && PyErr_ExceptionMatches(PyExc_SystemExit)) {
		    _activeEngine.storeRelease(nullptr);
			return handleSystemExit();
		}
		PyErr_Print();
	    _activeEngine.storeRelease(nullptr);
		throw Exception(tr("Python interpreter has exited with an error. See interpreter output for details."));
	}
	catch(const Exception&) {
	    _activeEngine.storeRelease(nullptr);
		throw;
	}
	catch(const std::exception& ex) {
	    _activeEngine.storeRelease(nullptr);
		throw Exception(tr("Script execution error: %1").arg(QString::fromLocal8Bit(ex.what())));
	}
	catch(...) {
	    _activeEngine.storeRelease(nullptr);
		throw Exception(tr("Unhandled exception thrown by Python interpreter."));
	}
}

/******************************************************************************
* Handles a call to sys.exit() in the Python interpreter.
* Returns the program exit code.
******************************************************************************/
int ScriptEngine::handleSystemExit()
{
	PyObject *exception, *value, *tb;
	int exitcode = 0;

	PyErr_Fetch(&exception, &value, &tb);
	if(Py_FlushLine())
		PyErr_Clear();
	if(value == NULL || value == Py_None)
		goto done;
#ifdef PyExceptionInstance_Check
	if(PyExceptionInstance_Check(value)) {	// Python 2.6 or newer
#else
	if(PyInstance_Check(value)) {			// Python 2.4
#endif
		// The error code should be in the code attribute.
		PyObject *code = PyObject_GetAttrString(value, "code");
		if(code) {
			Py_DECREF(value);
			value = code;
			if (value == Py_None)
				goto done;
		}
		// If we failed to dig out the 'code' attribute, just let the else clause below print the error.
	}
	if(PyInt_Check(value))
		exitcode = (int)PyInt_AsLong(value);
	else {
		PyObject *s = PyObject_Str(value);
		QString errorMsg;
		if(s) {
			if(const char* s2 = PyString_AsString(s))
				errorMsg = QString::fromLocal8Bit(s2) + '\n';
		}
		Py_XDECREF(s);
		if(errorMsg.isEmpty() == false)
			Q_EMIT scriptError(errorMsg);
		exitcode = 1;
	}

done:
	PyErr_Restore(exception, value, tb);
	PyErr_Clear();
	return exitcode;
}

};
