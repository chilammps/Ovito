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
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/Viewport.h>
#include <core/scene/SceneNode.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

BOOST_PYTHON_MODULE(PyScriptViewport)
{
	docstring_options docoptions(true, false);

	ovito_class<Viewport, RefTarget>()
		.add_property("isRendering", &Viewport::isRendering)
		.add_property("isPerspectiveProjection", &Viewport::isPerspectiveProjection)
		.add_property("viewType", &Viewport::viewType, (void (*)(Viewport&,Viewport::ViewType))([](Viewport& vp, Viewport::ViewType vt) { vp.setViewType(vt); }))
		.add_property("fieldOfView", &Viewport::fieldOfView, &Viewport::setFieldOfView)
		.add_property("cameraTransformation", make_function(&Viewport::cameraTransformation, return_value_policy<copy_const_reference>()), &Viewport::setCameraTransformation)
		.add_property("cameraDirection", &Viewport::cameraDirection, &Viewport::setCameraDirection)
		.add_property("cameraPosition", &Viewport::cameraPosition, &Viewport::setCameraPosition)
		.add_property("viewMatrix", make_function(&Viewport::viewMatrix, return_value_policy<copy_const_reference>()))
		.add_property("inverseViewMatrix", make_function(&Viewport::inverseViewMatrix, return_value_policy<copy_const_reference>()))
		.add_property("projectionMatrix", make_function(&Viewport::projectionMatrix, return_value_policy<copy_const_reference>()))
		.add_property("inverseProjectionMatrix", make_function(&Viewport::inverseProjectionMatrix, return_value_policy<copy_const_reference>()))
		.add_property("renderFrameShown", &Viewport::renderFrameShown, &Viewport::setRenderFrameShown)
		.add_property("gridVisible", &Viewport::isGridVisible, &Viewport::setGridVisible)
		.add_property("viewNode", make_function(&Viewport::viewNode, return_value_policy<ovito_object_reference>()), &Viewport::setViewNode)
		.add_property("gridMatrix", make_function(&Viewport::gridMatrix, return_value_policy<copy_const_reference>()), &Viewport::setGridMatrix)
		.add_property("viewportTitle", make_function(&Viewport::viewportTitle, return_value_policy<copy_const_reference>()))
		.def("updateViewport", &Viewport::updateViewport)
		.def("redrawViewport", &Viewport::redrawViewport)
		.def("nonScalingSize", &Viewport::nonScalingSize)
		.def("zoomToSceneExtents", &Viewport::zoomToSceneExtents)
		.def("zoomToSelectionExtents", &Viewport::zoomToSelectionExtents)
		.def("zoomToBox", &Viewport::zoomToBox)
	;

	enum_<Viewport::ViewType>("ViewType")
		.value("NONE", Viewport::VIEW_NONE)
		.value("TOP", Viewport::VIEW_TOP)
		.value("BOTTOM", Viewport::VIEW_BOTTOM)
		.value("FRONT", Viewport::VIEW_FRONT)
		.value("BACK", Viewport::VIEW_BACK)
		.value("LEFT", Viewport::VIEW_LEFT)
		.value("RIGHT", Viewport::VIEW_RIGHT)
		.value("ORTHO", Viewport::VIEW_ORTHO)
		.value("PERSPECTIVE", Viewport::VIEW_PERSPECTIVE)
		.value("SCENENODE", Viewport::VIEW_SCENENODE)
	;

	ovito_class<ViewportConfiguration, RefTarget>()
		.add_property("activeViewport", make_function(&ViewportConfiguration::activeViewport, return_value_policy<ovito_object_reference>()), &ViewportConfiguration::setActiveViewport)
		.add_property("maximizedViewport", make_function(&ViewportConfiguration::maximizedViewport, return_value_policy<ovito_object_reference>()), &ViewportConfiguration::setMaximizedViewport)
		.def("zoomToSelectionExtents", &ViewportConfiguration::zoomToSelectionExtents)
		.def("zoomToSceneExtents", &ViewportConfiguration::zoomToSceneExtents)
		.def("updateViewports", &ViewportConfiguration::updateViewports)
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(PyScriptViewport);

};
