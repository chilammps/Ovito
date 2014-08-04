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
#include <plugins/particles/data/ParticleDisplay.h>
#include <plugins/particles/data/VectorDisplay.h>
#include <plugins/particles/data/SimulationCellDisplay.h>
#include <plugins/particles/data/SurfaceMeshDisplay.h>
#include <plugins/particles/data/BondsDisplay.h>
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
#include <plugins/particles/modifier/ParticleModifier.h>
#include <plugins/particles/modifier/AsynchronousParticleModifier.h>

namespace Particles {

using namespace boost::python;
using namespace Ovito;
using namespace PyScript;

BOOST_PYTHON_MODULE(Particles)
{
	enum_<ParticleProperty::Type>("ParticlePropertyType")
		.value("UserProperty", ParticleProperty::UserProperty)
		.value("ParticleTypeProperty", ParticleProperty::ParticleTypeProperty)
		.value("PositionProperty", ParticleProperty::PositionProperty)
		.value("SelectionProperty", ParticleProperty::SelectionProperty)
		.value("ColorProperty", ParticleProperty::ColorProperty)
		.value("DisplacementProperty", ParticleProperty::DisplacementProperty)
		.value("DisplacementMagnitudeProperty", ParticleProperty::DisplacementMagnitudeProperty)
		.value("PotentialEnergyProperty", ParticleProperty::PotentialEnergyProperty)
		.value("KineticEnergyProperty", ParticleProperty::KineticEnergyProperty)
		.value("TotalEnergyProperty", ParticleProperty::TotalEnergyProperty)
		.value("VelocityProperty", ParticleProperty::VelocityProperty)
		.value("RadiusProperty", ParticleProperty::RadiusProperty)
		.value("ClusterProperty", ParticleProperty::ClusterProperty)
		.value("CoordinationProperty", ParticleProperty::CoordinationProperty)
		.value("StructureTypeProperty", ParticleProperty::StructureTypeProperty)
		.value("IdentifierProperty", ParticleProperty::IdentifierProperty)
		.value("StressTensorProperty", ParticleProperty::StressTensorProperty)
		.value("StrainTensorProperty", ParticleProperty::StrainTensorProperty)
		.value("DeformationGradientProperty", ParticleProperty::DeformationGradientProperty)
		.value("OrientationProperty", ParticleProperty::OrientationProperty)
		.value("ForceProperty", ParticleProperty::ForceProperty)
		.value("MassProperty", ParticleProperty::MassProperty)
		.value("ChargeProperty", ParticleProperty::ChargeProperty)
		.value("PeriodicImageProperty", ParticleProperty::PeriodicImageProperty)
		.value("TransparencyProperty", ParticleProperty::TransparencyProperty)
		.value("DipoleOrientationProperty", ParticleProperty::DipoleOrientationProperty)
		.value("DipoleMagnitudeProperty", ParticleProperty::DipoleMagnitudeProperty)
		.value("AngularVelocityProperty", ParticleProperty::AngularVelocityProperty)
		.value("AngularMomentumProperty", ParticleProperty::AngularMomentumProperty)
		.value("TorqueProperty", ParticleProperty::TorqueProperty)
		.value("SpinProperty", ParticleProperty::SpinProperty)
		.value("CentroSymmetryProperty", ParticleProperty::CentroSymmetryProperty)
		.value("VelocityMagnitudeProperty", ParticleProperty::VelocityMagnitudeProperty)
		.value("NonaffineSquaredDisplacementProperty", ParticleProperty::NonaffineSquaredDisplacementProperty)
		.value("MoleculeProperty", ParticleProperty::MoleculeProperty)
	;

	class_<ParticlePropertyObject, bases<SceneObject>, OORef<ParticlePropertyObject>, boost::noncopyable>("ParticlePropertyObject", init<DataSet*>())
		.def("createUserProperty", &ParticlePropertyObject::createUserProperty)
		.def("createStandardProperty", &ParticlePropertyObject::createStandardProperty)
		.def("findInState", make_function((ParticlePropertyObject* (*)(const PipelineFlowState&, ParticleProperty::Type))&ParticlePropertyObject::findInState, return_value_policy<ovito_object_reference>()))
		.def("findInState", make_function((ParticlePropertyObject* (*)(const PipelineFlowState&, const QString&))&ParticlePropertyObject::findInState, return_value_policy<ovito_object_reference>()))
		.staticmethod("createUserProperty")
		.staticmethod("createStandardProperty")
		.staticmethod("findInState")
		.def("changed", &ParticlePropertyObject::changed)
		.def("nameWithComponent", &ParticlePropertyObject::nameWithComponent)
		.add_property("name", make_function(&ParticlePropertyObject::name, return_value_policy<copy_const_reference>()), &ParticlePropertyObject::setName)
		.add_property("size", &ParticlePropertyObject::size, &ParticlePropertyObject::resize)
		.add_property("type", &ParticlePropertyObject::type, &ParticlePropertyObject::setType)
		.add_property("dataType", &ParticlePropertyObject::dataType)
		.add_property("dataTypeSize", &ParticlePropertyObject::dataTypeSize)
		.add_property("perParticleSize", &ParticlePropertyObject::perParticleSize)
		.add_property("componentCount", &ParticlePropertyObject::componentCount)
	;

	class_<ParticleTypeProperty, bases<ParticlePropertyObject>, OORef<ParticleTypeProperty>, boost::noncopyable>("ParticleTypeProperty", init<DataSet*>())
		.def("insertParticleType", &ParticleTypeProperty::insertParticleType)
		.def("particleType", make_function((ParticleType* (ParticleTypeProperty::*)(int) const)&ParticleTypeProperty::particleType, return_value_policy<ovito_object_reference>()))
		.def("particleType", make_function((ParticleType* (ParticleTypeProperty::*)(const QString&) const)&ParticleTypeProperty::particleType, return_value_policy<ovito_object_reference>()))
		.def("removeParticleType", &ParticleTypeProperty::removeParticleType)
		.def("clearParticleTypes", &ParticleTypeProperty::clearParticleTypes)
		.add_property("particleTypes", make_function(&ParticleTypeProperty::particleTypes, return_internal_reference<>()))
		.def("getDefaultParticleColorFromId", &ParticleTypeProperty::getDefaultParticleColorFromId)
		.def("getDefaultParticleColorFromName", &ParticleTypeProperty::getDefaultParticleColorFromName)
		.staticmethod("getDefaultParticleColorFromId")
		.staticmethod("getDefaultParticleColorFromName")
	;

	class_<SimulationCell, bases<SceneObject>, OORef<SimulationCell>, boost::noncopyable>("SimulationCell", init<DataSet*>())
		.add_property("pbcX", &SimulationCell::pbcX)
		.add_property("pbcY", &SimulationCell::pbcY)
		.add_property("pbcZ", &SimulationCell::pbcZ)
	;

	class_<ParticleType, bases<RefTarget>, OORef<ParticleType>, boost::noncopyable>("ParticleType", init<DataSet*>())
		.add_property("id", &ParticleType::id, &ParticleType::setId)
		.add_property("color", &ParticleType::color, &ParticleType::setColor)
		.add_property("radius", &ParticleType::radius, &ParticleType::setRadius)
		.add_property("name", make_function(&ParticleType::name, return_value_policy<copy_const_reference>()), &ParticleType::setName)
	;

	class_<ParticleDisplay, bases<DisplayObject>, OORef<ParticleDisplay>, boost::noncopyable>("ParticleDisplay", init<DataSet*>())
		.add_property("defaultParticleRadius", &ParticleDisplay::defaultParticleRadius, &ParticleDisplay::setDefaultParticleRadius)
		.add_property("defaultParticleColor", &ParticleDisplay::defaultParticleColor)
		.add_property("selectionParticleColor", &ParticleDisplay::selectionParticleColor)
		.add_property("shadingMode", &ParticleDisplay::shadingMode, &ParticleDisplay::setShadingMode)
		.add_property("renderingQuality", &ParticleDisplay::renderingQuality, &ParticleDisplay::setRenderingQuality)
		.add_property("particleShape", &ParticleDisplay::particleShape, &ParticleDisplay::setParticleShape)
	;

	class_<VectorDisplay, bases<DisplayObject>, OORef<VectorDisplay>, boost::noncopyable>("VectorDisplay", init<DataSet*>())
		.add_property("shadingMode", &VectorDisplay::shadingMode, &VectorDisplay::setShadingMode)
		.add_property("renderingQuality", &VectorDisplay::renderingQuality, &VectorDisplay::setRenderingQuality)
		.add_property("reverseArrowDirection", &VectorDisplay::reverseArrowDirection, &VectorDisplay::setReverseArrowDirection)
		.add_property("flipVectors", &VectorDisplay::flipVectors, &VectorDisplay::setFlipVectors)
		.add_property("arrowColor", make_function(&VectorDisplay::arrowColor, return_value_policy<copy_const_reference>()), &VectorDisplay::setArrowColor)
		.add_property("arrowWidth", &VectorDisplay::arrowWidth, &VectorDisplay::setArrowWidth)
		.add_property("scalingFactor", &VectorDisplay::scalingFactor, &VectorDisplay::setScalingFactor)
	;

	class_<SimulationCellDisplay, bases<DisplayObject>, OORef<SimulationCellDisplay>, boost::noncopyable>("SimulationCellDisplay", init<DataSet*>())
		.add_property("simulationCellLineWidth", &SimulationCellDisplay::simulationCellLineWidth, &SimulationCellDisplay::setSimulationCellLineWidth)
		.add_property("renderSimulationCell", &SimulationCellDisplay::renderSimulationCell, &SimulationCellDisplay::setRenderSimulationCell)
		.add_property("simulationCellRenderingColor", &SimulationCellDisplay::simulationCellRenderingColor, &SimulationCellDisplay::setSimulationCellRenderingColor)
	;

	class_<SurfaceMeshDisplay, bases<DisplayObject>, OORef<SurfaceMeshDisplay>, boost::noncopyable>("SurfaceMeshDisplay", init<DataSet*>())
		.add_property("surfaceColor", make_function(&SurfaceMeshDisplay::surfaceColor, return_value_policy<copy_const_reference>()), &SurfaceMeshDisplay::setSurfaceColor)
		.add_property("capColor", make_function(&SurfaceMeshDisplay::capColor, return_value_policy<copy_const_reference>()), &SurfaceMeshDisplay::setCapColor)
	;

	class_<BondsDisplay, bases<DisplayObject>, OORef<BondsDisplay>, boost::noncopyable>("BondsDisplay", init<DataSet*>())
		.add_property("bondWidth", &BondsDisplay::bondWidth, &BondsDisplay::setBondWidth)
		.add_property("bondColor", make_function(&BondsDisplay::bondColor, return_value_policy<copy_const_reference>()), &BondsDisplay::setBondColor)
		.add_property("shadingMode", &BondsDisplay::shadingMode, &BondsDisplay::setShadingMode)
		.add_property("renderingQuality", &BondsDisplay::renderingQuality, &BondsDisplay::setRenderingQuality)
		.add_property("useParticleColors", &BondsDisplay::useParticleColors, &BondsDisplay::setUseParticleColors)
	;

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

	class_<ParticleModifier, bases<Modifier>, OORef<ParticleModifier>, boost::noncopyable>("ParticleModifier", no_init)
	;

	class_<AsynchronousParticleModifier, bases<ParticleModifier>, OORef<AsynchronousParticleModifier>, boost::noncopyable>("AsynchronousParticleModifier", no_init)
		.add_property("autoUpdateEnabled", &AsynchronousParticleModifier::autoUpdateEnabled, &AsynchronousParticleModifier::setAutoUpdateEnabled)
		.add_property("storeResultsWithScene", &AsynchronousParticleModifier::storeResultsWithScene, &AsynchronousParticleModifier::setStoreResultsWithScene)
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(Particles);

};
