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

#ifndef __OVITO_WIGNER_SEITZ_ANALYSIS_MODIFIER_H
#define __OVITO_WIGNER_SEITZ_ANALYSIS_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/util/NearestNeighborFinder.h>
#include "../../AsynchronousParticleModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

/**
 * \brief Performs the Wigner-Seitz cell analysis to identify point defects in crystals.
 */
class OVITO_PARTICLES_EXPORT WignerSeitzAnalysisModifier : public AsynchronousParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE WignerSeitzAnalysisModifier(DataSet* dataset);

	/// Returns the object that contains the reference configuration of the particles
	/// used for the Wigner-Seitz analysis.
	DataObject* referenceConfiguration() const { return _referenceObject; }

	/// Sets the object that contains the reference configuration of the particles
	/// used for the Wigner-Seitz analysis.
	void setReferenceConfiguration(DataObject* refConf) { _referenceObject = refConf; }

	/// Returns the source URL of the reference configuration.
	QUrl referenceSource() const;

	/// Sets the source URL of the reference configuration.
	void setReferenceSource(const QUrl& sourceUrl, const OvitoObjectType* importerType = nullptr);

	/// Returns true if the homogeneous deformation of the simulation cell is eliminated before performing the analysis.
	bool eliminateCellDeformation() const { return _eliminateCellDeformation; }

	/// Sets whether the homogeneous deformation of the simulation cell is eliminated before performing the analysis.
	void setEliminateCellDeformation(bool enable) { _eliminateCellDeformation = enable; }

	/// Returns whether to use a reference frame relative to current frame.
	bool useReferenceFrameOffset() const { return _useReferenceFrameOffset; }

	/// Sets whether to use a reference frame relative to current frame.
	void setUseReferenceFrameOffset(bool useOffset) { _useReferenceFrameOffset = useOffset; }

	/// Returns the absolute frame number from reference file to use for the analysis.
	int referenceFrameNumber() const { return _referenceFrameNumber; }

	/// Sets the absolute frame number from reference file to use for the analysis.
	void setReferenceFrameNumber(int frame) { _referenceFrameNumber = frame; }

	/// Returns the relative frame offset to use.
	int referenceFrameOffset() const { return _referenceFrameOffset; }

	/// Sets the relative frame offset to use.
	void setReferenceFrameOffset(int frameOffset) { _referenceFrameOffset = frameOffset; }

	/// Returns the number of vacant sites found during the last analysis run.
	int vacancyCount() const { return _vacancyCount; }

	/// Returns the number of interstitial atoms found during the last analysis run.
	int interstitialCount() const { return _interstitialCount; }

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
	class WignerSeitzAnalysisEngine : public ComputeEngine
	{
	public:

		/// Constructor.
		WignerSeitzAnalysisEngine(const TimeInterval& validityInterval, ParticleProperty* positions, const SimulationCell& simCell,
				ParticleProperty* refPositions, const SimulationCell& simCellRef, bool eliminateCellDeformation) :
			ComputeEngine(validityInterval),
			_positions(positions), _simCell(simCell),
			_refPositions(refPositions), _simCellRef(simCellRef),
			_eliminateCellDeformation(eliminateCellDeformation),
			_occupancyNumbers(new ParticleProperty(refPositions->size(), qMetaTypeId<int>(), sizeof(int), 1, sizeof(int), tr("Occupancy"), true)) {}

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

		/// Returns the property storage that contains the computed occupancies.
		ParticleProperty* occupancyNumbers() const { return _occupancyNumbers.data(); }

		/// Returns the number of vacant sites found during the analysis.
		int vacancyCount() const { return _vacancyCount; }

		/// Returns the number of interstitial atoms found during the analysis.
		int interstitialCount() const { return _interstitialCount; }

	private:

		SimulationCell _simCell;
		SimulationCell _simCellRef;
		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<ParticleProperty> _refPositions;
		QExplicitlySharedDataPointer<ParticleProperty> _occupancyNumbers;
		bool _eliminateCellDeformation;
		int _vacancyCount;
		int _interstitialCount;
	};

	/// Returns the reference state to be used to perform the analysis at the given time.
	PipelineFlowState getReferenceState(TimePoint time);

	/// This stores the cached results of the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _occupancyNumbers;

	/// The reference configuration.
	ReferenceField<DataObject> _referenceObject;

	/// Controls the whether the homogeneous deformation of the simulation cell is eliminated from the calculated displacement vectors.
	PropertyField<bool> _eliminateCellDeformation;

	/// Specify reference frame relative to current frame.
	PropertyField<bool> _useReferenceFrameOffset;

	/// Absolute frame number from reference file to use when calculating displacement vectors.
	PropertyField<int> _referenceFrameNumber;

	/// Relative frame offset for reference coordinates.
	PropertyField<int> _referenceFrameOffset;

	/// The number of vacant sites found during the last analysis run.
	int _vacancyCount;

	/// The number of interstitial atoms found during the last analysis run.
	int _interstitialCount;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Wigner-Seitz defect analysis");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_REFERENCE_FIELD(_referenceObject);
	DECLARE_PROPERTY_FIELD(_eliminateCellDeformation);
	DECLARE_PROPERTY_FIELD(_useReferenceFrameOffset);
	DECLARE_PROPERTY_FIELD(_referenceFrameNumber);
	DECLARE_PROPERTY_FIELD(_referenceFrameOffset);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A properties editor for the WignerSeitzAnalysisModifier class.
 */
class WignerSeitzAnalysisModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE WignerSeitzAnalysisModifierEditor() {}

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

#endif // __OVITO_WIGNER_SEITZ_ANALYSIS_MODIFIER_H
