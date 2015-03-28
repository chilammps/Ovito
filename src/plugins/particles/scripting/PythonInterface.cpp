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
#include <plugins/particles/objects/ParticleDisplay.h>
#include <plugins/particles/objects/VectorDisplay.h>
#include <plugins/particles/objects/SimulationCellDisplay.h>
#include <plugins/particles/objects/SurfaceMesh.h>
#include <plugins/particles/objects/SurfaceMeshDisplay.h>
#include <plugins/particles/objects/BondsObject.h>
#include <plugins/particles/objects/BondsDisplay.h>
#include <plugins/particles/objects/SimulationCellObject.h>
#include <plugins/particles/util/CutoffNeighborFinder.h>
#include <core/utilities/io/CompressedTextWriter.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Internal)

using namespace boost::python;
using namespace PyScript;

dict ParticlePropertyObject__array_interface__(const ParticlePropertyObject& p)
{
	dict ai;
	if(p.componentCount() == 1) {
		ai["shape"] = boost::python::make_tuple(p.size());
		if(p.stride() != p.dataTypeSize())
			ai["strides"] = boost::python::make_tuple(p.stride());
	}
	else if(p.componentCount() > 1) {
		ai["shape"] = boost::python::make_tuple(p.size(), p.componentCount());
		ai["strides"] = boost::python::make_tuple(p.stride(), p.dataTypeSize());
	}
	else throw Exception("Cannot access empty particle property from Python.");
	if(p.dataType() == qMetaTypeId<int>()) {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		ai["typestr"] = str("<i") + str(sizeof(int));
#else
		ai["typestr"] = str(">i") + str(sizeof(int));
#endif
	}
	else if(p.dataType() == qMetaTypeId<FloatType>()) {
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		ai["typestr"] = str("<f") + str(sizeof(FloatType));
#else
		ai["typestr"] = str(">f") + str(sizeof(FloatType));
#endif
	}
	else throw Exception("Cannot access particle property of this data type from Python.");
	ai["data"] = boost::python::make_tuple((std::intptr_t)p.constData(), true);
	ai["version"] = 3;
	return ai;
}

dict BondsObject__array_interface__(const BondsObject& p)
{
	dict ai;
	ai["shape"] = boost::python::make_tuple(p.bonds().size(), 2);
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
	ai["typestr"] = str("<i") + str(sizeof(int));
#else
	ai["typestr"] = str(">i") + str(sizeof(int));
#endif
	const unsigned int* data = nullptr;
	if(!p.bonds().empty())
		data = &p.bonds().front().index1;
	ai["data"] = boost::python::make_tuple((std::intptr_t)data, true);
	ai["strides"] = boost::python::make_tuple(sizeof(BondsStorage::Bond), sizeof(int));
	ai["version"] = 3;
	return ai;
}

BOOST_PYTHON_MODULE(Particles)
{
	docstring_options docoptions(true, false);

	class_<ParticlePropertyReference>("ParticlePropertyReference", init<ParticleProperty::Type, optional<int>>())
		.def(init<const QString&, optional<int>>())
		.add_property("type", &ParticlePropertyReference::type, &ParticlePropertyReference::setType)
		.add_property("name", make_function(&ParticlePropertyReference::name, return_value_policy<copy_const_reference>()))
		.add_property("component", &ParticlePropertyReference::vectorComponent, &ParticlePropertyReference::setVectorComponent)
		.add_property("isNull", &ParticlePropertyReference::isNull)
		.def(self == other<ParticlePropertyReference>())
		.def("findInState", make_function(&ParticlePropertyReference::findInState, return_value_policy<ovito_object_reference>()))
		.def("__str__", &ParticlePropertyReference::nameWithComponent)
	;

	// Implement Python to ParticlePropertyReference conversion.
	auto convertible_ParticlePropertyReference = [](PyObject* obj_ptr) -> void* {
		if(obj_ptr == Py_None) return obj_ptr;
		if(extract<QString>(obj_ptr).check()) return obj_ptr;
		if(extract<ParticleProperty::Type>(obj_ptr).check()) return obj_ptr;
		return nullptr;
	};
	auto construct_ParticlePropertyReference = [](PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data) {
		void* storage = ((boost::python::converter::rvalue_from_python_storage<ParticlePropertyReference>*)data)->storage.bytes;
		if(obj_ptr == Py_None) {
			new (storage) ParticlePropertyReference();
			data->convertible = storage;
			return;
		}
		extract<ParticleProperty::Type> enumExtract(obj_ptr);
		if(enumExtract.check()) {
			ParticleProperty::Type ptype = enumExtract();
			if(ptype == ParticleProperty::UserProperty)
				throw Exception("User-defined particle property without a name is not acceptable.");
			new (storage) ParticlePropertyReference(ptype);
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

	{
		scope s = ovito_class<ParticlePropertyObject, DataObject>(
				":Base: :py:class:`ovito.data.DataObject`\n\n"
				"A data object that stores the values of a single particle property.",
				// Python class name:
				"ParticleProperty")
			.def("createUserProperty", &ParticlePropertyObject::createUserProperty)
			.def("createStandardProperty", &ParticlePropertyObject::createStandardProperty)
			.def("findInState", make_function((ParticlePropertyObject* (*)(const PipelineFlowState&, ParticleProperty::Type))&ParticlePropertyObject::findInState, return_value_policy<ovito_object_reference>()))
			.def("findInState", make_function((ParticlePropertyObject* (*)(const PipelineFlowState&, const QString&))&ParticlePropertyObject::findInState, return_value_policy<ovito_object_reference>()))
			.staticmethod("createUserProperty")
			.staticmethod("createStandardProperty")
			.staticmethod("findInState")
			.def("changed", &ParticlePropertyObject::changed)
			.def("nameWithComponent", &ParticlePropertyObject::nameWithComponent)
			.add_property("name", make_function(&ParticlePropertyObject::name, return_value_policy<copy_const_reference>()), &ParticlePropertyObject::setName,
					"The human-readable name of the particle property.")
			.add_property("__len__", &ParticlePropertyObject::size)
			.add_property("size", &ParticlePropertyObject::size, &ParticlePropertyObject::resize,
					"The number of particles.")
			.add_property("type", &ParticlePropertyObject::type, &ParticlePropertyObject::setType,
					".. _particle-types-list:"
					"\n\n"
					"The type of the particle property (user-defined or one of the standard types).\n"
					"One of the following constants:"
					"\n\n"
					"======================================================= ===================================================\n"
					"Type constant                                           Property name\n"
					"======================================================= ===================================================\n"
					"``ParticleProperty.Type.User``                          (a user-defined property with a non-standard name)\n"
					"``ParticleProperty.Type.ParticleType``                  :guilabel:`Particle Type`\n"
					"``ParticleProperty.Type.Position``                      :guilabel:`Position`\n"
					"``ParticleProperty.Type.Selection``                     :guilabel:`Selection`\n"
					"``ParticleProperty.Type.Color``                         :guilabel:`Color`\n"
					"``ParticleProperty.Type.Displacement``                  :guilabel:`Displacement`\n"
					"``ParticleProperty.Type.DisplacementMagnitude``         :guilabel:`Displacement Magnitude`\n"
					"``ParticleProperty.Type.PotentialEnergy``               :guilabel:`Potential Energy`\n"
					"``ParticleProperty.Type.KineticEnergy``                 :guilabel:`Kinetic Energy`\n"
					"``ParticleProperty.Type.TotalEnergy``                   :guilabel:`Total Energy`\n"
					"``ParticleProperty.Type.Velocity``                      :guilabel:`Velocity`\n"
					"``ParticleProperty.Type.Radius``                        :guilabel:`Radius`\n"
					"``ParticleProperty.Type.Cluster``                       :guilabel:`Cluster`\n"
					"``ParticleProperty.Type.Coordination``                  :guilabel:`Coordination`\n"
					"``ParticleProperty.Type.StructureType``                 :guilabel:`Structure Type`\n"
					"``ParticleProperty.Type.Identifier``                    :guilabel:`Particle Identifier`\n"
					"``ParticleProperty.Type.StressTensor``                  :guilabel:`Stress Tensor`\n"
					"``ParticleProperty.Type.StrainTensor``                  :guilabel:`Strain Tensor`\n"
					"``ParticleProperty.Type.DeformationGradient``           :guilabel:`Deformation Gradient`\n"
					"``ParticleProperty.Type.Orientation``                   :guilabel:`Orientation`\n"
					"``ParticleProperty.Type.Force``                         :guilabel:`Force`\n"
					"``ParticleProperty.Type.Mass``                          :guilabel:`Mass`\n"
					"``ParticleProperty.Type.Charge``                        :guilabel:`Charge`\n"
					"``ParticleProperty.Type.PeriodicImage``                 :guilabel:`Periodic Image`\n"
					"``ParticleProperty.Type.Transparency``                  :guilabel:`Transparency`\n"
					"``ParticleProperty.Type.DipoleOrientation``             :guilabel:`Dipole Orientation`\n"
					"``ParticleProperty.Type.DipoleMagnitude``               :guilabel:`Dipole Magnitude`\n"
					"``ParticleProperty.Type.AngularVelocity``               :guilabel:`Angular Velocity`\n"
					"``ParticleProperty.Type.AngularMomentum``               :guilabel:`Angular Momentum`\n"
					"``ParticleProperty.Type.Torque``                        :guilabel:`Torque`\n"
					"``ParticleProperty.Type.Spin``                          :guilabel:`Spin`\n"
					"``ParticleProperty.Type.CentroSymmetry``                :guilabel:`Centrosymmetry`\n"
					"``ParticleProperty.Type.VelocityMagnitude``             :guilabel:`Velocity Magnitude`\n"
					"``ParticleProperty.Type.NonaffineSquaredDisplacement``  :guilabel:`Nonaffine Squared Displacement`\n"
					"``ParticleProperty.Type.Molecule``                      :guilabel:`Molecule Identifier`\n"
					"``ParticleProperty.Type.AsphericalShape``               :guilabel:`Aspherical Shape`\n"
					"======================================================= ===================================================\n"
					)
			.add_property("dataType", &ParticlePropertyObject::dataType)
			.add_property("dataTypeSize", &ParticlePropertyObject::dataTypeSize)
			.add_property("stride", &ParticlePropertyObject::stride)
			.add_property("components", &ParticlePropertyObject::componentCount,
					"The number of vector components (if this is a vector particle property); otherwise 1 (= scalar property).")
			.add_property("__array_interface__", &ParticlePropertyObject__array_interface__)
		;

		enum_<ParticleProperty::Type>("Type")
			.value("User", ParticleProperty::UserProperty)
			.value("ParticleType", ParticleProperty::ParticleTypeProperty)
			.value("Position", ParticleProperty::PositionProperty)
			.value("Selection", ParticleProperty::SelectionProperty)
			.value("Color", ParticleProperty::ColorProperty)
			.value("Displacement", ParticleProperty::DisplacementProperty)
			.value("DisplacementMagnitude", ParticleProperty::DisplacementMagnitudeProperty)
			.value("PotentialEnergy", ParticleProperty::PotentialEnergyProperty)
			.value("KineticEnergy", ParticleProperty::KineticEnergyProperty)
			.value("TotalEnergy", ParticleProperty::TotalEnergyProperty)
			.value("Velocity", ParticleProperty::VelocityProperty)
			.value("Radius", ParticleProperty::RadiusProperty)
			.value("Cluster", ParticleProperty::ClusterProperty)
			.value("Coordination", ParticleProperty::CoordinationProperty)
			.value("StructureType", ParticleProperty::StructureTypeProperty)
			.value("Identifier", ParticleProperty::IdentifierProperty)
			.value("StressTensor", ParticleProperty::StressTensorProperty)
			.value("StrainTensor", ParticleProperty::StrainTensorProperty)
			.value("DeformationGradient", ParticleProperty::DeformationGradientProperty)
			.value("Orientation", ParticleProperty::OrientationProperty)
			.value("Force", ParticleProperty::ForceProperty)
			.value("Mass", ParticleProperty::MassProperty)
			.value("Charge", ParticleProperty::ChargeProperty)
			.value("PeriodicImage", ParticleProperty::PeriodicImageProperty)
			.value("Transparency", ParticleProperty::TransparencyProperty)
			.value("DipoleOrientation", ParticleProperty::DipoleOrientationProperty)
			.value("DipoleMagnitude", ParticleProperty::DipoleMagnitudeProperty)
			.value("AngularVelocity", ParticleProperty::AngularVelocityProperty)
			.value("AngularMomentum", ParticleProperty::AngularMomentumProperty)
			.value("Torque", ParticleProperty::TorqueProperty)
			.value("Spin", ParticleProperty::SpinProperty)
			.value("CentroSymmetry", ParticleProperty::CentroSymmetryProperty)
			.value("VelocityMagnitude", ParticleProperty::VelocityMagnitudeProperty)
			.value("NonaffineSquaredDisplacement", ParticleProperty::NonaffineSquaredDisplacementProperty)
			.value("Molecule", ParticleProperty::MoleculeProperty)
			.value("AsphericalShape", ParticleProperty::AsphericalShapeProperty)
		;
	}

	ovito_class<ParticleTypeProperty, ParticlePropertyObject>(
			":Base: :py:class:`ovito.data.ParticleProperty`\n\n"
			"A special :py:class:`ParticleProperty` that stores a list of :py:class:`ParticleType` instances in addition "
			"to the per-particle values. "
			"\n\n"
			"The particle properties ``Particle Type`` and ``Structure Type`` are represented by instances of this class. In addition to the regular per-particle "
			"data (consisting of an integer per particle, indicating its type ID), this class holds the list of defined particle types. These are "
			":py:class:`ParticleType` instances, which store the ID, name, color, and radius of each particle type.")
		.def("insertParticleType", &ParticleTypeProperty::insertParticleType)
		.def("particleType", make_function((ParticleType* (ParticleTypeProperty::*)(int) const)&ParticleTypeProperty::particleType, return_value_policy<ovito_object_reference>()))
		.def("particleType", make_function((ParticleType* (ParticleTypeProperty::*)(const QString&) const)&ParticleTypeProperty::particleType, return_value_policy<ovito_object_reference>()))
		.def("removeParticleType", &ParticleTypeProperty::removeParticleType)
		.def("clearParticleTypes", &ParticleTypeProperty::clearParticleTypes)
		.add_property("type_list", make_function(&ParticleTypeProperty::particleTypes, return_internal_reference<>()),
				"A list of :py:class:`ParticleType` instances.")
		.def("getDefaultParticleColorFromId", &ParticleTypeProperty::getDefaultParticleColorFromId)
		.def("getDefaultParticleColorFromName", &ParticleTypeProperty::getDefaultParticleColorFromName)
		.staticmethod("getDefaultParticleColorFromId")
		.staticmethod("getDefaultParticleColorFromName")
	;

	ovito_class<SimulationCellObject, DataObject>(
			":Base: :py:class:`ovito.data.DataObject`\n\n"
			"Stores the geometry and the boundary conditions of the simulation cell."
			"\n\n"
			"Instances of this class are associated with a :py:class:`~ovito.vis.SimulationCellDisplay` "
			"that controls the visual appearance of the simulation cell. It can be accessed through "
			"the :py:attr:`~DataObject.display` attribute of the :py:class:`~DataObject` base class.",
			// Python class name:
			"SimulationCell")
		.add_property("pbc_x", &SimulationCellObject::pbcX, &SimulationCellObject::setPbcX)
		.add_property("pbc_y", &SimulationCellObject::pbcY, &SimulationCellObject::setPbcY)
		.add_property("pbc_z", &SimulationCellObject::pbcZ, &SimulationCellObject::setPbcZ)
		.add_property("matrix", &SimulationCellObject::cellMatrix, &SimulationCellObject::setCellMatrix,
				"A 3x4 matrix containing the three edge vectors of the cell (matrix columns 0-2) "
				"and the cell origin (matrix column 3).")
		.add_property("vector1", make_function(&SimulationCellObject::edgeVector1, return_value_policy<copy_const_reference>()))
		.add_property("vector2", make_function(&SimulationCellObject::edgeVector2, return_value_policy<copy_const_reference>()))
		.add_property("vector3", make_function(&SimulationCellObject::edgeVector3, return_value_policy<copy_const_reference>()))
		.add_property("origin", make_function(&SimulationCellObject::origin, return_value_policy<copy_const_reference>()))
	;

	ovito_class<BondsObject, DataObject>(
			":Base: :py:class:`ovito.data.DataObject`\n\n"
			"This data object stores bonds between particles. One way of creating bonds is to use the :py:class:`~.ovito.modifiers.CreateBondsModifier`.",
			// Python class name:
			"Bonds")
		.add_property("__array_interface__", &BondsObject__array_interface__)
	;

	ovito_class<ParticleType, RefTarget>(
			"Defines the properties of a single particle type.")
		.add_property("id", &ParticleType::id, &ParticleType::setId,
				"The identifier of the particle type.")
		.add_property("color", &ParticleType::color, &ParticleType::setColor,
				"The display color to use for particles of this type.")
		.add_property("radius", &ParticleType::radius, &ParticleType::setRadius,
				"The display radius to use for particles of this type.")
		.add_property("name", make_function(&ParticleType::name, return_value_policy<copy_const_reference>()), &ParticleType::setName,
				"The display name of this particle type.")
	;

	class_<QVector<ParticleType*>, boost::noncopyable>("QVector<ParticleType*>", no_init)
		.def(QVector_OO_readonly_indexing_suite<ParticleType>())
	;
	python_to_container_conversion<QVector<ParticleType*>>();

	ovito_class<ParticleDisplay, DisplayObject>(
			":Base: :py:class:`ovito.vis.Display`\n\n"
			"Controls the visual appearance of particles.")
		.add_property("radius", &ParticleDisplay::defaultParticleRadius, &ParticleDisplay::setDefaultParticleRadius,
				"The default display radius of particles. "
				"Note that this setting only takes effect if no per-particle or per-type radii are defined."
				"\n\n"
				":Default: 1.2\n")
		.add_property("defaultParticleColor", &ParticleDisplay::defaultParticleColor)
		.add_property("selectionParticleColor", &ParticleDisplay::selectionParticleColor)
		.add_property("shading", &ParticleDisplay::shadingMode, &ParticleDisplay::setShadingMode,
				"The shading mode used to render particles.\n"
				"Possible values:"
				"\n\n"
				"   * ``ParticleDisplay.Shading.Normal`` (default) \n"
				"   * ``ParticleDisplay.Shading.Flat``\n"
				"\n")
		.add_property("renderingQuality", &ParticleDisplay::renderingQuality, &ParticleDisplay::setRenderingQuality)
		.add_property("shape", &ParticleDisplay::particleShape, &ParticleDisplay::setParticleShape,
				"The visual shape of particles.\n"
				"Possible values:"
				"\n\n"
				"   * ``ParticleDisplay.Shape.Spherical`` (default) \n"
				"   * ``ParticleDisplay.Shape.Square``\n"
				"\n")
	;

	ovito_class<VectorDisplay, DisplayObject>(
			":Base: :py:class:`ovito.vis.Display`\n\n"
			"Controls the visual appearance of vectors (arrows).")
		.add_property("shading", &VectorDisplay::shadingMode, &VectorDisplay::setShadingMode,
				"The shading style used for the arrows.\n"
				"Possible values:"
				"\n\n"
				"   * ``VectorDisplay.Shading.Normal`` (default) \n"
				"   * ``VectorDisplay.Shading.Flat``\n"
				"\n")
		.add_property("renderingQuality", &VectorDisplay::renderingQuality, &VectorDisplay::setRenderingQuality)
		.add_property("reverse", &VectorDisplay::reverseArrowDirection, &VectorDisplay::setReverseArrowDirection,
				"Boolean flag controlling the reversal of arrow directions."
				"\n\n"
				":Default: ``False``\n")
		.add_property("flip", &VectorDisplay::flipVectors, &VectorDisplay::setFlipVectors,
				"Boolean flag controlling the flipping of vectors."
				"\n\n"
				":Default: ``False``\n")
		.add_property("color", make_function(&VectorDisplay::arrowColor, return_value_policy<copy_const_reference>()), &VectorDisplay::setArrowColor,
				"The display color of arrows."
				"\n\n"
				":Default: ``(1.0, 1.0, 0.0)``\n")
		.add_property("width", &VectorDisplay::arrowWidth, &VectorDisplay::setArrowWidth,
				"Controls the width of arrows (in natural length units)."
				"\n\n"
				":Default: 0.5\n")
		.add_property("scaling", &VectorDisplay::scalingFactor, &VectorDisplay::setScalingFactor,
				"The uniform scaling factor applied to vectors."
				"\n\n"
				":Default: 1.0\n")
	;

	ovito_class<SimulationCellDisplay, DisplayObject>(
			":Base: :py:class:`ovito.vis.Display`\n\n"
			"Controls the visual appearance of :py:class:`~ovito.data.SimulationCellObject` data objects.")
		.add_property("line_width", &SimulationCellDisplay::simulationCellLineWidth, &SimulationCellDisplay::setSimulationCellLineWidth,
				"The width of the simulation cell line (in natural length units)."
				"\n\n"
				":Default: 0.14% of the simulation box diameter\n")
		.add_property("render_cell", &SimulationCellDisplay::renderSimulationCell, &SimulationCellDisplay::setRenderSimulationCell,
				"Boolean flag controlling the cell's visibility in rendered images. "
				"If ``False``, the cell will only be visible in the interactive viewports. "
				"\n\n"
				":Default: ``True``\n")
		.add_property("rendering_color", &SimulationCellDisplay::simulationCellRenderingColor, &SimulationCellDisplay::setSimulationCellRenderingColor,
				"The line color used when rendering the cell."
				"\n\n"
				":Default: ``(0, 0, 0)``\n")
	;

	ovito_class<SurfaceMeshDisplay, DisplayObject>(
			":Base: :py:class:`ovito.vis.Display`\n\n"
			"Controls the visual appearance of a surface mesh computed by the :py:class:`~ovito.modifiers.ConstructSurfaceModifier`.")
		.add_property("surface_color", make_function(&SurfaceMeshDisplay::surfaceColor, return_value_policy<copy_const_reference>()), &SurfaceMeshDisplay::setSurfaceColor,
				"The display color of the surface mesh."
				"\n\n"
				":Default: ``(1.0, 1.0, 1.0)``\n")
		.add_property("cap_color", make_function(&SurfaceMeshDisplay::capColor, return_value_policy<copy_const_reference>()), &SurfaceMeshDisplay::setCapColor,
				"The display color of the cap polygons at periodic boundaries."
				"\n\n"
				":Default: ``(0.8, 0.8, 1.0)``\n")
		.add_property("show_cap", &SurfaceMeshDisplay::showCap, &SurfaceMeshDisplay::setShowCap,
				"Controls the visibility of cap polygons, which are created at the intersection of the surface mesh with periodic box boundaries."
				"\n\n"
				":Default: ``True``\n")
		.add_property("surface_transparency", &SurfaceMeshDisplay::surfaceTransparency, &SurfaceMeshDisplay::setSurfaceTransparency,
				"The level of transparency of the displayed surface. Valid range is 0.0 -- 1.0."
				"\n\n"
				":Default: 0.0\n")
		.add_property("cap_transparency", &SurfaceMeshDisplay::capTransparency, &SurfaceMeshDisplay::setCapTransparency,
				"The level of transparency of the displayed cap polygons. Valid range is 0.0 -- 1.0."
				"\n\n"
				":Default: 0.0\n")
		.add_property("smooth_shading", &SurfaceMeshDisplay::smoothShading, &SurfaceMeshDisplay::setSmoothShading,
				"Enables smooth shading of the triangulated surface mesh."
				"\n\n"
				":Default: ``True``\n")
	;

	ovito_class<BondsDisplay, DisplayObject>(
			":Base: :py:class:`ovito.vis.Display`\n\n"
			"Controls the visual appearance of particle bonds. An instance of this class is attached to every :py:class:`~ovito.data.Bonds` data object.")
		.add_property("width", &BondsDisplay::bondWidth, &BondsDisplay::setBondWidth,
				"The display width of bonds (in natural length units)."
				"\n\n"
				":Default: 0.4\n")
		.add_property("color", make_function(&BondsDisplay::bondColor, return_value_policy<copy_const_reference>()), &BondsDisplay::setBondColor,
				"The display color of bonds. Used only if :py:attr:`.use_particle_colors` == False."
				"\n\n"
				":Default: ``(0.6, 0.6, 0.6)``\n")
		.add_property("shading", &BondsDisplay::shadingMode, &BondsDisplay::setShadingMode,
				"The shading style used for bonds.\n"
				"Possible values:"
				"\n\n"
				"   * ``BondsDisplay.Shading.Normal`` (default) \n"
				"   * ``BondsDisplay.Shading.Flat``\n"
				"\n")
		.add_property("renderingQuality", &BondsDisplay::renderingQuality, &BondsDisplay::setRenderingQuality)
		.add_property("use_particle_colors", &BondsDisplay::useParticleColors, &BondsDisplay::setUseParticleColors,
				"If ``True``, bonds are assigned the same color as the particles they are adjacent to."
				"\n\n"
				":Default: ``True``\n")
	;

	ovito_class<SurfaceMesh, DataObject>(
			":Base: :py:class:`ovito.data.DataObject`\n\n"
			"This data object stores the surface mesh computed by a :py:class:`~ovito.modifiers.ConstructSurfaceModifier`. "
			"\n\n"
			"Currently, no direct script access to the vertices and faces of the mesh is possible. But you can export the mesh to a VTK text file, "
			"which can be further processed by external tools such as ParaView. "
			"\n\n"
			"The visual appearance of the surface mesh within Ovito is controlled by its attached :py:class:`~ovito.vis.SurfaceMeshDisplay` instance, which is "
			"accessible through the :py:attr:`~DataObject.display` attribute of the :py:class:`DataObject` base class or through the :py:attr:`~ovito.modifiers.ConstructSurfaceModifier.mesh_display` attribute "
			"of the :py:class:`~ovito.modifiers.ConstructSurfaceModifier` that created the surface mesh from the input particle data."
			"\n\n"
			"Example:\n\n"
			".. literalinclude:: ../example_snippets/surface_mesh.py"
		)
		.add_property("isCompletelySolid", &SurfaceMesh::isCompletelySolid, &SurfaceMesh::setCompletelySolid)
		.def("clearMesh", &SurfaceMesh::clearMesh)
		.def("export_vtk", static_cast<void (*)(SurfaceMesh&,const QString&,SimulationCellObject*)>(
				[](SurfaceMesh& mesh, const QString& filename, SimulationCellObject* simCellObj) {
			if(!simCellObj)
				throw Exception("A simulation cell is required to generate non-periodic mesh for export.");
			TriMesh output;
			if(!SurfaceMeshDisplay::buildSurfaceMesh(mesh.mesh(), simCellObj->data(), output))
				throw Exception("Failed to generate non-periodic mesh for export. Simulation cell might be too small.");
			QFile file(filename);
			CompressedTextWriter writer(file);
			output.saveToVTK(writer);
		}),
		"Writes the surface mesh to a VTK file, which is a simple text-based format and which can be opened with the software ParaView. "
		"The method takes the output filename and a :py:class:`SimulationCell` object as input. The simulation cell information "
		"is needed by the method to generate a non-periodic version of the mesh, which is truncated at the periodic boundaries "
		"of the simulation cell (if it has any).")
		.def("export_cap_vtk", static_cast<void (*)(SurfaceMesh&,const QString&,SimulationCellObject*)>(
				[](SurfaceMesh& mesh, const QString& filename, SimulationCellObject* simCellObj) {
			if(!simCellObj)
				throw Exception("A simulation cell is required to generate cap mesh for export.");
			TriMesh output;
			SurfaceMeshDisplay::buildCapMesh(mesh.mesh(), simCellObj->data(), mesh.isCompletelySolid(), output);
			QFile file(filename);
			CompressedTextWriter writer(file);
			output.saveToVTK(writer);
		}),
		"If the surface mesh has been generated from a simulation cell with periodic boundary conditions, then this "
		"method computes the cap polygons from the intersection of the surface mesh with the periodic cell boundaries. "
		"The cap polygons are written to a VTK file, which is a simple text-based format and which can be opened with the software ParaView.")
	;

	{
		scope s = class_<CutoffNeighborFinder>("CutoffNeighborFinder", init<>())
			.def("prepare", static_cast<void (*)(CutoffNeighborFinder&,FloatType,ParticlePropertyObject&,SimulationCellObject&)>(
					[](CutoffNeighborFinder& finder, FloatType cutoff, ParticlePropertyObject& positions, SimulationCellObject& cell) {
				finder.prepare(cutoff, positions.storage(), cell.data());
			}))
		;

		class_<CutoffNeighborFinder::Query>("Query", init<const CutoffNeighborFinder&, size_t>())
			.def("next", &CutoffNeighborFinder::Query::next)
			.add_property("atEnd", &CutoffNeighborFinder::Query::atEnd)
			.add_property("current", &CutoffNeighborFinder::Query::current)
			.add_property("distanceSquared", &CutoffNeighborFinder::Query::distanceSquared)
			.add_property("delta", make_function(&CutoffNeighborFinder::Query::delta, return_value_policy<copy_const_reference>()))
			.add_property("pbcShift", static_cast<Vector3 (*)(CutoffNeighborFinder::Query&)>(
					[](CutoffNeighborFinder::Query& query) { return Vector3(query.pbcShift()); }))
		;
	}
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(Particles);

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
