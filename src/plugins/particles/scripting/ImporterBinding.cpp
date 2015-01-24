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

#include <plugins/particles/Particles.h>
#include <plugins/pyscript/binding/PythonBinding.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include <plugins/particles/objects/ParticleTypeProperty.h>
#include <plugins/particles/objects/SimulationCellObject.h>
#include <plugins/particles/import/InputColumnMapping.h>
#include <plugins/particles/import/ParticleImporter.h>
#include <plugins/particles/import/cfg/CFGImporter.h>
#include <plugins/particles/import/imd/IMDImporter.h>
#include <plugins/particles/import/parcas/ParcasFileImporter.h>
#include <plugins/particles/import/vasp/POSCARImporter.h>
#include <plugins/particles/import/xyz/XYZImporter.h>
#include <plugins/particles/import/pdb/PDBImporter.h>
#include <plugins/particles/import/lammps/LAMMPSTextDumpImporter.h>
#include <plugins/particles/import/lammps/LAMMPSBinaryDumpImporter.h>
#include <plugins/particles/import/lammps/LAMMPSDataImporter.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Internal)

using namespace boost::python;
using namespace PyScript;

BOOST_PYTHON_MODULE(ParticlesImporter)
{
	docstring_options docoptions(true, false);

	class_<InputColumnMapping>("InputColumnMapping", init<>())
		.add_property("fileExcerpt", make_function(&InputColumnMapping::fileExcerpt, return_value_policy<copy_const_reference>()), &InputColumnMapping::setFileExcerpt)
		.def("validate", &InputColumnMapping::validate)
	;

	// Install automatic Python sequence to InputColumnMapping conversion.
	auto convertible_InputColumnMapping = [](PyObject* obj_ptr) -> void* {
		if(!PySequence_Check(obj_ptr)) return nullptr;
		return obj_ptr;
	};
	auto construct_InputColumnMapping = [](PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data) {
		void* storage = ((boost::python::converter::rvalue_from_python_storage<InputColumnMapping>*)data)->storage.bytes;
		new (storage) InputColumnMapping();
		InputColumnMapping* mapping = (InputColumnMapping*)storage;
		mapping->resize(PySequence_Size(obj_ptr));
		for(size_t i = 0; i < mapping->size(); i++) {
			ParticlePropertyReference pref = extract<ParticlePropertyReference>(object(handle<>(PySequence_GetItem(obj_ptr, i))));
			if(!pref.isNull()) {
				if(pref.type() != ParticleProperty::UserProperty)
					(*mapping)[i].mapStandardColumn(pref.type(), pref.vectorComponent());
				else
					(*mapping)[i].mapCustomColumn(pref.name(), qMetaTypeId<FloatType>(), pref.vectorComponent());
			}
		}
		data->convertible = storage;
	};
	converter::registry::push_back(convertible_InputColumnMapping, construct_InputColumnMapping, boost::python::type_id<InputColumnMapping>());

	ovito_abstract_class<ParticleImporter, FileImporter>()
		.add_property("multiple_frames", &ParticleImporter::isMultiTimestepFile, &ParticleImporter::setMultiTimestepFile)
	;

	ovito_class<XYZImporter, ParticleImporter>()
		.add_property("columns", make_function(&XYZImporter::columnMapping, return_value_policy<copy_const_reference>()), &XYZImporter::setColumnMapping)
	;

	ovito_class<LAMMPSTextDumpImporter, ParticleImporter>()
		.add_property("customColumnMapping", make_function(&LAMMPSTextDumpImporter::customColumnMapping, return_value_policy<copy_const_reference>()), &LAMMPSTextDumpImporter::setCustomColumnMapping)
		.add_property("useCustomColumnMapping", &LAMMPSTextDumpImporter::useCustomColumnMapping, &LAMMPSTextDumpImporter::setUseCustomColumnMapping)
	;

	{
		scope s = ovito_class<LAMMPSDataImporter, ParticleImporter>()
			.add_property("atomStyle", &LAMMPSDataImporter::atomStyle, &LAMMPSDataImporter::setAtomStyle)
		;

		enum_<LAMMPSDataImporter::LAMMPSAtomStyle>("LAMMPSAtomStyle")
			.value("Unknown", LAMMPSDataImporter::AtomStyle_Unknown)
			.value("Angle", LAMMPSDataImporter::AtomStyle_Angle)
			.value("Atomic", LAMMPSDataImporter::AtomStyle_Atomic)
			.value("Body", LAMMPSDataImporter::AtomStyle_Body)
			.value("Bond", LAMMPSDataImporter::AtomStyle_Bond)
			.value("Charge", LAMMPSDataImporter::AtomStyle_Charge)
		;
	}

	ovito_class<LAMMPSBinaryDumpImporter, ParticleImporter>()
		.add_property("columns", make_function(&LAMMPSBinaryDumpImporter::columnMapping, return_value_policy<copy_const_reference>()), &LAMMPSBinaryDumpImporter::setColumnMapping)
	;

	ovito_class<CFGImporter, ParticleImporter>()
	;

	ovito_class<IMDImporter, ParticleImporter>()
	;

	ovito_class<ParcasFileImporter, ParticleImporter>()
	;

	ovito_class<PDBImporter, ParticleImporter>()
	;

	ovito_class<POSCARImporter, ParticleImporter>()
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(ParticlesImporter);

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
