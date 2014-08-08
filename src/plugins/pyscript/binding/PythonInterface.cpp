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
#include <core/dataset/DataSet.h>
#include <core/dataset/DataSetContainer.h>
#include <core/scene/SceneNode.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/objects/SceneObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/pipeline/ModifierApplication.h>
#include <core/scene/display/DisplayObject.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/SelectionSet.h>
#include <core/animation/AnimationSettings.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/rendering/RenderSettings.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

void setupRenderingBinding();
void setupAnimationBinding();
void setupViewportBinding();
void setupSceneBinding();
void setupFileIOBinding();

// Implementation of the __str__ method for OvitoObject derived classes.
inline object OvitoObject__str__(OvitoObject* o) {
	if(!o) return str("<OvitoObject nullptr>");
	else return "<%s at address 0x%x>" % make_tuple(o->getOOType().name(), (std::intptr_t)o);
}

BOOST_PYTHON_MODULE(PyScript)
{
	// Make Ovito program version number available to script.
	scope().attr("__dict__")["version"] = make_tuple(OVITO_VERSION_MAJOR, OVITO_VERSION_MINOR, OVITO_VERSION_REVISION);

	class_<OvitoObject, OORef<OvitoObject>, boost::noncopyable>("OvitoObject", no_init)
		.def("__str__", OvitoObject__str__)
	;

	class_<RefMaker, bases<OvitoObject>, OORef<RefMaker>, boost::noncopyable>("RefMaker", no_init)
		.add_property("dataset", make_function(&RefMaker::dataset, return_value_policy<ovito_object_reference>()))
	;

	class_<RefTarget, bases<RefMaker>, OORef<RefTarget>, boost::noncopyable>("RefTarget", no_init)
		.def("isReferencedBy", &RefTarget::isReferencedBy)
		.def("deleteReferenceObject", &RefTarget::deleteReferenceObject)
		.add_property("isBeingEdited", &RefTarget::isBeingEdited)
		.add_property("objectTitle", &RefTarget::objectTitle)
	;

	class_<QVector<DisplayObject*>, boost::noncopyable>("DisplayObjectQVector", no_init)
		.def(QVector_OO_readonly_indexing_suite<DisplayObject>())
	;
	class_<QVector<SceneNode*>, boost::noncopyable>("SceneNodeQVector", no_init)
		.def(QVector_OO_readonly_indexing_suite<SceneNode>())
	;
	class_<QVector<SceneObject*>, boost::noncopyable>("SceneObjectQVector", no_init)
		.def(QVector_OO_readonly_indexing_suite<SceneObject>())
	;
	class_<QVector<OORef<SceneObject>>, boost::noncopyable>("OORefSceneObjectQVector", no_init)
		.def(QVector_OO_readonly_indexing_suite<SceneObject, QVector<OORef<SceneObject>>>())
	;
	class_<QVector<ModifierApplication*>, boost::noncopyable>("ModifierApplicationQVector", no_init)
		.def(QVector_OO_readonly_indexing_suite<ModifierApplication>())
	;

	class_<DataSet, bases<RefTarget>, OORef<DataSet>, boost::noncopyable>("DataSet", no_init)
		.add_property("filePath", make_function(&DataSet::filePath, return_value_policy<copy_const_reference>()), &DataSet::setFilePath)
		.add_property("animationSettings", make_function(&DataSet::animationSettings, return_value_policy<ovito_object_reference>()))
		.add_property("viewportConfig", make_function(&DataSet::viewportConfig, return_value_policy<ovito_object_reference>()))
		.add_property("renderSettings", make_function(&DataSet::renderSettings, return_value_policy<ovito_object_reference>()))
		.add_property("selection", make_function(&DataSet::selection, return_value_policy<ovito_object_reference>()))
		.add_property("sceneRoot", make_function(&DataSet::sceneRoot, return_value_policy<ovito_object_reference>()))
		.add_property("container", make_function(&DataSet::container, return_value_policy<ovito_object_reference>()))
		.def("clearScene", &DataSet::clearScene)
		.def("rescaleTime", &DataSet::rescaleTime)
	;

	class_<DataSetContainer, bases<RefMaker>, boost::noncopyable>("DataSetContainer", no_init)
		.add_property("currentSet", make_function(&DataSetContainer::currentSet, return_value_policy<ovito_object_reference>()), &DataSetContainer::setCurrentSet)
		.def("fileNew", &DataSetContainer::fileNew)
		.def("fileLoad", &DataSetContainer::fileLoad)
		.def("fileSave", &DataSetContainer::fileSave)
		.def("fileSaveAs", &DataSetContainer::fileSaveAs)
		.def("askForSaveChanges", &DataSetContainer::askForSaveChanges)
	;
	
	setupRenderingBinding();
	setupAnimationBinding();
	setupViewportBinding();
	setupSceneBinding();
	setupFileIOBinding();	
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(PyScript);

};
