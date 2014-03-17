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
#include <core/scene/ObjectNode.h>
#include <core/scene/SelectionSet.h>
#include <core/scene/pipeline/PipelineObject.h>
#include "ObjectNodeBinding.h"

namespace Scripting {

IMPLEMENT_OVITO_OBJECT(Scripting, ObjectNodeBinding, ScriptBinding);

/******************************************************************************
* Sets up the global object of the script engine.
******************************************************************************/
void ObjectNodeBinding::setupBinding(ScriptEngine& engine)
{
	// Install this prototype for ObjectNode values.
	engine.setDefaultPrototype(qMetaTypeId<ObjectNode*>(), engine.newQObject(this));

	// This is required for the 'modifiers' property of the ObjectNode prototype.
	qScriptRegisterSequenceMetaType<QVector<Modifier*>>(&engine);
}

/******************************************************************************
* Returns the list of modifiers that are in the pipeline of the ObjectNode.
******************************************************************************/
QVector<Modifier*> ObjectNodeBinding::modifiers()
{
	QVector<Modifier*> result;
	ObjectNode* objNode = ScriptEngine::getThisObject<ObjectNode>(context());
	if(!objNode) {
		context()->throwError(QScriptContext::TypeError, tr("ObjectNode.prototype.modifiers: This is not an ObjectNode."));
		return result;
	}

	// Go through the modification pipeline and collect all modifiers in a list.
	PipelineObject* pipelineObj = dynamic_object_cast<PipelineObject>(objNode->sceneObject());
	while(pipelineObj) {
		for(ModifierApplication* modApp : pipelineObj->modifierApplications()) {
			result.push_back(modApp->modifier());
		}
		pipelineObj = dynamic_object_cast<PipelineObject>(pipelineObj->inputObject());
	}
	return result;
}

/******************************************************************************
* Returns the SceneObject that is the data source of the modification pipeline.
******************************************************************************/
SceneObject* ObjectNodeBinding::source()
{
	ObjectNode* objNode = ScriptEngine::getThisObject<ObjectNode>(context());
	if(!objNode) {
		context()->throwError(QScriptContext::TypeError, tr("ObjectNode.prototype.source: This is not an ObjectNode."));
		return nullptr;
	}

	SceneObject* sceneObj = objNode->sceneObject();
	while(sceneObj) {
		if(sceneObj->inputObjectCount() == 0) break;
		SceneObject* inputObj = sceneObj->inputObject(0);
		if(!inputObj) break;
		sceneObj = inputObj;
	}
	return sceneObj;
}

};
