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
#include <core/scene/SelectionSet.h>
#include "DataSetBinding.h"

namespace Scripting {

IMPLEMENT_OVITO_OBJECT(Scripting, DataSetBinding, ScriptBinding);

/******************************************************************************
* Sets up the global object of the script engine.
******************************************************************************/
void DataSetBinding::setupBinding(ScriptEngine& engine)
{
	// Create a prototype for DataSet values.
	QScriptValue datasetPrototype = engine.newQObject(this);
	// The 'version' property returns the version string of the application.
	datasetPrototype.setProperty("version", QCoreApplication::applicationVersion());
	// The 'selectedNode' property gets/sets the currently selected scene node.
	datasetPrototype.setProperty("selectedNode", engine.newFunction(&DataSetBinding::selectedNode, 0), QScriptValue::PropertyGetter);
	datasetPrototype.setProperty("selectedNode", engine.newFunction(&DataSetBinding::setSelectedNode, 1), QScriptValue::PropertySetter);

	// Install a prototype for DataSet values.
	engine.setDefaultPrototype(qMetaTypeId<DataSet*>(), datasetPrototype);

	// Make the current dataset accessible to the script through the 'ovito' property.
	engine.globalObject().setProperty("ovito", engine.wrapOvitoObject(engine.dataset()));
}

/******************************************************************************
* Implementation of the 'selectedNode' property.
******************************************************************************/
QScriptValue DataSetBinding::selectedNode(QScriptContext* context, QScriptEngine* engine)
{
	DataSet* dataset = qscriptvalue_cast<DataSet*>(context->thisObject());
	if(dataset)
		return engine->toScriptValue(dataset->selection()->firstNode());
	else
		return context->throwError(QScriptContext::TypeError, tr("DataSet.prototype.selectedNode: this is not a DataSet."));
}

/******************************************************************************
* Implementation of the 'selectedNode' property.
******************************************************************************/
QScriptValue DataSetBinding::setSelectedNode(QScriptContext* context, QScriptEngine* engine)
{
	DataSet* dataset = qscriptvalue_cast<DataSet*>(context->thisObject());
	if(dataset) {
		if(context->argumentCount() != 1)
			return context->throwError(tr("DataSet.prototype.selectedNode: expected 1 argument."));
		if(context->argument(0).isNull())
			dataset->selection()->clear();
		else {
			SceneNode* node = qscriptvalue_cast<SceneNode*>(context->argument(0));
			if(!node)
				return context->throwError(QScriptContext::TypeError, tr("DataSet.prototype.selectedNode: argument is not a SceneNode."));
			dataset->selection()->setNode(node);
		}
		return engine->undefinedValue();
	}
	else
		return context->throwError(QScriptContext::TypeError, tr("DataSet.prototype.selectedNode: this is not a DataSet."));
}


};
