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
#include <core/gui/app/Application.h>
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
#include <core/rendering/FrameBuffer.h>
#include <core/gui/widgets/rendering/FrameBufferWindow.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

// Implementation of the __str__ method for OvitoObject derived classes.
inline object OvitoObject__str__(OvitoObject* o) {
	if(!o) return str("<OvitoObject nullptr>");
	else return "<%s at address 0x%x>" % make_tuple(o->getOOType().name(), (std::intptr_t)o);
}

// Implementation of the __eq__ method for OvitoObject derived classes.
inline bool OvitoObject__eq__(OvitoObject* o, const object& other) {
	extract<OvitoObject*> other_oo(other);
	if(!other_oo.check()) return false;
	return o == other_oo();
}

// Implementation of the __ne__ method for OvitoObject derived classes.
inline bool OvitoObject__ne__(OvitoObject* o, const object& other) {
	return !OvitoObject__eq__(o, other);
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(DataSet_waitUntilSceneIsReady_overloads, waitUntilSceneIsReady, 1, 2);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(DataSet_renderScene_overloads, renderScene, 2, 4);

BOOST_PYTHON_MODULE(PyScriptApp)
{
	docstring_options docoptions(true, false);

	class_<OvitoObject, OORef<OvitoObject>, boost::noncopyable>("OvitoObject", no_init)
		.def("__str__", &OvitoObject__str__)
		.def("__eq__", &OvitoObject__eq__)
		.def("__ne__", &OvitoObject__ne__)
	;

	ovito_abstract_class<RefMaker, OvitoObject>()
		.add_property("dataset", make_function(&RefMaker::dataset, return_value_policy<ovito_object_reference>()))
	;

	ovito_abstract_class<RefTarget, RefMaker>()
		.def("isReferencedBy", &RefTarget::isReferencedBy)
		.def("deleteReferenceObject", &RefTarget::deleteReferenceObject)
		.add_property("isBeingEdited", &RefTarget::isBeingEdited)
		.add_property("objectTitle", &RefTarget::objectTitle)
	;

	ovito_abstract_class<DataSet, RefTarget>(
			"A container object holding all data associated with an OVITO program session. "
			"It provides access to the scene data, the viewports, the current selection, and the animation settings. "
			"Basically everything that gets saved in an OVITO file. "
			"\n\n"
			"There exists exactly one instance of this class, which can be accessed via the :py:data:`ovito.dataset` module-level attribute.")
		.add_property("sceneRoot", make_function(&DataSet::sceneRoot, return_value_policy<ovito_object_reference>()),
				"The root :py:class:`~ovito.scene.SceneNode` of the scene graph. This provides access to the objects "
				"in the three-dimensional scene.")
		.add_property("filePath", make_function(&DataSet::filePath, return_value_policy<copy_const_reference>()), &DataSet::setFilePath)
		.add_property("anim", make_function(&DataSet::animationSettings, return_value_policy<ovito_object_reference>()),
				"An :py:class:`~ovito.anim.AnimationSettings` object, which manages various animation-related settings such as the number of frames, the current frame, playback speed etc.")
		.add_property("viewports", make_function(&DataSet::viewportConfig, return_value_policy<ovito_object_reference>()),
				"A :py:class:`~ovito.view.ViewportConfiguration` object that contains OVITO's 3d viewports.")
		.add_property("renderSettings", make_function(&DataSet::renderSettings, return_value_policy<ovito_object_reference>()),
				"A :py:class:`~ovito.render.RenderSettings` object, which stores the current settings for rendering pictures and movies.")
		.add_property("selection", make_function(&DataSet::selection, return_value_policy<ovito_object_reference>()))
		.add_property("container", make_function(&DataSet::container, return_value_policy<ovito_object_reference>()))
		.def("clearScene", &DataSet::clearScene)
		.def("rescaleTime", &DataSet::rescaleTime)
		.def("waitUntilSceneIsReady", &DataSet::waitUntilSceneIsReady, DataSet_waitUntilSceneIsReady_overloads())
		.def("renderScene", &DataSet::renderScene, DataSet_renderScene_overloads())
	;

	ovito_abstract_class<DataSetContainer, RefMaker>()
		.add_property("currentSet", make_function(&DataSetContainer::currentSet, return_value_policy<ovito_object_reference>()), &DataSetContainer::setCurrentSet)
		.def("fileNew", &DataSetContainer::fileNew)
		.def("fileLoad", &DataSetContainer::fileLoad)
		.def("fileSave", &DataSetContainer::fileSave)
		.def("fileSaveAs", &DataSetContainer::fileSaveAs)
		.def("askForSaveChanges", &DataSetContainer::askForSaveChanges)
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(PyScriptApp);

};
