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
#include <core/scene/objects/DisplayObject.h>
#include <core/scene/objects/geometry/TriMeshDisplay.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

BOOST_PYTHON_MODULE(PyScriptRendering)
{
	docstring_options docoptions(true, false);

	{
		scope s = ovito_class<RenderSettings, RefTarget>(
				"Stores settings and parameters for rendering images and movies."
				"\n\n"
				"A instance of this class can be passed to the :py:func:`~Viewport.render` function "
				"of the :py:class:`Viewport` class to control various aspects such as the resolution of the generated image. "
				"The ``RenderSettings`` object contains a :py:attr:`.renderer`, which is the rendering engine "
				"that will be used to generate images of the three-dimensional scene. OVITO comes with two different "
				"rendering engines:"
				"\n\n"
				"  * :py:class:`OpenGLRenderer` -- An OpenGL-based renderer, which is also used for the interactive display in OVITO's viewports.\n"
				"  * :py:class:`TachyonRenderer` -- A software-based, high-quality raytracing renderer.\n"
				"\n"
				"Usage example::"
				"\n\n"
				"    rs = RenderSettings(\n"
				"        filename = 'image.png',\n"
				"        size = (1024,768),\n"
				"        background_color = (0.8,0.8,1.0)\n"
				"    )\n"
				"    rs.renderer.antialiasing = False\n"
				"    dataset.viewports.active_vp.render(rs)\n")
			.add_property("renderer", make_function(&RenderSettings::renderer, return_value_policy<ovito_object_reference>()), &RenderSettings::setRenderer,
					"The renderer that is used to generate the image or movie. Depending on the selected renderer you "
					"can use this to set additional parameters such as the anti-aliasing level."
					"\n\n"
					"See the :py:class:`OpenGLRenderer` and :py:class:`TachyonRenderer` classes "
					"for a list of renderer-specific parameters.")
			.add_property("range", &RenderSettings::renderingRangeType, &RenderSettings::setRenderingRangeType,
					"Selects the animation frames to be rendered."
					"\n\n"
					"Possible values:\n"
					"  * ``RenderSettings.Range.CURRENT_FRAME`` (default): Renders a single image at the current animation time.\n"
					"  * ``RenderSettings.Range.ANIMATION``: Renders a movie of the entire animation sequence.\n"
					"  * ``RenderSettings.Range.CUSTOM_INTERVAL``: Renders a movie of the animation interval given by the :py:attr:`.custom_range` attribute.\n")
			.add_property("outputImageWidth", &RenderSettings::outputImageWidth, &RenderSettings::setOutputImageWidth)
			.add_property("outputImageHeight", &RenderSettings::outputImageHeight, &RenderSettings::setOutputImageHeight)
			.add_property("outputImageAspectRatio", &RenderSettings::outputImageAspectRatio)
			.add_property("imageFilename", make_function(&RenderSettings::imageFilename, return_value_policy<copy_const_reference>()), &RenderSettings::setImageFilename)
			.add_property("background_color", &RenderSettings::backgroundColor, &RenderSettings::setBackgroundColor,
					"Controls the background color of the rendered image."
					"\n\n"
					":Default: ``(1,1,1)`` -- white")
			.add_property("generate_alpha", &RenderSettings::generateAlphaChannel, &RenderSettings::setGenerateAlphaChannel,
					"When saving the generated image to a file format that can store transparency information (e.g. PNG), this option will make "
					"those parts of the output image transparent that are not covered by an object."
					"\n\n"
					":Default: ``False``")
			.add_property("saveToFile", &RenderSettings::saveToFile, &RenderSettings::setSaveToFile)
			.add_property("skipExistingImages", &RenderSettings::skipExistingImages, &RenderSettings::setSkipExistingImages)
			.add_property("customRangeStart", &RenderSettings::customRangeStart, &RenderSettings::setCustomRangeStart)
			.add_property("customRangeEnd", &RenderSettings::customRangeEnd, &RenderSettings::setCustomRangeEnd)
			.add_property("everyNthFrame", &RenderSettings::everyNthFrame, &RenderSettings::setEveryNthFrame)
			.add_property("fileNumberBase", &RenderSettings::fileNumberBase, &RenderSettings::setFileNumberBase)
		;

		enum_<RenderSettings::RenderingRangeType>("Range")
			.value("CURRENT_FRAME", RenderSettings::CURRENT_FRAME)
			.value("ANIMATION", RenderSettings::ANIMATION_INTERVAL)
			.value("CUSTOM_INTERVAL", RenderSettings::CUSTOM_INTERVAL)
		;
	}

	ovito_abstract_class<SceneRenderer, RefTarget>()
		.add_property("isInteractive", &SceneRenderer::isInteractive)
	;

	ovito_class<StandardSceneRenderer, SceneRenderer>(
			"The standard OpenGL-based renderer."
			"\n\n"
			"This is the default built-in rendering engine that is also used by OVITO to render the contents of the interactive viewports. "
			"Since it accelerates the generation of images by using the computer's graphics hardware, it is very fast.",
			"OpenGLRenderer")
		.add_property("antialiasing_level", &StandardSceneRenderer::antialiasingLevel, &StandardSceneRenderer::setAntialiasingLevel,
				"A positive integer controlling the level of supersampling. If 1, no supersampling is performed. For larger values, "
				"the image in rendered at a higher resolution and then scaled back to the output size to reduce aliasing artifacts."
				"\n\n"
				":Default: 3")
	;

	ovito_abstract_class<NonInteractiveSceneRenderer, SceneRenderer>()
	;

	ovito_abstract_class<DisplayObject, RefTarget>(
			"Abstract base class for display setting objects that control the visual appearance of data. "
			":py:class:`DataObjects <ovito.data.DataObject>` may be associated with an instance of this class, which can be accessed via "
			"their :py:attr:`~ovito.data.DataObject.display` property.",
			// Python class name:
			"Display")
		.add_property("enabled", &DisplayObject::isEnabled, &DisplayObject::setEnabled,
				"Boolean flag controlling the visibility of the data. If set to ``False``, the "
				"data will not be visible in the viewports or in rendered images."
				"\n\n"
				":Default: ``True``\n")
	;

	ovito_class<TriMeshDisplay, DisplayObject>()
		.add_property("color", make_function(&TriMeshDisplay::color, return_value_policy<copy_const_reference>()), &TriMeshDisplay::setColor)
		.add_property("transparency", &TriMeshDisplay::transparency, &TriMeshDisplay::setTransparency)
	;

	enum_<ParticlePrimitive::ShadingMode>("ParticleShadingMode")
		.value("Normal", ParticlePrimitive::NormalShading)
		.value("Flat", ParticlePrimitive::FlatShading)
	;

	enum_<ParticlePrimitive::RenderingQuality>("ParticleRenderingQuality")
		.value("LowQuality", ParticlePrimitive::LowQuality)
		.value("MediumQuality", ParticlePrimitive::MediumQuality)
		.value("HighQuality", ParticlePrimitive::HighQuality)
		.value("AutoQuality", ParticlePrimitive::AutoQuality)
	;

	enum_<ParticlePrimitive::ParticleShape>("ParticleShape")
		.value("Spherical", ParticlePrimitive::SphericalShape)
		.value("Square", ParticlePrimitive::SquareShape)
	;

	enum_<ArrowPrimitive::ShadingMode>("ArrowShadingMode")
		.value("Normal", ArrowPrimitive::NormalShading)
		.value("Flat", ArrowPrimitive::FlatShading)
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
