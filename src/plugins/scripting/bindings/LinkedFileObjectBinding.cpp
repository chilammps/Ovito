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

#include <plugins/scripting/Scripting.h>
#include <plugins/scripting/engine/ScriptEngine.h>
#include <core/dataset/importexport/ImportExportManager.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/dataset/DataSetContainer.h>
#include <core/utilities/io/FileManager.h>
#include "LinkedFileObjectBinding.h"

namespace Scripting {

IMPLEMENT_OVITO_OBJECT(Scripting, LinkedFileObjectBinding, ScriptBinding);

/******************************************************************************
* Sets up the global object of the script engine.
******************************************************************************/
void LinkedFileObjectBinding::setupBinding(ScriptEngine& engine)
{
	// Install this prototype for LinkedFileObject values.
	QScriptValue prototype = engine.newQObject(this);
	prototype.setProperty("load", engine.newStdFunction(&LinkedFileObjectBinding::load, 1));
	engine.setDefaultPrototype(qMetaTypeId<LinkedFileObject*>(), prototype);
}

/******************************************************************************
* Loads an external file.
******************************************************************************/
QScriptValue LinkedFileObjectBinding::load(QScriptContext* context, ScriptEngine* engine)
{
	// Get this object.
	LinkedFileObject* obj = ScriptEngine::getThisObject<LinkedFileObject>(context);
	if(!obj)
		return context->throwError(QScriptContext::TypeError, tr("LinkedFileObject.prototype.load: This is not a LinkedFileObject."));

	// Process function arguments.
	if(context->argumentCount() < 1 || context->argumentCount() > 2)
		return context->throwError(tr("load() method expects 1 or 2 arguments."));
	const QString urlString = context->argument(0).toString();
	QUrl importURL = FileManager::instance().urlFromUserInput(urlString);
	if(!importURL.isValid())
		return context->throwError(tr("load(): Invalid path or URL."));

	// Download file so we can determine its format.
	DataSet* dataset = engine->dataset();
	DataSetContainer* container = dataset->container();
	Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(*container, importURL);
	if(!container->taskManager().waitForTask(fetchFileFuture))
		return context->throwError(tr("Operation has been canceled by the user."));

	// Detect file format.
	OORef<LinkedFileImporter> importer = dynamic_object_cast<LinkedFileImporter>(ImportExportManager::instance().autodetectFileFormat(dataset, fetchFileFuture.result(), importURL.path()));
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
	else {
		// Re-use the old importer if possible.
		if(obj->importer() && obj->importer()->getOOType() == importer->getOOType())
			importer = obj->importer();
	}

	// Import data.
	if(!obj->setSource(importURL, importer, true))
		return context->throwError(tr("Operation has been canceled by the user."));

	return engine->undefinedValue();
}

};
