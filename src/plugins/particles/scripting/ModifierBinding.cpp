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
#include <core/scene/pipeline/ModifierApplication.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/ParticlePropertyObject.h>
#include <plugins/particles/data/ParticleTypeProperty.h>
#include <plugins/particles/modifier/ParticleModifier.h>
#include <plugins/particles/modifier/AsynchronousParticleModifier.h>
#include <plugins/particles/modifier/coloring/AssignColorModifier.h>
#include <plugins/particles/modifier/coloring/ColorCodingModifier.h>
#include <plugins/particles/modifier/coloring/AmbientOcclusionModifier.h>
#include <plugins/particles/modifier/delete/DeleteParticlesModifier.h>
#include <plugins/particles/modifier/pbc/ShowPeriodicImagesModifier.h>
#include <plugins/particles/modifier/pbc/WrapPeriodicImagesModifier.h>
#include <plugins/particles/modifier/properties/CreateExpressionPropertyModifier.h>
#include <plugins/particles/modifier/properties/FreezePropertyModifier.h>
#include <plugins/particles/modifier/selection/ClearSelectionModifier.h>
#include <plugins/particles/modifier/selection/FreezeSelectionModifier.h>
#include <plugins/particles/modifier/selection/InvertSelectionModifier.h>
#include <plugins/particles/modifier/selection/ManualSelectionModifier.h>
#include <plugins/particles/modifier/selection/SelectExpressionModifier.h>
#include <plugins/particles/modifier/selection/SelectParticleTypeModifier.h>
#include <plugins/particles/modifier/slice/SliceModifier.h>
#include <plugins/particles/modifier/transformation/AffineTransformationModifier.h>
#include <plugins/particles/modifier/analysis/StructureIdentificationModifier.h>
#include <plugins/particles/modifier/analysis/binandreduce/BinAndReduceModifier.h>
#include <plugins/particles/modifier/analysis/bondangle/BondAngleAnalysisModifier.h>
#include <plugins/particles/modifier/analysis/cna/CommonNeighborAnalysisModifier.h>
#include <plugins/particles/modifier/analysis/bonds/CreateBondsModifier.h>
#include <plugins/particles/modifier/analysis/centrosymmetry/CentroSymmetryModifier.h>
#include <plugins/particles/modifier/analysis/cluster/ClusterAnalysisModifier.h>
#include <plugins/particles/modifier/analysis/coordination/CoordinationNumberModifier.h>
#include <plugins/particles/modifier/analysis/displacements/CalculateDisplacementsModifier.h>
#include <plugins/particles/modifier/analysis/histogram/HistogramModifier.h>
#include <plugins/particles/modifier/analysis/scatterplot/ScatterPlotModifier.h>
#include <plugins/particles/modifier/analysis/strain/AtomicStrainModifier.h>
#include <plugins/particles/modifier/analysis/wignerseitz/WignerSeitzAnalysisModifier.h>

namespace Particles {

using namespace boost::python;
using namespace Ovito;
using namespace PyScript;

BOOST_PYTHON_MODULE(ParticlesModify)
{

	ovito_abstract_class<ParticleModifier, Modifier>()
	;

	ovito_abstract_class<AsynchronousParticleModifier, ParticleModifier>()
		.add_property("autoUpdateEnabled", &AsynchronousParticleModifier::autoUpdateEnabled, &AsynchronousParticleModifier::setAutoUpdateEnabled)
		.add_property("storeResultsWithScene", &AsynchronousParticleModifier::storeResultsWithScene, &AsynchronousParticleModifier::setStoreResultsWithScene)
	;

	ovito_class<AssignColorModifier, ParticleModifier>(
			"Assigns a color to the selected particles.")
		.add_property("color", &AssignColorModifier::color, &AssignColorModifier::setColor,
				"The color that will be assigned to particles."
				"\n\n"
				"Default: ``(0.3,0.3,1.0)``\n")
		.add_property("colorController", make_function(&AssignColorModifier::colorController, return_value_policy<ovito_object_reference>()), &AssignColorModifier::setColorController)
		.add_property("keepSelection", &AssignColorModifier::keepSelection, &AssignColorModifier::setKeepSelection,
				"If false, the modifier resets the selection after coloring the selected particles."
				"\n\n"
				"Default: ``False``")
	;

	ovito_class<ColorCodingModifier, ParticleModifier>(
			"Colors particles based on the value of an arbitrary particle property."
			"\n\n"
			"Usage example::"
			"\n\n"
			"    from ovito.modify.particles import *\n"
			"    \n"
			"    modifier = ColorCodingModifier(\n"
			"        sourceProperty = \"Potential Energy\",\n"
			"        colorGradient = ColorCodingHotGradient()\n"
			"    )\n"
			"    node.modifiers.append(modifier)\n"
			"\n"
			"If, as in the example above, the :py:attr:`.startValue` and :py:attr:`.endValue` parameters are not explicitly set, "
			"then the modifier automatically adjusts them to the minimum and maximum values of the particle property when the modifier "
			"is inserted into the modification pipeline.")
		.add_property("sourceProperty", make_function(&ColorCodingModifier::sourceProperty, return_value_policy<copy_const_reference>()), &ColorCodingModifier::setSourceProperty,
				"The input particle property that determines the color of particles.")
		.add_property("startValue", &ColorCodingModifier::startValue, &ColorCodingModifier::setStartValue,
				"This determines the value range when mapping the input property to a color.")
		.add_property("startValueController", make_function(&ColorCodingModifier::startValueController, return_value_policy<ovito_object_reference>()), &ColorCodingModifier::setStartValueController)
		.add_property("endValue", &ColorCodingModifier::endValue, &ColorCodingModifier::setEndValue,
				"This determines the value range when mapping the input property to a color.")
		.add_property("endValueController", make_function(&ColorCodingModifier::endValueController, return_value_policy<ovito_object_reference>()), &ColorCodingModifier::setEndValueController)
		.add_property("colorGradient", make_function(&ColorCodingModifier::colorGradient, return_value_policy<ovito_object_reference>()), &ColorCodingModifier::setColorGradient,
				"The color gradient object, which is responsible for mapping normalized property values to colors."
				"\n\n"
				"Available gradient types are:\n"
				" * ``ColorCodingHSVGradient`` -- Rainbow (default)\n"
				" * ``ColorCodingGrayscaleGradient`` -- Grayscale\n"
				" * ``ColorCodingHotGradient`` -- Hot\n"
				" * ``ColorCodingJetGradient`` -- Jet\n"
				"\n")
		.add_property("colorOnlySelected", &ColorCodingModifier::colorOnlySelected, &ColorCodingModifier::setColorOnlySelected,
				"If true, only selected particles will be assigned a color by the modifier; if false, all particles will be colored."
				"\n\n"
				"Default: ``False``\n")
		.add_property("keepSelection", &ColorCodingModifier::keepSelection, &ColorCodingModifier::setKeepSelection,
				"If :py:attr:`.colorOnlySelected` == ``True`` and this property is false, then the modifier resets the particle selection after coloring the particles."
				"\n\n"
				"Default: ``False``")
		.add_property("renderLegend", &ColorCodingModifier::renderLegend, &ColorCodingModifier::setRenderLegend)
		.add_property("legendViewport", make_function(&ColorCodingModifier::legendViewport, return_value_policy<ovito_object_reference>()), &ColorCodingModifier::setLegendViewport)
	;

	ovito_abstract_class<ColorCodingGradient, RefTarget>()
		.def("valueToColor", pure_virtual(&ColorCodingGradient::valueToColor))
	;

	ovito_class<ColorCodingHSVGradient, ColorCodingGradient>()
	;
	ovito_class<ColorCodingGrayscaleGradient, ColorCodingGradient>()
	;
	ovito_class<ColorCodingHotGradient, ColorCodingGradient>()
	;
	ovito_class<ColorCodingJetGradient, ColorCodingGradient>()
	;

	ovito_class<AmbientOcclusionModifier, AsynchronousParticleModifier>()
		.add_property("intensity", &AmbientOcclusionModifier::intensity, &AmbientOcclusionModifier::setIntensity)
		.add_property("samplingCount", &AmbientOcclusionModifier::samplingCount, &AmbientOcclusionModifier::setSamplingCount)
		.add_property("bufferResolution", &AmbientOcclusionModifier::bufferResolution, &AmbientOcclusionModifier::setBufferResolution)
	;

	ovito_class<DeleteParticlesModifier, ParticleModifier>()
	;

	ovito_class<ShowPeriodicImagesModifier, ParticleModifier>()
		.add_property("showImageX", &ShowPeriodicImagesModifier::showImageX, &ShowPeriodicImagesModifier::setShowImageX)
		.add_property("showImageY", &ShowPeriodicImagesModifier::showImageY, &ShowPeriodicImagesModifier::setShowImageY)
		.add_property("showImageZ", &ShowPeriodicImagesModifier::showImageZ, &ShowPeriodicImagesModifier::setShowImageZ)
		.add_property("numImagesX", &ShowPeriodicImagesModifier::numImagesX, &ShowPeriodicImagesModifier::setNumImagesX)
		.add_property("numImagesY", &ShowPeriodicImagesModifier::numImagesY, &ShowPeriodicImagesModifier::setNumImagesY)
		.add_property("numImagesZ", &ShowPeriodicImagesModifier::numImagesZ, &ShowPeriodicImagesModifier::setNumImagesZ)
		.add_property("adjustBoxSize", &ShowPeriodicImagesModifier::adjustBoxSize, &ShowPeriodicImagesModifier::setAdjustBoxSize)
	;

	ovito_class<WrapPeriodicImagesModifier, ParticleModifier>()
	;

	ovito_class<CreateExpressionPropertyModifier, ParticleModifier>()
	// Requires binding for QStringList
		.add_property("expressions", make_function(&CreateExpressionPropertyModifier::expressions, return_value_policy<copy_const_reference>()), &CreateExpressionPropertyModifier::setExpressions)
		.add_property("outputProperty", make_function(&CreateExpressionPropertyModifier::outputProperty, return_value_policy<copy_const_reference>()), &CreateExpressionPropertyModifier::setOutputProperty)
		.add_property("propertyComponentCount", &CreateExpressionPropertyModifier::propertyComponentCount, &CreateExpressionPropertyModifier::setPropertyComponentCount)
		.add_property("onlySelectedParticles", &CreateExpressionPropertyModifier::onlySelectedParticles, &CreateExpressionPropertyModifier::setOnlySelectedParticles)
	;

	ovito_class<FreezePropertyModifier, ParticleModifier>()
		.add_property("sourceProperty", make_function(&FreezePropertyModifier::sourceProperty, return_value_policy<copy_const_reference>()), &FreezePropertyModifier::setSourceProperty)
		.add_property("destinationProperty", make_function(&FreezePropertyModifier::destinationProperty, return_value_policy<copy_const_reference>()), &FreezePropertyModifier::setDestinationProperty)
		.def("takePropertySnapshot", &FreezePropertyModifier::takePropertySnapshot)
	;

	ovito_class<ClearSelectionModifier, ParticleModifier>()
	;

	ovito_class<InvertSelectionModifier, ParticleModifier>()
	;

	ovito_class<FreezeSelectionModifier, ParticleModifier>()
		.def("takeSelectionSnapshot", &FreezeSelectionModifier::takeSelectionSnapshot)
	;

	ovito_class<ManualSelectionModifier, ParticleModifier>()
		.def("resetSelection", &ManualSelectionModifier::resetSelection)
		.def("selectAll", &ManualSelectionModifier::selectAll)
		.def("clearSelection", &ManualSelectionModifier::clearSelection)
		.def("toggleParticleSelection", &ManualSelectionModifier::toggleParticleSelection)
	;

	ovito_class<SelectExpressionModifier, ParticleModifier>()
		.add_property("expression", make_function(&SelectExpressionModifier::expression, return_value_policy<copy_const_reference>()), &SelectExpressionModifier::setExpression)
	;

	ovito_class<SelectParticleTypeModifier, ParticleModifier>()
		.add_property("sourceProperty", make_function(&SelectParticleTypeModifier::sourceProperty, return_value_policy<copy_const_reference>()), &SelectParticleTypeModifier::setSourceProperty)
	// Requires binding for QSet<int>
		.add_property("selectedParticleTypes", make_function(&SelectParticleTypeModifier::selectedParticleTypes, return_value_policy<copy_const_reference>()), &SelectParticleTypeModifier::setSelectedParticleTypes)
	;

	ovito_class<SliceModifier, ParticleModifier>()
		.add_property("distance", &SliceModifier::distance, &SliceModifier::setDistance)
		.add_property("distanceController", make_function(&SliceModifier::distanceController, return_value_policy<ovito_object_reference>()), &SliceModifier::setDistanceController)
		.add_property("normal", &SliceModifier::normal, &SliceModifier::setNormal)
		.add_property("normalController", make_function(&SliceModifier::normalController, return_value_policy<ovito_object_reference>()), &SliceModifier::setNormalController)
		.add_property("sliceWidth", &SliceModifier::sliceWidth, &SliceModifier::setSliceWidth)
		.add_property("sliceWidthController", make_function(&SliceModifier::sliceWidthController, return_value_policy<ovito_object_reference>()), &SliceModifier::setSliceWidthController)
		.add_property("inverse", &SliceModifier::inverse, &SliceModifier::setInverse)
		.add_property("createSelection", &SliceModifier::createSelection, &SliceModifier::setCreateSelection)
		.add_property("applyToSelection", &SliceModifier::applyToSelection, &SliceModifier::setApplyToSelection)
	;

	ovito_class<AffineTransformationModifier, ParticleModifier>()
		.add_property("transformation", make_function(&AffineTransformationModifier::transformation, return_value_policy<copy_const_reference>()), &AffineTransformationModifier::setTransformation)
		.add_property("targetCell", make_function(&AffineTransformationModifier::targetCell, return_value_policy<copy_const_reference>()), &AffineTransformationModifier::setTargetCell)
		.add_property("relativeMode", &AffineTransformationModifier::relativeMode, &AffineTransformationModifier::setRelativeMode)
		.add_property("applyToParticles", &AffineTransformationModifier::applyToParticles, &AffineTransformationModifier::setApplyToParticles)
		.add_property("selectionOnly", &AffineTransformationModifier::selectionOnly, &AffineTransformationModifier::selectionOnly)
		.add_property("applyToSimulationBox", &AffineTransformationModifier::applyToSimulationBox, &AffineTransformationModifier::setApplyToSimulationBox)
		.add_property("applyToSurfaceMesh", &AffineTransformationModifier::applyToSurfaceMesh, &AffineTransformationModifier::setApplyToSurfaceMesh)
	;

	ovito_class<BinAndReduceModifier, ParticleModifier>()
		.add_property("sourceProperty", make_function(&BinAndReduceModifier::sourceProperty, return_value_policy<copy_const_reference>()), &BinAndReduceModifier::setSourceProperty)
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
		.add_property("structureCounts", make_function(&StructureIdentificationModifier::structureCounts, return_value_policy<copy_const_reference>()))
	;

	{
		scope s = ovito_class<BondAngleAnalysisModifier, StructureIdentificationModifier>()
		;

		enum_<BondAngleAnalysisModifier::StructureType>("StructureTypes")
			.value("OTHER", BondAngleAnalysisModifier::OTHER)
			.value("FCC", BondAngleAnalysisModifier::FCC)
			.value("HCP", BondAngleAnalysisModifier::HCP)
			.value("BCC", BondAngleAnalysisModifier::BCC)
			.value("ICO", BondAngleAnalysisModifier::ICO)
		;
	}

	{
		scope s = ovito_class<CommonNeighborAnalysisModifier, StructureIdentificationModifier>()
			.add_property("cutoff", &CommonNeighborAnalysisModifier::cutoff, &CommonNeighborAnalysisModifier::setCutoff)
			.add_property("adaptiveMode", &CommonNeighborAnalysisModifier::adaptiveMode, &CommonNeighborAnalysisModifier::setAdaptiveMode)
		;

		enum_<CommonNeighborAnalysisModifier::StructureType>("StructureTypes")
			.value("OTHER", CommonNeighborAnalysisModifier::OTHER)
			.value("FCC", CommonNeighborAnalysisModifier::FCC)
			.value("HCP", CommonNeighborAnalysisModifier::HCP)
			.value("BCC", CommonNeighborAnalysisModifier::BCC)
			.value("ICO", CommonNeighborAnalysisModifier::ICO)
			.value("DIA", CommonNeighborAnalysisModifier::DIA)
		;
	}

	{
		scope s = ovito_class<CreateBondsModifier, AsynchronousParticleModifier>()
			.add_property("cutoffMode", &CreateBondsModifier::cutoffMode, &CreateBondsModifier::setCutoffMode)
			.add_property("uniformCutoff", &CreateBondsModifier::uniformCutoff, &CreateBondsModifier::setUniformCutoff)
			.add_property("bondsDisplay", make_function(&CreateBondsModifier::bondsDisplay, return_value_policy<ovito_object_reference>()))
			.add_property("bondsObject", make_function(&CreateBondsModifier::bondsObject, return_value_policy<ovito_object_reference>()))
		;

		enum_<CreateBondsModifier::CutoffMode>("CutoffMode")
			.value("Uniform", CreateBondsModifier::UniformCutoff)
			.value("Pair", CreateBondsModifier::PairCutoff)
		;
	}

	ovito_class<CentroSymmetryModifier, AsynchronousParticleModifier>()
		.add_property("numNeighbors", &CentroSymmetryModifier::numNeighbors, &CentroSymmetryModifier::setNumNeighbors)
	;

	ovito_class<ClusterAnalysisModifier, AsynchronousParticleModifier>()
		.add_property("cutoff", &ClusterAnalysisModifier::cutoff, &ClusterAnalysisModifier::setCutoff)
		.add_property("clusterCount", &ClusterAnalysisModifier::clusterCount)
	;

	ovito_class<CoordinationNumberModifier, AsynchronousParticleModifier>()
		.add_property("cutoff", &CoordinationNumberModifier::cutoff, &CoordinationNumberModifier::setCutoff)
	;

	ovito_class<CalculateDisplacementsModifier, ParticleModifier>()
		.add_property("referenceConfiguration", make_function(&CalculateDisplacementsModifier::referenceConfiguration, return_value_policy<ovito_object_reference>()), &CalculateDisplacementsModifier::setReferenceConfiguration)
		.add_property("eliminateCellDeformation", &CalculateDisplacementsModifier::eliminateCellDeformation, &CalculateDisplacementsModifier::setEliminateCellDeformation)
		.add_property("assumeUnwrappedCoordinates", &CalculateDisplacementsModifier::assumeUnwrappedCoordinates, &CalculateDisplacementsModifier::setAssumeUnwrappedCoordinates)
		.add_property("useReferenceFrameOffset", &CalculateDisplacementsModifier::useReferenceFrameOffset, &CalculateDisplacementsModifier::setUseReferenceFrameOffset)
		.add_property("referenceFrameNumber", &CalculateDisplacementsModifier::referenceFrameNumber, &CalculateDisplacementsModifier::setReferenceFrameNumber)
		.add_property("referenceFrameOffset", &CalculateDisplacementsModifier::referenceFrameOffset, &CalculateDisplacementsModifier::setReferenceFrameOffset)
		.add_property("vectorDisplay", make_function(&CalculateDisplacementsModifier::vectorDisplay, return_value_policy<ovito_object_reference>()))
	;

	ovito_class<HistogramModifier, ParticleModifier>()
		.add_property("sourceProperty", make_function(&HistogramModifier::sourceProperty, return_value_policy<copy_const_reference>()), &HistogramModifier::setSourceProperty)
		.add_property("numberOfBins", &HistogramModifier::numberOfBins, &HistogramModifier::setNumberOfBins)
	;

	ovito_class<ScatterPlotModifier, ParticleModifier>()
		.add_property("xAxisProperty", make_function(&ScatterPlotModifier::xAxisProperty, return_value_policy<copy_const_reference>()), &ScatterPlotModifier::setXAxisProperty)
		.add_property("yAxisProperty", make_function(&ScatterPlotModifier::yAxisProperty, return_value_policy<copy_const_reference>()), &ScatterPlotModifier::setYAxisProperty)
	;

	ovito_class<AtomicStrainModifier, AsynchronousParticleModifier>()
		.add_property("referenceConfiguration", make_function(&AtomicStrainModifier::referenceConfiguration, return_value_policy<ovito_object_reference>()), &AtomicStrainModifier::setReferenceConfiguration)
		.add_property("eliminateCellDeformation", &AtomicStrainModifier::eliminateCellDeformation, &AtomicStrainModifier::setEliminateCellDeformation)
		.add_property("assumeUnwrappedCoordinates", &AtomicStrainModifier::assumeUnwrappedCoordinates, &AtomicStrainModifier::setAssumeUnwrappedCoordinates)
		.add_property("useReferenceFrameOffset", &AtomicStrainModifier::useReferenceFrameOffset, &AtomicStrainModifier::setUseReferenceFrameOffset)
		.add_property("referenceFrameNumber", &AtomicStrainModifier::referenceFrameNumber, &AtomicStrainModifier::setReferenceFrameNumber)
		.add_property("referenceFrameOffset", &AtomicStrainModifier::referenceFrameOffset, &AtomicStrainModifier::setReferenceFrameOffset)
		.add_property("cutoff", &AtomicStrainModifier::cutoff, &AtomicStrainModifier::setCutoff)
		.add_property("calculateDeformationGradients", &AtomicStrainModifier::calculateDeformationGradients, &AtomicStrainModifier::setCalculateDeformationGradients)
		.add_property("calculateStrainTensors", &AtomicStrainModifier::calculateStrainTensors, &AtomicStrainModifier::setCalculateStrainTensors)
		.add_property("calculateNonaffineSquaredDisplacements", &AtomicStrainModifier::calculateNonaffineSquaredDisplacements, &AtomicStrainModifier::setCalculateNonaffineSquaredDisplacements)
		.add_property("selectInvalidParticles", &AtomicStrainModifier::selectInvalidParticles, &AtomicStrainModifier::setSelectInvalidParticles)
		.add_property("invalidParticleCount", &AtomicStrainModifier::invalidParticleCount)
	;

	ovito_class<WignerSeitzAnalysisModifier, AsynchronousParticleModifier>()
		.add_property("referenceConfiguration", make_function(&WignerSeitzAnalysisModifier::referenceConfiguration, return_value_policy<ovito_object_reference>()), &WignerSeitzAnalysisModifier::setReferenceConfiguration)
		.add_property("eliminateCellDeformation", &WignerSeitzAnalysisModifier::eliminateCellDeformation, &WignerSeitzAnalysisModifier::setEliminateCellDeformation)
		.add_property("useReferenceFrameOffset", &WignerSeitzAnalysisModifier::useReferenceFrameOffset, &WignerSeitzAnalysisModifier::setUseReferenceFrameOffset)
		.add_property("referenceFrameNumber", &WignerSeitzAnalysisModifier::referenceFrameNumber, &WignerSeitzAnalysisModifier::setReferenceFrameNumber)
		.add_property("referenceFrameOffset", &WignerSeitzAnalysisModifier::referenceFrameOffset, &WignerSeitzAnalysisModifier::setReferenceFrameOffset)
		.add_property("vacancyCount", &WignerSeitzAnalysisModifier::vacancyCount)
		.add_property("interstitialCount", &WignerSeitzAnalysisModifier::interstitialCount)
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(ParticlesModify);

};
