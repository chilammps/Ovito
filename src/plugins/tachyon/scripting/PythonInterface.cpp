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

namespace TachyonPlugin {

using namespace boost::python;
using namespace Ovito;
using namespace PyScript;

BOOST_PYTHON_MODULE(Tachyon)
{
	ovito_class<TachyonRenderer, NonInteractiveSceneRenderer>()
		.add_property("antialiasingEnabled", &TachyonRenderer::antialiasingEnabled, &TachyonRenderer::setAntialiasingEnabled)
		.add_property("antialiasingSamples", &TachyonRenderer::antialiasingSamples, &TachyonRenderer::setAntialiasingSamples)
		.add_property("directLightSourceEnabled", &TachyonRenderer::directLightSourceEnabled, &TachyonRenderer::setDirectLightSourceEnabled)
		.add_property("defaultLightSourceIntensity", &TachyonRenderer::defaultLightSourceIntensity, &TachyonRenderer::setDefaultLightSourceIntensity)
		.add_property("shadowsEnabled", &TachyonRenderer::shadowsEnabled, &TachyonRenderer::setShadowsEnabled)
		.add_property("ambientOcclusionEnabled", &TachyonRenderer::ambientOcclusionEnabled, &TachyonRenderer::setAmbientOcclusionEnabled)
		.add_property("ambientOcclusionBrightness", &TachyonRenderer::ambientOcclusionBrightness, &TachyonRenderer::setAmbientOcclusionBrightness)
		.add_property("ambientOcclusionSamples", &TachyonRenderer::ambientOcclusionSamples, &TachyonRenderer::setAmbientOcclusionSamples)
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(Tachyon);

};
