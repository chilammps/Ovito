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
	// Add a getter function property to workaround bug in Qt script.
	datasetPrototype.setProperty("__qtsworksround__", engine.noopFunction(), QScriptValue::PropertyGetter);

	// Make command line arguments accessible to script.
	datasetPrototype.setProperty("arguments", engine.toScriptValue(QCoreApplication::arguments()));

	// Install a prototype for DataSet values.
	engine.setDefaultPrototype(qMetaTypeId<DataSet*>(), datasetPrototype);

	// Make the current dataset accessible to the script through the 'ovito' property.
	engine.globalObject().setProperty("ovito", engine.wrapOvitoObject(engine.dataset()));
}

/******************************************************************************
* Implementation of the 'selectedNode' property.
******************************************************************************/
SceneNode* DataSetBinding::selectedNode() const
{
	DataSet* dataset = ScriptEngine::getThisObject<DataSet>(context());
	if(dataset)
		return dataset->selection()->firstNode();
	else {
		context()->throwError(QScriptContext::TypeError, tr("DataSet.prototype.selectedNode: this is not a DataSet."));
		return nullptr;
	}
}

/******************************************************************************
* Implementation of the 'selectedNode' property.
******************************************************************************/
void DataSetBinding::setSelectedNode(SceneNode* node)
{
	DataSet* dataset = ScriptEngine::getThisObject<DataSet>(context());
	if(dataset) {
		if(!node)
			dataset->selection()->clear();
		else
			dataset->selection()->setNode(node);
	}
	else
		context()->throwError(QScriptContext::TypeError, tr("DataSet.prototype.selectedNode: this is not a DataSet."));
}


};
