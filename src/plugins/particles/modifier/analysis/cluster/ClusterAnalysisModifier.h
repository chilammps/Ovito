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

#ifndef __OVITO_CLUSTER_ANALYSIS_MODIFIER_H
#define __OVITO_CLUSTER_ANALYSIS_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/util/OnTheFlyNeighborListBuilder.h>
#include "../../AsynchronousParticleModifier.h"

namespace Particles {

/*
 * This modifier determines decomposes the particle set into clusters.
 */
class OVITO_PARTICLES_EXPORT ClusterAnalysisModifier : public AsynchronousParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE ClusterAnalysisModifier(DataSet* dataset);

	/// Returns the cutoff radius used to build the neighbor lists for the analysis.
	FloatType cutoff() const { return _cutoff; }

	/// \brief Sets the cutoff radius used to build the neighbor lists for the analysis.
	void setCutoff(FloatType newCutoff) { _cutoff = newCutoff; }

	/// Returns the cluster numbers assigned to particles.
	const ParticleProperty& particleClusters() const { OVITO_CHECK_POINTER(_particleClusters.constData()); return *_particleClusters; }

	/// Returns the number of clusters found during the last successful evaluation of the modifier.
	size_t clusterCount() const { return _numClusters; }

public:

	Q_PROPERTY(FloatType cutoff READ cutoff WRITE setCutoff);
	Q_PROPERTY(int clusterCount READ clusterCount);

private:

	/// Computes the modifier's results.
	class ClusterAnalysisEngine : public AsynchronousParticleModifier::Engine
	{
	public:

		/// Constructor.
		ClusterAnalysisEngine(ParticleProperty* positions, const SimulationCellData& simCell, FloatType cutoff) :
			_positions(positions), _simCell(simCell), _cutoff(cutoff),
			_particleClusters(new ParticleProperty(positions->size(), ParticleProperty::ClusterProperty)) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void compute(FutureInterfaceBase& futureInterface) override;

		/// Returns the property storage that contains the input particle positions.
		ParticleProperty* positions() const { return _positions.data(); }

		/// Returns the simulation cell data.
		const SimulationCellData& cell() const { return _simCell; }

		/// Returns the property storage that contains the computed cluster number of each particle.
		ParticleProperty* particleClusters() const { return _particleClusters.data(); }

		/// Returns the cutoff radius.
		FloatType cutoff() const { return _cutoff; }

		/// Returns the number of clusters.
		size_t numClusters() const { return _numClusters; }

	private:

		FloatType _cutoff;
		SimulationCellData _simCell;
		size_t _numClusters;
		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<ParticleProperty> _particleClusters;
	};

protected:

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Creates and initializes a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<Engine> createEngine(TimePoint time) override;

	/// Unpacks the computation results stored in the given engine object.
	virtual void retrieveModifierResults(Engine* engine) override;

	/// Inserts the computed and cached modifier results into the modification pipeline.
	virtual PipelineStatus applyModifierResults(TimePoint time, TimeInterval& validityInterval) override;

	/// This stores the cached results of the modifier.
	QExplicitlySharedDataPointer<ParticleProperty> _particleClusters;

	/// Controls the cutoff radius for the neighbor lists.
	PropertyField<FloatType> _cutoff;

	/// The number of clusters identified during the last evaluation of the modifier.
	size_t _numClusters;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Cluster analysis");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_PROPERTY_FIELD(_cutoff);
};

/******************************************************************************
* A properties editor for the ClusterAnalysisModifier class.
******************************************************************************/
class ClusterAnalysisModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE ClusterAnalysisModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_CLUSTER_ANALYSIS_MODIFIER_H
