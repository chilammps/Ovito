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

#include <plugins/particles/Particles.h>
#include <core/scene/objects/SceneObject.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <3rdparty/qcustomplot/qcustomplot.h>
#include "CoordinationNumberModifier.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, CoordinationNumberModifier, AsynchronousParticleModifier)
IMPLEMENT_OVITO_OBJECT(Particles, CoordinationNumberModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(CoordinationNumberModifier, CoordinationNumberModifierEditor)
DEFINE_FLAGS_PROPERTY_FIELD(CoordinationNumberModifier, _cutoff, "Cutoff", PROPERTY_FIELD_MEMORIZE)
SET_PROPERTY_FIELD_LABEL(CoordinationNumberModifier, _cutoff, "Cutoff radius")
SET_PROPERTY_FIELD_UNITS(CoordinationNumberModifier, _cutoff, WorldParameterUnit)

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
CoordinationNumberModifier::CoordinationNumberModifier() :
	_cutoff(3.2),
	_coordinationNumbers(new ParticleProperty(0, ParticleProperty::CoordinationProperty))
{
	INIT_PROPERTY_FIELD(CoordinationNumberModifier::_cutoff);
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::Engine> CoordinationNumberModifier::createEngine(TimePoint time)
{
	if(inputParticleCount() == 0)
		throw Exception(tr("There are no input particles"));

	// Get the current positions.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);

	// Get simulation cell.
	SimulationCell* inputCell = expectSimulationCell();

	// The number of sampling intervals for the radial distribution function.
	int rdfSampleCount = 400;

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<CoordinationAnalysisEngine>(posProperty->storage(), inputCell->data(), cutoff(), rdfSampleCount);
}

/******************************************************************************
* Performs the actual computation. This method is executed in a worker thread.
******************************************************************************/
void CoordinationNumberModifier::CoordinationAnalysisEngine::compute(FutureInterfaceBase& futureInterface)
{
	size_t particleCount = positions()->size();
	futureInterface.setProgressText(tr("Computing coordination numbers"));

	// Prepare the neighbor list.
	OnTheFlyNeighborListBuilder neighborListBuilder(_cutoff);
	if(!neighborListBuilder.prepare(positions(), cell()) || futureInterface.isCanceled())
		return;

	futureInterface.setProgressRange(particleCount / 1000);
	futureInterface.setProgressValue(0);

	// Perform analysis on each particle in parallel.
	std::vector<std::thread> workers;
	int num_threads = std::max(1, QThread::idealThreadCount());
	size_t chunkSize = particleCount / num_threads;
	size_t startIndex = 0;
	size_t endIndex = chunkSize;
	std::mutex mutex;
	for(int t = 0; t < num_threads; t++) {
		if(t == num_threads - 1)
			endIndex += particleCount % num_threads;
		workers.push_back(std::thread([&futureInterface, &neighborListBuilder, startIndex, endIndex, &mutex, this]() {
			int* coordOutput = _coordinationNumbers->dataInt();
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
void CoordinationNumberModifier::retrieveModifierResults(Engine* engine)
{
	CoordinationAnalysisEngine* eng = static_cast<CoordinationAnalysisEngine*>(engine);
	_coordinationNumbers = eng->coordinationNumbers();
	_rdfY.resize(eng->rdfHistogram().size());
	_rdfX.resize(eng->rdfHistogram().size());
	double rho = eng->positions()->size() / eng->cell().volume();
	double constant = 4.0/3.0 * M_PI * rho * eng->positions()->size();
	double stepSize = eng->cutoff() / _rdfX.size();
	for(int i = 0; i < _rdfX.size(); i++) {
		double r = stepSize * i;
		double r2 = r + stepSize;
		_rdfX[i] = r;
		_rdfY[i] = eng->rdfHistogram()[i] / (constant * (r2*r2*r2 - r*r*r));
	}
}

/******************************************************************************
* Inserts the computed and cached modifier results into the modification pipeline.
******************************************************************************/
ObjectStatus CoordinationNumberModifier::applyModifierResults(TimePoint time, TimeInterval& validityInterval)
{
	if(inputParticleCount() != coordinationNumbers().size())
		throw Exception(tr("The number of input particles has changed. The stored results have become invalid."));

	outputStandardProperty(ParticleProperty::CoordinationProperty)->setStorage(_coordinationNumbers.data());

	return ObjectStatus::Success;
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void CoordinationNumberModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	// Recompute modifier results when the parameters have been changed.
	if(autoUpdateEnabled()) {
		if(field == PROPERTY_FIELD(CoordinationNumberModifier::_cutoff))
			invalidateCachedResults();
	}

	AsynchronousParticleModifier::propertyChanged(field);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CoordinationNumberModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Coordination analysis"), rolloutParams, "particles.modifiers.coordination_analysis.html");

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QGridLayout* gridlayout = new QGridLayout();
	gridlayout->setContentsMargins(4,4,4,4);
	gridlayout->setColumnStretch(1, 1);

	// Cutoff parameter.
	FloatParameterUI* cutoffRadiusPUI = new FloatParameterUI(this, PROPERTY_FIELD(CoordinationNumberModifier::_cutoff));
	gridlayout->addWidget(cutoffRadiusPUI->label(), 0, 0);
	gridlayout->addLayout(cutoffRadiusPUI->createFieldLayout(), 0, 1);
	cutoffRadiusPUI->setMinValue(0);

	layout->addLayout(gridlayout);

	_rdfPlot = new QCustomPlot();
	_rdfPlot->setMinimumHeight(180);
	_rdfPlot->xAxis->setLabel("Pair separation distance");
	_rdfPlot->yAxis->setLabel("g(r)");
	_rdfPlot->addGraph();

	layout->addWidget(new QLabel(tr("Radial distribution function:")));
	layout->addWidget(_rdfPlot);
	connect(this, SIGNAL(contentsReplaced(RefTarget*)), this, SLOT(plotRDF()));

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool CoordinationNumberModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->sender() == editObject() && event->type() == ReferenceEvent::ObjectStatusChanged) {
		plotRDF();
	}
	return ParticleModifierEditor::referenceEvent(source, event);
}

/******************************************************************************
* Updates the plot of the RDF computed by the modifier.
******************************************************************************/
void CoordinationNumberModifierEditor::plotRDF()
{
	CoordinationNumberModifier* modifier = static_object_cast<CoordinationNumberModifier>(editObject());
	if(!modifier)
		return;

	if(modifier->rdfX().empty())
		return;

	_rdfPlot->graph()->setData(modifier->rdfX(), modifier->rdfY());
	_rdfPlot->graph()->rescaleAxes();
	_rdfPlot->replot();
}


};	// End of namespace
