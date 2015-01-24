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

#ifndef __OVITO_ATOMIC_STRAIN_MODIFIER_H
#define __OVITO_ATOMIC_STRAIN_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/util/CutoffNeighborFinder.h>
#include "../../AsynchronousParticleModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

/**
 * \brief Calculates the per-particle strain tensors based on a reference configuration.
 */
class OVITO_PARTICLES_EXPORT AtomicStrainModifier : public AsynchronousParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE AtomicStrainModifier(DataSet* dataset);

	/// Returns the object that contains the reference configuration of the particles
	/// used for calculating the displacement vectors.
	DataObject* referenceConfiguration() const { return _referenceObject; }

	/// Sets the object that contains the reference configuration of the particles
	/// used for calculating the displacement vectors.
	void setReferenceConfiguration(DataObject* refConf) { _referenceObject = refConf; }

	/// Returns the source URL of the reference configuration.
	QUrl referenceSource() const;

	/// Sets the source URL of the reference configuration.
	void setReferenceSource(const QUrl& sourceUrl, const OvitoObjectType* importerType = nullptr);

	/// Returns whether the reference configuration is shown instead of the current configuration.
	bool referenceShown() const { return _referenceShown; }

	/// Sets whether the reference configuration is shown instead of the current configuration.
	void setReferenceShown(bool show) { _referenceShown = show; }

	/// Returns true if the homogeneous deformation of the simulation cell is eliminated from the calculated displacement vectors.
	bool eliminateCellDeformation() const { return _eliminateCellDeformation; }

	/// Sets whether the homogeneous deformation of the simulation cell is eliminated from the calculated displacement vectors.
	void setEliminateCellDeformation(bool enable) { _eliminateCellDeformation = enable; }

	/// Returns true if we assume the particle coordinates are unwrapped when calculating the displacement vectors.
	bool assumeUnwrappedCoordinates() const { return _assumeUnwrappedCoordinates; }

	/// Sets we assume the particle coordinates are unwrapped when calculating the displacement vectors.
	void setAssumeUnwrappedCoordinates(bool enable) { _assumeUnwrappedCoordinates = enable; }

	/// Returns the cutoff radius used to build the neighbor lists for the analysis.
	FloatType cutoff() const { return _cutoff; }

	/// \brief Sets the cutoff radius used to build the neighbor lists for the analysis.
	void setCutoff(FloatType newCutoff) { _cutoff = newCutoff; }

	/// Returns whether atomic deformation gradient tensors should be computed and stored.
	bool calculateDeformationGradients() const { return _calculateDeformationGradients; }

	/// Sets whether atomic deformation gradient tensors should be computed and stored.
	void setCalculateDeformationGradients(bool enableCalculation) { _calculateDeformationGradients = enableCalculation; }

	/// Returns whether atomic strain tensors should be computed and stored.
	bool calculateStrainTensors() const { return _calculateStrainTensors; }

	/// Sets whether atomic strain tensors should be computed and stored.
	void setCalculateStrainTensors(bool enableCalculation) { _calculateStrainTensors = enableCalculation; }

	/// Returns whether non-affine displacements should be computed and stored.
	bool calculateNonaffineSquaredDisplacements() const { return _calculateNonaffineSquaredDisplacements; }

	/// Sets whether non-affine displacements should be computed and stored.
	void setCalculateNonaffineSquaredDisplacements(bool enableCalculation) { _calculateNonaffineSquaredDisplacements = enableCalculation; }

	/// Returns whether particles, for which the strain tensor could not be computed, are selected.
	bool selectInvalidParticles() const { return _selectInvalidParticles; }

	/// Sets whether particles, for which the strain tensor could not be computed, are selected.
	void setSelectInvalidParticles(bool enableSelection) { _selectInvalidParticles = enableSelection; }

	/// Returns the computed von Mises shear strain values.
	const ParticleProperty& shearStrainValues() const { OVITO_CHECK_POINTER(_shearStrainValues.constData()); return *_shearStrainValues; }

	/// After a successful evaluation of the modifier, this returns the number of invalid particles for which
	/// the strain tensor could not be computed.
	size_t invalidParticleCount() const { return _numInvalidParticles; }

	/// Returns whether to use a reference frame relative to current frame.
	bool useReferenceFrameOffset() const { return _useReferenceFrameOffset; }

	/// Sets whether to use a reference frame relative to current frame.
	void setUseReferenceFrameOffset(bool useOffset) { _useReferenceFrameOffset = useOffset; }

	/// Returns the absolute frame number from reference file to use when calculating displacement vectors.
	int referenceFrameNumber() const { return _referenceFrameNumber; }

	/// Sets the absolute frame number from reference file to use when calculating displacement vectors.
	void setReferenceFrameNumber(int frame) { _referenceFrameNumber = frame; }

	/// Returns the relative frame offset to use.
	int referenceFrameOffset() const { return _referenceFrameOffset; }

	/// Sets the relative frame offset to use.
	void setReferenceFrameOffset(int frameOffset) { _referenceFrameOffset = frameOffset; }

protected:

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Creates a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<ComputeEngine> createEngine(TimePoint time, TimeInterval validityInterval) override;

	/// Unpacks the results of the computation engine and stores them in the modifier.
	virtual void transferComputationResults(ComputeEngine* engine) override;

	/// Lets the modifier insert the cached computation results into the modification pipeline.
	virtual PipelineStatus applyComputationResults(TimePoint time, TimeInterval& validityInterval) override;

private:

	/// Computes the modifier's results.
	class AtomicStrainEngine : public ComputeEngine
	{
	public:

		/// Constructor.
		AtomicStrainEngine(const TimeInterval& validityInterval, ParticleProperty* positions, const SimulationCell& simCell,
				ParticleProperty* refPositions, const SimulationCell& simCellRef,
				ParticleProperty* identifiers, ParticleProperty* refIdentifiers,
				FloatType cutoff, bool eliminateCellDeformation, bool assumeUnwrappedCoordinates,
				bool calculateDeformationGradients, bool calculateStrainTensors,
				bool calculateNonaffineSquaredDisplacements) :
			ComputeEngine(validityInterval),
			_positions(positions), _simCell(simCell),
			_refPositions(refPositions), _simCellRef(simCellRef),
			_identifiers(identifiers), _refIdentifiers(refIdentifiers),
			_cutoff(cutoff), _eliminateCellDeformation(eliminateCellDeformation), _assumeUnwrappedCoordinates(assumeUnwrappedCoordinates),
			_calculateDeformationGradients(calculateDeformationGradients), _calculateStrainTensors(calculateStrainTensors),
            _calculateNonaffineSquaredDisplacements(calculateNonaffineSquaredDisplacements),
			_shearStrains(new ParticleProperty(positions->size(), qMetaTypeId<FloatType>(), sizeof(FloatType), 1, sizeof(FloatType), tr("Shear Strain"), false)),
			_volumetricStrains(new ParticleProperty(positions->size(), qMetaTypeId<FloatType>(), sizeof(FloatType), 1, sizeof(FloatType), tr("Volumetric Strain"), false)),
			_strainTensors(calculateStrainTensors ? new ParticleProperty(positions->size(), ParticleProperty::StrainTensorProperty, 0, false) : nullptr),
			_deformationGradients(calculateDeformationGradients ? new ParticleProperty(positions->size(), ParticleProperty::DeformationGradientProperty, 0, false) : nullptr),
			_nonaffineSquaredDisplacements(calculateNonaffineSquaredDisplacements ? new ParticleProperty(positions->size(), ParticleProperty::NonaffineSquaredDisplacementProperty, 0, false) : nullptr),
			_invalidParticles(new ParticleProperty(positions->size(), ParticleProperty::SelectionProperty, 0, false)),
			_currentSimCellInv(simCell.inverseMatrix()),
			_reducedToAbsolute(eliminateCellDeformation ? simCellRef.matrix() : simCell.matrix()) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void perform() override;

		/// Returns the property storage that contains the input particle positions.
		ParticleProperty* positions() const { return _positions.data(); }

		/// Returns the property storage that contains the reference particle positions.
		ParticleProperty* refPositions() const { return _refPositions.data(); }

		/// Returns the simulation cell data.
		const SimulationCell& cell() const { return _simCell; }

		/// Returns the reference simulation cell data.
		const SimulationCell& refCell() const { return _simCellRef; }

		/// Returns the property storage that contains the computed per-particle shear strain values.
		ParticleProperty* shearStrains() const { return _shearStrains.data(); }

		/// Returns the property storage that contains the computed per-particle volumetric strain values.
		ParticleProperty* volumetricStrains() const { return _volumetricStrains.data(); }

		/// Returns the property storage that contains the computed per-particle strain tensors.
		ParticleProperty* strainTensors() const { return _strainTensors.data(); }

		/// Returns the property storage that contains the computed per-particle deformation gradient tensors.
		ParticleProperty* deformationGradients() const { return _deformationGradients.data(); }

		/// Returns the property storage that contains the computed per-particle deformation gradient tensors.
		ParticleProperty* nonaffineSquaredDisplacements() const { return _nonaffineSquaredDisplacements.data(); }

		/// Returns the property storage that contains the selection of invalid particles.
		ParticleProperty* invalidParticles() const { return _invalidParticles.data(); }

		/// Returns the number of invalid particles for which the strain tensor could not be computed.
		size_t numInvalidParticles() const { return _numInvalidParticles.load(); }

	private:

		/// Computes the strain tensor of a single particle.
		bool computeStrain(size_t particleIndex, CutoffNeighborFinder& neighborListBuilder, const std::vector<int>& refToCurrentIndexMap, const std::vector<int>& currentToRefIndexMap);

		FloatType _cutoff;
		SimulationCell _simCell;
		SimulationCell _simCellRef;
		AffineTransformation _currentSimCellInv;
		AffineTransformation _reducedToAbsolute;
		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<ParticleProperty> _refPositions;
		QExplicitlySharedDataPointer<ParticleProperty> _identifiers;
		QExplicitlySharedDataPointer<ParticleProperty> _refIdentifiers;
		QExplicitlySharedDataPointer<ParticleProperty> _shearStrains;
		QExplicitlySharedDataPointer<ParticleProperty> _volumetricStrains;
		QExplicitlySharedDataPointer<ParticleProperty> _strainTensors;
		QExplicitlySharedDataPointer<ParticleProperty> _deformationGradients;
		QExplicitlySharedDataPointer<ParticleProperty> _nonaffineSquaredDisplacements;
		QExplicitlySharedDataPointer<ParticleProperty> _invalidParticles;
		bool _eliminateCellDeformation;
		bool _assumeUnwrappedCoordinates;
		bool _calculateDeformationGradients;
		bool _calculateStrainTensors;
		bool _calculateNonaffineSquaredDisplacements;
		QAtomicInt _numInvalidParticles;
	};

	/// This stores the cached results of the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _shearStrainValues;

	/// This stores the cached results of the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _volumetricStrainValues;

	/// This stores the cached results of the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _strainTensors;

	/// This stores the cached results of the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _deformationGradients;

	/// This stores the cached results of the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _nonaffineSquaredDisplacements;

	/// This stores the selection of invalid particles.
	QExplicitlySharedDataPointer<ParticleProperty> _invalidParticles;

	/// The reference configuration.
	ReferenceField<DataObject> _referenceObject;

	/// Controls the whether the reference configuration is shown instead of the current configuration.
	PropertyField<bool> _referenceShown;

	/// Controls the whether the homogeneous deformation of the simulation cell is eliminated from the calculated displacement vectors.
	PropertyField<bool> _eliminateCellDeformation;

	/// Controls the whether we assume the particle coordinates are unwrapped when calculating the displacement vectors.
	PropertyField<bool> _assumeUnwrappedCoordinates;

	/// Controls the cutoff radius for the neighbor lists.
	PropertyField<FloatType> _cutoff;

	/// Controls the whether atomic deformation gradient tensors should be computed and stored.
	PropertyField<bool> _calculateDeformationGradients;

	/// Controls the whether atomic strain tensors should be computed and stored.
	PropertyField<bool> _calculateStrainTensors;

	/// Controls the whether non-affine displacements should be computed and stored.
	PropertyField<bool> _calculateNonaffineSquaredDisplacements;

	/// Controls the whether particles, for which the strain tensor could not be computed, are selected.
	PropertyField<bool> _selectInvalidParticles;

	/// Specify reference frame relative to current frame.
	PropertyField<bool> _useReferenceFrameOffset;

	/// Absolute frame number from reference file to use when calculating displacement vectors.
	PropertyField<int> _referenceFrameNumber;

	/// Relative frame offset for reference coordinates.
	PropertyField<int> _referenceFrameOffset;

	/// Counts the number of invalid particles for which the strain tensor could not be computed.
	size_t _numInvalidParticles;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Atomic strain");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_REFERENCE_FIELD(_referenceObject);
	DECLARE_PROPERTY_FIELD(_referenceShown);
	DECLARE_PROPERTY_FIELD(_eliminateCellDeformation);
	DECLARE_PROPERTY_FIELD(_assumeUnwrappedCoordinates);
	DECLARE_PROPERTY_FIELD(_cutoff)
	DECLARE_PROPERTY_FIELD(_calculateDeformationGradients)
	DECLARE_PROPERTY_FIELD(_calculateStrainTensors)
	DECLARE_PROPERTY_FIELD(_calculateNonaffineSquaredDisplacements)
	DECLARE_PROPERTY_FIELD(_selectInvalidParticles)
	DECLARE_PROPERTY_FIELD(_useReferenceFrameOffset);
	DECLARE_PROPERTY_FIELD(_referenceFrameNumber);
	DECLARE_PROPERTY_FIELD(_referenceFrameOffset);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A properties editor for the AtomicStrainModifier class.
 */
class AtomicStrainModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE AtomicStrainModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_ATOMIC_STRAIN_MODIFIER_H
