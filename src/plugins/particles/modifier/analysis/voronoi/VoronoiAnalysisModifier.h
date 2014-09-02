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

#ifndef __OVITO_VORONOI_ANALYSIS_MODIFIER_H
#define __OVITO_VORONOI_ANALYSIS_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/util/OnTheFlyNeighborListBuilder.h>
#include <plugins/particles/modifier/AsynchronousParticleModifier.h>

namespace Particles {

/*
 * This modifier computes the Voronoi indices of particles.
 */
class OVITO_PARTICLES_EXPORT VoronoiAnalysisModifier : public AsynchronousParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE VoronoiAnalysisModifier(DataSet* dataset);

	/// Returns the cutoff radius used to build the neighbor lists for the analysis.
	FloatType cutoff() const { return _cutoff; }

	/// \brief Sets the cutoff radius used to build the neighbor lists for the analysis.
	void setCutoff(FloatType newCutoff) { _cutoff = newCutoff; }

	/// Returns the computed coordination numbers.
	const ParticleProperty& coordinationNumbers() const { OVITO_CHECK_POINTER(_coordinationNumbers.constData()); return *_coordinationNumbers; }

	/// Returns the computed Voronoi indices.
	//const ParticleProperty& voronoiIndices() const { OVITO_CHECK_POINTER(_voronoiIndices.constData()); return *_voronoiIndices; }

private:

	/// Computes the modifier's results.
	class VoronoiAnalysisEngine : public AsynchronousParticleModifier::Engine
	{
	public:

		/// Constructor.
		VoronoiAnalysisEngine(ParticleProperty* positions, const SimulationCellData& simCell, FloatType cutoff, int indexVectorLength) :
			_positions(positions), _simCell(simCell),
			_cutoff(cutoff),
			_coordinationNumbers(new ParticleProperty(positions->size(), ParticleProperty::CoordinationProperty)) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void compute(FutureInterfaceBase& futureInterface) override;

		/// Returns the property storage that contains the input particle positions.
		ParticleProperty* positions() const { return _positions.data(); }

		/// Returns the simulation cell data.
		const SimulationCellData& cell() const { return _simCell; }

		/// Returns the property storage that contains the computed coordination numbers.
		ParticleProperty* coordinationNumbers() const { return _coordinationNumbers.data(); }

		/// Returns the cutoff radius.
		FloatType cutoff() const { return _cutoff; }

	private:

		FloatType _cutoff;
		SimulationCellData _simCell;
		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<ParticleProperty> _coordinationNumbers;
	};

protected:

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Creates and initializes a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<Engine> createEngine(TimePoint time, TimeInterval& validityInterval) override;

	/// Unpacks the computation results stored in the given engine object.
	virtual void retrieveModifierResults(Engine* engine) override;

	/// Inserts the computed and cached modifier results into the modification pipeline.
	virtual PipelineStatus applyModifierResults(TimePoint time, TimeInterval& validityInterval) override;

	/// This stores the cached coordination numbers computed by the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _coordinationNumbers;

	/// Controls the cutoff radius for Voronoi cell generation.
	PropertyField<FloatType> _cutoff;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Voronoi analysis");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_PROPERTY_FIELD(_cutoff);
};

/******************************************************************************
* A properties editor for the VoronoiAnalysisModifier class.
******************************************************************************/
class VoronoiAnalysisModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE VoronoiAnalysisModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_VORONOI_ANALYSIS_MODIFIER_H
