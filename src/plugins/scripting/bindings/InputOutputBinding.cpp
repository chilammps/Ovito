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

#include <plugins/scripting/Scripting.h>
#include <plugins/scripting/engine/ScriptEngine.h>
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/importexport/ImportExportManager.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/scene/SelectionSet.h>
#include <core/scene/ObjectNode.h>
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/utilities/io/FileManager.h>
#include "InputOutputBinding.h"

namespace Scripting {

IMPLEMENT_OVITO_OBJECT(Scripting, InputOutputBinding, ScriptBinding);

/******************************************************************************
* Sets up the global object of the script engine.
******************************************************************************/
void InputOutputBinding::setupBinding(ScriptEngine& engine)
{
	engine.globalObject().setProperty("load", engine.newStdFunction(&InputOutputBinding::load, 1));
	engine.globalObject().setProperty("save", engine.newStdFunction(&InputOutputBinding::save, 2));
	engine.globalObject().setProperty("cd", engine.newStdFunction(&InputOutputBinding::cd, 1));
	engine.globalObject().setProperty("pwd", engine.newStdFunction(&InputOutputBinding::pwd, 0));
	engine.globalObject().setProperty("wait", engine.newStdFunction(&InputOutputBinding::wait, 0));

	// The version() script function returns the version string of the application.
	engine.globalObject().setProperty("version", engine.newStdFunction(
			[](QScriptContext* context, ScriptEngine* engine) -> QScriptValue {
		return QCoreApplication::applicationVersion();
	}, 0));

	// Marshaling of URLs.
	qScriptRegisterMetaType<QUrl>(&engine, fromQUrl, toQUrl);
}

/******************************************************************************
* Implementation of the 'load' script command.
*
* Imports an external data file into the scene.
******************************************************************************/
QScriptValue InputOutputBinding::load(QScriptContext* context, ScriptEngine* engine)
{
	// Process function arguments.
	if(context->argumentCount() < 1 || context->argumentCount() > 2)
		return context->throwError(tr("load() takes 1 or 2 arguments."));
	const QString urlString = context->argument(0).toString();
	QUrl importURL = FileManager::instance().urlFromUserInput(urlString);
	if(!importURL.isValid())
		return context->throwError(tr("Invalid path or URL."));

	// Download file so we can determine its format.
	DataSet* dataset = engine->dataset();
	DataSetContainer* container = dataset->container();
	Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(*container, importURL);
	if(!container->taskManager().waitForTask(fetchFileFuture))
		return context->throwError(tr("Operation has been canceled by the user."));

	// Detect file format.
	OORef<FileImporter> importer = ImportExportManager::instance().autodetectFileFormat(dataset, fetchFileFuture.result(), importURL.path());
	if(!importer)
		return context->throwError(tr("Could not detect the file format. The format might not be supported."));

	// Set import parameters passed as second argument to the load() function.
	if(context->argumentCount() >= 2) {
		QScriptValue importerScriptValue = engine->wrapOvitoObject(importer);
		QScriptValueIterator it(context->argument(1));
		while(it.hasNext()) {
			it.next();
			importerScriptValue.setProperty(it.name(), it.value());
		}
	}

	// Import data.
	if(!importer->importFile(importURL, FileImporter::AddToScene))
		return context->throwError(tr("Operation has been canceled by the user."));

	// Return the newly created ObjectNode.
	ObjectNode* objNode = dynamic_object_cast<ObjectNode>(engine->dataset()->selection()->firstNode());
	if(!objNode)
		throw Exception(tr("File import was not successful."));

	return engine->wrapOvitoObject(objNode);
}

/******************************************************************************
* Implementation of the 'save' script command.
*
* Exports a scene object to an external data file.
******************************************************************************/
QScriptValue InputOutputBinding::save(QScriptContext* context, ScriptEngine* engine)
{
	// Process function arguments.
	if(context->argumentCount() < 2 || context->argumentCount() > 4)
		return context->throwError(tr("save() takes between 2 and 4 arguments."));

	// Get output filename.
	const QString outputPath = context->argument(0).toString();
	if(outputPath.isEmpty())
		return context->throwError(tr("Invalid output path (first argument passed to save() function)."));

	// Create exporter instance. The constructor function is passed as second argument to the save() function.
	if(!context->argument(1).isFunction())
		return context->throwError(tr("Invalid exporter type (second argument passed to save() function)."));
	QScriptValueList constructorArgs;
	if(context->argumentCount() >= 3)
		constructorArgs << context->argument(2);
	// Use the property-value map passed to save() function to initialize the exporter object.
	QScriptValue exporterObject = context->argument(1).construct(constructorArgs);
	if(exporterObject.isError() || engine->hasUncaughtException())
		return exporterObject;
	FileExporter* exporter = qscriptvalue_cast<FileExporter*>(exporterObject);
	if(!exporter)
		return context->throwError(tr("Could not create an instance of the exporter type (second argument passed to save() function)."));

	// Get the scene nodes to be exported.
	DataSet* dataset = engine->dataset();
	QVector<SceneNode*> nodes;
	if(context->argumentCount() >= 4) {
		SceneNode* node = qscriptvalue_cast<SceneNode*>(context->argument(3));
		if(!node)
			return context->throwError(tr("That's not a scene node (fourth argument passed to save() function)."));
		nodes.push_back(node);
	}
	else nodes = dataset->selection()->nodes();

	// Export data.
	if(!exporter->exportToFile(nodes, outputPath, true))
		return context->throwError(tr("Operation has been canceled by the user."));

	return QScriptValue();
}

/******************************************************************************
* Implementation of the 'cd' script command.
*
* Changes the current working directory.
******************************************************************************/
QScriptValue InputOutputBinding::cd(QScriptContext* context, ScriptEngine* engine)
{
	// Process function arguments.
	if(context->argumentCount() != 1)
		return context->throwError("cd() takes one argument.");
	QString newDirectory = context->argument(0).toString();

	// Change current working directory.
	if(QDir::setCurrent(newDirectory))
		return QScriptValue(QDir::currentPath());
	else
		return context->throwError(tr("Could not set current directory to '%1'").arg(newDirectory));
}

/******************************************************************************
* Implementation of the 'pwd' script command.
*
* Returns the current working directory.
******************************************************************************/
QScriptValue InputOutputBinding::pwd(QScriptContext* context, ScriptEngine* engine)
{
	// Process function arguments.
	if(context->argumentCount() != 0)
		return context->throwError("pwd() takes no arguments.");

	return QScriptValue(QDir::currentPath());
}

/******************************************************************************
* Implementation of the 'wait' script command.
*
* This functions blocks execution of the script until the scene is ready,
* that is, until all files have been loaded and all modifiers have been computed.
******************************************************************************/
QScriptValue InputOutputBinding::wait(QScriptContext* context, ScriptEngine* engine)
{
	// Process function arguments.
	if(context->argumentCount() != 0)
		return context->throwError("wait() takes no arguments.");

	// Wait until scene is ready.
	volatile bool sceneIsReady = false;
	engine->dataset()->runWhenSceneIsReady( [&sceneIsReady]() { sceneIsReady = true; } );
	if(!sceneIsReady) {

		if(Application::instance().guiMode()) {

			// Show a modal progress dialog to block user interface while waiting for the scene to become ready.
			QProgressDialog progressDialog(engine->dataset()->mainWindow());
			progressDialog.setWindowModality(Qt::WindowModal);
			progressDialog.setAutoClose(false);
			progressDialog.setAutoReset(false);
			progressDialog.setMinimumDuration(0);
			progressDialog.setValue(0);
			progressDialog.setLabelText(tr("Script is waiting for scene graph to become ready."));

			// Poll the flag that indicates if the scene is ready.
			while(!sceneIsReady) {
				if(progressDialog.wasCanceled())
					return QScriptValue(false);
				QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 50);
			}
		}
		else {
			// Poll the flag that indicates if the scene is ready.
			while(!sceneIsReady) {
				QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 50);
			}
		}
	}

	return QScriptValue(true);
}

/******************************************************************************
* Converts a QUrl to a script value.
******************************************************************************/
QScriptValue InputOutputBinding::fromQUrl(QScriptEngine *engine, const QUrl& url)
{
	return url.toDisplayString();
}

/******************************************************************************
* Converts a script value to a QUrl.
******************************************************************************/
void InputOutputBinding::toQUrl(const QScriptValue& sv, QUrl& url)
{
	url = FileManager::instance().urlFromUserInput(sv.toString());
	if(!url.isValid())
		sv.engine()->currentContext()->throwError("Invalid path or URL.");
}

};
