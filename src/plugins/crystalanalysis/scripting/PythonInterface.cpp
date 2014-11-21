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

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <plugins/crystalanalysis/modifier/ConstructSurfaceModifier.h>
#include <plugins/crystalanalysis/modifier/ShiftModifier.h>
#include <plugins/crystalanalysis/modifier/SmoothDislocationsModifier.h>
#include <plugins/crystalanalysis/modifier/SmoothSurfaceModifier.h>
#include <plugins/crystalanalysis/importer/CAImporter.h>
#include <plugins/pyscript/binding/PythonBinding.h>

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

using namespace boost::python;
using namespace PyScript;

BOOST_PYTHON_MODULE(CrystalAnalysis)
{
	docstring_options docoptions(true, false);

	ovito_class<ConstructSurfaceModifier, AsynchronousParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Constructs the geometric surface from a set of point-like particles.")
		.add_property("radius", &ConstructSurfaceModifier::radius, &ConstructSurfaceModifier::setRadius,
				"The radius of the probe sphere used in the surface construction algorithm."
				"\n\n"
				":Default: 4.0\n")
		.add_property("smoothing_level", &ConstructSurfaceModifier::smoothingLevel, &ConstructSurfaceModifier::setSmoothingLevel,
				"The number of iterations of the smoothing algorithm to perform."
				"\n\n"
				":Default: 8\n")
		.add_property("only_selected", &ConstructSurfaceModifier::onlySelectedParticles, &ConstructSurfaceModifier::setOnlySelectedParticles,
				"If ``True``, the modifier acts only on selected particles and ignores other particles; "
				"if ``False``, the modifier constructs the surface around all particles."
				"\n\n"
				":Default: ``False``\n")
		.add_property("solid_volume", &ConstructSurfaceModifier::solidVolume,
				"After the modifier has computed the surface, this field contains the volume of the solid region enclosed "
				"by the surface."
				"\n\n"
				"Note that this value is only available after the modifier has computed its results. "
				"Thus, you have to call :py:meth:`ovito.ObjectNode.compute` first to ensure that this information is up to date. ")
		.add_property("total_volume", &ConstructSurfaceModifier::totalVolume,
				"After the modifier has computed the surface, this field contains the volume of the simulation cell.")
		.add_property("surface_area", &ConstructSurfaceModifier::surfaceArea,
				"After the modifier has computed the surface, this field contains the area of the surface."
				"\n\n"
				"Note that this value is only available after the modifier has computed its results. "
				"Thus, you have to call :py:meth:`ovito.ObjectNode.compute` first to ensure that this information is up to date. ")
		.add_property("surfaceMesh", make_function(&ConstructSurfaceModifier::surfaceMesh, return_value_policy<ovito_object_reference>()))
		.add_property("mesh_display", make_function(&ConstructSurfaceModifier::surfaceMeshDisplay, return_value_policy<ovito_object_reference>()),
				"A :py:class:`~ovito.vis.SurfaceMeshDisplay` instance controlling the visual representation of the computed surface.\n")
	;

	ovito_class<ShiftModifier, Modifier>()
	;

	ovito_class<SmoothDislocationsModifier, Modifier>()
		.add_property("smoothingEnabled", &SmoothDislocationsModifier::smoothingEnabled, &SmoothDislocationsModifier::setSmoothingEnabled)
		.add_property("smoothingLevel", &SmoothDislocationsModifier::smoothingLevel, &SmoothDislocationsModifier::setSmoothingLevel)
		.add_property("coarseningEnabled", &SmoothDislocationsModifier::coarseningEnabled, &SmoothDislocationsModifier::setCoarseningEnabled)
		.add_property("linePointInterval", &SmoothDislocationsModifier::linePointInterval, &SmoothDislocationsModifier::setLinePointInterval)
	;

	ovito_class<SmoothSurfaceModifier, Modifier>()
		.add_property("smoothingLevel", &SmoothSurfaceModifier::smoothingLevel, &SmoothSurfaceModifier::setSmoothingLevel)
	;

	ovito_class<CAImporter, FileSourceImporter>()
		.add_property("loadParticles", &CAImporter::loadParticles, &CAImporter::setLoadParticles)
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(CrystalAnalysis);

}}}	// End of namespace
