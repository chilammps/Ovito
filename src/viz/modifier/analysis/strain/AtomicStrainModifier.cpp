///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
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

#include <core/Core.h>
#include <core/scene/objects/SceneObject.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/SubObjectParameterUI.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include "AtomicStrainModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, AtomicStrainModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, AtomicStrainModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(AtomicStrainModifier, AtomicStrainModifierEditor)
DEFINE_REFERENCE_FIELD(AtomicStrainModifier, _referenceObject, "Reference Configuration", SceneObject)
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _referenceShown, "ShowReferenceConfiguration")
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _eliminateCellDeformation, "EliminateCellDeformation")
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _assumeUnwrappedCoordinates, "AssumeUnwrappedCoordinates")
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _cutoff, "Cutoff")
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _calculateDeformationGradients, "CalculateDeformationGradients")
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _calculateStrainTensors, "CalculateStrainTensors")
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _selectInvalidParticles, "SelectInvalidParticles")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _referenceObject, "Reference Configuration")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _referenceShown, "Show reference configuration")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _eliminateCellDeformation, "Eliminate homogeneous cell deformation")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _assumeUnwrappedCoordinates, "Assume unwrapped coordinates")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _cutoff, "Cutoff radius")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _calculateDeformationGradients, "Output deformation gradient tensors")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _calculateStrainTensors, "Output strain tensors")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _selectInvalidParticles, "Select invalid particles")
SET_PROPERTY_FIELD_UNITS(AtomicStrainModifier, _cutoff, WorldParameterUnit)

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AtomicStrainModifier::AtomicStrainModifier() :
	_referenceShown(false), _eliminateCellDeformation(false), _assumeUnwrappedCoordinates(false),
	_cutoff(3), _calculateDeformationGradients(false), _calculateStrainTensors(false), _selectInvalidParticles(true),
	_shearStrainValues(new ParticleProperty(0, qMetaTypeId<FloatType>(), sizeof(FloatType), 1, tr("Shear Strain"))),
	_volumetricStrainValues(new ParticleProperty(0, qMetaTypeId<FloatType>(), sizeof(FloatType), 1, tr("Volumetric Strain"))),
	_strainTensors(new ParticleProperty(0, ParticleProperty::StrainTensorProperty)),
	_deformationGradients(new ParticleProperty(0, ParticleProperty::DeformationGradientProperty)),
	_invalidParticles(new ParticleProperty(0, ParticleProperty::SelectionProperty))
{
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_referenceObject);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_referenceShown);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_eliminateCellDeformation);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_assumeUnwrappedCoordinates);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_cutoff);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_calculateDeformationGradients);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_calculateStrainTensors);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_selectInvalidParticles);

	OORef<LinkedFileObject> importObj(new LinkedFileObject());
	importObj->setAdjustAnimationIntervalEnabled(false);
	_referenceObject = importObj;

	// Load the last cutoff radius from the application settings store.
	QSettings settings;
	settings.beginGroup("viz/strain");
	setCutoff(settings.value("DefaultCutoff", 3.5).value<FloatType>());
	settings.endGroup();
}

/******************************************************************************
* Asks the modifier for its validity interval at the given time.
******************************************************************************/
TimeInterval AtomicStrainModifier::modifierValidity(TimePoint time)
{
	TimeInterval interval = ParticleModifier::modifierValidity(time);
	if(_referenceObject) {
		interval.intersect(_referenceObject->objectValidity(time));
		PipelineFlowState refState = _referenceObject->evaluate(time);
		interval.intersect(refState.stateValidity());
	}
	return interval;
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::Engine> AtomicStrainModifier::createEngine(TimePoint time)
{
	if(inputParticleCount() == 0)
		throw Exception(tr("There are no input particles"));

	// Get the current positions.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);

	// Get the reference positions of the particles.
	if(!referenceConfiguration())
		throw Exception(tr("Cannot calculate displacements. Reference configuration has not been specified."));

	// Get the reference configuration.
	PipelineFlowState refState = referenceConfiguration()->evaluate(time);
	if(refState.status().type() == ObjectStatus::Pending)
		throw ObjectStatus(ObjectStatus::Pending, QString(), tr("Waiting for input data to become ready..."));
	if(refState.isEmpty())
		throw Exception(tr("Reference configuration has not been specified yet."));

	// Get the reference position property.
	ParticlePropertyObject* refPosProperty = nullptr;
	for(const auto& o : refState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(property && property->type() == ParticleProperty::PositionProperty) {
			refPosProperty = property;
			break;
		}
	}
	if(!refPosProperty)
		throw Exception(tr("The reference configuration does not contain particle positions."));

	// Get simulation cells.
	SimulationCell* inputCell = expectSimulationCell();
	SimulationCell* refCell = refState.findObject<SimulationCell>();
	if(!refCell)
		throw Exception(tr("Reference configuration does not contain simulation cell info."));

	// Check simulation cell(s).
	if(inputCell->volume() < FLOATTYPE_EPSILON)
		throw Exception(tr("Simulation cell is degenerate in the deformed configuration."));
	if(refCell->volume() < FLOATTYPE_EPSILON)
		throw Exception(tr("Simulation cell is degenerate in the reference configuration."));

	// Get particle identifiers.
	ParticlePropertyObject* identifierProperty = inputStandardProperty(ParticleProperty::IdentifierProperty);
	ParticlePropertyObject* refIdentifierProperty = nullptr;
	for(const auto& o : refState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(property && property->type() == ParticleProperty::IdentifierProperty) {
			refIdentifierProperty = property;
			break;
		}
	}

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<AtomicStrainEngine>(posProperty->storage(), inputCell->data(), refPosProperty->storage(), refCell->data(),
			identifierProperty ? identifierProperty->storage() : nullptr, refIdentifierProperty ? refIdentifierProperty->storage() : nullptr,
			cutoff(), eliminateCellDeformation(), assumeUnwrappedCoordinates(), calculateDeformationGradients(), calculateStrainTensors());
}

/******************************************************************************
* Performs the actual computation. This method is executed in a worker thread.
******************************************************************************/
void AtomicStrainModifier::AtomicStrainEngine::compute(FutureInterfaceBase& futureInterface)
{
	futureInterface.setProgressText(tr("Computing atomic strain tensors"));

	// Build particle-to-particle index map.
	std::vector<size_t> indexToIndexMap(positions()->size());
	if(_identifiers && _refIdentifiers) {

		// Build map of particle identifiers in reference configuration.
		std::map<int, size_t> refMap;
		size_t index = 0;
		const int* id = _refIdentifiers->constDataInt();
		const int* id_end = id + _refIdentifiers->size();
		for(; id != id_end; ++id, ++index) {
			if(refMap.insert(std::make_pair(*id, index)).second == false)
				throw Exception(tr("Particles with duplicate identifiers detected in reference configuration."));
		}

		if(futureInterface.isCanceled())
			return;

		// Check for duplicate identifiers in current configuration.
		std::vector<size_t> idSet(_identifiers->constDataInt(), _identifiers->constDataInt() + _identifiers->size());
		std::sort(idSet.begin(), idSet.end());
		if(std::adjacent_find(idSet.begin(), idSet.end()) != idSet.end())
			throw Exception(tr("Particles with duplicate identifiers detected in input configuration."));

		if(futureInterface.isCanceled())
			return;

		// Build index map.
		index = 0;
		id = _identifiers->constDataInt();
		for(auto& mappedIndex : indexToIndexMap) {
			auto iter = refMap.find(*id);
			if(iter == refMap.end())
				throw Exception(tr("Particle id %1 not found in reference configuration.").arg(*id));
			mappedIndex = iter->second;
			index++;
			++id;
		}
	}
	else {
		// Deformed and reference configuration must contain the same number of particles.
		if(positions()->size() != refPositions()->size())
			throw Exception(tr("Cannot calculate displacements. Number of particles in reference configuration and current configuration does not match."));
		// When particle identifiers are not available, use trivial 1-to-1 mapping.
		std::iota(indexToIndexMap.begin(), indexToIndexMap.end(), size_t(0));
	}
	if(futureInterface.isCanceled())
		return;

	// Prepare the neighbor list for the reference configuration.
	OnTheFlyNeighborListBuilder neighborListBuilder(_cutoff);
	if(!neighborListBuilder.prepare(refPositions(), refCell()) || futureInterface.isCanceled())
		return;

	// Perform analysis on each particle.
	parallelFor(positions()->size(), futureInterface, [&neighborListBuilder, &indexToIndexMap, this](size_t index) {
		this->computeStrain(index, neighborListBuilder, indexToIndexMap);
	});
}

/******************************************************************************
* Computes the strain tensor of a single particle.
******************************************************************************/
bool AtomicStrainModifier::AtomicStrainEngine::computeStrain(size_t particleIndex, OnTheFlyNeighborListBuilder& neighborListBuilder, std::vector<size_t>& indexToIndexMap)
{
	// We do the following calculations using double precision to
	// achieve best results. Final results will be converted back to
	// standard precision numbers.

	Matrix_3<double> V = Matrix_3<double>::Zero();
	Matrix_3<double> W = Matrix_3<double>::Zero();

	// Iterate over neighbor vectors of central particle.
	size_t refParticleIndex = indexToIndexMap[particleIndex];
	const Point3 x = positions()->getPoint3(particleIndex);
	int numNeighbors = 0;
	for(OnTheFlyNeighborListBuilder::iterator neighborIter(neighborListBuilder, refParticleIndex); !neighborIter.atEnd(); neighborIter.next()) {
		const Vector3& r0 = neighborIter.delta();
		Vector3 r = positions()->getPoint3(neighborIter.current()) - x;
		Vector3 sr = _currentSimCellInv * r;
		if(!_assumeUnwrappedCoordinates) {
			for(size_t k = 0; k < 3; k++) {
				if(!_simCell.pbcFlags()[k]) continue;
				while(sr[k] > FloatType(+0.5)) sr[k] -= FloatType(1);
				while(sr[k] < FloatType(-0.5)) sr[k] += FloatType(1);
			}
		}
		r = _simCell.matrix() * sr;

		for(size_t i = 0; i < 3; i++) {
			for(size_t j = 0; j < 3; j++) {
				V(i,j) += r0[j] * r0[i];
				W(i,j) += r0[j] * r[i];
			}
		}

		numNeighbors++;
	}

	// Check if matrix can be inverted.
	Matrix_3<double> inverseV;
	if(numNeighbors < 3 || !V.inverse(inverseV, 1e-4) || std::abs(W.determinant()) < 1e-4) {
		_invalidParticles->setInt(particleIndex, 1);
		if(_deformationGradients)
			_deformationGradients->setTensor2(particleIndex, Tensor2::Zero());
		if(_strainTensors)
			_strainTensors->setSymmetricTensor2(particleIndex, SymmetricTensor2::Zero());
		_shearStrains->setFloat(particleIndex, 0);
		_volumetricStrains->setFloat(particleIndex, 0);
		return false;
	}

	// Calculate deformation gradient tensor.
	Matrix_3<double> F = W * inverseV;
	if(_deformationGradients)
		_deformationGradients->setTensor2(particleIndex, (Tensor2)F);

	// Calculate strain tensor.
	SymmetricTensor2T<double> strain = (Product_AtA(F) - SymmetricTensor2T<double>::Identity()) * 0.5;
	if(_strainTensors)
		_strainTensors->setSymmetricTensor2(particleIndex, (SymmetricTensor2)strain);

	// Calculate von Mises shear strain.
	double xydiff = strain.xx() - strain.yy();
	double xzdiff = strain.xx() - strain.zz();
	double yzdiff = strain.yy() - strain.zz();
	double shearStrain = sqrt(strain.xy()*strain.xy() + strain.xz()*strain.xz() + strain.yz()*strain.yz() +
			(xydiff*xydiff + xydiff*xydiff + yzdiff*yzdiff) / 6.0);
	OVITO_ASSERT(!std::isnan(shearStrain) && !std::isinf(shearStrain));
	_shearStrains->setFloat(particleIndex, (FloatType)shearStrain);

	// Calculate volumetric component.
	double volumetricStrain = (strain(0,0) + strain(1,1) + strain(2,2)) / 3;
	OVITO_ASSERT(!std::isnan(volumetricStrain) && !std::isinf(volumetricStrain));
	_volumetricStrains->setFloat(particleIndex, (FloatType)volumetricStrain);

	_invalidParticles->setInt(particleIndex, 0);
	return true;
}

/******************************************************************************
* Unpacks the computation results stored in the given engine object.
******************************************************************************/
void AtomicStrainModifier::retrieveModifierResults(Engine* engine)
{
	AtomicStrainEngine* eng = static_cast<AtomicStrainEngine*>(engine);
	if(eng->shearStrains())
		_shearStrainValues = eng->shearStrains();
	else
		_shearStrainValues->resize(0);
	if(eng->volumetricStrains())
		_volumetricStrainValues = eng->volumetricStrains();
	else
		_volumetricStrainValues->resize(0);
	if(eng->strainTensors())
		_strainTensors = eng->strainTensors();
	else
		_strainTensors->resize(0);
	if(eng->deformationGradients())
		_deformationGradients = eng->deformationGradients();
	else
		_deformationGradients->resize(0);
	if(eng->invalidParticles())
		_invalidParticles = eng->invalidParticles();
	else
		_invalidParticles->resize(0);
}

/******************************************************************************
* Inserts the computed and cached modifier results into the modification pipeline.
******************************************************************************/
ObjectStatus AtomicStrainModifier::applyModifierResults(TimePoint time, TimeInterval& validityInterval)
{
	if(inputParticleCount() != shearStrainValues().size() || inputParticleCount() != volumetricStrainValues().size())
		throw Exception(tr("The number of input particles has changed. The stored results have become invalid."));

	if(selectInvalidParticles() && invalidParticles().size() == inputParticleCount())
		outputStandardProperty(ParticleProperty::SelectionProperty)->replaceStorage(_invalidParticles.data());

	if(calculateStrainTensors() && strainTensors().size() == inputParticleCount())
		outputStandardProperty(ParticleProperty::StrainTensorProperty)->replaceStorage(_strainTensors.data());

	if(calculateDeformationGradients() && deformationGradients().size() == inputParticleCount())
		outputStandardProperty(ParticleProperty::DeformationGradientProperty)->replaceStorage(_deformationGradients.data());

	outputCustomProperty(volumetricStrainValues().name(), qMetaTypeId<FloatType>(), sizeof(FloatType), 1)->replaceStorage(_volumetricStrainValues.data());
	outputCustomProperty(shearStrainValues().name(), qMetaTypeId<FloatType>(), sizeof(FloatType), 1)->replaceStorage(_shearStrainValues.data());

	return ObjectStatus::Success;
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void AtomicStrainModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	// Recompute brightness values when the parameters have been changed.
	if(autoUpdateEnabled()) {
		if(field == PROPERTY_FIELD(AtomicStrainModifier::_eliminateCellDeformation) ||
				field == PROPERTY_FIELD(AtomicStrainModifier::_assumeUnwrappedCoordinates) ||
				field == PROPERTY_FIELD(AtomicStrainModifier::_cutoff) ||
				field == PROPERTY_FIELD(AtomicStrainModifier::_calculateDeformationGradients) ||
				field == PROPERTY_FIELD(AtomicStrainModifier::_calculateStrainTensors))
			invalidateCachedResults();
	}

	AsynchronousParticleModifier::propertyChanged(field);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void AtomicStrainModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Atomic strain"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QGridLayout* gridlayout = new QGridLayout();
	gridlayout->setContentsMargins(4,4,4,4);
	gridlayout->setColumnStretch(1, 1);

	// Cutoff parameter.
	FloatParameterUI* cutoffRadiusPUI = new FloatParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_cutoff));
	gridlayout->addWidget(cutoffRadiusPUI->label(), 0, 0);
	gridlayout->addLayout(cutoffRadiusPUI->createFieldLayout(), 0, 1);
	cutoffRadiusPUI->setMinValue(0);
	connect(cutoffRadiusPUI->spinner(), SIGNAL(spinnerValueChanged()), this, SLOT(memorizeCutoff()));

	layout->addLayout(gridlayout);

	BooleanParameterUI* eliminateCellDeformationUI = new BooleanParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_eliminateCellDeformation));
	layout->addWidget(eliminateCellDeformationUI->checkBox());

	BooleanParameterUI* assumeUnwrappedUI = new BooleanParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_assumeUnwrappedCoordinates));
	layout->addWidget(assumeUnwrappedUI->checkBox());

#if 0
	BooleanParameterUI* showReferenceUI = new BooleanParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_referenceShown));
	layout->addWidget(showReferenceUI->checkBox());
#endif

	QCheckBox* calculateShearStrainsBox = new QCheckBox(tr("Output von Mises shear strains"));
	calculateShearStrainsBox->setEnabled(false);
	calculateShearStrainsBox->setChecked(true);
	layout->addWidget(calculateShearStrainsBox);

	QCheckBox* calculateVolumetricStrainsBox = new QCheckBox(tr("Output volumetric strains"));
	calculateVolumetricStrainsBox->setEnabled(false);
	calculateVolumetricStrainsBox->setChecked(true);
	layout->addWidget(calculateVolumetricStrainsBox);

	BooleanParameterUI* calculateDeformationGradientsUI = new BooleanParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_calculateDeformationGradients));
	layout->addWidget(calculateDeformationGradientsUI->checkBox());

	BooleanParameterUI* calculateStrainTensorsUI = new BooleanParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_calculateStrainTensors));
	layout->addWidget(calculateStrainTensorsUI->checkBox());

	BooleanParameterUI* selectInvalidParticlesUI = new BooleanParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_selectInvalidParticles));
	layout->addWidget(selectInvalidParticlesUI->checkBox());

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());

	// Open a sub-editor for the reference object.
	new SubObjectParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_referenceObject));
}

/******************************************************************************
* Stores the current cutoff radius in the application settings
* so it can be used as default value for new modifiers in the future.
******************************************************************************/
void AtomicStrainModifierEditor::memorizeCutoff()
{
	if(!editObject()) return;
	AtomicStrainModifier* modifier = static_object_cast<AtomicStrainModifier>(editObject());

	QSettings settings;
	settings.beginGroup("viz/strain");
	settings.setValue("DefaultCutoff", modifier->cutoff());
	settings.endGroup();
}


};	// End of namespace
