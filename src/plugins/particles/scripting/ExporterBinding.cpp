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
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/ParticlePropertyObject.h>
#include <plugins/particles/data/ParticleTypeProperty.h>
#include <plugins/particles/data/SimulationCell.h>
#include <plugins/particles/exporter/ParticleExporter.h>
#include <plugins/particles/exporter/imd/IMDExporter.h>
#include <plugins/particles/exporter/vasp/POSCARExporter.h>
#include <plugins/particles/exporter/xyz/XYZExporter.h>
#include <plugins/particles/exporter/lammps/LAMMPSDumpExporter.h>
#include <plugins/particles/exporter/lammps/LAMMPSDataExporter.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

namespace Particles {

using namespace boost::python;
using namespace Ovito;
using namespace PyScript;

BOOST_PYTHON_MODULE(ParticlesExporter)
{
	class_<OutputColumnMapping>("OutputColumnMapping", init<>())
		.def(vector_indexing_suite<OutputColumnMapping>())
	;
	python_to_container_conversion<OutputColumnMapping>();

	ovito_abstract_class<ParticleExporter, FileExporter>()
		.add_property("outputFilename", make_function(&ParticleExporter::outputFilename, return_value_policy<copy_const_reference>()), &ParticleExporter::setOutputFilename)
		.add_property("exportAnimation", &ParticleExporter::exportAnimation, &ParticleExporter::setExportAnimation)
		.add_property("useWildcardFilename", &ParticleExporter::useWildcardFilename, &ParticleExporter::setUseWildcardFilename)
		.add_property("wildcardFilename", make_function(&ParticleExporter::wildcardFilename, return_value_policy<copy_const_reference>()), &ParticleExporter::setWildcardFilename)
		.add_property("startFrame", &ParticleExporter::startFrame, &ParticleExporter::setStartFrame)
		.add_property("endFrame", &ParticleExporter::endFrame, &ParticleExporter::setEndFrame)
		.add_property("everyNthFrame", &ParticleExporter::everyNthFrame, &ParticleExporter::setEveryNthFrame)
	;

	ovito_class<IMDExporter, ParticleExporter>()
	;

	ovito_class<POSCARExporter, ParticleExporter>()
	;

	ovito_class<LAMMPSDataExporter, ParticleExporter>()
	;

	ovito_class<LAMMPSDumpExporter, ParticleExporter>()
		.add_property("columnMapping", make_function(&LAMMPSDumpExporter::columnMapping, return_value_policy<copy_const_reference>()), &LAMMPSDumpExporter::setColumnMapping)
	;

	ovito_class<XYZExporter, ParticleExporter>()
		.add_property("columnMapping", make_function(&XYZExporter::columnMapping, return_value_policy<copy_const_reference>()), &XYZExporter::setColumnMapping)
		.add_property("subFormat", &XYZExporter::subFormat, &XYZExporter::setSubFormat)
	;

	enum_<XYZExporter::XYZSubFormat>("XYZSubFormat")
		.value("Parcas", XYZExporter::ParcasFormat)
		.value("Extended", XYZExporter::ExtendedFormat)
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(ParticlesExporter);

};
