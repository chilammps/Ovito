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
#include "VoronoiAnalysisModifier.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, VoronoiAnalysisModifier, AsynchronousParticleModifier);
IMPLEMENT_OVITO_OBJECT(Particles, VoronoiAnalysisModifierEditor, ParticleModifierEditor);
SET_OVITO_OBJECT_EDITOR(VoronoiAnalysisModifier, VoronoiAnalysisModifierEditor);
DEFINE_FLAGS_PROPERTY_FIELD(VoronoiAnalysisModifier, _cutoff, "Cutoff", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _cutoff, "Cutoff distance");
SET_PROPERTY_FIELD_UNITS(VoronoiAnalysisModifier, _cutoff, WorldParameterUnit);

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
VoronoiAnalysisModifier::VoronoiAnalysisModifier(DataSet* dataset) : AsynchronousParticleModifier(dataset),
	_cutoff(6.0),
	_coordinationNumbers(new ParticleProperty(0, ParticleProperty::CoordinationProperty))
{
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_cutoff);
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::Engine> VoronoiAnalysisModifier::createEngine(TimePoint time, TimeInterval& validityInterval)
{
	// Get the current positions.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);

	// Get simulation cell.
	SimulationCell* inputCell = expectSimulationCell();

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<VoronoiAnalysisEngine>(posProperty->storage(), inputCell->data(), cutoff(), 4);
}

/******************************************************************************
* Performs the actual computation. This method is executed in a worker thread.
******************************************************************************/
void VoronoiAnalysisModifier::VoronoiAnalysisEngine::compute(FutureInterfaceBase& futureInterface)
{
	size_t particleCount = positions()->size();
	futureInterface.setProgressText(tr("Computing Voronoi polyhedra"));

	// Prepare the neighbor list generator.
	OnTheFlyNeighborListBuilder neighborListBuilder(_cutoff);
	if(!neighborListBuilder.prepare(positions(), cell()) || futureInterface.isCanceled())
		return;

	futureInterface.setProgressRange(particleCount / 1000);
	futureInterface.setProgressValue(0);

	// Perform analysis.
	std::vector<std::thread> workers;
	int num_threads = std::max(1, QThread::idealThreadCount());
	size_t chunkSize = particleCount / num_threads;
	size_t startIndex = 0;
	size_t endIndex = chunkSize;
	for(int t = 0; t < num_threads; t++) {
		if(t == num_threads - 1)
			endIndex += particleCount % num_threads;
		workers.push_back(std::thread([&futureInterface, &neighborListBuilder, startIndex, endIndex, this]() {
			int* coordOutput = _coordinationNumbers->dataInt();
#if 0
			FloatType rdfBinSize = (_cutoff + FLOATTYPE_EPSILON) / _rdfHistogram.size();
			std::vector<size_t> threadLocalRDF(_rdfHistogram.size(), 0);
			for(size_t i = startIndex; i < endIndex;) {

				int coordNumber = 0;
				for(OnTheFlyNeighborListBuilder::iterator neighborIter(neighborListBuilder, i); !neighborIter.atEnd(); neighborIter.next()) {
					coordNumber++;
					size_t rdfInterval = (size_t)(sqrt(neighborIter.distanceSquared()) / rdfBinSize);
					threadLocalRDF[rdfInterval]++;
				}
				coordOutput[i] = coordNumber;

				i++;

				// Update progress indicator.
				if((i % 1000) == 0) {
					if(i != 0)
						futureInterface.incrementProgressValue();
					if(futureInterface.isCanceled())
						return;
				}
			}
			std::lock_guard<std::mutex> lock(mutex);
			auto iter_out = _rdfHistogram.begin();
			for(auto iter = threadLocalRDF.cbegin(); iter != threadLocalRDF.cend(); ++iter, ++iter_out)
				*iter_out += *iter;
#endif
		}));
		startIndex = endIndex;
		endIndex += chunkSize;
	}

	for(auto& t : workers)
		t.join();
}

/******************************************************************************
* Unpacks the computation results stored in the given engine object.
******************************************************************************/
void VoronoiAnalysisModifier::retrieveModifierResults(Engine* engine)
{
	VoronoiAnalysisEngine* eng = static_cast<VoronoiAnalysisEngine*>(engine);
	_coordinationNumbers = eng->coordinationNumbers();
}

/******************************************************************************
* Inserts the computed and cached modifier results into the modification pipeline.
******************************************************************************/
PipelineStatus VoronoiAnalysisModifier::applyModifierResults(TimePoint time, TimeInterval& validityInterval)
{
	if(inputParticleCount() != coordinationNumbers().size())
		throw Exception(tr("The number of input particles has changed. The stored results have become invalid."));

	outputStandardProperty(ParticleProperty::CoordinationProperty)->setStorage(_coordinationNumbers.data());

	return PipelineStatus::Success;
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void VoronoiAnalysisModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	// Recompute modifier results when the parameters have been changed.
	if(autoUpdateEnabled()) {
		if(field == PROPERTY_FIELD(VoronoiAnalysisModifier::_cutoff))
			invalidateCachedResults();
	}

	AsynchronousParticleModifier::propertyChanged(field);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void VoronoiAnalysisModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Voronoi analysis"), rolloutParams, "particles.modifiers.voronoi_analysis.html");

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QGridLayout* gridlayout = new QGridLayout();
	gridlayout->setContentsMargins(4,4,4,4);
	gridlayout->setColumnStretch(1, 1);

	// Cutoff parameter.
	FloatParameterUI* cutoffRadiusPUI = new FloatParameterUI(this, PROPERTY_FIELD(VoronoiAnalysisModifier::_cutoff));
	gridlayout->addWidget(cutoffRadiusPUI->label(), 0, 0);
	gridlayout->addLayout(cutoffRadiusPUI->createFieldLayout(), 0, 1);
	cutoffRadiusPUI->setMinValue(0);

	layout->addLayout(gridlayout);

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());
}

};	// End of namespace
