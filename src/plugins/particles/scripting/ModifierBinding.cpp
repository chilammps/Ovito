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
#include <core/scene/pipeline/ModifierApplication.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include <plugins/particles/objects/ParticleTypeProperty.h>
#include <plugins/particles/modifier/ParticleModifier.h>
#include <plugins/particles/modifier/AsynchronousParticleModifier.h>
#include <plugins/particles/modifier/coloring/AssignColorModifier.h>
#include <plugins/particles/modifier/coloring/ColorCodingModifier.h>
#include <plugins/particles/modifier/coloring/AmbientOcclusionModifier.h>
#include <plugins/particles/modifier/modify/DeleteParticlesModifier.h>
#include <plugins/particles/modifier/modify/ShowPeriodicImagesModifier.h>
#include <plugins/particles/modifier/modify/WrapPeriodicImagesModifier.h>
#include <plugins/particles/modifier/modify/SliceModifier.h>
#include <plugins/particles/modifier/modify/AffineTransformationModifier.h>
#include <plugins/particles/modifier/modify/CreateBondsModifier.h>
#include <plugins/particles/modifier/properties/ComputePropertyModifier.h>
#include <plugins/particles/modifier/properties/FreezePropertyModifier.h>
#include <plugins/particles/modifier/selection/ClearSelectionModifier.h>
#include <plugins/particles/modifier/selection/InvertSelectionModifier.h>
#include <plugins/particles/modifier/selection/ManualSelectionModifier.h>
#include <plugins/particles/modifier/selection/SelectExpressionModifier.h>
#include <plugins/particles/modifier/selection/SelectParticleTypeModifier.h>
#include <plugins/particles/modifier/analysis/StructureIdentificationModifier.h>
#include <plugins/particles/modifier/analysis/binandreduce/BinAndReduceModifier.h>
#include <plugins/particles/modifier/analysis/bondangle/BondAngleAnalysisModifier.h>
#include <plugins/particles/modifier/analysis/cna/CommonNeighborAnalysisModifier.h>
#include <plugins/particles/modifier/analysis/centrosymmetry/CentroSymmetryModifier.h>
#include <plugins/particles/modifier/analysis/cluster/ClusterAnalysisModifier.h>
#include <plugins/particles/modifier/analysis/coordination/CoordinationNumberModifier.h>
#include <plugins/particles/modifier/analysis/displacements/CalculateDisplacementsModifier.h>
#include <plugins/particles/modifier/analysis/histogram/HistogramModifier.h>
#include <plugins/particles/modifier/analysis/scatterplot/ScatterPlotModifier.h>
#include <plugins/particles/modifier/analysis/strain/AtomicStrainModifier.h>
#include <plugins/particles/modifier/analysis/wignerseitz/WignerSeitzAnalysisModifier.h>
#include <plugins/particles/modifier/analysis/voronoi/VoronoiAnalysisModifier.h>
#include <plugins/particles/modifier/analysis/diamond/IdentifyDiamondModifier.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Internal)

using namespace boost::python;
using namespace PyScript;

BOOST_PYTHON_MODULE(ParticlesModify)
{
	ovito_abstract_class<ParticleModifier, Modifier>()
	;

	ovito_abstract_class<AsynchronousParticleModifier, ParticleModifier>()
	;

	ovito_class<AssignColorModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Assigns a uniform color to all selected particles. "
			"If no particle selection is defined (i.e. the ``\"Selection\"`` particle property does not exist), "
			"the modifier assigns the color to all particles. ")
		.add_property("color", &AssignColorModifier::color, &AssignColorModifier::setColor,
				"The color that will be assigned to particles."
				"\n\n"
				":Default: ``(0.3,0.3,1.0)``\n")
		.add_property("colorController", make_function(&AssignColorModifier::colorController, return_value_policy<ovito_object_reference>()), &AssignColorModifier::setColorController)
		.add_property("keepSelection", &AssignColorModifier::keepSelection, &AssignColorModifier::setKeepSelection)
	;

	{
		scope s = ovito_class<ColorCodingModifier, ParticleModifier>(
				":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
				"Colors particles based on the values of an arbitrary particle property."
				"\n\n"
				"Usage example::"
				"\n\n"
				"    from ovito.modifiers import *\n"
				"    \n"
				"    modifier = ColorCodingModifier(\n"
				"        property = \"Potential Energy\",\n"
				"        gradient = ColorCodingModifier.Hot()\n"
				"    )\n"
				"    node.modifiers.append(modifier)\n"
				"\n"
				"If, as in the example above, the :py:attr:`.start_value` and :py:attr:`.end_value` parameters are not explicitly set, "
				"then the modifier automatically adjusts them to the minimum and maximum values of the particle property when the modifier "
				"is inserted into the modification pipeline.")
			.add_property("property", make_function(&ColorCodingModifier::sourceProperty, return_value_policy<copy_const_reference>()), &ColorCodingModifier::setSourceProperty,
					"The name of the input property that should be used to color particles. "
					"This can be one of the :ref:`standard particle properties <particle-types-list>` or a custom particle property. "
					"When using vector properties the component must be included in the name, e.g. ``\"Velocity.X\"``. ")
			.add_property("start_value", &ColorCodingModifier::startValue, &ColorCodingModifier::setStartValue,
					"This parameter defines the value range when mapping the input property to a color.")
			.add_property("startValueController", make_function(&ColorCodingModifier::startValueController, return_value_policy<ovito_object_reference>()), &ColorCodingModifier::setStartValueController)
			.add_property("end_value", &ColorCodingModifier::endValue, &ColorCodingModifier::setEndValue,
					"This parameter defines the value range when mapping the input property to a color.")
			.add_property("endValueController", make_function(&ColorCodingModifier::endValueController, return_value_policy<ovito_object_reference>()), &ColorCodingModifier::setEndValueController)
			.add_property("gradient", make_function(&ColorCodingModifier::colorGradient, return_value_policy<ovito_object_reference>()), &ColorCodingModifier::setColorGradient,
					"The color gradient object, which is responsible for mapping normalized property values to colors. "
					"Available gradient types are:\n"
					" * ``ColorCodingModifier.Rainbow()`` (default)\n"
					" * ``ColorCodingModifier.Grayscale()``\n"
					" * ``ColorCodingModifier.Hot()``\n"
					" * ``ColorCodingModifier.Jet()``\n"
					" * ``ColorCodingModifier.Custom(\"<image file>\")``\n"
					"\n"
					"The last color map constructor expects the path to an image file on disk, "
					"which will be used to create a custom color gradient from a row of pixels in the image.")
			.add_property("only_selected", &ColorCodingModifier::colorOnlySelected, &ColorCodingModifier::setColorOnlySelected,
					"If ``True``, only selected particles will be affected by the modifier and the existing colors "
					"of unselected particles will be preserved; if ``False``, all particles will be colored."
					"\n\n"
					":Default: ``False``\n")
			.add_property("keepSelection", &ColorCodingModifier::keepSelection, &ColorCodingModifier::setKeepSelection)
		;

		ovito_abstract_class<ColorCodingGradient, RefTarget>()
			.def("valueToColor", pure_virtual(&ColorCodingGradient::valueToColor))
		;

		ovito_class<ColorCodingHSVGradient, ColorCodingGradient>(nullptr, "Rainbow")
		;
		ovito_class<ColorCodingGrayscaleGradient, ColorCodingGradient>(nullptr, "Grayscale")
		;
		ovito_class<ColorCodingHotGradient, ColorCodingGradient>(nullptr, "Hot")
		;
		ovito_class<ColorCodingJetGradient, ColorCodingGradient>(nullptr, "Jet")
		;
		ovito_class<ColorCodingImageGradient, ColorCodingGradient>(nullptr, "Image")
			.def("loadImage", &ColorCodingImageGradient::loadImage)
		;
	}

	ovito_class<AmbientOcclusionModifier, AsynchronousParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Performs a quick lighting calculation to shade particles according to the degree of occlusion by other particles. ")
		.add_property("intensity", &AmbientOcclusionModifier::intensity, &AmbientOcclusionModifier::setIntensity,
				"A number controlling the strength of the applied shading effect. "
				"\n\n"
				":Valid range: [0.0, 1.0]\n"
				":Default: 0.7")
		.add_property("sample_count", &AmbientOcclusionModifier::samplingCount, &AmbientOcclusionModifier::setSamplingCount,
				"The number of light exposure samples to compute. More samples give a more even light distribution "
				"but take longer to compute."
				"\n\n"
				":Default: 40\n")
		.add_property("buffer_resolution", &AmbientOcclusionModifier::bufferResolution, &AmbientOcclusionModifier::setBufferResolution,
				"A positive integer controlling the resolution of the internal render buffer, which is used to compute how much "
				"light each particle receives. When the number of particles is large, a larger buffer resolution should be used."
				"\n\n"
				":Valid range: [1, 4]\n"
				":Default: 3\n")
	;

	ovito_class<DeleteParticlesModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"This modifier deletes the selected particles. It has no parameters.",
			// Python class name:
			"DeleteSelectedParticlesModifier")
	;

	ovito_class<ShowPeriodicImagesModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"This modifier replicates all particles to display periodic images of the system.")
		.add_property("replicate_x", &ShowPeriodicImagesModifier::showImageX, &ShowPeriodicImagesModifier::setShowImageX,
				"Enables replication of particles along *x*."
				"\n\n"
				":Default: ``False``\n")
		.add_property("replicate_y", &ShowPeriodicImagesModifier::showImageY, &ShowPeriodicImagesModifier::setShowImageY,
				"Enables replication of particles along *y*."
				"\n\n"
				":Default: ``False``\n")
		.add_property("replicate_z", &ShowPeriodicImagesModifier::showImageZ, &ShowPeriodicImagesModifier::setShowImageZ,
				"Enables replication of particles along *z*."
				"\n\n"
				":Default: ``False``\n")
		.add_property("num_x", &ShowPeriodicImagesModifier::numImagesX, &ShowPeriodicImagesModifier::setNumImagesX,
				"A positive integer specifying the number of copies to generate in the *x* direction (including the existing primary image)."
				"\n\n"
				":Default: 3\n")
		.add_property("num_y", &ShowPeriodicImagesModifier::numImagesY, &ShowPeriodicImagesModifier::setNumImagesY,
				"A positive integer specifying the number of copies to generate in the *y* direction (including the existing primary image)."
				"\n\n"
				":Default: 3\n")
		.add_property("num_z", &ShowPeriodicImagesModifier::numImagesZ, &ShowPeriodicImagesModifier::setNumImagesZ,
				"A positive integer specifying the number of copies to generate in the *z* direction (including the existing primary image)."
				"\n\n"
				":Default: 3\n")
		.add_property("adjust_box", &ShowPeriodicImagesModifier::adjustBoxSize, &ShowPeriodicImagesModifier::setAdjustBoxSize,
				"A boolean flag controlling the modification of the simulation cell geometry. "
				"If ``True``, the simulation cell is extended to fit the multiplied system. "
				"If ``False``, the original simulation cell (containing only the primary image of the system) is kept. "
				"\n\n"
				":Default: ``False``\n")
		.add_property("unique_ids", &ShowPeriodicImagesModifier::uniqueIdentifiers, &ShowPeriodicImagesModifier::setUniqueIdentifiers,
				"If ``True``, the modifier automatically generates a new unique ID for each copy of a particle. "
				"This option has no effect if the input system does not contain particle IDs. "
				"\n\n"
				":Default: ``True``\n")
	;

	ovito_class<WrapPeriodicImagesModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"This modifier maps particles located outside the simulation cell back into the box by \"wrapping\" their coordinates "
			"around at the periodic boundaries of the simulation cell. This modifier has no parameters.")
	;

	ovito_class<ComputePropertyModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Evaluates a user-defined math expression to compute the values of a particle property."
			"\n\n"
			"Example::"
			"\n\n"
			"    from ovito.modifiers import *\n"
			"    \n"
			"    modifier = ComputePropertyModifier()\n"
			"    modifier.output_property = \"Color\"\n"
			"    modifier.expressions = [\"Position.X / CellSize.X\", \"0.0\", \"0.5\"]\n"
			"\n")
		.add_property("expressions", make_function(&ComputePropertyModifier::expressions, return_value_policy<copy_const_reference>()), &ComputePropertyModifier::setExpressions,
				"A list of strings containing the math expressions to compute, one for each vector component of the output property. "
				"If the output property is a scalar property, the list should comprise exactly one string. "
				"\n\n"
				":Default: ``[\"0\"]``\n")
		.add_property("output_property", make_function(&ComputePropertyModifier::outputProperty, return_value_policy<copy_const_reference>()), &ComputePropertyModifier::setOutputProperty,
				"The output particle property in which the modifier should store the computed values. "
				"\n\n"
				":Default: ``\"Custom property\"``\n")
		.add_property("propertyComponentCount", &ComputePropertyModifier::propertyComponentCount, &ComputePropertyModifier::setPropertyComponentCount)
		.add_property("only_selected", &ComputePropertyModifier::onlySelectedParticles, &ComputePropertyModifier::setOnlySelectedParticles,
				"If ``True``, the property is only computed for selected particles and existing property values "
				"are preserved for unselected particles."
				"\n\n"
				":Default: ``False``\n")
	;

	ovito_class<FreezePropertyModifier, ParticleModifier>()
		.add_property("source_property", make_function(&FreezePropertyModifier::sourceProperty, return_value_policy<copy_const_reference>()), &FreezePropertyModifier::setSourceProperty)
		.add_property("destination_property", make_function(&FreezePropertyModifier::destinationProperty, return_value_policy<copy_const_reference>()), &FreezePropertyModifier::setDestinationProperty)
		.def("take_snapshot", &FreezePropertyModifier::takePropertySnapshot)
	;

	ovito_class<ClearSelectionModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"This modifier clears the particle selection by deleting the ``\"Selection\"`` particle property. "
			"It has no parameters.")
	;

	ovito_class<InvertSelectionModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"This modifier inverts the particle selection. It has no parameters.")
	;

	ovito_class<ManualSelectionModifier, ParticleModifier>()
		.def("resetSelection", &ManualSelectionModifier::resetSelection)
		.def("selectAll", &ManualSelectionModifier::selectAll)
		.def("clearSelection", &ManualSelectionModifier::clearSelection)
		.def("toggleParticleSelection", &ManualSelectionModifier::toggleParticleSelection)
	;

	ovito_class<SelectExpressionModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"This modifier selects particles based on a user-defined Boolean expression."
			"\n\n"
			"Example::"
			"\n\n"
			"    from ovito.modifiers import *\n"
			"    \n"
			"    mod = SelectExpressionModifier(expression = 'PotentialEnergy > 3.6')\n"
			"    node.modifiers.append(mod)\n"
			"\n")
		.add_property("expression", make_function(&SelectExpressionModifier::expression, return_value_policy<copy_const_reference>()), &SelectExpressionModifier::setExpression,
				"A string with a Boolean expression. The syntax is documented in OVITO's user manual.")
	;

	ovito_class<SelectParticleTypeModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Selects all particles of a certain type (or types)."
			"\n\n"
			"Example::"
			"\n\n"
			"    from ovito.modifiers import *\n"
			"    \n"
			"    modifier = SelectParticleTypeModifier()\n"
			"    modifier.property = \"Structure Type\"\n"
			"    modifier.types = { CommonNeighborAnalysisModifier.Type.FCC,\n"
			"                       CommonNeighborAnalysisModifier.Type.HCP }\n"
			"\n")
		.add_property("property", make_function(&SelectParticleTypeModifier::sourceProperty, return_value_policy<copy_const_reference>()), &SelectParticleTypeModifier::setSourceProperty,
				"The name of the integer particle property to be used as input, which contains the particle types. "
				"This can be a :ref:`standard particle property <particle-types-list>` such as ``\"Particle Type\"`` or ``\"Structure Type\"``, or "
				"a custom particle property."
				"\n\n"
				":Default: ``\"Particle Type\"``\n")
		.add_property("types", make_function(&SelectParticleTypeModifier::selectedParticleTypes, return_value_policy<copy_const_reference>()), &SelectParticleTypeModifier::setSelectedParticleTypes,
				"A Python ``set`` of integers, which specifies the particle types to select. "
				"\n\n"
				":Default: ``set([])``\n")
	;

	ovito_class<SliceModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Deletes or selects particles based on a plane in three-dimensional space.")
		.add_property("distance", &SliceModifier::distance, &SliceModifier::setDistance,
				"The distance of the slicing plane from the origin (along its normal vector)."
				"\n\n"
				":Default: 0.0\n")
		.add_property("distanceController", make_function(&SliceModifier::distanceController, return_value_policy<ovito_object_reference>()), &SliceModifier::setDistanceController)
		.add_property("normal", &SliceModifier::normal, &SliceModifier::setNormal,
				"The normal vector of the slicing plane. Does not have to be a unit vector."
				"\n\n"
				":Default: ``(1,0,0)``\n")
		.add_property("normalController", make_function(&SliceModifier::normalController, return_value_policy<ovito_object_reference>()), &SliceModifier::setNormalController)
		.add_property("slice_width", &SliceModifier::sliceWidth, &SliceModifier::setSliceWidth,
				"The width of the slab to cut. If zero, the modifier cuts all particles on one "
				"side of the slicing plane."
				"\n\n"
				":Default: 0.0\n")
		.add_property("sliceWidthController", make_function(&SliceModifier::sliceWidthController, return_value_policy<ovito_object_reference>()), &SliceModifier::setSliceWidthController)
		.add_property("inverse", &SliceModifier::inverse, &SliceModifier::setInverse,
				"Reverses the sense of the slicing plane."
				"\n\n"
				":Default: ``False``\n")
		.add_property("select", &SliceModifier::createSelection, &SliceModifier::setCreateSelection,
				"If ``True``, the modifier selects particles instead of deleting them."
				"\n\n"
				":Default: ``False``\n")
		.add_property("only_selected", &SliceModifier::applyToSelection, &SliceModifier::setApplyToSelection,
				"If ``True``, the modifier acts only on selected particles; if ``False``, the modifier acts on all particles."
				"\n\n"
				":Default: ``False``\n")
	;

	ovito_class<AffineTransformationModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Applies an affine transformation to particles and/or the simulation cell."
			"\n\n"
			"Example::"
			"\n\n"
			"    from ovito.modifiers import *\n"
			"    \n"
			"    xy_shear = 0.05\n"
			"    mod = AffineTransformationModifier(\n"
			"              transform_particles = True,\n"
			"              transform_box = True,\n"
			"              transformation = [[1,xy_shear,0,0],\n"
			"                                [0,       1,0,0],\n"
			"                                [0,       0,1,0]])\n"
			"\n")
		.add_property("transformation", make_function(&AffineTransformationModifier::transformation, return_value_policy<copy_const_reference>()), &AffineTransformationModifier::setTransformation,
				"The 3x4 transformation matrix being applied to particle positions and/or the simulation cell. "
				"The first three matrix columns define the linear part of the transformation, while the fourth "
				"column specifies the translation vector. "
				"\n\n"
				"This matrix describes a relative transformation and is used only if :py:attr:`.relative_mode` == ``True``."
				"\n\n"
				":Default: ``[[ 1.  0.  0.  0.] [ 0.  1.  0.  0.] [ 0.  0.  1.  0.]]``\n")
		.add_property("target_cell", make_function(&AffineTransformationModifier::targetCell, return_value_policy<copy_const_reference>()), &AffineTransformationModifier::setTargetCell,
				"This 3x4 matrix specifies the target cell shape. It is used when :py:attr:`.relative_mode` == ``False``. "
				"\n\n"
				"The first three columns of the matrix specify the three edge vectors of the target cell. "
				"The fourth column defines the origin vector of the target cell.")
		.add_property("relative_mode", &AffineTransformationModifier::relativeMode, &AffineTransformationModifier::setRelativeMode,
				"Selects the operation mode of the modifier."
				"\n\n"
				"If ``relative_mode==True``, the modifier transforms the particles and/or the simulation cell "
				"by applying the matrix given by the :py:attr:`.transformation` parameter."
				"\n\n"
				"If ``relative_mode==False``, the modifier transforms the particles and/or the simulation cell "
				"such that the old simulation cell will have the shape given by the the :py:attr:`.target_cell` parameter after the transformation."
				"\n\n"
				":Default: ``True``\n")
		.add_property("transform_particles", &AffineTransformationModifier::applyToParticles, &AffineTransformationModifier::setApplyToParticles,
				"If ``True``, the modifier transforms the particle positions."
				"\n\n"
				":Default: ``True``\n")
		.add_property("only_selected", &AffineTransformationModifier::selectionOnly, &AffineTransformationModifier::setSelectionOnly,
				"If ``True``, the modifier acts only on selected particles; if ``False``, the modifier acts on all particles."
				"\n\n"
				":Default: ``False``\n")
		.add_property("transform_box", &AffineTransformationModifier::applyToSimulationBox, &AffineTransformationModifier::setApplyToSimulationBox,
				"If ``True``, the modifier transforms the simulation cell."
				"\n\n"
				":Default: ``False``\n")
		.add_property("transform_surface", &AffineTransformationModifier::applyToSurfaceMesh, &AffineTransformationModifier::setApplyToSurfaceMesh,
				"If ``True``, the modifier transforms the surface mesh (if any) that has previously been generated by a :py:class:`ConstructSurfaceModifier`."
				"\n\n"
				":Default: ``True``\n")
	;

	ovito_class<BinAndReduceModifier, ParticleModifier>()
		.add_property("property", make_function(&BinAndReduceModifier::sourceProperty, return_value_policy<copy_const_reference>()), &BinAndReduceModifier::setSourceProperty)
		.add_property("reductionOperation", &BinAndReduceModifier::reductionOperation, &BinAndReduceModifier::setReductionOperation)
		.add_property("firstDerivative", &BinAndReduceModifier::firstDerivative, &BinAndReduceModifier::setFirstDerivative)
		.add_property("binDirection", &BinAndReduceModifier::binDirection, &BinAndReduceModifier::setBinDirection)
		.add_property("numberOfBinsX", &BinAndReduceModifier::numberOfBinsX, &BinAndReduceModifier::setNumberOfBinsX)
		.add_property("numberOfBinsY", &BinAndReduceModifier::numberOfBinsY, &BinAndReduceModifier::setNumberOfBinsY)
	;

	enum_<BinAndReduceModifier::ReductionOperationType>("ReductionOperationType")
		.value("RED_MEAN", BinAndReduceModifier::RED_MEAN)
		.value("RED_SUM", BinAndReduceModifier::RED_SUM)
		.value("RED_SUM_VOL", BinAndReduceModifier::RED_SUM_VOL)
		.value("RED_MIN", BinAndReduceModifier::RED_MIN)
		.value("RED_MAX", BinAndReduceModifier::RED_MAX)
	;

	enum_<BinAndReduceModifier::BinDirectionType>("BinDirectionType")
		.value("CELL_VECTOR_1", BinAndReduceModifier::CELL_VECTOR_1)
		.value("CELL_VECTOR_2", BinAndReduceModifier::CELL_VECTOR_2)
		.value("CELL_VECTOR_3", BinAndReduceModifier::CELL_VECTOR_3)
		.value("CELL_VECTORS_1_2", BinAndReduceModifier::CELL_VECTORS_1_2)
		.value("CELL_VECTORS_1_3", BinAndReduceModifier::CELL_VECTORS_1_3)
		.value("CELL_VECTORS_2_3", BinAndReduceModifier::CELL_VECTORS_2_3)
	;

	ovito_abstract_class<StructureIdentificationModifier, AsynchronousParticleModifier>()
	;

	{
		scope s = ovito_class<BondAngleAnalysisModifier, StructureIdentificationModifier>(
				":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
				"Performs the bond-angle analysis described by Ackland & Jones to classify the local "
				"structure of each particle. "
				"\n\n"
				"The modifier stores its results as integer values in the ``\"Structure Type\"`` particle property. "
				"The following constants are defined: "
				"\n\n"
				"   * ``BondAngleAnalysisModifier.Type.OTHER`` (0)\n"
				"   * ``BondAngleAnalysisModifier.Type.FCC`` (1)\n"
				"   * ``BondAngleAnalysisModifier.Type.HCP`` (2)\n"
				"   * ``BondAngleAnalysisModifier.Type.BCC`` (3)\n"
				"   * ``BondAngleAnalysisModifier.Type.ICO`` (4)\n"
				"\n"
				"For example, to count the number of FCC atoms in a system::"
				"\n\n"
				"    from ovito.modifiers import *\n"
				"    \n"
				"    modifier = BondAngleAnalysisModifier()\n"
				"    node.modifiers.append(modifier)\n"
				"    node.compute()\n"
				"    print(\"Number of FCC atoms: %i\" % modifier.counts[BondAngleAnalysisModifier.Type.FCC])\n"
				"\n"
				"Furthermore, the modifier assigns a color to particles based on their structural types. "
				"You can change the color of a structural type as shown in the following example::"
				"\n\n"
				"    modifier = BondAngleAnalysisModifier()\n"
				"    node.modifiers.append(modifier)\n"
				"    \n"
				"    # Give FCC atoms a blue color:\n"
				"    modifier.structures[BondAngleAnalysisModifier.Type.FCC].color = (0,0,1)\n"
				"    \n"
				"    # Select all disordered atoms:\n"
				"    node.modifiers.append(SelectParticleTypeModifier(\n"
				"        property = ParticleProperty.Type.StructureType,\n"
				"        types = { BondAngleAnalysisModifier.Type.OTHER }\n"
				"    ))\n"
				"\n")
			.add_property("structures", make_function(&BondAngleAnalysisModifier::structureTypes, return_internal_reference<>()),
					"A list of :py:class:`~ovito.data.ParticleType` instances managed by this modifier, one for each structural type. "
					"You can adjust the color of structural types as shown in the code example above.")
			.add_property("counts", make_function(&BondAngleAnalysisModifier::structureCounts, return_value_policy<copy_const_reference>()),
					"A list of integers indicating the number of particles found for each structure type. "
					"Note that accessing this output field is only possible after the modifier has computed its results. "
					"Thus, you have to call :py:meth:`ovito.ObjectNode.compute` first to ensure that this information is up to date. ")
		;

		enum_<BondAngleAnalysisModifier::StructureType>("Type")
			.value("OTHER", BondAngleAnalysisModifier::OTHER)
			.value("FCC", BondAngleAnalysisModifier::FCC)
			.value("HCP", BondAngleAnalysisModifier::HCP)
			.value("BCC", BondAngleAnalysisModifier::BCC)
			.value("ICO", BondAngleAnalysisModifier::ICO)
		;
	}

	{
		scope s = ovito_class<CommonNeighborAnalysisModifier, StructureIdentificationModifier>(
				":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
				"Performs the common neighbor analysis (CNA) to classify the local "
				"structure of each particle. "
				"\n\n"
				"The modifier stores its results as integer values in the ``\"Structure Type\"`` particle property. "
				"The following constants are defined: "
				"\n\n"
				"   * ``CommonNeighborAnalysisModifier.Type.OTHER`` (0)\n"
				"   * ``CommonNeighborAnalysisModifier.Type.FCC`` (1)\n"
				"   * ``CommonNeighborAnalysisModifier.Type.HCP`` (2)\n"
				"   * ``CommonNeighborAnalysisModifier.Type.BCC`` (3)\n"
				"   * ``CommonNeighborAnalysisModifier.Type.ICO`` (4)\n"
				"   * ``CommonNeighborAnalysisModifier.Type.DIA`` (5)\n"
				"\n"
				"For example, to count the number of FCC atoms in a system::"
				"\n\n"
				"    from ovito.modifiers import *\n"
				"    \n"
				"    modifier = CommonNeighborAnalysisModifier(adaptive_mode = True)\n"
				"    node.modifiers.append(modifier)\n"
				"    node.compute()\n"
				"    print(\"Number of FCC atoms: %i\" % modifier.counts[CommonNeighborAnalysisModifier.Type.FCC])\n"
				"\n"
				"Furthermore, the modifier assigns a color to particles based on their structural types. "
				"You can change the color of a structural type as shown in the following example::"
				"\n\n"
				"    modifier = CommonNeighborAnalysisModifier()\n"
				"    node.modifiers.append(modifier)\n"
				"    \n"
				"    # Give FCC atoms a blue color:\n"
				"    modifier.structures[CommonNeighborAnalysisModifier.Type.FCC].color = (0,0,1)\n"
				"    \n"
				"    # Select all disordered atoms:\n"
				"    node.modifiers.append(SelectParticleTypeModifier(\n"
				"        property = ParticleProperty.Type.StructureType,\n"
				"        types = { CommonNeighborAnalysisModifier.Type.OTHER }\n"
				"    ))\n"
				"\n")
			.add_property("structures", make_function(&CommonNeighborAnalysisModifier::structureTypes, return_internal_reference<>()),
					"A list of :py:class:`~ovito.data.ParticleType` instances managed by this modifier, one for each structural type. "
					"You can adjust the color of structural types here as shown in the code example above.")
			.add_property("counts", make_function(&CommonNeighborAnalysisModifier::structureCounts, return_value_policy<copy_const_reference>()),
					"A list of integers indicating the number of particles found for each structure type. "
					"Note that accessing this output field is only possible after the modifier has computed its results. "
					"Thus, you have to call :py:meth:`ovito.ObjectNode.compute` first to ensure that this information is up to date. ")
			.add_property("cutoff", &CommonNeighborAnalysisModifier::cutoff, &CommonNeighborAnalysisModifier::setCutoff,
					"The cutoff radius used for the conventional common neighbor analysis (:py:attr:`.adaptive_mode` == ``False``)."
					"\n\n"
					":Default: 3.2\n")
			.add_property("adaptive_mode", &CommonNeighborAnalysisModifier::adaptiveMode, &CommonNeighborAnalysisModifier::setAdaptiveMode,
					"Activate the adaptive version of the common neighbor analysis, which automatically determine the optimal cutoff radius "
					"for each atom. If ``False``, the conventional CNA is performed using a fixed neighbor cutoff radius."
					"\n\n"
					":Default: ``True``\n")
		;

		enum_<CommonNeighborAnalysisModifier::StructureType>("Type")
			.value("OTHER", CommonNeighborAnalysisModifier::OTHER)
			.value("FCC", CommonNeighborAnalysisModifier::FCC)
			.value("HCP", CommonNeighborAnalysisModifier::HCP)
			.value("BCC", CommonNeighborAnalysisModifier::BCC)
			.value("ICO", CommonNeighborAnalysisModifier::ICO)
			.value("DIA", CommonNeighborAnalysisModifier::DIA)
		;
	}

	{
		scope s = ovito_class<IdentifyDiamondModifier, StructureIdentificationModifier>(
				":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
				"This analysis modifier finds atoms that are arranged in a cubic or hexagonal diamond lattice."
				"\n\n"
				"The modifier stores its results as integer values in the ``\"Structure Type\"`` particle property. "
				"The following constants are defined: "
				"\n\n"
				"   * ``IdentifyDiamondModifier.Type.OTHER`` (0)\n"
				"   * ``IdentifyDiamondModifier.Type.CUBIC_DIAMOND`` (1)\n"
				"   * ``IdentifyDiamondModifier.Type.CUBIC_DIAMOND_FIRST_NEIGHBOR`` (2)\n"
				"   * ``IdentifyDiamondModifier.Type.CUBIC_DIAMOND_SECOND_NEIGHBOR`` (3)\n"
				"   * ``IdentifyDiamondModifier.Type.HEX_DIAMOND`` (4)\n"
				"   * ``IdentifyDiamondModifier.Type.HEX_DIAMOND_FIRST_NEIGHBOR`` (5)\n"
				"   * ``IdentifyDiamondModifier.Type.HEX_DIAMOND_SECOND_NEIGHBOR`` (6)\n"
				"\n"
				"For example, to count the number of cubic diamond atoms in a system::"
				"\n\n"
				"    from ovito.modifiers import *\n"
				"    \n"
				"    modifier = IdentifyDiamondModifier()\n"
				"    node.modifiers.append(modifier)\n"
				"    node.compute()\n"
				"    print(\"Number of cubic diamond atoms:\")\n"
				"    print(modifier.counts[IdentifyDiamondModifier.Type.CUBIC_DIAMOND])\n"
				"\n")
			.add_property("structures", make_function(&IdentifyDiamondModifier::structureTypes, return_internal_reference<>()),
					"A list of :py:class:`~ovito.data.ParticleType` instances managed by this modifier, one for each structural type. "
					"You can adjust the color of structural types here as shown in the code example above.")
			.add_property("counts", make_function(&IdentifyDiamondModifier::structureCounts, return_value_policy<copy_const_reference>()),
					"A list of integers indicating the number of particles found for each structure type. "
					"Note that accessing this output field is only possible after the modifier has computed its results. "
					"Thus, you have to call :py:meth:`ovito.ObjectNode.compute` first to ensure that this information is up to date. ")
		;

		enum_<IdentifyDiamondModifier::StructureType>("Type")
			.value("OTHER", IdentifyDiamondModifier::OTHER)
			.value("CUBIC_DIAMOND", IdentifyDiamondModifier::CUBIC_DIAMOND)
			.value("CUBIC_DIAMOND_FIRST_NEIGHBOR", IdentifyDiamondModifier::CUBIC_DIAMOND_FIRST_NEIGH)
			.value("CUBIC_DIAMOND_SECOND_NEIGHBOR", IdentifyDiamondModifier::CUBIC_DIAMOND_SECOND_NEIGH)
			.value("HEX_DIAMOND", IdentifyDiamondModifier::HEX_DIAMOND)
			.value("HEX_DIAMOND_FIRST_NEIGHBOR", IdentifyDiamondModifier::HEX_DIAMOND_FIRST_NEIGH)
			.value("HEX_DIAMOND_SECOND_NEIGHBOR", IdentifyDiamondModifier::HEX_DIAMOND_SECOND_NEIGH)
		;
	}


	{
		scope s = ovito_class<CreateBondsModifier, AsynchronousParticleModifier>(
				":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
				"Creates bonds between nearby particles. The modifier outputs its computation results as a :py:class:`~ovito.data.Bonds` data object.")
			.add_property("mode", &CreateBondsModifier::cutoffMode, &CreateBondsModifier::setCutoffMode)
			.add_property("cutoff", &CreateBondsModifier::uniformCutoff, &CreateBondsModifier::setUniformCutoff,
					"The cutoff distance for the creation of bonds between particles."
					"\n\n"
					":Default: 3.2\n")
			.add_property("bonds_display", make_function(&CreateBondsModifier::bondsDisplay, return_value_policy<ovito_object_reference>()),
					"A :py:class:`~ovito.vis.BondsDisplay` instance controlling the visual appearance of the bonds created by this modifier.")
		;

		enum_<CreateBondsModifier::CutoffMode>("CutoffMode")
			.value("Uniform", CreateBondsModifier::UniformCutoff)
			.value("Pair", CreateBondsModifier::PairCutoff)
		;
	}

	ovito_class<CentroSymmetryModifier, AsynchronousParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Computes the centro-symmetry parameter (CSP) of each particle."
			"\n\n"
			"The modifier outputs the computed values in the ``\"Centrosymmetry\"`` particle property.")
		.add_property("num_neighbors", &CentroSymmetryModifier::numNeighbors, &CentroSymmetryModifier::setNumNeighbors,
				"The number of neighbors to take into account (12 for FCC crystals, 8 for BCC crystals)."
				"\n\n"
				":Default: 12\n")
	;

	ovito_class<ClusterAnalysisModifier, AsynchronousParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Groups particles into clusters."
			"\n\n"
			"The modifier stores the assigned cluster IDs in the ``\"Cluster\"`` particle property.")
		.add_property("cutoff", &ClusterAnalysisModifier::cutoff, &ClusterAnalysisModifier::setCutoff,
				"The cutoff radius used when forming clusters."
				"\n\n"
				":Default: 3.2\n")
		.add_property("count", &ClusterAnalysisModifier::clusterCount,
				"This output field contains the number of clusters found. "
				"Note that accessing this value is only possible after the modifier has computed its results. "
				"Thus, you have to call :py:meth:`ovito.ObjectNode.compute` first to ensure that this information is up to date. ")
	;

	ovito_class<CoordinationNumberModifier, AsynchronousParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Computes coordination numbers of particles and the radial distribution function (RDF) of the system."
			"\n\n"
			"The modifier stores the computed coordination numbers in the ``\"Coordination\"`` particle property.")
		.add_property("cutoff", &CoordinationNumberModifier::cutoff, &CoordinationNumberModifier::setCutoff,
				"The neighbor cutoff distance."
				"\n\n"
				":Default: 3.2\n")
		.add_property("rdf_x", make_function(&CoordinationNumberModifier::rdfX, return_internal_reference<>()))
		.add_property("rdf_y", make_function(&CoordinationNumberModifier::rdfY, return_internal_reference<>()))
	;

	ovito_class<CalculateDisplacementsModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Computes the displacement vectors of particles based on a separate reference configuration. "
			"The modifier requires you to load a reference configuration from an external file::"
			"\n\n"
			"    from ovito.modifiers import *\n"
			"    \n"
			"    modifier = CalculateDisplacementsModifier()\n"
			"    modifier.reference.load(\"frame0000.dump\")\n"
			"\n\n"
			"The modifier stores the computed displacement vectors in the ``\"Displacement\"`` particle property. "
			"The displacement magnitudes are stored in the ``\"Displacement Magnitude\"`` property. ")
		.add_property("reference", make_function(&CalculateDisplacementsModifier::referenceConfiguration, return_value_policy<ovito_object_reference>()), &CalculateDisplacementsModifier::setReferenceConfiguration,
				"A :py:class:`~ovito.io.FileSource` that provides the reference positions of particles. "
				"You can call its :py:meth:`~ovito.io.FileSource.load` function to load a reference simulation file "
				"as shown in the code example above.")
		.add_property("eliminate_cell_deformation", &CalculateDisplacementsModifier::eliminateCellDeformation, &CalculateDisplacementsModifier::setEliminateCellDeformation,
				"Boolean flag that controls the elimination of the affine cell deformation prior to calculating the "
				"displacement vectors."
				"\n\n"
				":Default: ``False``\n")
		.add_property("assume_unwrapped_coordinates", &CalculateDisplacementsModifier::assumeUnwrappedCoordinates, &CalculateDisplacementsModifier::setAssumeUnwrappedCoordinates,
				"If ``True``, the particle coordinates of the reference and of the current configuration are taken as is. "
				"If ``False``, the minimum image convention is used to deal with particles that have crossed a periodic boundary. "
				"\n\n"
				":Default: ``False``\n")
		.add_property("reference_frame", &CalculateDisplacementsModifier::referenceFrameNumber, &CalculateDisplacementsModifier::setReferenceFrameNumber,
				"The frame number to use as reference configuration if the reference data comprises multiple "
				"simulation frames. Only used if ``use_frame_offset==False``."
				"\n\n"
				":Default: 0\n")
		.add_property("use_frame_offset", &CalculateDisplacementsModifier::useReferenceFrameOffset, &CalculateDisplacementsModifier::setUseReferenceFrameOffset,
				"Determines whether a sliding reference configuration is taken at a constant time offset (specified by :py:attr:`.frame_offset`) "
				"relative to the current frame. If ``False``, a constant reference configuration is used (set by the :py:attr:`.reference_frame` parameter) "
				"irrespective of the current frame."
				"\n\n"
				":Default: ``False``\n")
		.add_property("frame_offset", &CalculateDisplacementsModifier::referenceFrameOffset, &CalculateDisplacementsModifier::setReferenceFrameOffset,
				"The relative frame offset when using a sliding reference configuration (``use_frame_offset==True``)."
				"\n\n"
				":Default: -1\n")
		.add_property("vector_display", make_function(&CalculateDisplacementsModifier::vectorDisplay, return_value_policy<ovito_object_reference>()),
				"A :py:class:`~ovito.vis.VectorDisplay` instance controlling the visual representation of the computed "
				"displacement vectors. \n"
				"Note that the computed displacement vectors are not shown by default. You can enable "
				"the arrow display as follows::"
				"\n\n"
				"   modifier = CalculateDisplacementsModifier()\n"
				"   modifier.vector_display.enabled = True\n"
				"   modifier.vector_display.color = (0,0,0)\n"
				"\n")
	;

	ovito_class<HistogramModifier, ParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Generates a histogram from the values of a particle property. "
			"\n\n"
			"The value range of the histogram is determined automatically from the minimum and maximum values of the selected property "
			"unless :py:attr:`.fix_xrange` is set to ``True``. In this case the range of the histogram is controlled by the "
			":py:attr:`.xrange_start` and :py:attr:`.xrange_end` parameters."
			"\n\n"
			"Example::"
			"\n\n"
			"    from ovito.modifiers import *\n"
			"    modifier = HistogramModifier(bin_count=100, property=\"Potential Energy\")\n"
			"    node.modifiers.append(modifier)\n"
			"    node.compute()\n"
			"    \n"
			"    import numpy\n"
			"    numpy.savetxt(\"histogram.txt\", modifier.histogram)\n"
			"\n")
		.add_property("property", make_function(&HistogramModifier::sourceProperty, return_value_policy<copy_const_reference>()), &HistogramModifier::setSourceProperty,
				"The name of the input particle property for which to compute the histogram. "
				"This can be one of the :ref:`standard particle properties <particle-types-list>` or a custom particle property. "
				"For vector properties a specific component name must be included in the string, e.g. ``\"Velocity.X\"``. ")
		.add_property("bin_count", &HistogramModifier::numberOfBins, &HistogramModifier::setNumberOfBins,
				"The number of histogram bins."
				"\n\n"
				":Default: 200\n")
		.add_property("fix_xrange", &HistogramModifier::fixXAxisRange, &HistogramModifier::setFixXAxisRange,
				"Controls how the value range of the histogram is determined. If false, the range is chosen automatically by the modifier to include "
				"all particle property values. If true, the range is specified manually using the :py:attr:`.xrange_start` and :py:attr:`.xrange_end` attributes."
				"\n\n"
				":Default: ``False``\n")
		.add_property("xrange_start", &HistogramModifier::xAxisRangeStart, &HistogramModifier::setXAxisRangeStart,
				"If :py:attr:`.fix_xrange` is true, then this specifies the lower end of the value range covered by the histogram."
				"\n\n"
				":Default: 0.0\n")
		.add_property("xrange_end", &HistogramModifier::xAxisRangeEnd, &HistogramModifier::setXAxisRangeEnd,
				"If :py:attr:`.fix_xrange` is true, then this specifies the upper end of the value range covered by the histogram."
				"\n\n"
				":Default: 0.0\n")
		.add_property("histogramData", make_function(&HistogramModifier::histogramData, return_internal_reference<>()))
	;

	ovito_class<ScatterPlotModifier, ParticleModifier>()
		.add_property("xAxisProperty", make_function(&ScatterPlotModifier::xAxisProperty, return_value_policy<copy_const_reference>()), &ScatterPlotModifier::setXAxisProperty)
		.add_property("yAxisProperty", make_function(&ScatterPlotModifier::yAxisProperty, return_value_policy<copy_const_reference>()), &ScatterPlotModifier::setYAxisProperty)
	;

	ovito_class<AtomicStrainModifier, AsynchronousParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Computes the atomic strain tensors of particles based on a separate reference configuration. "
			"The modifier requires you to load a reference configuration from an external file::"
			"\n\n"
			"    from ovito.modifiers import *\n"
			"    \n"
			"    modifier = AtomicStrainModifier()\n"
			"    modifier.reference.load(\"frame0000.dump\")\n"
			"\n\n"
			"The modifier stores the computed per-particle strain tensors in the ``\"Strain Tensor\"`` particle property (only if :py:attr:`.output_strain_tensors` == True). "
			"The computed deformation gradient tensors are output in the  ``\"Deformation Gradient\"`` particle property (only if :py:attr:`.output_deformation_gradients` == True). "
			"The von Mises shear strain invariants are stored in the ``\"Shear Strain\"`` property. "
			"The hydrostatic component of the strain tensors gets stored in the ``\"Volumetric Strain\"`` property. ")
		.add_property("reference", make_function(&AtomicStrainModifier::referenceConfiguration, return_value_policy<ovito_object_reference>()), &AtomicStrainModifier::setReferenceConfiguration,
				"A :py:class:`~ovito.io.FileSource` that provides the reference positions of particles. "
				"You can call its :py:meth:`~ovito.io.FileSource.load` function to load a reference simulation file "
				"as shown in the code example above.")
		.add_property("eliminate_cell_deformation", &AtomicStrainModifier::eliminateCellDeformation, &AtomicStrainModifier::setEliminateCellDeformation,
				"Boolean flag that controls the elimination of the affine cell deformation prior to calculating the "
				"local strain."
				"\n\n"
				":Default: ``False``\n")
		.add_property("assume_unwrapped_coordinates", &AtomicStrainModifier::assumeUnwrappedCoordinates, &AtomicStrainModifier::setAssumeUnwrappedCoordinates,
				"If ``True``, the particle coordinates of the reference and of the current configuration are taken as is. "
				"If ``False``, the minimum image convention is used to deal with particles that have crossed a periodic boundary. "
				"\n\n"
				":Default: ``False``\n")
		.add_property("use_frame_offset", &AtomicStrainModifier::useReferenceFrameOffset, &AtomicStrainModifier::setUseReferenceFrameOffset,
				"Determines whether a sliding reference configuration is taken at a constant time offset (specified by :py:attr:`.frame_offset`) "
				"relative to the current frame. If ``False``, a constant reference configuration is used (set by the :py:attr:`.reference_frame` parameter) "
				"irrespective of the current frame."
				"\n\n"
				":Default: ``False``\n")
		.add_property("reference_frame", &AtomicStrainModifier::referenceFrameNumber, &AtomicStrainModifier::setReferenceFrameNumber,
				"The frame number to use as reference configuration if the reference data comprises multiple "
				"simulation frames. Only used if ``use_frame_offset==False``."
				"\n\n"
				":Default: 0\n")
		.add_property("frame_offset", &AtomicStrainModifier::referenceFrameOffset, &AtomicStrainModifier::setReferenceFrameOffset,
				"The relative frame offset when using a sliding reference configuration (``use_frame_offset==True``)."
				"\n\n"
				":Default: -1\n")
		.add_property("cutoff", &AtomicStrainModifier::cutoff, &AtomicStrainModifier::setCutoff,
				"Sets the distance up to which neighbor atoms are taken into account in the local strain calculation."
				"\n\n"
				":Default: 3.0\n")
		.add_property("output_deformation_gradients", &AtomicStrainModifier::calculateDeformationGradients, &AtomicStrainModifier::setCalculateDeformationGradients,
				"Controls the output of the per-particle deformation gradient tensors. If ``False``, the computed tensors are not output as a particle property to save memory."
				"\n\n"
				":Default: ``False``\n")
		.add_property("output_strain_tensors", &AtomicStrainModifier::calculateStrainTensors, &AtomicStrainModifier::setCalculateStrainTensors,
				"Controls the output of the per-particle strain tensors tensors. If ``False``, the computed strain tensors are not output as a particle property to save memory."
				"\n\n"
				":Default: ``False``\n")
		.add_property("output_nonaffine_squared_displacements", &AtomicStrainModifier::calculateNonaffineSquaredDisplacements, &AtomicStrainModifier::setCalculateNonaffineSquaredDisplacements,
				"Enables the computation of the squared magnitude of the non-affine part of the atomic displacements. The computed values are output in the ``\"Nonaffine Squared Displacement\"`` particle property."
				"\n\n"
				":Default: ``False``\n")
		.add_property("select_invalid_particles", &AtomicStrainModifier::selectInvalidParticles, &AtomicStrainModifier::setSelectInvalidParticles,
				"If ``True``, the modifier selects all particle for which the local strain tensor could not be computed (because of an insufficient number of neighbors within the cutoff)."
				"\n\n"
				":Default: ``True``\n")
		.add_property("invalid_particle_count", &AtomicStrainModifier::invalidParticleCount,
				"After the modifier has computed the atomic strain tensors this field contains the number of particles "
				"for which the strain calculation failed. "
				"Note that accessing this value is only possible after the modifier has computed its results. "
				"Thus, you have to call :py:meth:`ovito.ObjectNode.compute` first to ensure that this information is up to date. ")
	;

	ovito_class<WignerSeitzAnalysisModifier, AsynchronousParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Performs the Wigner-Seitz cell analysis to identify point defects in crystals. "
			"The modifier requires you to load a reference configuration from an external file::"
			"\n\n"
			"    from ovito.modifiers import *\n"
			"    \n"
			"    mod = WignerSeitzAnalysisModifier()\n"
			"    mod.reference.load(\"frame0000.dump\")\n"
			"    node.modifiers.append(mod)\n"
			"    node.compute()\n"
			"    print(\"Number of vacant sites: %i\" % mod.vacancy_count)\n"
			"\n\n"
			"The modifier stores the computed occupation numbers in the ``\"Occupancy\"`` particle property. "
			"The number of vacancies and the number of interstitial sites found by the modifier are reported in "
			"the :py:attr:`.vacancy_count` and :py:attr:`.interstitial_count` output fields.")
		.add_property("reference", make_function(&WignerSeitzAnalysisModifier::referenceConfiguration, return_value_policy<ovito_object_reference>()), &WignerSeitzAnalysisModifier::setReferenceConfiguration,
				"A :py:class:`~ovito.io.FileSource` that provides the reference positions of particles. "
				"You can call its :py:meth:`~ovito.io.FileSource.load` function to load a reference simulation file "
				"as shown in the code example above.")
		.add_property("eliminate_cell_deformation", &WignerSeitzAnalysisModifier::eliminateCellDeformation, &WignerSeitzAnalysisModifier::setEliminateCellDeformation,
				"Boolean flag that controls the elimination of the affine cell deformation prior to performing the analysis."
				"\n\n"
				":Default: ``False``\n")
		.add_property("use_frame_offset", &WignerSeitzAnalysisModifier::useReferenceFrameOffset, &WignerSeitzAnalysisModifier::setUseReferenceFrameOffset,
				"Determines whether a sliding reference configuration is taken at a constant time offset (specified by :py:attr:`.frame_offset`) "
				"relative to the current frame. If ``False``, a constant reference configuration is used (set by the :py:attr:`.reference_frame` parameter) "
				"irrespective of the current frame."
				"\n\n"
				":Default: ``False``\n")
		.add_property("reference_frame", &WignerSeitzAnalysisModifier::referenceFrameNumber, &WignerSeitzAnalysisModifier::setReferenceFrameNumber,
				"The frame number to use as reference configuration if the reference data comprises multiple "
				"simulation frames. Only used if ``use_frame_offset==False``."
				"\n\n"
				":Default: 0\n")
		.add_property("frame_offset", &WignerSeitzAnalysisModifier::referenceFrameOffset, &WignerSeitzAnalysisModifier::setReferenceFrameOffset,
				"The relative frame offset when using a sliding reference configuration (``use_frame_offset==True``)."
				"\n\n"
				":Default: -1\n")
		.add_property("vacancy_count", &WignerSeitzAnalysisModifier::vacancyCount,
				"After the modifier has performed the analysis, this field contains the number of vacant sites. "
				"Note that accessing this value is only possible after the modifier has computed its results. "
				"Thus, you have to call :py:meth:`ovito.ObjectNode.compute` first to ensure that this information is up to date. ")
		.add_property("interstitial_count", &WignerSeitzAnalysisModifier::interstitialCount,
				"After the modifier has performed the analysis, this field contains the number of interstitial atoms. "
				"Note that accessing this value is only possible after the modifier has computed its results. "
				"Thus, you have to call :py:meth:`ovito.ObjectNode.compute` first to ensure that this information is up to date. ")
	;

	ovito_class<VoronoiAnalysisModifier, AsynchronousParticleModifier>(
			":Base: :py:class:`ovito.modifiers.Modifier`\n\n"
			"Computes the atomic volumes and coordination numbers using a Voronoi tessellation of the particle system."
			"\n\n"
			"The modifier stores the computed per-particle volume in the ``\"Atomic Volume\"`` particle property and the number of neighbors "
			"of each particle in the ``\"Coordination\"`` property.")
		.add_property("only_selected", &VoronoiAnalysisModifier::onlySelected, &VoronoiAnalysisModifier::setOnlySelected,
				"Lets the modifier perform the analysis only for selected particles. Particles that are not selected will be treated as if they did not exist."
				"\n\n"
				":Default: ``False``\n")
		.add_property("use_radii", &VoronoiAnalysisModifier::useRadii, &VoronoiAnalysisModifier::setUseRadii,
				"If ``True``, the modifier computes the poly-disperse Voronoi tessellation, which takes into account the radii of particles. "
				"Otherwise a mono-disperse Voronoi tessellation is computed, which is independent of the particle sizes. "
				"\n\n"
				":Default: ``False``\n")
		.add_property("face_threshold", &VoronoiAnalysisModifier::faceThreshold, &VoronoiAnalysisModifier::setFaceThreshold,
				"Specifies a minimum area for faces of a Voronoi cell. The modifier will ignore any Voronoi cell faces with an area smaller than this "
				"threshold when computing the coordination number and the Voronoi index of particles."
				"\n\n"
				":Default: 0.0\n")
		.add_property("edge_threshold", &VoronoiAnalysisModifier::edgeThreshold, &VoronoiAnalysisModifier::setEdgeThreshold,
				"Specifies the minimum length an edge must have to be considered in the Voronoi index calculation. Edges that are shorter "
				"than this threshold will be ignored when counting the number of edges of a Voronoi face."
				"\n\n"
				":Default: 0.0\n")
		.add_property("compute_indices", &VoronoiAnalysisModifier::computeIndices, &VoronoiAnalysisModifier::setComputeIndices,
				"If ``True``, the modifier calculates the Voronoi indices of particles. The modifier stores the computed indices in a vector particle property "
				"named ``Voronoi Index``. The *i*-th component of this property will contain the number of faces of the "
				"Voronoi cell that have *i* edges. Thus, the first two components of the per-particle vector will always be zero, because the minimum "
				"number of edges a polygon can have is three. "
				"\n\n"
				":Default: ``False``\n")
		.add_property("edge_count", &VoronoiAnalysisModifier::edgeCount, &VoronoiAnalysisModifier::setEdgeCount,
				"Integer parameter controlling the order up to which Voronoi indices are computed by the modifier. "
				"Any Voronoi face with more edges than this maximum value will not be counted! Computed Voronoi index vectors are truncated at the index specified by :py:attr:`.edge_count`. "
				"\n\n"
				"See the :py:attr:`.max_face_order` output property on how to avoid truncated Voronoi index vectors."
				"\n\n"
				"This parameter is ignored if :py:attr:`.compute_indices` is false."
				"\n\n"
				":Minimum: 3\n"
				":Default: 6\n")
		.add_property("max_face_order", &VoronoiAnalysisModifier::maxFaceOrder,
				"This is an output value computed by the modifier, which reports the maximum number of edges of any face in the computed Voronoi tessellation "
				"(ignoring edges and faces that are below the area and length thresholds)."
				"\n\n"
				"Note that accessing this property is only possible after the modifier has computed the Voronoi tessellation, i.e. after "
				"the modification pipeline has been evaluated. "
				"That means you have to call :py:meth:`ovito.ObjectNode.compute` first to ensure that this information is up to date."
				"\n\n"
				"Note that, if calculation of Voronoi indices is enabled (:py:attr:`.compute_indices` == true), and :py:attr:`.edge_count` < :py:attr:`.max_face_order`, then "
				"the computed Voronoi index vectors will be truncated because there exists at least one Voronoi face having more edges than "
				"the maximum Voronoi vector length specified by :py:attr:`.edge_count`. In such a case you should consider increasing "
				":py:attr:`.edge_count` (to at least :py:attr:`.max_face_order`) to not lose information because of truncated index vectors.")
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(ParticlesModify);

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
