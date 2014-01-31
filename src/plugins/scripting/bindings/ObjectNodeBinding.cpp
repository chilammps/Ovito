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
	// Install a prototype for ObjectNode values.
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
	ObjectNode* objNode = qscriptvalue_cast<ObjectNode*>(thisObject());
	if(objNode) {

		// Go through the modification pipeline and collect all modifiers in a list.
		PipelineObject* pipelineObj = dynamic_object_cast<PipelineObject>(objNode->sceneObject());
		while(pipelineObj) {
			for(ModifierApplication* modApp : pipelineObj->modifierApplications()) {
				result.push_back(modApp->modifier());
			}
			pipelineObj = dynamic_object_cast<PipelineObject>(pipelineObj->inputObject());
		}
	}
	return result;
}


};
