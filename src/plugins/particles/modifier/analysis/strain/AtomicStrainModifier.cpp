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
#include <core/scene/objects/DataObject.h>
#include <core/animation/AnimationSettings.h>
#include <core/dataset/importexport/FileSource.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/SubObjectParameterUI.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include "AtomicStrainModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, AtomicStrainModifier, AsynchronousParticleModifier);
SET_OVITO_OBJECT_EDITOR(AtomicStrainModifier, AtomicStrainModifierEditor);
DEFINE_FLAGS_REFERENCE_FIELD(AtomicStrainModifier, _referenceObject, "Reference Configuration", DataObject, PROPERTY_FIELD_NO_SUB_ANIM);
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _referenceShown, "ShowReferenceConfiguration");
DEFINE_FLAGS_PROPERTY_FIELD(AtomicStrainModifier, _eliminateCellDeformation, "EliminateCellDeformation", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _assumeUnwrappedCoordinates, "AssumeUnwrappedCoordinates");
DEFINE_FLAGS_PROPERTY_FIELD(AtomicStrainModifier, _cutoff, "Cutoff", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _calculateDeformationGradients, "CalculateDeformationGradients");
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _calculateStrainTensors, "CalculateStrainTensors");
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _calculateNonaffineSquaredDisplacements, "CalculateNonaffineSquaredDisplacements");
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _selectInvalidParticles, "SelectInvalidParticles");
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _useReferenceFrameOffset, "UseReferenceFrameOffet");
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, _referenceFrameNumber, "ReferenceFrameNumber");
DEFINE_FLAGS_PROPERTY_FIELD(AtomicStrainModifier, _referenceFrameOffset, "ReferenceFrameOffset", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _referenceObject, "Reference Configuration");
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _referenceShown, "Show reference configuration");
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _eliminateCellDeformation, "Eliminate homogeneous cell deformation");
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _assumeUnwrappedCoordinates, "Assume unwrapped coordinates");
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _cutoff, "Cutoff radius");
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _calculateDeformationGradients, "Output deformation gradient tensors");
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _calculateStrainTensors, "Output strain tensors");
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _calculateNonaffineSquaredDisplacements, "Output non-affine squared displacements (D^2_min)");
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _selectInvalidParticles, "Select invalid particles");
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _useReferenceFrameOffset, "Use reference frame offset");
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _referenceFrameNumber, "Reference frame number");
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _referenceFrameOffset, "Reference frame offset");
SET_PROPERTY_FIELD_UNITS(AtomicStrainModifier, _cutoff, WorldParameterUnit);

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, AtomicStrainModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AtomicStrainModifier::AtomicStrainModifier(DataSet* dataset) : AsynchronousParticleModifier(dataset),
	_referenceShown(false), _eliminateCellDeformation(false), _assumeUnwrappedCoordinates(false),
    _cutoff(3), _calculateDeformationGradients(false), _calculateStrainTensors(false), _calculateNonaffineSquaredDisplacements(false),
    _selectInvalidParticles(true),
    _useReferenceFrameOffset(false), _referenceFrameNumber(0), _referenceFrameOffset(-1)
{
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_referenceObject);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_referenceShown);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_eliminateCellDeformation);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_assumeUnwrappedCoordinates);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_cutoff);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_calculateDeformationGradients);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_calculateStrainTensors);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_calculateNonaffineSquaredDisplacements);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_selectInvalidParticles);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_useReferenceFrameOffset);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_referenceFrameNumber);
	INIT_PROPERTY_FIELD(AtomicStrainModifier::_referenceFrameOffset);

	// Create the file source object, which will be responsible for loading
	// and storing the reference configuration.
	OORef<FileSource> linkedFileObj(new FileSource(dataset));

	// Disable automatic adjustment of animation length for the reference object.
	// We don't want the scene's animation interval to be affected by an animation
	// loaded into the reference configuration object.
	linkedFileObj->setAdjustAnimationIntervalEnabled(false);
	setReferenceConfiguration(linkedFileObj);
}

/******************************************************************************
* Returns the source URL of the reference configuration.
******************************************************************************/
QUrl AtomicStrainModifier::referenceSource() const
{
	if(FileSource* linkedFileObj = dynamic_object_cast<FileSource>(referenceConfiguration()))
		return linkedFileObj->sourceUrl();
	else
		return QUrl();
}

/******************************************************************************
* Sets the source URL of the reference configuration.
******************************************************************************/
void AtomicStrainModifier::setReferenceSource(const QUrl& sourceUrl, const OvitoObjectType* importerType)
{
	if(FileSource* linkedFileObj = dynamic_object_cast<FileSource>(referenceConfiguration())) {
		linkedFileObj->setSource(sourceUrl, importerType);
	}
	else {
		OORef<FileSource> newObj(new FileSource(dataset()));
		newObj->setSource(sourceUrl, importerType);
		setReferenceConfiguration(newObj);
	}
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::ComputeEngine> AtomicStrainModifier::createEngine(TimePoint time, TimeInterval validityInterval)
{
	// Get the current positions.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);

	// Get the reference positions of the particles.
	if(!referenceConfiguration())
		throw Exception(tr("Cannot calculate displacements. Reference configuration has not been specified."));

	// What is the reference frame number to use?
	int referenceFrame;
	if(_useReferenceFrameOffset) {
		// Determine the current frame, preferably from the attributes stored with the pipeline flow state.
		// If the "Frame" attribute is not present, infer it from the current animation time.
		int currentFrame = input().attributes().value(QStringLiteral("Frame"),
				dataset()->animationSettings()->timeToFrame(time)).toInt();

		// Use frame offset relative to current configuration.
		referenceFrame = currentFrame + _referenceFrameOffset;

		// Results will only be valid for duration of current frame.
		validityInterval.intersect(time);
}
	else {
		// Always use the same, user-specified frame as reference configuration.
		referenceFrame = _referenceFrameNumber;
	}

	// Get the reference configuration.
	PipelineFlowState refState;
	if(FileSource* linkedFileObj = dynamic_object_cast<FileSource>(referenceConfiguration())) {
		if(linkedFileObj->numberOfFrames() > 0) {
			if(referenceFrame < 0 || referenceFrame >= linkedFileObj->numberOfFrames())
				throw Exception(tr("Requested reference frame %1 is out of range.").arg(referenceFrame));
			refState = linkedFileObj->requestFrame(referenceFrame);
		}
	}
	else refState = referenceConfiguration()->evaluate(dataset()->animationSettings()->frameToTime(referenceFrame));

	// Make sure the obtained reference configuration is valid and ready to use.
	if(refState.status().type() == PipelineStatus::Error)
		throw refState.status();
	if(refState.status().type() == PipelineStatus::Pending)
		throw PipelineStatus(PipelineStatus::Pending, tr("Waiting for input data to become ready..."));
	if(refState.isEmpty())
		throw Exception(tr("Reference configuration has not been specified yet or is empty. Please pick a reference simulation file."));
	// Make sure we really got back the requested reference frame.
	if(refState.attributes().value(QStringLiteral("Frame"), referenceFrame).toInt() != referenceFrame)
		throw Exception(tr("Requested reference frame %1 is out of range.").arg(referenceFrame));

	// Get the reference position property.
	ParticlePropertyObject* refPosProperty = ParticlePropertyObject::findInState(refState, ParticleProperty::PositionProperty);
	if(!refPosProperty)
		throw Exception(tr("The reference configuration does not contain particle positions."));

	// Get simulation cells.
	SimulationCellObject* inputCell = expectSimulationCell();
	SimulationCellObject* refCell = refState.findObject<SimulationCellObject>();
	if(!refCell)
		throw Exception(tr("Reference configuration does not contain simulation cell info."));

	// Check simulation cell(s).
	if(inputCell->volume() < FLOATTYPE_EPSILON)
		throw Exception(tr("Simulation cell is degenerate in the deformed configuration."));
	if(refCell->volume() < FLOATTYPE_EPSILON)
		throw Exception(tr("Simulation cell is degenerate in the reference configuration."));

	// Get particle identifiers.
	ParticlePropertyObject* identifierProperty = inputStandardProperty(ParticleProperty::IdentifierProperty);
	ParticlePropertyObject* refIdentifierProperty = ParticlePropertyObject::findInState(refState, ParticleProperty::IdentifierProperty);

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<AtomicStrainEngine>(validityInterval, posProperty->storage(), inputCell->data(), refPosProperty->storage(), refCell->data(),
			identifierProperty ? identifierProperty->storage() : nullptr, refIdentifierProperty ? refIdentifierProperty->storage() : nullptr,
            cutoff(), eliminateCellDeformation(), assumeUnwrappedCoordinates(), calculateDeformationGradients(), calculateStrainTensors(),
            calculateNonaffineSquaredDisplacements());
}

/******************************************************************************
* Performs the actual computation. This method is executed in a worker thread.
******************************************************************************/
void AtomicStrainModifier::AtomicStrainEngine::perform()
{
	setProgressText(tr("Computing atomic strain tensors"));

	// Build particle-to-particle index maps.
	std::vector<int> currentToRefIndexMap(positions()->size());
	std::vector<int> refToCurrentIndexMap(refPositions()->size());
	if(_identifiers && _refIdentifiers) {
		OVITO_ASSERT(_identifiers->size() == positions()->size());
		OVITO_ASSERT(_refIdentifiers->size() == refPositions()->size());

		// Build map of particle identifiers in reference configuration.
		std::map<int, int> refMap;
		int index = 0;
		for(int id : _refIdentifiers->constIntRange()) {
			if(refMap.insert(std::make_pair(id, index)).second == false)
				throw Exception(tr("Particles with duplicate identifiers detected in reference configuration."));
			index++;
		}

		if(isCanceled())
			return;

		// Check for duplicate identifiers in current configuration
#if 0
		std::vector<size_t> idSet(_identifiers->constDataInt(), _identifiers->constDataInt() + _identifiers->size());
		std::sort(idSet.begin(), idSet.end());
		if(std::adjacent_find(idSet.begin(), idSet.end()) != idSet.end())
			throw Exception(tr("Particles with duplicate identifiers detected in input configuration."));
#else
		std::map<int, int> currentMap;
		index = 0;
		for(int id : _identifiers->constIntRange()) {
			if(currentMap.insert(std::make_pair(id, index)).second == false)
				throw Exception(tr("Particles with duplicate identifiers detected in current configuration."));
			index++;
		}
#endif

		if(isCanceled())
			return;

		// Build index maps.
		const int* id = _identifiers->constDataInt();
		for(auto& mappedIndex : currentToRefIndexMap) {
			auto iter = refMap.find(*id);
			if(iter != refMap.end())
				mappedIndex = iter->second;
			else
				mappedIndex = -1;
			++id;
		}

		if(isCanceled())
			return;

		id = _refIdentifiers->constDataInt();
		for(auto& mappedIndex : refToCurrentIndexMap) {
			auto iter = currentMap.find(*id);
			if(iter != currentMap.end())
				mappedIndex = iter->second;
			else
				mappedIndex = -1;
			++id;
		}
	}
	else {
		// Deformed and reference configuration must contain the same number of particles.
		if(positions()->size() != refPositions()->size())
			throw Exception(tr("Cannot calculate displacements. Numbers of particles in reference configuration and current configuration do not match."));
		// When particle identifiers are not available, use trivial 1-to-1 mapping.
		std::iota(refToCurrentIndexMap.begin(), refToCurrentIndexMap.end(), 0);
		std::iota(currentToRefIndexMap.begin(), currentToRefIndexMap.end(), 0);
	}
	if(isCanceled())
		return;

	// Prepare the neighbor list for the reference configuration.
	CutoffNeighborFinder neighborFinder;
	if(!neighborFinder.prepare(_cutoff, refPositions(), refCell(), this))
		return;

	// Perform analysis on each particle.
	parallelFor(positions()->size(), *this, [&neighborFinder, &refToCurrentIndexMap, &currentToRefIndexMap, this](size_t index) {
		if(!this->computeStrain(index, neighborFinder, refToCurrentIndexMap, currentToRefIndexMap))
			_numInvalidParticles.fetchAndAddRelaxed(1);
	});
}

/******************************************************************************
* Computes the strain tensor of a single particle.
******************************************************************************/
bool AtomicStrainModifier::AtomicStrainEngine::computeStrain(size_t particleIndex, CutoffNeighborFinder& neighborFinder, const std::vector<int>& refToCurrentIndexMap, const std::vector<int>& currentToRefIndexMap)
{
	// We do the following calculations using double precision to
	// get best results. Final results will be converted back to
	// standard precision numbers.

	Matrix_3<double> V = Matrix_3<double>::Zero();
	Matrix_3<double> W = Matrix_3<double>::Zero();
	int numNeighbors = 0;

	// Iterate over neighbors of central particle.
	int particleIndexReference = currentToRefIndexMap[particleIndex];
	if(particleIndexReference != -1) {
		const Point3 x = positions()->getPoint3(particleIndex);
		for(CutoffNeighborFinder::Query neighQuery(neighborFinder, particleIndexReference); !neighQuery.atEnd(); neighQuery.next()) {
			const Vector3& r0 = neighQuery.delta();
			int neighborIndexCurrent = refToCurrentIndexMap[neighQuery.current()];
			if(neighborIndexCurrent == -1) continue;
			Vector3 r = positions()->getPoint3(neighborIndexCurrent) - x;
			Vector3 sr = _currentSimCellInv * r;
			if(!_assumeUnwrappedCoordinates) {
				for(size_t k = 0; k < 3; k++) {
					if(_simCell.pbcFlags()[k])
						sr[k] -= floor(sr[k] + FloatType(0.5));
				}
			}
			r = _reducedToAbsolute * sr;

			for(size_t i = 0; i < 3; i++) {
				for(size_t j = 0; j < 3; j++) {
					V(i,j) += r0[j] * r0[i];
					W(i,j) += r0[j] * r[i];
				}
			}

			numNeighbors++;
		}
	}

	// Check if matrix can be inverted.
	Matrix_3<double> inverseV;
	if(numNeighbors < 3 || !V.inverse(inverseV, 1e-4) || std::abs(W.determinant()) < 1e-4) {
		_invalidParticles->setInt(particleIndex, 1);
		if(_deformationGradients) {
			for(Matrix_3<double>::size_type col = 0; col < 3; col++) {
				for(Matrix_3<double>::size_type row = 0; row < 3; row++) {
					_deformationGradients->setFloatComponent(particleIndex, col*3+row, FloatType(0));
				}
			}
		}
		if(_strainTensors)
			_strainTensors->setSymmetricTensor2(particleIndex, SymmetricTensor2::Zero());
        if(_nonaffineSquaredDisplacements)
            _nonaffineSquaredDisplacements->setFloat(particleIndex, 0);
		_shearStrains->setFloat(particleIndex, 0);
		_volumetricStrains->setFloat(particleIndex, 0);
		return false;
	}

	// Calculate deformation gradient tensor.
	Matrix_3<double> F = W * inverseV;
	if(_deformationGradients) {
		for(Matrix_3<double>::size_type col = 0; col < 3; col++) {
			for(Matrix_3<double>::size_type row = 0; row < 3; row++) {
				_deformationGradients->setFloatComponent(particleIndex, col*3+row, (FloatType)F(row,col));
			}
		}
	}

	// Calculate strain tensor.
	SymmetricTensor2T<double> strain = (Product_AtA(F) - SymmetricTensor2T<double>::Identity()) * 0.5;
	if(_strainTensors)
		_strainTensors->setSymmetricTensor2(particleIndex, (SymmetricTensor2)strain);

    // Calculate nonaffine displacements.
    if(_nonaffineSquaredDisplacements) {
        double D2min = 0.0;

        // Again iterate over neighbor vectors of central particle.
        numNeighbors = 0;
        const Point3 x = positions()->getPoint3(particleIndex);
        for(CutoffNeighborFinder::Query neighQuery(neighborFinder, particleIndexReference); !neighQuery.atEnd(); neighQuery.next()) {
            const Vector3& r0 = neighQuery.delta();
			int neighborIndexCurrent = refToCurrentIndexMap[neighQuery.current()];
			if(neighborIndexCurrent == -1) continue;
            Vector3 r = positions()->getPoint3(neighborIndexCurrent) - x;
            Vector3 sr = _currentSimCellInv * r;
            if(!_assumeUnwrappedCoordinates) {
                for(size_t k = 0; k < 3; k++) {
                    if(_simCell.pbcFlags()[k])
						sr[k] -= floor(sr[k] + FloatType(0.5));
                }
            }
            r = _reducedToAbsolute * sr;

            Vector_3<double> rDouble(r.x(), r.y(), r.z());
            Vector_3<double> r0Double(r0.x(), r0.y(), r0.z());
            Vector_3<double> dr = rDouble - F * r0Double;
            D2min += dr.squaredLength();
		}

        _nonaffineSquaredDisplacements->setFloat(particleIndex, D2min);
	}


	// Calculate von Mises shear strain.
	double xydiff = strain.xx() - strain.yy();
	double xzdiff = strain.xx() - strain.zz();
	double yzdiff = strain.yy() - strain.zz();
	double shearStrain = sqrt(strain.xy()*strain.xy() + strain.xz()*strain.xz() + strain.yz()*strain.yz() +
			(xydiff*xydiff + xzdiff*xzdiff + yzdiff*yzdiff) / 6.0);
	OVITO_ASSERT(std::isfinite(shearStrain));
	_shearStrains->setFloat(particleIndex, (FloatType)shearStrain);

	// Calculate volumetric component.
	double volumetricStrain = (strain(0,0) + strain(1,1) + strain(2,2)) / 3.0;
	OVITO_ASSERT(std::isfinite(volumetricStrain));
	_volumetricStrains->setFloat(particleIndex, (FloatType)volumetricStrain);

	_invalidParticles->setInt(particleIndex, 0);
	return true;
}

/******************************************************************************
* Unpacks the results of the computation engine and stores them in the modifier.
******************************************************************************/
void AtomicStrainModifier::transferComputationResults(ComputeEngine* engine)
{
	AtomicStrainEngine* eng = static_cast<AtomicStrainEngine*>(engine);
	_shearStrainValues = eng->shearStrains();
	_volumetricStrainValues = eng->volumetricStrains();
	_strainTensors = eng->strainTensors();
	_deformationGradients = eng->deformationGradients();
	_nonaffineSquaredDisplacements = eng->nonaffineSquaredDisplacements();
	_invalidParticles = eng->invalidParticles();
	_numInvalidParticles = eng->numInvalidParticles();
}

/******************************************************************************
* Lets the modifier insert the cached computation results into the
* modification pipeline.
******************************************************************************/
PipelineStatus AtomicStrainModifier::applyComputationResults(TimePoint time, TimeInterval& validityInterval)
{
	if(!_shearStrainValues || !_volumetricStrainValues)
		throw Exception(tr("No computation results available."));

	if(outputParticleCount() != _shearStrainValues->size() || outputParticleCount() != _volumetricStrainValues->size())
		throw Exception(tr("The number of input particles has changed. The stored results have become invalid."));

	if(selectInvalidParticles() && _invalidParticles)
		outputStandardProperty(_invalidParticles.data());

	if(calculateStrainTensors() && _strainTensors)
		outputStandardProperty(_strainTensors.data());

	if(calculateDeformationGradients() && _deformationGradients)
		outputStandardProperty(_deformationGradients.data());

	if(calculateNonaffineSquaredDisplacements() && _nonaffineSquaredDisplacements)
		outputStandardProperty(_nonaffineSquaredDisplacements.data());

	outputCustomProperty(_volumetricStrainValues.data());
	outputCustomProperty(_shearStrainValues.data());

	if(invalidParticleCount() == 0)
		return PipelineStatus::Success;
	else
		return PipelineStatus(PipelineStatus::Warning, tr("Could not compute compute strain tensor for %1 particles. Increase cutoff radius to include more neighbors.").arg(invalidParticleCount()));
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void AtomicStrainModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	AsynchronousParticleModifier::propertyChanged(field);

	// Recompute brightness values when the parameters have been changed.
	if(field == PROPERTY_FIELD(AtomicStrainModifier::_eliminateCellDeformation) ||
			field == PROPERTY_FIELD(AtomicStrainModifier::_assumeUnwrappedCoordinates) ||
			field == PROPERTY_FIELD(AtomicStrainModifier::_cutoff) ||
			field == PROPERTY_FIELD(AtomicStrainModifier::_calculateDeformationGradients) ||
			field == PROPERTY_FIELD(AtomicStrainModifier::_calculateStrainTensors) ||
			field == PROPERTY_FIELD(AtomicStrainModifier::_calculateNonaffineSquaredDisplacements) ||
			field == PROPERTY_FIELD(AtomicStrainModifier::_useReferenceFrameOffset) ||
			field == PROPERTY_FIELD(AtomicStrainModifier::_referenceFrameNumber) ||
			field == PROPERTY_FIELD(AtomicStrainModifier::_referenceFrameOffset))
		invalidateCachedResults();
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void AtomicStrainModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Atomic strain"), rolloutParams, "particles.modifiers.atomic_strain.html");

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

	BooleanParameterUI* calculateNonaffineSquaredDisplacementsUI = new BooleanParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_calculateNonaffineSquaredDisplacements));
	layout->addWidget(calculateNonaffineSquaredDisplacementsUI->checkBox());

	BooleanParameterUI* selectInvalidParticlesUI = new BooleanParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_selectInvalidParticles));
	layout->addWidget(selectInvalidParticlesUI->checkBox());

	QGroupBox* referenceFrameGroupBox = new QGroupBox(tr("Reference frame"));
	layout->addWidget(referenceFrameGroupBox);

	QGridLayout* sublayout = new QGridLayout(referenceFrameGroupBox);
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(4);
	sublayout->setColumnStretch(0, 5);
	sublayout->setColumnStretch(2, 95);

	// Add box for selection between absolute and relative reference frames.
	BooleanRadioButtonParameterUI* useFrameOffsetUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_useReferenceFrameOffset));
	useFrameOffsetUI->buttonTrue()->setText(tr("Relative to current frame"));
	useFrameOffsetUI->buttonFalse()->setText(tr("Fixed reference configuration"));
	sublayout->addWidget(useFrameOffsetUI->buttonFalse(), 0, 0, 1, 3);

	IntegerParameterUI* frameNumberUI = new IntegerParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_referenceFrameNumber));
	frameNumberUI->label()->setText(tr("Frame number:"));
	sublayout->addWidget(frameNumberUI->label(), 1, 1, 1, 1);
	sublayout->addLayout(frameNumberUI->createFieldLayout(), 1, 2, 1, 1);
	frameNumberUI->setMinValue(0);
	frameNumberUI->setEnabled(false);
	connect(useFrameOffsetUI->buttonFalse(), &QRadioButton::toggled, frameNumberUI, &IntegerParameterUI::setEnabled);

	sublayout->addWidget(useFrameOffsetUI->buttonTrue(), 2, 0, 1, 3);
	IntegerParameterUI* frameOffsetUI = new IntegerParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_referenceFrameOffset));
	frameOffsetUI->label()->setText(tr("Frame offset:"));
	sublayout->addWidget(frameOffsetUI->label(), 3, 1, 1, 1);
	sublayout->addLayout(frameOffsetUI->createFieldLayout(), 3, 2, 1, 1);
	frameOffsetUI->setEnabled(false);
	connect(useFrameOffsetUI->buttonTrue(), &QRadioButton::toggled, frameOffsetUI, &IntegerParameterUI::setEnabled);

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());

	// Open a sub-editor for the reference object.
	new SubObjectParameterUI(this, PROPERTY_FIELD(AtomicStrainModifier::_referenceObject), RolloutInsertionParameters().setTitle(tr("Reference")));
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
