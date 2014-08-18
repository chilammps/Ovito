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
#include <core/rendering/RenderSettings.h>
#include <core/rendering/SceneRenderer.h>
#include <core/rendering/standard/StandardSceneRenderer.h>
#include <core/rendering/noninteractive/NonInteractiveSceneRenderer.h>
#include <core/rendering/ParticlePrimitive.h>
#include <core/rendering/ArrowPrimitive.h>
#include <core/scene/display/DisplayObject.h>
#include <core/scene/display/geometry/TriMeshDisplay.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

BOOST_PYTHON_MODULE(PyScriptRendering)
{
	docstring_options docoptions(true, false);

	ovito_class<RenderSettings, RefTarget>(
			"Stores settings and parameters for rendering images and movies."
			"\n\n"
			"There exists an instance of this class which can be accessed via the :py:attr:`~ovito.app.DataSet.renderSettings` attribute of the :py:attr:`~ovito.app.DataSet`.\n"
			"This global settings object is the one the user can manipulate on the Render tab of the main window. In addition, it is possible "
			"to construct new ``RenderSettings`` instances from a script, which are passed to the :py:func:`ovito.view.Viewport.render` function."
			"\n\n"
			"Example::"
			"\n\n"
			"    settings = RenderSettings(\n"
			"        filename = 'image.png',\n"
			"        size = (1024,768),\n"
			"        backgroundColor = (0.8,0.8,1.0)\n"
			"    )\n"
			"    settings.renderer.antialiasing = False\n"
			"    dataset.viewports.activeViewport.render(settings)\n")
		.add_property("renderer", make_function(&RenderSettings::renderer, return_value_policy<ovito_object_reference>()), &RenderSettings::setRenderer,
				"The renderer that is used to generate the image or movie. Depending on the selected renderer you "
				"can use this to set additional parameters such as the anti-aliasing level."
				"\n\n"
				"See the :py:class:`~ovito.render.StandardSceneRenderer` and :py:class:`~ovito.render.tachyon.TachyonRenderer` classes "
				"for a list of renderer-specific parameters.")
		.add_property("renderingRange", &RenderSettings::renderingRangeType, &RenderSettings::setRenderingRangeType,
				"Selects the animation frames to be rendered."
				"\n\n"
				"Possible values:\n"
				"  * ``ovito.render.RenderingRange.CURRENT_FRAME`` (default): Renders a single image at the current animation time.\n"
				"  * ``ovito.render.RenderingRange.ANIMATION_INTERVAL``: Renders a movie of the entire animation sequence.\n"
				"  * ``ovito.render.RenderingRange.CUSTOM_INTERVAL``: Renders a movie of the animation interval given by the :py:attr:`.customRange` attribute.\n")
		.add_property("outputImageWidth", &RenderSettings::outputImageWidth, &RenderSettings::setOutputImageWidth)
		.add_property("outputImageHeight", &RenderSettings::outputImageHeight, &RenderSettings::setOutputImageHeight)
		.add_property("outputImageAspectRatio", &RenderSettings::outputImageAspectRatio)
		.add_property("imageFilename", make_function(&RenderSettings::imageFilename, return_value_policy<copy_const_reference>()), &RenderSettings::setImageFilename)
		.add_property("backgroundColor", &RenderSettings::backgroundColor, &RenderSettings::setBackgroundColor,
				"Controls the background color of the rendered image."
				"\n\n"
				"Default: ``(1,1,1)`` -- white")
		.add_property("generateAlphaChannel", &RenderSettings::generateAlphaChannel, &RenderSettings::setGenerateAlphaChannel,
				"When saving the generated image to a file format that stores transparency information, this option will make "
				"those parts of the output image transparent, which are not covered by an object."
				"\n\n"
				"Default: ``False``")
		.add_property("saveToFile", &RenderSettings::saveToFile, &RenderSettings::setSaveToFile)
		.add_property("skipExistingImages", &RenderSettings::skipExistingImages, &RenderSettings::setSkipExistingImages)
		.add_property("customRangeStart", &RenderSettings::customRangeStart, &RenderSettings::setCustomRangeStart)
		.add_property("customRangeEnd", &RenderSettings::customRangeEnd, &RenderSettings::setCustomRangeEnd)
		.add_property("everyNthFrame", &RenderSettings::everyNthFrame, &RenderSettings::setEveryNthFrame)
		.add_property("fileNumberBase", &RenderSettings::fileNumberBase, &RenderSettings::setFileNumberBase)
	;

	enum_<RenderSettings::RenderingRangeType>("RenderingRange")
		.value("CURRENT_FRAME", RenderSettings::CURRENT_FRAME)
		.value("ANIMATION_INTERVAL", RenderSettings::ANIMATION_INTERVAL)
		.value("CUSTOM_INTERVAL", RenderSettings::CUSTOM_INTERVAL)
	;

	ovito_abstract_class<SceneRenderer, RefTarget>()
		.add_property("isInteractive", &SceneRenderer::isInteractive)
	;

	ovito_class<StandardSceneRenderer, SceneRenderer>(
			"The standard OpenGL-based renderer."
			"\n\n"
			"This is the default built-in rendering engine that is also used by OVITO to render the contents of the interactive viewports. "
			"Since it accelerates the generation of images by using the graphics hardware, it is very fast.")
		.add_property("antialiasingLevel", &StandardSceneRenderer::antialiasingLevel, &StandardSceneRenderer::setAntialiasingLevel,
				"A positive integer controlling the level of antialiasing. If 1, no antialiasing is performed. For larger parameter values, "
				"the image in rendered at a higher resolution and scaled back to the desired output size to reduce aliasing artifacts."
				"\n\n"
				"Default: 3")
	;

	ovito_abstract_class<NonInteractiveSceneRenderer, SceneRenderer>()
	;

	ovito_abstract_class<DisplayObject, RefTarget>()
		.add_property("enabled", &DisplayObject::isEnabled, &DisplayObject::setEnabled)
	;

	ovito_class<TriMeshDisplay, DisplayObject>()
		.add_property("color", make_function(&TriMeshDisplay::color, return_value_policy<copy_const_reference>()), &TriMeshDisplay::setColor)
		.add_property("transparency", &TriMeshDisplay::transparency, &TriMeshDisplay::setTransparency)
	;

	enum_<ParticlePrimitive::ShadingMode>("ParticleShadingMode")
		.value("NormalShading", ParticlePrimitive::NormalShading)
		.value("FlatShading", ParticlePrimitive::FlatShading)
	;

	enum_<ParticlePrimitive::RenderingQuality>("ParticleRenderingQuality")
		.value("LowQuality", ParticlePrimitive::LowQuality)
		.value("MediumQuality", ParticlePrimitive::MediumQuality)
		.value("HighQuality", ParticlePrimitive::HighQuality)
		.value("AutoQuality", ParticlePrimitive::AutoQuality)
	;

	enum_<ParticlePrimitive::ParticleShape>("ParticleShape")
		.value("SphericalShape", ParticlePrimitive::SphericalShape)
		.value("SquareShape", ParticlePrimitive::SquareShape)
	;

	enum_<ArrowPrimitive::ShadingMode>("ArrowShadingMode")
		.value("NormalShading", ArrowPrimitive::NormalShading)
		.value("FlatShading", ArrowPrimitive::FlatShading)
	;

	enum_<ArrowPrimitive::RenderingQuality>("ArrowRenderingQuality")
		.value("LowQuality", ArrowPrimitive::LowQuality)
		.value("MediumQuality", ArrowPrimitive::MediumQuality)
		.value("HighQuality", ArrowPrimitive::HighQuality)
	;

	enum_<ArrowPrimitive::Shape>("ArrowShape")
		.value("CylinderShape", ArrowPrimitive::CylinderShape)
		.value("ArrowShape", ArrowPrimitive::ArrowShape)
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(PyScriptRendering);


};
