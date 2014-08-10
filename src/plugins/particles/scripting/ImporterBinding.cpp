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
#include <plugins/particles/importer/InputColumnMapping.h>
#include <plugins/particles/importer/ParticleImporter.h>
#include <plugins/particles/importer/cfg/CFGImporter.h>
#include <plugins/particles/importer/imd/IMDImporter.h>
#include <plugins/particles/importer/parcas/ParcasFileImporter.h>
#include <plugins/particles/importer/vasp/POSCARImporter.h>
#include <plugins/particles/importer/xyz/XYZImporter.h>
#include <plugins/particles/importer/pdb/PDBImporter.h>
#include <plugins/particles/importer/lammps/LAMMPSTextDumpImporter.h>
#include <plugins/particles/importer/lammps/LAMMPSBinaryDumpImporter.h>
#include <plugins/particles/importer/lammps/LAMMPSDataImporter.h>

namespace Particles {

using namespace boost::python;
using namespace Ovito;
using namespace PyScript;

void setupImporterBinding()
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
		.add_property("useCustomColumnMapping", &LAMMPSTextDumpImporter::useCustomColumnMapping, &LAMMPSTextDumpImporter::setUseCustomColumnMapping)
	;

	class_<LAMMPSDataImporter, bases<ParticleImporter>, OORef<LAMMPSDataImporter>, boost::noncopyable>("LAMMPSDataImporter", init<DataSet*>())
		.add_property("LAMMPSDataImporter", &LAMMPSDataImporter::atomStyle, &LAMMPSDataImporter::setAtomStyle)
	;

	enum_<LAMMPSDataImporter::LAMMPSAtomStyle>("LAMMPSAtomStyle")
		.value("Unknown", LAMMPSDataImporter::AtomStyle_Unknown)
		.value("Angle", LAMMPSDataImporter::AtomStyle_Angle)
		.value("Atomic", LAMMPSDataImporter::AtomStyle_Atomic)
		.value("Body", LAMMPSDataImporter::AtomStyle_Body)
		.value("Bond", LAMMPSDataImporter::AtomStyle_Bond)
		.value("Charge", LAMMPSDataImporter::AtomStyle_Charge)
	;

	class_<LAMMPSBinaryDumpImporter, bases<ParticleImporter>, OORef<LAMMPSBinaryDumpImporter>, boost::noncopyable>("LAMMPSBinaryDumpImporter", init<DataSet*>())
		.add_property("columnMapping", make_function(&LAMMPSBinaryDumpImporter::columnMapping, return_value_policy<copy_const_reference>()), &LAMMPSBinaryDumpImporter::setColumnMapping)
	;

	class_<CFGImporter, bases<ParticleImporter>, OORef<CFGImporter>, boost::noncopyable>("CFGImporter", init<DataSet*>())
	;

	class_<IMDImporter, bases<ParticleImporter>, OORef<IMDImporter>, boost::noncopyable>("IMDImporter", init<DataSet*>())
	;

	class_<ParcasFileImporter, bases<ParticleImporter>, OORef<ParcasFileImporter>, boost::noncopyable>("ParcasFileImporter", init<DataSet*>())
	;

	class_<PDBImporter, bases<ParticleImporter>, OORef<PDBImporter>, boost::noncopyable>("PDBImporter", init<DataSet*>())
	;

	class_<POSCARImporter, bases<ParticleImporter>, OORef<POSCARImporter>, boost::noncopyable>("POSCARImporter", init<DataSet*>())
	;

}

};
