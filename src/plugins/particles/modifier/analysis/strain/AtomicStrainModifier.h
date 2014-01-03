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

#ifndef __OVITO_ATOMIC_STRAIN_MODIFIER_H
#define __OVITO_ATOMIC_STRAIN_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/util/OnTheFlyNeighborListBuilder.h>
#include "../../AsynchronousParticleModifier.h"

namespace Particles {

/******************************************************************************
* Calculates the per-particle displacement vectors by comparing the current
* positions to a reference configuration.
******************************************************************************/
class OVITO_PARTICLES_EXPORT AtomicStrainModifier : public AsynchronousParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE AtomicStrainModifier(DataSet* dataset);

	/// Returns the object that contains the reference configuration of the particles
	/// used for calculating the displacement vectors.
	SceneObject* referenceConfiguration() const { return _referenceObject; }

	/// Sets the object that contains the reference configuration of the particles
	/// used for calculating the displacement vectors.
	void setReferenceConfiguration(SceneObject* refConf) { _referenceObject = refConf; }

	/// Returns the source URL of the reference configuration.
	QUrl referenceSource() const;

	/// Sets the source URL of the reference configuration.
	void setReferenceSource(const QUrl& sourceUrl, const FileImporterDescription* importerType = nullptr);

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

	/// Returns whether particles, for which the strain tensor could not be computed, are selected.
	bool selectInvalidParticles() const { return _selectInvalidParticles; }

	/// Sets whether particles, for which the strain tensor could not be computed, are selected.
	void setSelectInvalidParticles(bool enableSelection) { _selectInvalidParticles = enableSelection; }

	/// Returns the computed von Mises shear strain values.
	const ParticleProperty& shearStrainValues() const { OVITO_CHECK_POINTER(_shearStrainValues.constData()); return *_shearStrainValues; }

	/// Returns the computed volumetric strain values.
	const ParticleProperty& volumetricStrainValues() const { OVITO_CHECK_POINTER(_volumetricStrainValues.constData()); return *_volumetricStrainValues; }

	/// Returns the computed strain tensors.
	const ParticleProperty& strainTensors() const { OVITO_CHECK_POINTER(_strainTensors.constData()); return *_strainTensors; }

	/// Returns the computed deformation gradient tensors.
	const ParticleProperty& deformationGradients() const { OVITO_CHECK_POINTER(_deformationGradients.constData()); return *_deformationGradients; }

	/// Returns the selection of invalid particles.
	const ParticleProperty& invalidParticles() const { OVITO_CHECK_POINTER(_invalidParticles.constData()); return *_invalidParticles; }

	/// After a successful evaluation of the modifier, this returns the number of invalid particles for which
	/// the strain tensor could not be computed.
	size_t invalidParticleCount() const { return _numInvalidParticles; }

public:

	Q_PROPERTY(bool eliminateCellDeformation READ eliminateCellDeformation WRITE setEliminateCellDeformation)
	Q_PROPERTY(bool assumeUnwrappedCoordinates READ assumeUnwrappedCoordinates WRITE setAssumeUnwrappedCoordinates)
	Q_PROPERTY(bool calculateDeformationGradients READ calculateDeformationGradients WRITE setCalculateDeformationGradients)
	Q_PROPERTY(bool calculateStrainTensors READ calculateStrainTensors WRITE setCalculateStrainTensors)
	Q_PROPERTY(bool selectInvalidParticles READ selectInvalidParticles WRITE setSelectInvalidParticles)
	Q_PROPERTY(FloatType cutoff READ cutoff WRITE setCutoff)
	Q_PROPERTY(SceneObject* referenceConfiguration READ referenceConfiguration WRITE setReferenceConfiguration);
	Q_PROPERTY(QUrl referenceSource READ referenceSource WRITE setReferenceSource);
	Q_PROPERTY(int invalidParticleCount READ invalidParticleCount);

private:

	/// Computes the modifier's results.
	class AtomicStrainEngine : public AsynchronousParticleModifier::Engine
	{
	public:

		/// Constructor.
		AtomicStrainEngine(ParticleProperty* positions, const SimulationCellData& simCell,
				ParticleProperty* refPositions, const SimulationCellData& simCellRef,
				ParticleProperty* identifiers, ParticleProperty* refIdentifiers,
				FloatType cutoff, bool eliminateCellDeformation, bool assumeUnwrappedCoordinates,
				bool calculateDeformationGradients, bool calculateStrainTensors) :
			_positions(positions), _simCell(simCell),
			_refPositions(refPositions), _simCellRef(simCellRef),
			_identifiers(identifiers), _refIdentifiers(refIdentifiers),
			_cutoff(cutoff), _eliminateCellDeformation(eliminateCellDeformation), _assumeUnwrappedCoordinates(assumeUnwrappedCoordinates),
			_calculateDeformationGradients(calculateDeformationGradients), _calculateStrainTensors(calculateStrainTensors),
			_shearStrains(new ParticleProperty(positions->size(), qMetaTypeId<FloatType>(), sizeof(FloatType), 1, tr("Shear Strain"))),
			_volumetricStrains(new ParticleProperty(positions->size(), qMetaTypeId<FloatType>(), sizeof(FloatType), 1, tr("Volumetric Strain"))),
			_strainTensors(calculateStrainTensors ? new ParticleProperty(positions->size(), ParticleProperty::StrainTensorProperty) : nullptr),
			_deformationGradients(calculateDeformationGradients ? new ParticleProperty(positions->size(), ParticleProperty::DeformationGradientProperty) : nullptr),
			_invalidParticles(new ParticleProperty(positions->size(), ParticleProperty::SelectionProperty)),
			_currentSimCellInv(simCell.matrix().inverse()),
			_reducedToAbsolute(eliminateCellDeformation ? simCellRef.matrix() : simCell.matrix()) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void compute(FutureInterfaceBase& futureInterface) override;

		/// Returns the property storage that contains the input particle positions.
		ParticleProperty* positions() const { return _positions.data(); }

		/// Returns the property storage that contains the reference particle positions.
		ParticleProperty* refPositions() const { return _refPositions.data(); }

		/// Returns the simulation cell data.
		const SimulationCellData& cell() const { return _simCell; }

		/// Returns the reference simulation cell data.
		const SimulationCellData& refCell() const { return _simCellRef; }

		/// Returns the property storage that contains the computed per-particle shear strain values.
		ParticleProperty* shearStrains() const { return _shearStrains.data(); }

		/// Returns the property storage that contains the computed per-particle volumetric strain values.
		ParticleProperty* volumetricStrains() const { return _volumetricStrains.data(); }

		/// Returns the property storage that contains the computed per-particle strain tensors.
		ParticleProperty* strainTensors() const { return _strainTensors.data(); }

		/// Returns the property storage that contains the computed per-particle deformation gradient tensors.
		ParticleProperty* deformationGradients() const { return _deformationGradients.data(); }

		/// Returns the property storage that contains the selection of invalid particles.
		ParticleProperty* invalidParticles() const { return _invalidParticles.data(); }

		/// Returns the number of invalid particles for which the strain tensor could not be computed.
		size_t numInvalidParticles() const { return _numInvalidParticles.load(); }

	private:

		/// Computes the strain tensor of a single particle.
		bool computeStrain(size_t particleIndex, OnTheFlyNeighborListBuilder& neighborListBuilder, const std::vector<size_t>& refToCurrentIndexMap, const std::vector<size_t>& currentToRefIndexMap);

		FloatType _cutoff;
		SimulationCellData _simCell;
		SimulationCellData _simCellRef;
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
		QExplicitlySharedDataPointer<ParticleProperty> _invalidParticles;
		bool _eliminateCellDeformation;
		bool _assumeUnwrappedCoordinates;
		bool _calculateDeformationGradients;
		bool _calculateStrainTensors;
		QAtomicInt _numInvalidParticles;
	};

protected:

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Creates and initializes a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<Engine> createEngine(TimePoint time, TimeInterval& validityInterval) override;

	/// Unpacks the computation results stored in the given engine object.
	virtual void retrieveModifierResults(Engine* engine) override;

	/// Inserts the computed and cached modifier results into the modification pipeline.
	virtual ObjectStatus applyModifierResults(TimePoint time, TimeInterval& validityInterval) override;

	/// This stores the cached results of the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _shearStrainValues;

	/// This stores the cached results of the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _volumetricStrainValues;

	/// This stores the cached results of the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _strainTensors;

	/// This stores the cached results of the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _deformationGradients;

	/// This stores the selection of invalid particles.
	QExplicitlySharedDataPointer<ParticleProperty> _invalidParticles;

	/// The reference configuration.
	ReferenceField<SceneObject> _referenceObject;

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

	/// Controls the whether particles, for which the strain tensor could not be computed, are selected.
	PropertyField<bool> _selectInvalidParticles;

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
	DECLARE_PROPERTY_FIELD(_selectInvalidParticles)
};

/******************************************************************************
* A properties editor for the AtomicStrainModifier class.
******************************************************************************/
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

};	// End of namespace

#endif // __OVITO_ATOMIC_STRAIN_MODIFIER_H
