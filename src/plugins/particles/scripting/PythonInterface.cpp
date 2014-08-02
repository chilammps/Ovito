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
#include <plugins/particles/importer/InputColumnMapping.h>
#include <plugins/particles/importer/ParticleImporter.h>
#include <plugins/particles/importer/xyz/XYZImporter.h>
#include <plugins/particles/importer/lammps/LAMMPSTextDumpImporter.h>

namespace Particles {

using namespace boost::python;
using namespace Ovito;

BOOST_PYTHON_MODULE(Particles)
{
	class_<InputColumnMapping>("InputColumnMapping", init<>())
		.add_property("columnCount", &InputColumnMapping::columnCount, &InputColumnMapping::setColumnCount)
		.add_property("fileExcerpt", make_function(&InputColumnMapping::fileExcerpt, return_value_policy<copy_const_reference>()), &InputColumnMapping::setFileExcerpt)
		.def("shrink", &InputColumnMapping::shrink)
		.def("mapCustomColumn", &InputColumnMapping::mapCustomColumn)
		.def("mapStandardColumn", &InputColumnMapping::mapStandardColumn)
		.def("unmapColumn", &InputColumnMapping::unmapColumn)
		.def("columnName", &InputColumnMapping::columnName)
		.def("setColumnName", &InputColumnMapping::setColumnName)
		.def("resetColumnNames", &InputColumnMapping::resetColumnNames)
		.def("propertyType", &InputColumnMapping::propertyType)
		.def("propertyName", &InputColumnMapping::propertyName)
		.def("dataType", &InputColumnMapping::dataType)
		.def("isMapped", &InputColumnMapping::isMapped)
		.def("vectorComponent", &InputColumnMapping::vectorComponent)
		.def("validate", &InputColumnMapping::validate)
	;

	class_<ParticleImporter, bases<FileImporter>, OORef<ParticleImporter>, boost::noncopyable>("ParticleImporter", no_init)
		.add_property("multiTimestepFile", &ParticleImporter::isMultiTimestepFile, &ParticleImporter::setMultiTimestepFile)
	;

	class_<XYZImporter, bases<ParticleImporter>, OORef<XYZImporter>, boost::noncopyable>("XYZImporter", init<DataSet*>())
		.add_property("columnMapping", make_function(&XYZImporter::columnMapping, return_value_policy<copy_const_reference>()), &XYZImporter::setColumnMapping)
	;

	class_<LAMMPSTextDumpImporter, bases<ParticleImporter>, OORef<LAMMPSTextDumpImporter>, boost::noncopyable>("LAMMPSTextDumpImporter", init<DataSet*>())
		.add_property("customColumnMapping", make_function(&LAMMPSTextDumpImporter::customColumnMapping, return_value_policy<copy_const_reference>()), &LAMMPSTextDumpImporter::setCustomColumnMapping)
	;
}

};
