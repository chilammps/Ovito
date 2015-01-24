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
#include <core/dataset/importexport/FileSource.h>
#include <core/gui/mainwin/MainWindow.h>
#include <qcustomplot.h>
#include "CoordinationNumberModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, CoordinationNumberModifier, AsynchronousParticleModifier);
SET_OVITO_OBJECT_EDITOR(CoordinationNumberModifier, CoordinationNumberModifierEditor);
DEFINE_FLAGS_PROPERTY_FIELD(CoordinationNumberModifier, _cutoff, "Cutoff", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(CoordinationNumberModifier, _cutoff, "Cutoff radius");
SET_PROPERTY_FIELD_UNITS(CoordinationNumberModifier, _cutoff, WorldParameterUnit);

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, CoordinationNumberModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
CoordinationNumberModifier::CoordinationNumberModifier(DataSet* dataset) : AsynchronousParticleModifier(dataset),
	_cutoff(3.2)
{
	INIT_PROPERTY_FIELD(CoordinationNumberModifier::_cutoff);
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::ComputeEngine> CoordinationNumberModifier::createEngine(TimePoint time, TimeInterval validityInterval)
{
	// Get the current positions.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);

	// Get simulation cell.
	SimulationCellObject* inputCell = expectSimulationCell();

	// The number of sampling intervals for the radial distribution function.
	int rdfSampleCount = 500;

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<CoordinationAnalysisEngine>(validityInterval, posProperty->storage(), inputCell->data(), cutoff(), rdfSampleCount);
}

/******************************************************************************
* Performs the actual computation. This method is executed in a worker thread.
******************************************************************************/
void CoordinationNumberModifier::CoordinationAnalysisEngine::perform()
{
	setProgressText(tr("Computing coordination numbers"));

	// Prepare the neighbor list.
	CutoffNeighborFinder neighborListBuilder;
	if(!neighborListBuilder.prepare(_cutoff, positions(), cell(), this))
		return;

	size_t particleCount = positions()->size();
	setProgressValue(0);
	setProgressRange(particleCount / 1000);

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
		workers.push_back(std::thread([&neighborListBuilder, startIndex, endIndex, &mutex, this]() {
			int* coordOutput = _coordinationNumbers->dataInt();
			FloatType rdfBinSize = (_cutoff + FLOATTYPE_EPSILON) / _rdfHistogram.size();
			std::vector<size_t> threadLocalRDF(_rdfHistogram.size(), 0);
			for(size_t i = startIndex; i < endIndex;) {

				int coordNumber = 0;
				for(CutoffNeighborFinder::Query neighQuery(neighborListBuilder, i); !neighQuery.atEnd(); neighQuery.next()) {
					coordNumber++;
					size_t rdfInterval = (size_t)(sqrt(neighQuery.distanceSquared()) / rdfBinSize);
					threadLocalRDF[rdfInterval]++;
				}
				coordOutput[i] = coordNumber;

				i++;

				// Update progress indicator.
				if((i % 1000) == 0) {
					if(i != 0)
						incrementProgressValue();
					if(isCanceled())
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
* Unpacks the results of the computation engine and stores them in the modifier.
******************************************************************************/
void CoordinationNumberModifier::transferComputationResults(ComputeEngine* engine)
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
		_rdfX[i] = r + 0.5 * stepSize;
		_rdfY[i] = eng->rdfHistogram()[i] / (constant * (r2*r2*r2 - r*r*r));
	}
}

/******************************************************************************
* Lets the modifier insert the cached computation results into the
* modification pipeline.
******************************************************************************/
PipelineStatus CoordinationNumberModifier::applyComputationResults(TimePoint time, TimeInterval& validityInterval)
{
	if(!_coordinationNumbers)
		throw Exception(tr("No computation results available."));

	if(inputParticleCount() != _coordinationNumbers->size())
		throw Exception(tr("The number of input particles has changed. The stored results have become invalid."));

	outputStandardProperty(_coordinationNumbers.data());
	return PipelineStatus::Success;
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void CoordinationNumberModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	AsynchronousParticleModifier::propertyChanged(field);

	// Recompute modifier results when the parameters have been changed.
	if(field == PROPERTY_FIELD(CoordinationNumberModifier::_cutoff))
		invalidateCachedResults();
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

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
	connect(this, &CoordinationNumberModifierEditor::contentsReplaced, this, &CoordinationNumberModifierEditor::plotRDF);

	QPushButton* saveDataButton = new QPushButton(tr("Export data to file"));
	layout->addWidget(saveDataButton);
	connect(saveDataButton, &QPushButton::clicked, this, &CoordinationNumberModifierEditor::onSaveData);

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

	// Determine lower X bound where the histogram is non-zero.
	double maxx = modifier->rdfX().back();
	for(int i = 0; i < modifier->rdfX().size(); i++) {
		if(modifier->rdfY()[i] != 0) {
			double minx = std::floor(modifier->rdfX()[i] * 9.0 / maxx) / 10.0 * maxx;
			_rdfPlot->xAxis->setRange(minx, maxx);
			break;
		}
	}

	_rdfPlot->replot();
}

/******************************************************************************
* This is called when the user has clicked the "Save Data" button.
******************************************************************************/
void CoordinationNumberModifierEditor::onSaveData()
{
	CoordinationNumberModifier* modifier = static_object_cast<CoordinationNumberModifier>(editObject());
	if(!modifier)
		return;

	if(modifier->rdfX().empty())
		return;

	QString fileName = QFileDialog::getSaveFileName(mainWindow(),
	    tr("Save RDF Data"), QString(), tr("Text files (*.txt);;All files (*)"));
	if(fileName.isEmpty())
		return;

	try {

		QFile file(fileName);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
			throw Exception(tr("Could not open file for writing: %1").arg(file.errorString()));

		QTextStream stream(&file);

		stream << "# 1: Bin number" << endl;
		stream << "# 2: r" << endl;
		stream << "# 3: g(r)" << endl;
		for(int i = 0; i < modifier->rdfX().size(); i++) {
			stream << i << "\t" << modifier->rdfX()[i] << "\t" << modifier->rdfY()[i] << endl;
		}
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
