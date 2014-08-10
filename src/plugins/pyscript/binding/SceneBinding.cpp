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
#include <core/scene/SceneNode.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/GroupNode.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/SelectionSet.h>
#include <core/scene/objects/SceneObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/pipeline/ModifierApplication.h>
#include <core/scene/pipeline/PipelineObject.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

void setupSceneBinding()
{
	class_<PipelineStatus>("PipelineStatus", init<optional<PipelineStatus::StatusType, const QString&>>())
		.add_property("type", &PipelineStatus::type)
		.add_property("text", make_function(&PipelineStatus::text, return_value_policy<copy_const_reference>()))
		.def(self == PipelineStatus())
		.def(self != PipelineStatus())
	;

	enum_<PipelineStatus::StatusType>("PipelineStatusType")
		.value("Success", PipelineStatus::Success)
		.value("Warning", PipelineStatus::Warning)
		.value("Error", PipelineStatus::Error)
		.value("Pending", PipelineStatus::Pending)
	;

	class_<PipelineFlowState>("PipelineFlowState", init<>())
		.def(init<SceneObject*, TimeInterval>())
		.def(init<const PipelineStatus&, const QVector<SceneObject*>&, const TimeInterval&>())
		.def("clear", &PipelineFlowState::clear)
		.def("addObject", &PipelineFlowState::addObject)
		.def("replaceObject", &PipelineFlowState::replaceObject)
		.def("removeObject", &PipelineFlowState::removeObject)
		.add_property("count", &PipelineFlowState::count)
		.add_property("isEmpty", &PipelineFlowState::isEmpty)
		.add_property("status", make_function(&PipelineFlowState::status, return_internal_reference<>()), &PipelineFlowState::setStatus)
		.add_property("objects", make_function(&PipelineFlowState::objects, return_internal_reference<>()))
	;

	ovito_abstract_class<SceneObject, RefTarget>()
		.def("objectValidity", &SceneObject::objectValidity)
		.def("evaluate", &SceneObject::evaluate)
		.def("addDisplayObject", &SceneObject::addDisplayObject)
		.def("setDisplayObject", &SceneObject::setDisplayObject)
		.def("inputObject", make_function(&SceneObject::inputObject, return_value_policy<ovito_object_reference>()))
		.add_property("status", &SceneObject::status)
		.add_property("displayObjects", make_function(&SceneObject::displayObjects, return_internal_reference<>()))
		.add_property("saveWithScene", &SceneObject::saveWithScene, &SceneObject::setSaveWithScene)
		.add_property("inputObjectCount", &SceneObject::inputObjectCount)
	;

	ovito_abstract_class<Modifier, RefTarget>()
		.add_property("enabled", &Modifier::isEnabled, &Modifier::setEnabled)
		.add_property("status", &Modifier::status)
		.def("modifierValidity", &Modifier::modifierValidity)
		.def("modifierApplications", &Modifier::modifierApplications)
		.def("isApplicableTo", &Modifier::isApplicableTo)
	;

	ovito_class<ModifierApplication, RefTarget>()
		.def(init<DataSet*, Modifier*>())
		.add_property("modifier", make_function(&ModifierApplication::modifier, return_value_policy<ovito_object_reference>()))
		.add_property("pipelineObject", make_function(&ModifierApplication::pipelineObject, return_value_policy<ovito_object_reference>()))
		.add_property("objectNodes", &ModifierApplication::objectNodes)
	;

	ovito_class<PipelineObject, SceneObject>()
		.add_property("inputObject", make_function((SceneObject* (PipelineObject::*)() const)&PipelineObject::inputObject, return_value_policy<ovito_object_reference>()), &PipelineObject::setInputObject)
		.add_property("modifierApplications", make_function(&PipelineObject::modifierApplications, return_internal_reference<>()))
		.def("insertModifier", make_function(&PipelineObject::insertModifier, return_value_policy<ovito_object_reference>()))
		.def("insertModifierApplication", &PipelineObject::insertModifierApplication)
		.def("removeModifier", &PipelineObject::removeModifier)
	;

	ovito_abstract_class<SceneNode, RefTarget>()
		.add_property("name", make_function(&SceneNode::name, return_value_policy<copy_const_reference>()), &SceneNode::setName)
		.add_property("displayColor", make_function(&SceneNode::displayColor, return_value_policy<copy_const_reference>()), &SceneNode::setDisplayColor)
		.add_property("parentNode", make_function(&SceneNode::parentNode, return_value_policy<ovito_object_reference>()))
		.add_property("childCount", &SceneNode::childCount)
		.add_property("children", make_function(&SceneNode::children, return_internal_reference<>()))
		.add_property("targetNode", make_function(&SceneNode::targetNode, return_value_policy<ovito_object_reference>()))
		.add_property("selected", &SceneNode::isSelected, &SceneNode::setSelected)
		.def("deleteNode", &SceneNode::deleteNode)
		.def("addChild", &SceneNode::addChild)
		.def("removeChild", &SceneNode::removeChild)
		.def("localBoundingBox", &SceneNode::localBoundingBox)
		.def("worldBoundingBox", make_function(&SceneNode::worldBoundingBox, return_value_policy<copy_const_reference>()))
	;

	ovito_class<ObjectNode, SceneNode>()
		.add_property("sceneObject", make_function(&ObjectNode::sceneObject, return_value_policy<ovito_object_reference>()), &ObjectNode::setSceneObject)
		.add_property("displayObjects", make_function(&ObjectNode::displayObjects, return_internal_reference<>()))
		.def("evalPipeline", make_function(&ObjectNode::evalPipeline, return_value_policy<copy_const_reference>()))
		.def("applyModifier", &ObjectNode::applyModifier)
	;

	ovito_class<SceneRoot, SceneNode>()
		.def("getNodeByName", make_function(&SceneRoot::getNodeByName, return_value_policy<ovito_object_reference>()))
		.def("makeNameUnique", &SceneRoot::makeNameUnique)
	;

	ovito_class<GroupNode, SceneNode>()
		.add_property("open", &GroupNode::isGroupOpen, &GroupNode::setGroupOpen)
	;

	ovito_class<SelectionSet, RefTarget>()
		.add_property("count", &SelectionSet::count)
		.add_property("empty", &SelectionSet::empty)
		.add_property("firstNode", make_function(&SelectionSet::firstNode, return_value_policy<ovito_object_reference>()))
		.add_property("nodes", make_function(&SelectionSet::nodes, return_internal_reference<>()))
		.def("contains", &SelectionSet::contains)
		.def("add", &SelectionSet::add)
		.def("clear", &SelectionSet::clear)
		.def("remove", &SelectionSet::remove)
		.def("boundingBox", &SelectionSet::boundingBox)
		.def("setNode", &SelectionSet::setNode)
	;
}

};
