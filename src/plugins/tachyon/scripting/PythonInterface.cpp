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
#include <plugins/pyscript/binding/PythonBinding.h>
#include <plugins/tachyon/renderer/TachyonRenderer.h>

namespace Ovito { namespace Tachyon { OVITO_BEGIN_INLINE_NAMESPACE(Internal)

using namespace boost::python;
using namespace Ovito;
using namespace PyScript;

BOOST_PYTHON_MODULE(Tachyon)
{
	docstring_options docoptions(true, false);

	ovito_class<TachyonRenderer, NonInteractiveSceneRenderer>(
			"This is the software-based raytracing renderer of OVITO."
			"\n\n"
			"It can render scenes with ambient occlusion lighting and semi-transparent objects.")
		.add_property("antialiasing", &TachyonRenderer::antialiasingEnabled, &TachyonRenderer::setAntialiasingEnabled,
				"Enables supersampling to reduce aliasing effects."
				"\n\n"
				"Default: ``True``")
		.add_property("antialiasing_samples", &TachyonRenderer::antialiasingSamples, &TachyonRenderer::setAntialiasingSamples,
				"The number of supersampling rays to generate per pixel to reduce aliasing effects."
				"\n\n"
				"Default: 12")
		.add_property("direct_light", &TachyonRenderer::directLightSourceEnabled, &TachyonRenderer::setDirectLightSourceEnabled,
				"Enables the parallel light source, which is positioned at an angle behind the camera."
				"\n\n"
				"Default: ``True``")
		.add_property("direct_light_intensity", &TachyonRenderer::defaultLightSourceIntensity, &TachyonRenderer::setDefaultLightSourceIntensity,
				"Controls the brightness of the directional light source."
				"\n\n"
				"Default: 0.9")
		.add_property("shadows", &TachyonRenderer::shadowsEnabled, &TachyonRenderer::setShadowsEnabled,
				"Enables cast shadows for the directional light source."
				"\n\n"
				"Default: ``True``")
		.add_property("ambient_occlusion", &TachyonRenderer::ambientOcclusionEnabled, &TachyonRenderer::setAmbientOcclusionEnabled,
				"Enables ambient occlusion shading. Enabling this lighting technique mimics some of the effects that occur "
				"under conditions of omnidirectional diffuse illumination, e.g. outdoors on an overcast day."
				"\n\n"
				"Default: ``True``")
		.add_property("ambient_occlusion_brightness", &TachyonRenderer::ambientOcclusionBrightness, &TachyonRenderer::setAmbientOcclusionBrightness,
				"Controls the brightness of the sky light source used for ambient occlusion."
				"\n\n"
				"Default: 0.8")
		.add_property("ambient_occlusion_samples", &TachyonRenderer::ambientOcclusionSamples, &TachyonRenderer::setAmbientOcclusionSamples,
				"Ambient occlusion is implemented using a Monte Carlo technique. This parameters controls the number of samples to compute. "
				"A higher sample count leads to a more even shading, but requires more computation time."
				"\n\n"
				"Default: 12")
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(Tachyon);

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
