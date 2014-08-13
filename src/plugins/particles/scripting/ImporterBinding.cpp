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

/// Constructs an InputColumnMapping from a list of strings.
InputColumnMapping* InputColumnMapping_from_python_list(const list& list_) {
	std::unique_ptr<InputColumnMapping> mapping(new InputColumnMapping());
	mapping->resize(len(list_));
	for(int i = 0; i < mapping->size(); i++) {
		ParticlePropertyReference pref = extract<ParticlePropertyReference>(list_[i]);
		if(!pref.isNull()) {
			if(pref.type() != ParticleProperty::UserProperty)
				(*mapping)[i].mapStandardColumn(pref.type(), pref.vectorComponent());
			else
				(*mapping)[i].mapCustomColumn(pref.name(), qMetaTypeId<FloatType>(), pref.vectorComponent());
		}
	}
	return mapping.release();
}

void setupImporterBinding()
{
	class_<InputColumnMapping>("InputColumnMapping", init<>())
		.add_property("columnCount", &InputColumnMapping::size, (void (InputColumnMapping::*)(InputColumnMapping::size_type))&InputColumnMapping::resize)
		.add_property("fileExcerpt", make_function(&InputColumnMapping::fileExcerpt, return_value_policy<copy_const_reference>()), &InputColumnMapping::setFileExcerpt)
		.def("validate", &InputColumnMapping::validate)
	;

	// Install automatic Python string to ParticlePropertyReference conversion.
	auto convertible_ParticlePropertyReference = [](PyObject* obj_ptr) -> void* {
		if(!PyString_Check(obj_ptr) && obj_ptr != Py_None) return nullptr;
		return obj_ptr;
	};
	auto construct_ParticlePropertyReference = [](PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data) {
		void* storage = ((boost::python::converter::rvalue_from_python_storage<ParticlePropertyReference>*)data)->storage.bytes;
		if(obj_ptr == Py_None) {
			new (storage) ParticlePropertyReference();
			data->convertible = storage;
			return;
		}

		QStringList parts = extract<QString>(obj_ptr)().split(QChar('.'));
		if(parts.length() > 2)
			throw Exception("Too many dots in particle property name string.");
		else if(parts.length() == 0 || parts[0].isEmpty())
			throw Exception("Particle property name string is empty.");
		// Determine property type.
		QString name = parts[0];
		ParticleProperty::Type type = ParticleProperty::standardPropertyList().value(name, ParticleProperty::UserProperty);

		// Determine vector component.
		int component = -1;
		if(parts.length() == 2) {
			// First try to convert component to integer.
			bool ok;
			component = parts[1].toInt(&ok);
			if(!ok) {
				if(type == ParticleProperty::UserProperty)
					throw Exception(QString("Invalid component name or index for particle property '%1': %2").arg(parts[0]).arg(parts[1]));

				// Perhaps the name was used instead of an integer.
				const QString componentName = parts[1].toUpper();
				QStringList standardNames = ParticleProperty::standardPropertyComponentNames(type);
				component = standardNames.indexOf(componentName);
				if(component < 0)
					throw Exception(QString("Unknown component name '%1' for particle property '%2'. Possible components are: %3").arg(parts[1]).arg(parts[0]).arg(standardNames.join(',')));
			}
		}

		// Construct object.
		if(type == Particles::ParticleProperty::UserProperty)
			new (storage) ParticlePropertyReference(name, component);
		else
			new (storage) ParticlePropertyReference(type, component);
		data->convertible = storage;
	};
	converter::registry::push_back(convertible_ParticlePropertyReference, construct_ParticlePropertyReference, boost::python::type_id<ParticlePropertyReference>());

	// Install automatic Python list to InputColumnMapping conversion.
	auto convertible_InputColumnMapping = [](PyObject* obj_ptr) -> void* {
		if(!PyList_Check(obj_ptr)) return nullptr;
		return obj_ptr;
	};
	auto construct_InputColumnMapping = [](PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data) {
		void* storage = ((boost::python::converter::rvalue_from_python_storage<InputColumnMapping>*)data)->storage.bytes;
		new (storage) InputColumnMapping();
		InputColumnMapping* mapping = (InputColumnMapping*)storage;
		Py_ssize_t count = PyList_Size(obj_ptr);
		mapping->resize(count);
		for(int i = 0; i < mapping->size(); i++) {
			ParticlePropertyReference pref = extract<ParticlePropertyReference>(object(handle<>(borrowed(PyList_GetItem(obj_ptr, i)))));
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
		.add_property("multiTimestepFile", &ParticleImporter::isMultiTimestepFile, &ParticleImporter::setMultiTimestepFile)
	;

	ovito_class<XYZImporter, ParticleImporter>()
		.add_property("columnMapping", make_function(&XYZImporter::columnMapping, return_value_policy<copy_const_reference>()), &XYZImporter::setColumnMapping)
	;

	ovito_class<LAMMPSTextDumpImporter, ParticleImporter>()
		.add_property("customColumnMapping", make_function(&LAMMPSTextDumpImporter::customColumnMapping, return_value_policy<copy_const_reference>()), &LAMMPSTextDumpImporter::setCustomColumnMapping)
		.add_property("useCustomColumnMapping", &LAMMPSTextDumpImporter::useCustomColumnMapping, &LAMMPSTextDumpImporter::setUseCustomColumnMapping)
	;

	ovito_class<LAMMPSDataImporter, ParticleImporter>()
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

	ovito_class<LAMMPSBinaryDumpImporter, ParticleImporter>()
		.add_property("columnMapping", make_function(&LAMMPSBinaryDumpImporter::columnMapping, return_value_policy<copy_const_reference>()), &LAMMPSBinaryDumpImporter::setColumnMapping)
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

};
