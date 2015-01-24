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
#include "ClusterAnalysisModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ClusterAnalysisModifier, AsynchronousParticleModifier);
SET_OVITO_OBJECT_EDITOR(ClusterAnalysisModifier, ClusterAnalysisModifierEditor);
DEFINE_FLAGS_PROPERTY_FIELD(ClusterAnalysisModifier, _cutoff, "Cutoff", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(ClusterAnalysisModifier, _cutoff, "Cutoff radius");
SET_PROPERTY_FIELD_UNITS(ClusterAnalysisModifier, _cutoff, WorldParameterUnit);

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, ClusterAnalysisModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
ClusterAnalysisModifier::ClusterAnalysisModifier(DataSet* dataset) : AsynchronousParticleModifier(dataset),
	_cutoff(3.2), _numClusters(0)
{
	INIT_PROPERTY_FIELD(ClusterAnalysisModifier::_cutoff);
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::ComputeEngine> ClusterAnalysisModifier::createEngine(TimePoint time, TimeInterval validityInterval)
{
	// Get the current positions.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);

	// Get simulation cell.
	SimulationCellObject* inputCell = expectSimulationCell();

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<ClusterAnalysisEngine>(validityInterval, posProperty->storage(), inputCell->data(), cutoff());
}

/******************************************************************************
* Performs the actual computation. This method is executed in a worker thread.
******************************************************************************/
void ClusterAnalysisModifier::ClusterAnalysisEngine::perform()
{
	setProgressText(tr("Performing cluster analysis"));

	// Prepare the neighbor finder.
	CutoffNeighborFinder neighborFinder;
	if(!neighborFinder.prepare(_cutoff, positions(), cell(), this))
		return;

	size_t particleCount = positions()->size();
	setProgressRange(particleCount);

	// Initialize.
	std::fill(_particleClusters->dataInt(), _particleClusters->dataInt() + _particleClusters->size(), -1);
	_numClusters = 0;

	for(size_t seedParticleIndex = 0; seedParticleIndex < particleCount; seedParticleIndex++) {

		// Skip particles that have already been assigned to a cluster.
		if(_particleClusters->getInt(seedParticleIndex) != -1)
			continue;

		// Start a new cluster.
		int cluster = ++_numClusters;
		_particleClusters->setInt(seedParticleIndex, cluster);

		// Now recursively iterate over all neighbors of the seed particle and add them to the cluster too.
		std::deque<int> toProcess;
		toProcess.push_back(seedParticleIndex);

		do {
			incrementProgressValue();
			if(isCanceled())
				return;

			int currentParticle = toProcess.front();
			toProcess.pop_front();
			for(CutoffNeighborFinder::Query neighQuery(neighborFinder, currentParticle); !neighQuery.atEnd(); neighQuery.next()) {
				int neighborIndex = neighQuery.current();
				if(_particleClusters->getInt(neighborIndex) == -1) {
					_particleClusters->setInt(neighborIndex, cluster);
					toProcess.push_back(neighborIndex);
				}
			}
		}
		while(toProcess.empty() == false);
	}
}

/******************************************************************************
* Unpacks the results of the computation engine and stores them in the modifier.
******************************************************************************/
void ClusterAnalysisModifier::transferComputationResults(ComputeEngine* engine)
{
	ClusterAnalysisEngine* eng = static_cast<ClusterAnalysisEngine*>(engine);
	_particleClusters = eng->particleClusters();
	_numClusters = eng->numClusters();
}

/******************************************************************************
* Lets the modifier insert the cached computation results into the
* modification pipeline.
******************************************************************************/
PipelineStatus ClusterAnalysisModifier::applyComputationResults(TimePoint time, TimeInterval& validityInterval)
{
	if(!_particleClusters)
		throw Exception(tr("No computation results available."));

	if(inputParticleCount() != _particleClusters->size())
		throw Exception(tr("The number of input particles has changed. The stored results have become invalid."));

	outputStandardProperty(_particleClusters.data());
	return PipelineStatus(PipelineStatus::Success, tr("Found %1 clusters").arg(_numClusters));
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void ClusterAnalysisModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	AsynchronousParticleModifier::propertyChanged(field);

	// Recompute modifier results when the parameters have been changed.
	if(field == PROPERTY_FIELD(ClusterAnalysisModifier::_cutoff))
		invalidateCachedResults();
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ClusterAnalysisModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Cluster analysis"), rolloutParams, "particles.modifiers.cluster_analysis.html");

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QGridLayout* gridlayout = new QGridLayout();
	gridlayout->setContentsMargins(4,4,4,4);
	gridlayout->setColumnStretch(1, 1);

	// Cutoff parameter.
	FloatParameterUI* cutoffRadiusPUI = new FloatParameterUI(this, PROPERTY_FIELD(ClusterAnalysisModifier::_cutoff));
	gridlayout->addWidget(cutoffRadiusPUI->label(), 0, 0);
	gridlayout->addLayout(cutoffRadiusPUI->createFieldLayout(), 0, 1);
	cutoffRadiusPUI->setMinValue(0);

	layout->addLayout(gridlayout);

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
