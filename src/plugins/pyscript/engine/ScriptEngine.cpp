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
#include <core/plugins/PluginManager.h>
#include <core/dataset/DataSet.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/dataset/importexport/LinkedFileImporter.h>
#include <core/scene/SelectionSet.h>
#include <core/scene/SceneNode.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/GroupNode.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/pipeline/ModifierApplication.h>
#include <core/animation/AnimationSettings.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/controller/PRSTransformationController.h>
#include <core/animation/controller/LookAtController.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/Viewport.h>
#include <core/rendering/RenderSettings.h>
#include <core/rendering/SceneRenderer.h>
#include "ScriptEngine.h"
#include "ScriptBinding.h"
#include "moc_ScriptBinding.cpp"

namespace PyScript {

IMPLEMENT_OVITO_OBJECT(PyScript, ScriptBinding, OvitoObject);

/// The prototype namespace that is used to initialize new script engines with.
boost::python::object ScriptEngine::_prototypeMainNamespace;

/******************************************************************************
* Initializes the scripting engine and sets up the environment.
******************************************************************************/
ScriptEngine::ScriptEngine(DataSet* dataset, QObject* parent)
	: QObject(parent), _dataset(dataset)
{
	using namespace std;
	using namespace std::placeholders;

	initializeInterpreter();

	// Create script binding objects and let them set up the script environment.
	for(const OvitoObjectType* bindingClass : PluginManager::instance().listClasses(ScriptBinding::OOType)) {
		OORef<ScriptBinding> binding = static_object_cast<ScriptBinding>(bindingClass->createInstance(nullptr));
		_bindings.push_back(binding);
		binding->setupBinding(*this);
	}

	// Initialize state of the script engine.
	try {
		// Make a local copy of the global main namespace for this engine.
		// The original namespace dictionary is not touched.
		_mainNamespace = boost::python::object(boost::python::handle<>(PyDict_Copy(_prototypeMainNamespace.ptr())));
	}
	catch(const boost::python::error_already_set&) {
		PyErr_Print();
		throw Exception(tr("Could not initialize Python interpreter. See console output for error details."));
	}
}

/******************************************************************************
* Initializes the Python interpreter and sets up the global namespace.
******************************************************************************/
void ScriptEngine::initializeInterpreter()
{
	try {
		qDebug() << "Initializing embedded Python interpreter." << endl;
		qDebug() << "Python program name: " << Py_GetProgramName() << endl;
		qDebug() << "Python program full path: " << Py_GetProgramFullPath() << endl;
		qDebug() << "Python prefix: " << Py_GetPrefix() << endl;
		qDebug() << "Python exec prefix: " << Py_GetExecPrefix() << endl;
		qDebug() << "Python path: " << Py_GetPath() << endl;

		// Initialize the Python interpreter.
		Py_Initialize();
	}
	catch(const Exception&) {
		throw;
	}
	catch(const boost::python::error_already_set&) {
		PyErr_Print();
		throw Exception(tr("Python interpreter has exited with an error."));
	}
	catch(...) {
		throw Exception(tr("Unhandled exception thrown by Python interpreter."));
	}
}

/******************************************************************************
* Executes one or more Python statements.
******************************************************************************/
void ScriptEngine::execute(const QString& commands)
{
	try {
	    boost::python::exec(commands.toLatin1().constData());
	}
	catch(const boost::python::error_already_set& ex) {
		PyErr_Print();
		throw Exception(tr("Python interpreter has exited with an error. See interpreter output for details."));
	}
}

/******************************************************************************
* Executes a Python program.
******************************************************************************/
void ScriptEngine::executeFile(const QString& file)
{
	try {
	    boost::python::exec_file(file.toLatin1().constData());
	}
	catch(const boost::python::error_already_set& ex) {
		PyErr_Print();
		throw Exception(tr("Python interpreter has exited with an error. See interpreter output for details."));
	}
}

};
