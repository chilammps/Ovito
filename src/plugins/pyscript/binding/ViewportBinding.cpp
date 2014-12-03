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
#include <core/viewport/overlay/ViewportOverlay.h>
#include <core/viewport/overlay/CoordinateTripodOverlay.h>
#include <core/scene/SceneNode.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

BOOST_PYTHON_MODULE(PyScriptViewport)
{
	docstring_options docoptions(true, false);

	{
		scope s = ovito_class<Viewport, RefTarget>(
				"A viewport defines the view on the three-dimensional scene. "
				"\n\n"
				"You can create an instance of this class to define a camera position from which "
				"a picture of the three-dimensional scene should be generated. After the camera "
				"has been set up, you can render an image or movie using the viewport's "
				":py:meth:`.render` method::"
				"\n\n"
				"    vp = Viewport()\n"
				"    vp.type = Viewport.Type.PERSPECTIVE\n"
				"    vp.camera_pos = (100, 50, 50)\n"
				"    vp.camera_dir = (-100, -50, -50)\n"
				"\n"
				"    rs = RenderSettings(size=(800,600), filename=\"image.png\")\n"
				"    vp.render(rs)\n"
				"\n"
				"Note that the four interactive viewports in OVITO's main window are instances of this class. If you want to "
				"manipulate these existing viewports, you can access them through the "
				":py:attr:`DataSet.viewports <ovito.DataSet.viewports>` attribute.")
			.add_property("isRendering", &Viewport::isRendering)
			.add_property("isPerspective", &Viewport::isPerspectiveProjection)
            .add_property("type", &Viewport::viewType, static_cast<void (*)(Viewport&,Viewport::ViewType)>
                          ([](Viewport& vp, Viewport::ViewType vt) { vp.setViewType(vt); }),
					"The type of projection:"
					"\n\n"
					"  * ``Viewport.Type.PERSPECTIVE``\n"
					"  * ``Viewport.Type.ORTHO``\n"
					"  * ``Viewport.Type.TOP``\n"
					"  * ``Viewport.Type.BOTTOM``\n"
					"  * ``Viewport.Type.FRONT``\n"
					"  * ``Viewport.Type.BACK``\n"
					"  * ``Viewport.Type.LEFT``\n"
					"  * ``Viewport.Type.RIGHT``\n"
					"  * ``Viewport.Type.NONE``\n"
					"\n"
					"The first two types (``PERSPECTIVE`` and ``ORTHO``) allow you to set up custom views with arbitrary camera orientation.\n")
			.add_property("fov", &Viewport::fieldOfView, &Viewport::setFieldOfView,
					"The field of view of the viewport's camera. "
					"For perspective projections this is the camera's angle in the vertical direction (in radians). For orthogonal projections this is the visible range in the vertical direction (in world units).")
			.add_property("cameraTransformation", make_function(&Viewport::cameraTransformation, return_value_policy<copy_const_reference>()), &Viewport::setCameraTransformation)
			.add_property("camera_dir", &Viewport::cameraDirection, &Viewport::setCameraDirection,
					"The viewing direction vector of the viewport's camera. This can be an arbitrary vector with non-zero length.")
			.add_property("camera_pos", &Viewport::cameraPosition, &Viewport::setCameraPosition,
					"\nThe position of the viewport's camera. For example, to move the camera of the active viewport in OVITO's main window to a new location in space::"
					"\n\n"
					"    dataset.viewports.active_vp.camera_pos = (100, 80, -30)\n"
					"\n\n")
			.add_property("viewMatrix", make_function(&Viewport::viewMatrix, return_value_policy<copy_const_reference>()))
			.add_property("inverseViewMatrix", make_function(&Viewport::inverseViewMatrix, return_value_policy<copy_const_reference>()))
			.add_property("projectionMatrix", make_function(&Viewport::projectionMatrix, return_value_policy<copy_const_reference>()))
			.add_property("inverseProjectionMatrix", make_function(&Viewport::inverseProjectionMatrix, return_value_policy<copy_const_reference>()))
			.add_property("renderPreviewMode", &Viewport::renderPreviewMode, &Viewport::setRenderPreviewMode)
			.add_property("gridVisible", &Viewport::isGridVisible, &Viewport::setGridVisible)
			.add_property("viewNode", make_function(&Viewport::viewNode, return_value_policy<ovito_object_reference>()), &Viewport::setViewNode)
			.add_property("gridMatrix", make_function(&Viewport::gridMatrix, return_value_policy<copy_const_reference>()), &Viewport::setGridMatrix)
			.add_property("title", make_function(&Viewport::viewportTitle, return_value_policy<copy_const_reference>()),
					"The title string of the viewport shown in its top left corner (read-only).")
			.def("updateViewport", &Viewport::updateViewport)
			.def("redrawViewport", &Viewport::redrawViewport)
			.def("nonScalingSize", &Viewport::nonScalingSize)
			.def("zoom_all", &Viewport::zoomToSceneExtents,
					"Repositions the viewport camera such that all objects in the scene become completely visible. "
					"The camera direction is not changed.")
			.def("zoomToSelectionExtents", &Viewport::zoomToSelectionExtents)
			.def("zoomToBox", &Viewport::zoomToBox)
			.add_property("overlays", make_function(&Viewport::overlays, return_internal_reference<>()))
		;

		enum_<Viewport::ViewType>("Type")
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
	}

	class_<ViewProjectionParameters>("ViewProjectionParameters")
		.def_readwrite("aspectRatio", &ViewProjectionParameters::aspectRatio)
		.def_readwrite("isPerspective", &ViewProjectionParameters::isPerspective)
		.def_readwrite("znear", &ViewProjectionParameters::znear)
		.def_readwrite("zfar", &ViewProjectionParameters::zfar)
		.def_readwrite("fieldOfView", &ViewProjectionParameters::fieldOfView)
		.def_readwrite("viewMatrix", &ViewProjectionParameters::viewMatrix)
		.def_readwrite("inverseViewMatrix", &ViewProjectionParameters::inverseViewMatrix)
		.def_readwrite("projectionMatrix", &ViewProjectionParameters::projectionMatrix)
		.def_readwrite("inverseProjectionMatrix", &ViewProjectionParameters::inverseProjectionMatrix)
	;

	ovito_class<ViewportConfiguration, RefTarget>(
			"Manages the viewports in OVITO's main window."
			"\n\n"
			"This list-like object can be accessed through the :py:attr:`~ovito.DataSet.viewports` attribute of the :py:attr:`~ovito.DataSet` class. "
			"It contains all viewports in OVITO's main window::"
			"\n\n"
			"    for viewport in dataset.viewports:\n"
			"        print viewport.title\n"
			"\n"
			"By default OVITO creates four predefined :py:class:`Viewport` instances. Note that in the current program version it is not possible to add or remove "
			"viewports from the main window. "
			"The ``ViewportConfiguration`` object also manages the :py:attr:`active <.active_vp>` and the :py:attr:`maximized <.maximized_vp>` viewport.")
		.add_property("active_vp", make_function(&ViewportConfiguration::activeViewport, return_value_policy<ovito_object_reference>()), &ViewportConfiguration::setActiveViewport,
				"The viewport that is currently active. It is marked with a colored border in OVITO's main window.")
		.add_property("maximized_vp", make_function(&ViewportConfiguration::maximizedViewport, return_value_policy<ovito_object_reference>()), &ViewportConfiguration::setMaximizedViewport,
				"The viewport that is currently maximized; or ``None`` if no viewport is maximized.\n"
				"Assign a viewport to this attribute to maximize it, e.g.::"
				"\n\n"
				"    dataset.viewports.maximized_vp = dataset.viewports.active_vp\n")
		.def("zoomToSelectionExtents", &ViewportConfiguration::zoomToSelectionExtents)
		.def("zoomToSceneExtents", &ViewportConfiguration::zoomToSceneExtents)
		.def("updateViewports", &ViewportConfiguration::updateViewports)
		.add_property("viewports", make_function(&ViewportConfiguration::viewports, return_internal_reference<>()))
	;

	ovito_abstract_class<ViewportOverlay, RefTarget>();

	ovito_class<CoordinateTripodOverlay, ViewportOverlay>();
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(PyScriptViewport);

};
