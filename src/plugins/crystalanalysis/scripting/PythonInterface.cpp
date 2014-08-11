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

namespace CrystalAnalysis {

using namespace boost::python;
using namespace Ovito;
using namespace PyScript;

BOOST_PYTHON_MODULE(CrystalAnalysis)
{
	ovito_class<ConstructSurfaceModifier, AsynchronousParticleModifier>()
		.add_property("radius", &ConstructSurfaceModifier::radius, &ConstructSurfaceModifier::setRadius)
		.add_property("smoothingLevel", &ConstructSurfaceModifier::smoothingLevel, &ConstructSurfaceModifier::setSmoothingLevel)
		.add_property("onlySelectedParticles", &ConstructSurfaceModifier::onlySelectedParticles, &ConstructSurfaceModifier::setOnlySelectedParticles)
		.add_property("solidVolume", &ConstructSurfaceModifier::solidVolume)
		.add_property("totalVolume", &ConstructSurfaceModifier::totalVolume)
		.add_property("surfaceArea", &ConstructSurfaceModifier::surfaceArea)
		.add_property("surfaceMesh", make_function(&ConstructSurfaceModifier::surfaceMesh, return_value_policy<ovito_object_reference>()))
		.add_property("surfaceMeshDisplay", make_function(&ConstructSurfaceModifier::surfaceMeshDisplay, return_value_policy<ovito_object_reference>()))
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

	ovito_class<CAImporter, LinkedFileImporter>()
		.add_property("loadParticles", &CAImporter::loadParticles, &CAImporter::setLoadParticles)
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(CrystalAnalysis);

};
