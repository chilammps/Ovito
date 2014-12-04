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
#include <core/scene/objects/DataObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/pipeline/ModifierApplication.h>
#include <core/scene/objects/DisplayObject.h>
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
inline object OvitoObject__str__(const object& pyobj) {
	OvitoObject* o = extract<OvitoObject*>(pyobj);
	return "<%s at 0x%x>" % make_tuple(pyobj.attr("__class__").attr("__name__"), (std::intptr_t)o);
}

// Implementation of the __repr__ method for OvitoObject derived classes.
inline object OvitoObject__repr__(const object& pyobj) {
	return "%s()" % pyobj.attr("__class__").attr("__name__");
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
		.def("__repr__", &OvitoObject__repr__)
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
			"Basically everything that would get saved in an OVITO file. "
			"\n\n"
			"There exists only one global instance of this class, which can be accessed via the :py:data:`ovito.dataset` module-level attribute.")
		.add_property("scene_nodes", make_function(&DataSet::sceneRoot, return_value_policy<ovito_object_reference>()),
				"A list-like object containing the :py:class:`~ovito.ObjectNode` instances that are part of the three-dimensional scene. "
				"Only nodes in this list are visible in the viewports. You can add or remove nodes from this list.")
		.add_property("filePath", make_function(&DataSet::filePath, return_value_policy<copy_const_reference>()), &DataSet::setFilePath)
		.add_property("anim", make_function(&DataSet::animationSettings, return_value_policy<ovito_object_reference>()),
				"An :py:class:`~ovito.anim.AnimationSettings` object, which manages various animation-related settings in OVITO such as the number of frames, the current frame, playback speed etc.")
		.add_property("viewports", make_function(&DataSet::viewportConfig, return_value_policy<ovito_object_reference>()),
				"A :py:class:`~ovito.vis.ViewportConfiguration` object managing the viewports in OVITO's main window.")
		.add_property("render_settings", make_function(&DataSet::renderSettings, return_value_policy<ovito_object_reference>()),
				"The global :py:class:`~ovito.vis.RenderSettings` object, which stores the current settings for rendering pictures and movies. "
				"These are the settings the user edits on the :guilabel:`Render` tab of OVITO's main window.")
		.add_property("selection", make_function(&DataSet::selection, return_value_policy<ovito_object_reference>()))
		.add_property("container", make_function(&DataSet::container, return_value_policy<ovito_object_reference>()))
		.def("clearScene", &DataSet::clearScene)
		.def("rescaleTime", &DataSet::rescaleTime)
		.def("waitUntilSceneIsReady", &DataSet::waitUntilSceneIsReady, DataSet_waitUntilSceneIsReady_overloads())
		.def("renderScene", &DataSet::renderScene, DataSet_renderScene_overloads())
		.def("saveToFile", &DataSet::saveToFile)
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
