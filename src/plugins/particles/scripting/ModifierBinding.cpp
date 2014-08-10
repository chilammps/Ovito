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

void setupModifierBinding()
{

	class_<ParticleModifier, bases<Modifier>, OORef<ParticleModifier>, boost::noncopyable>("ParticleModifier", no_init)
	;

	class_<AsynchronousParticleModifier, bases<ParticleModifier>, OORef<AsynchronousParticleModifier>, boost::noncopyable>("AsynchronousParticleModifier", no_init)
		.add_property("autoUpdateEnabled", &AsynchronousParticleModifier::autoUpdateEnabled, &AsynchronousParticleModifier::setAutoUpdateEnabled)
		.add_property("storeResultsWithScene", &AsynchronousParticleModifier::storeResultsWithScene, &AsynchronousParticleModifier::setStoreResultsWithScene)
	;

	class_<AssignColorModifier, bases<ParticleModifier>, OORef<AssignColorModifier>, boost::noncopyable>("AssignColorModifier", init<DataSet*>())
		.add_property("color", &AssignColorModifier::color, &AssignColorModifier::setColor)
		.add_property("colorController", make_function(&AssignColorModifier::colorController, return_value_policy<ovito_object_reference>()), &AssignColorModifier::setColorController)
		.add_property("keepSelection", &AssignColorModifier::keepSelection, &AssignColorModifier::setKeepSelection)
	;

	class_<ColorCodingModifier, bases<ParticleModifier>, OORef<ColorCodingModifier>, boost::noncopyable>("ColorCodingModifier", init<DataSet*>())
		.add_property("sourceProperty", make_function(&ColorCodingModifier::sourceProperty, return_value_policy<copy_const_reference>()), &ColorCodingModifier::setSourceProperty)
		.add_property("startValue", &ColorCodingModifier::startValue, &ColorCodingModifier::setStartValue)
		.add_property("startValueController", make_function(&ColorCodingModifier::startValueController, return_value_policy<ovito_object_reference>()), &ColorCodingModifier::setStartValueController)
		.add_property("endValue", &ColorCodingModifier::endValue, &ColorCodingModifier::setEndValue)
		.add_property("endValueController", make_function(&ColorCodingModifier::endValueController, return_value_policy<ovito_object_reference>()), &ColorCodingModifier::setEndValueController)
		.add_property("colorGradient", make_function(&ColorCodingModifier::colorGradient, return_value_policy<ovito_object_reference>()), &ColorCodingModifier::setColorGradient)
		.add_property("colorOnlySelected", &ColorCodingModifier::colorOnlySelected, &ColorCodingModifier::setColorOnlySelected)
		.add_property("keepSelection", &ColorCodingModifier::keepSelection, &ColorCodingModifier::setKeepSelection)
		.add_property("renderLegend", &ColorCodingModifier::renderLegend, &ColorCodingModifier::setRenderLegend)
		.add_property("legendViewport", make_function(&ColorCodingModifier::legendViewport, return_value_policy<ovito_object_reference>()), &ColorCodingModifier::setLegendViewport)
	;

	class_<AmbientOcclusionModifier, bases<AsynchronousParticleModifier>, OORef<AmbientOcclusionModifier>, boost::noncopyable>("AmbientOcclusionModifier", init<DataSet*>())
		.add_property("intensity", &AmbientOcclusionModifier::intensity, &AmbientOcclusionModifier::setIntensity)
		.add_property("samplingCount", &AmbientOcclusionModifier::samplingCount, &AmbientOcclusionModifier::setSamplingCount)
		.add_property("bufferResolution", &AmbientOcclusionModifier::bufferResolution, &AmbientOcclusionModifier::setBufferResolution)
	;

	class_<DeleteParticlesModifier, bases<ParticleModifier>, OORef<DeleteParticlesModifier>, boost::noncopyable>("DeleteParticlesModifier", init<DataSet*>())
	;

	class_<ShowPeriodicImagesModifier, bases<ParticleModifier>, OORef<ShowPeriodicImagesModifier>, boost::noncopyable>("ShowPeriodicImagesModifier", init<DataSet*>())
		.add_property("showImageX", &ShowPeriodicImagesModifier::showImageX, &ShowPeriodicImagesModifier::setShowImageX)
		.add_property("showImageY", &ShowPeriodicImagesModifier::showImageY, &ShowPeriodicImagesModifier::setShowImageY)
		.add_property("showImageZ", &ShowPeriodicImagesModifier::showImageZ, &ShowPeriodicImagesModifier::setShowImageZ)
		.add_property("numImagesX", &ShowPeriodicImagesModifier::numImagesX, &ShowPeriodicImagesModifier::setNumImagesX)
		.add_property("numImagesY", &ShowPeriodicImagesModifier::numImagesY, &ShowPeriodicImagesModifier::setNumImagesY)
		.add_property("numImagesZ", &ShowPeriodicImagesModifier::numImagesZ, &ShowPeriodicImagesModifier::setNumImagesZ)
		.add_property("adjustBoxSize", &ShowPeriodicImagesModifier::adjustBoxSize, &ShowPeriodicImagesModifier::setAdjustBoxSize)
	;

	class_<WrapPeriodicImagesModifier, bases<ParticleModifier>, OORef<WrapPeriodicImagesModifier>, boost::noncopyable>("WrapPeriodicImagesModifier", init<DataSet*>())
	;

	class_<CreateExpressionPropertyModifier, bases<ParticleModifier>, OORef<CreateExpressionPropertyModifier>, boost::noncopyable>("CreateExpressionPropertyModifier", init<DataSet*>())
	// Requires binding for QStringList
		.add_property("expressions", make_function(&CreateExpressionPropertyModifier::expressions, return_value_policy<copy_const_reference>()), &CreateExpressionPropertyModifier::setExpressions)
		.add_property("outputProperty", make_function(&CreateExpressionPropertyModifier::outputProperty, return_value_policy<copy_const_reference>()), &CreateExpressionPropertyModifier::setOutputProperty)
		.add_property("propertyComponentCount", &CreateExpressionPropertyModifier::propertyComponentCount, &CreateExpressionPropertyModifier::setPropertyComponentCount)
		.add_property("onlySelectedParticles", &CreateExpressionPropertyModifier::onlySelectedParticles, &CreateExpressionPropertyModifier::setOnlySelectedParticles)
	;

	class_<FreezePropertyModifier, bases<ParticleModifier>, OORef<FreezePropertyModifier>, boost::noncopyable>("FreezePropertyModifier", init<DataSet*>())
		.add_property("sourceProperty", make_function(&FreezePropertyModifier::sourceProperty, return_value_policy<copy_const_reference>()), &FreezePropertyModifier::setSourceProperty)
		.add_property("destinationProperty", make_function(&FreezePropertyModifier::destinationProperty, return_value_policy<copy_const_reference>()), &FreezePropertyModifier::setDestinationProperty)
		.def("takePropertySnapshot", &FreezePropertyModifier::takePropertySnapshot)
	;

	class_<ClearSelectionModifier, bases<ParticleModifier>, OORef<ClearSelectionModifier>, boost::noncopyable>("ClearSelectionModifier", init<DataSet*>())
	;

	class_<InvertSelectionModifier, bases<ParticleModifier>, OORef<InvertSelectionModifier>, boost::noncopyable>("InvertSelectionModifier", init<DataSet*>())
	;

	class_<FreezeSelectionModifier, bases<ParticleModifier>, OORef<FreezeSelectionModifier>, boost::noncopyable>("FreezeSelectionModifier", init<DataSet*>())
		.def("takeSelectionSnapshot", &FreezeSelectionModifier::takeSelectionSnapshot)
	;

	class_<ManualSelectionModifier, bases<ParticleModifier>, OORef<ManualSelectionModifier>, boost::noncopyable>("ManualSelectionModifier", init<DataSet*>())
		.def("resetSelection", &ManualSelectionModifier::resetSelection)
		.def("selectAll", &ManualSelectionModifier::selectAll)
		.def("clearSelection", &ManualSelectionModifier::clearSelection)
		.def("toggleParticleSelection", &ManualSelectionModifier::toggleParticleSelection)
	;

	class_<SelectExpressionModifier, bases<ParticleModifier>, OORef<SelectExpressionModifier>, boost::noncopyable>("SelectExpressionModifier", init<DataSet*>())
		.add_property("expression", make_function(&SelectExpressionModifier::expression, return_value_policy<copy_const_reference>()), &SelectExpressionModifier::setExpression)
	;

	class_<SelectParticleTypeModifier, bases<ParticleModifier>, OORef<SelectParticleTypeModifier>, boost::noncopyable>("SelectParticleTypeModifier", init<DataSet*>())
		.add_property("sourceProperty", make_function(&SelectParticleTypeModifier::sourceProperty, return_value_policy<copy_const_reference>()), &SelectParticleTypeModifier::setSourceProperty)
	// Requires binding for QSet<int>
		.add_property("selectedParticleTypes", make_function(&SelectParticleTypeModifier::selectedParticleTypes, return_value_policy<copy_const_reference>()), &SelectParticleTypeModifier::setSelectedParticleTypes)
	;

	class_<SliceModifier, bases<ParticleModifier>, OORef<SliceModifier>, boost::noncopyable>("SliceModifier", init<DataSet*>())
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

	class_<AffineTransformationModifier, bases<ParticleModifier>, OORef<AffineTransformationModifier>, boost::noncopyable>("AffineTransformationModifier", init<DataSet*>())
		.add_property("transformation", make_function(&AffineTransformationModifier::transformation, return_value_policy<copy_const_reference>()), &AffineTransformationModifier::setTransformation)
		.add_property("targetCell", make_function(&AffineTransformationModifier::targetCell, return_value_policy<copy_const_reference>()), &AffineTransformationModifier::setTargetCell)
		.add_property("relativeMode", &AffineTransformationModifier::relativeMode, &AffineTransformationModifier::setRelativeMode)
		.add_property("applyToParticles", &AffineTransformationModifier::applyToParticles, &AffineTransformationModifier::setApplyToParticles)
		.add_property("selectionOnly", &AffineTransformationModifier::selectionOnly, &AffineTransformationModifier::selectionOnly)
		.add_property("applyToSimulationBox", &AffineTransformationModifier::applyToSimulationBox, &AffineTransformationModifier::setApplyToSimulationBox)
		.add_property("applyToSurfaceMesh", &AffineTransformationModifier::applyToSurfaceMesh, &AffineTransformationModifier::setApplyToSurfaceMesh)
	;

	class_<BinAndReduceModifier, bases<ParticleModifier>, OORef<BinAndReduceModifier>, boost::noncopyable>("BinAndReduceModifier", init<DataSet*>())
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

	class_<StructureIdentificationModifier, bases<AsynchronousParticleModifier>, OORef<StructureIdentificationModifier>, boost::noncopyable>("StructureIdentificationModifier", no_init)
	;

	{
		scope s = class_<BondAngleAnalysisModifier, bases<StructureIdentificationModifier>, OORef<BondAngleAnalysisModifier>, boost::noncopyable>("BondAngleAnalysisModifier", init<DataSet*>())
		;

		enum_<BondAngleAnalysisModifier::StructureType>("StructureType")
			.value("OTHER", BondAngleAnalysisModifier::OTHER)
			.value("FCC", BondAngleAnalysisModifier::FCC)
			.value("HCP", BondAngleAnalysisModifier::HCP)
			.value("BCC", BondAngleAnalysisModifier::BCC)
			.value("ICO", BondAngleAnalysisModifier::ICO)
		;
	}

	{
		scope s = class_<CommonNeighborAnalysisModifier, bases<StructureIdentificationModifier>, OORef<CommonNeighborAnalysisModifier>, boost::noncopyable>("CommonNeighborAnalysisModifier", init<DataSet*>())
			.add_property("cutoff", &CommonNeighborAnalysisModifier::cutoff, &CommonNeighborAnalysisModifier::setCutoff)
			.add_property("adaptiveMode", &CommonNeighborAnalysisModifier::adaptiveMode, &CommonNeighborAnalysisModifier::setAdaptiveMode)
		;

		enum_<CommonNeighborAnalysisModifier::StructureType>("StructureType")
			.value("OTHER", CommonNeighborAnalysisModifier::OTHER)
			.value("FCC", CommonNeighborAnalysisModifier::FCC)
			.value("HCP", CommonNeighborAnalysisModifier::HCP)
			.value("BCC", CommonNeighborAnalysisModifier::BCC)
			.value("ICO", CommonNeighborAnalysisModifier::ICO)
			.value("DIA", CommonNeighborAnalysisModifier::DIA)
		;
	}

	{
		scope s = class_<CreateBondsModifier, bases<AsynchronousParticleModifier>, OORef<CreateBondsModifier>, boost::noncopyable>("CreateBondsModifier", init<DataSet*>())
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

	class_<CentroSymmetryModifier, bases<AsynchronousParticleModifier>, OORef<CentroSymmetryModifier>, boost::noncopyable>("CentroSymmetryModifier", init<DataSet*>())
		.add_property("numNeighbors", &CentroSymmetryModifier::numNeighbors, &CentroSymmetryModifier::setNumNeighbors)
	;

	class_<ClusterAnalysisModifier, bases<AsynchronousParticleModifier>, OORef<ClusterAnalysisModifier>, boost::noncopyable>("ClusterAnalysisModifier", init<DataSet*>())
		.add_property("cutoff", &ClusterAnalysisModifier::cutoff, &ClusterAnalysisModifier::setCutoff)
		.add_property("clusterCount", &ClusterAnalysisModifier::clusterCount)
	;

	class_<CoordinationNumberModifier, bases<AsynchronousParticleModifier>, OORef<CoordinationNumberModifier>, boost::noncopyable>("CoordinationNumberModifier", init<DataSet*>())
		.add_property("cutoff", &CoordinationNumberModifier::cutoff, &CoordinationNumberModifier::setCutoff)
	;

	class_<CalculateDisplacementsModifier, bases<ParticleModifier>, OORef<CalculateDisplacementsModifier>, boost::noncopyable>("CalculateDisplacementsModifier", init<DataSet*>())
		.add_property("referenceConfiguration", make_function(&CalculateDisplacementsModifier::referenceConfiguration, return_value_policy<ovito_object_reference>()), &CalculateDisplacementsModifier::setReferenceConfiguration)
		.add_property("eliminateCellDeformation", &CalculateDisplacementsModifier::eliminateCellDeformation, &CalculateDisplacementsModifier::setEliminateCellDeformation)
		.add_property("assumeUnwrappedCoordinates", &CalculateDisplacementsModifier::assumeUnwrappedCoordinates, &CalculateDisplacementsModifier::setAssumeUnwrappedCoordinates)
		.add_property("useReferenceFrameOffset", &CalculateDisplacementsModifier::useReferenceFrameOffset, &CalculateDisplacementsModifier::setUseReferenceFrameOffset)
		.add_property("referenceFrameNumber", &CalculateDisplacementsModifier::referenceFrameNumber, &CalculateDisplacementsModifier::setReferenceFrameNumber)
		.add_property("referenceFrameOffset", &CalculateDisplacementsModifier::referenceFrameOffset, &CalculateDisplacementsModifier::setReferenceFrameOffset)
		.add_property("vectorDisplay", make_function(&CalculateDisplacementsModifier::vectorDisplay, return_value_policy<ovito_object_reference>()))
	;

	class_<HistogramModifier, bases<ParticleModifier>, OORef<HistogramModifier>, boost::noncopyable>("HistogramModifier", init<DataSet*>())
		.add_property("sourceProperty", make_function(&HistogramModifier::sourceProperty, return_value_policy<copy_const_reference>()), &HistogramModifier::setSourceProperty)
		.add_property("numberOfBins", &HistogramModifier::numberOfBins, &HistogramModifier::setNumberOfBins)
	;

	class_<ScatterPlotModifier, bases<ParticleModifier>, OORef<ScatterPlotModifier>, boost::noncopyable>("ScatterPlotModifier", init<DataSet*>())
		.add_property("xAxisProperty", make_function(&ScatterPlotModifier::xAxisProperty, return_value_policy<copy_const_reference>()), &ScatterPlotModifier::setXAxisProperty)
		.add_property("yAxisProperty", make_function(&ScatterPlotModifier::yAxisProperty, return_value_policy<copy_const_reference>()), &ScatterPlotModifier::setYAxisProperty)
	;

	class_<AtomicStrainModifier, bases<AsynchronousParticleModifier>, OORef<AtomicStrainModifier>, boost::noncopyable>("AtomicStrainModifier", init<DataSet*>())
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

	class_<WignerSeitzAnalysisModifier, bases<AsynchronousParticleModifier>, OORef<WignerSeitzAnalysisModifier>, boost::noncopyable>("WignerSeitzAnalysisModifier", init<DataSet*>())
		.add_property("referenceConfiguration", make_function(&WignerSeitzAnalysisModifier::referenceConfiguration, return_value_policy<ovito_object_reference>()), &WignerSeitzAnalysisModifier::setReferenceConfiguration)
		.add_property("eliminateCellDeformation", &WignerSeitzAnalysisModifier::eliminateCellDeformation, &WignerSeitzAnalysisModifier::setEliminateCellDeformation)
		.add_property("useReferenceFrameOffset", &WignerSeitzAnalysisModifier::useReferenceFrameOffset, &WignerSeitzAnalysisModifier::setUseReferenceFrameOffset)
		.add_property("referenceFrameNumber", &WignerSeitzAnalysisModifier::referenceFrameNumber, &WignerSeitzAnalysisModifier::setReferenceFrameNumber)
		.add_property("referenceFrameOffset", &WignerSeitzAnalysisModifier::referenceFrameOffset, &WignerSeitzAnalysisModifier::setReferenceFrameOffset)
		.add_property("vacancyCount", &WignerSeitzAnalysisModifier::vacancyCount)
		.add_property("interstitialCount", &WignerSeitzAnalysisModifier::interstitialCount)
	;
}

};
