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
#include <plugins/particles/data/SurfaceMesh.h>

namespace Particles {

using namespace boost::python;
using namespace Ovito;
using namespace PyScript;

extern void setupImporterBinding();
extern void setupModifierBinding();

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

	class_<ParticlePropertyReference>("ParticlePropertyReference", init<ParticleProperty::Type, optional<int>>())
		.def(init<const QString&, optional<int>>())
		.add_property("type", &ParticlePropertyReference::type, &ParticlePropertyReference::setType)
		.add_property("name", make_function(&ParticlePropertyReference::name, return_value_policy<copy_const_reference>()))
		.add_property("vectorComponent", &ParticlePropertyReference::vectorComponent, &ParticlePropertyReference::setVectorComponent)
		.add_property("isNull", &ParticlePropertyReference::isNull)
		.def(self == other<ParticlePropertyReference>())
		.def("findInState", make_function(&ParticlePropertyReference::findInState, return_value_policy<ovito_object_reference>()))
	;

	ovito_class<ParticlePropertyObject, SceneObject>()
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

	ovito_class<ParticleTypeProperty, ParticlePropertyObject>()
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

	ovito_class<SimulationCell, SceneObject>()
		.add_property("pbcX", &SimulationCell::pbcX)
		.add_property("pbcY", &SimulationCell::pbcY)
		.add_property("pbcZ", &SimulationCell::pbcZ)
	;

	ovito_class<ParticleType, RefTarget>()
		.add_property("id", &ParticleType::id, &ParticleType::setId)
		.add_property("color", &ParticleType::color, &ParticleType::setColor)
		.add_property("radius", &ParticleType::radius, &ParticleType::setRadius)
		.add_property("name", make_function(&ParticleType::name, return_value_policy<copy_const_reference>()), &ParticleType::setName)
	;

	ovito_class<ParticleDisplay, DisplayObject>()
		.add_property("defaultParticleRadius", &ParticleDisplay::defaultParticleRadius, &ParticleDisplay::setDefaultParticleRadius)
		.add_property("defaultParticleColor", &ParticleDisplay::defaultParticleColor)
		.add_property("selectionParticleColor", &ParticleDisplay::selectionParticleColor)
		.add_property("shadingMode", &ParticleDisplay::shadingMode, &ParticleDisplay::setShadingMode)
		.add_property("renderingQuality", &ParticleDisplay::renderingQuality, &ParticleDisplay::setRenderingQuality)
		.add_property("particleShape", &ParticleDisplay::particleShape, &ParticleDisplay::setParticleShape)
	;

	ovito_class<VectorDisplay, DisplayObject>()
		.add_property("shadingMode", &VectorDisplay::shadingMode, &VectorDisplay::setShadingMode)
		.add_property("renderingQuality", &VectorDisplay::renderingQuality, &VectorDisplay::setRenderingQuality)
		.add_property("reverseArrowDirection", &VectorDisplay::reverseArrowDirection, &VectorDisplay::setReverseArrowDirection)
		.add_property("flipVectors", &VectorDisplay::flipVectors, &VectorDisplay::setFlipVectors)
		.add_property("arrowColor", make_function(&VectorDisplay::arrowColor, return_value_policy<copy_const_reference>()), &VectorDisplay::setArrowColor)
		.add_property("arrowWidth", &VectorDisplay::arrowWidth, &VectorDisplay::setArrowWidth)
		.add_property("scalingFactor", &VectorDisplay::scalingFactor, &VectorDisplay::setScalingFactor)
	;

	ovito_class<SimulationCellDisplay, DisplayObject>()
		.add_property("simulationCellLineWidth", &SimulationCellDisplay::simulationCellLineWidth, &SimulationCellDisplay::setSimulationCellLineWidth)
		.add_property("renderSimulationCell", &SimulationCellDisplay::renderSimulationCell, &SimulationCellDisplay::setRenderSimulationCell)
		.add_property("simulationCellRenderingColor", &SimulationCellDisplay::simulationCellRenderingColor, &SimulationCellDisplay::setSimulationCellRenderingColor)
	;

	ovito_class<SurfaceMeshDisplay, DisplayObject>()
		.add_property("surfaceColor", make_function(&SurfaceMeshDisplay::surfaceColor, return_value_policy<copy_const_reference>()), &SurfaceMeshDisplay::setSurfaceColor)
		.add_property("capColor", make_function(&SurfaceMeshDisplay::capColor, return_value_policy<copy_const_reference>()), &SurfaceMeshDisplay::setCapColor)
	;

	ovito_class<BondsDisplay, DisplayObject>()
		.add_property("bondWidth", &BondsDisplay::bondWidth, &BondsDisplay::setBondWidth)
		.add_property("bondColor", make_function(&BondsDisplay::bondColor, return_value_policy<copy_const_reference>()), &BondsDisplay::setBondColor)
		.add_property("shadingMode", &BondsDisplay::shadingMode, &BondsDisplay::setShadingMode)
		.add_property("renderingQuality", &BondsDisplay::renderingQuality, &BondsDisplay::setRenderingQuality)
		.add_property("useParticleColors", &BondsDisplay::useParticleColors, &BondsDisplay::setUseParticleColors)
	;

	ovito_class<SurfaceMesh, SceneObject>()
		.add_property("isCompletelySolid", &SurfaceMesh::isCompletelySolid, &SurfaceMesh::setCompletelySolid)
		.def("clearMesh", &SurfaceMesh::clearMesh)
	;

	setupImporterBinding();
	setupModifierBinding();
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(Particles);

};
