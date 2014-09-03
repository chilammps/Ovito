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
#include <core/utilities/concurrent/ParallelFor.h>
#include "VoronoiAnalysisModifier.h"

#include <voro++.hh>

namespace Particles {

using namespace voro;

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, VoronoiAnalysisModifier, AsynchronousParticleModifier);
IMPLEMENT_OVITO_OBJECT(Particles, VoronoiAnalysisModifierEditor, ParticleModifierEditor);
SET_OVITO_OBJECT_EDITOR(VoronoiAnalysisModifier, VoronoiAnalysisModifierEditor);
DEFINE_FLAGS_PROPERTY_FIELD(VoronoiAnalysisModifier, _cutoff, "Cutoff", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _onlySelected, "OnlySelected");
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _useRadii, "UseRadii");
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _computeIndices, "ComputeIndices");
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _edgeCount, "EdgeCount");
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _edgeThreshold, "EdgeThreshold");
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _faceThreshold, "FaceThreshold");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _cutoff, "Cutoff distance");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _onlySelected, "Only selected particles");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _useRadii, "Use atomic radii");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _computeIndices, "Compute Voronoi indices");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _edgeCount, "Maximum edge count");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _edgeThreshold, "Edge length threshold");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _faceThreshold, "Face area threshold");
SET_PROPERTY_FIELD_UNITS(VoronoiAnalysisModifier, _cutoff, WorldParameterUnit);
SET_PROPERTY_FIELD_UNITS(VoronoiAnalysisModifier, _edgeThreshold, WorldParameterUnit);

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
VoronoiAnalysisModifier::VoronoiAnalysisModifier(DataSet* dataset) : AsynchronousParticleModifier(dataset),
	_cutoff(6.0), _onlySelected(false), _computeIndices(false), _edgeCount(6),
	_useRadii(false), _edgeThreshold(0), _faceThreshold(0),
	_coordinationNumbers(new ParticleProperty(0, ParticleProperty::CoordinationProperty))
{
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_cutoff);
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_onlySelected);
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_useRadii);
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_computeIndices);
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_edgeCount);
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_edgeThreshold);
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_faceThreshold);
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

	// Get selection particle property.
	ParticlePropertyObject* selectionProperty = nullptr;
	if(onlySelected())
		selectionProperty = expectStandardProperty(ParticleProperty::SelectionProperty);

	// Get particle radii.
	std::vector<FloatType> radii;
	if(useRadii())
		radii = std::move(inputParticleRadii(time, validityInterval));

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<VoronoiAnalysisEngine>(
			posProperty->storage(),
			selectionProperty ? selectionProperty->storage() : nullptr,
			std::move(radii),
			inputCell->data(),
			cutoff(),
			qMax(0, edgeCount()),
			computeIndices(),
			edgeThreshold(),
			faceThreshold());
}

/******************************************************************************
* Performs the actual computation. This method is executed in a worker thread.
******************************************************************************/
void VoronoiAnalysisModifier::VoronoiAnalysisEngine::compute(FutureInterfaceBase& futureInterface)
{
	futureInterface.setProgressText(tr("Computing Voronoi polyhedra"));

	// Prepare the neighbor list generator.
	OnTheFlyNeighborListBuilder neighborListBuilder(_cutoff);
	if(!neighborListBuilder.prepare(_positions.data(), _simCell) || futureInterface.isCanceled())
		return;

	// Perform analysis.
	parallelFor(_positions->size(), futureInterface, [&neighborListBuilder, this](size_t index) {
		// Build Voronoi cell.
		voronoicell v;

		// Initialize the Voronoi cell to be a cube, centered at the origin.
		v.init(-_cutoff, _cutoff, -_cutoff, _cutoff, -_cutoff, _cutoff);

		for(OnTheFlyNeighborListBuilder::iterator niter(neighborListBuilder, index); !niter.atEnd(); niter.next()) {
			v.plane(niter.delta().x(), niter.delta().y(), niter.delta().z(), niter.distanceSquared());
		}

		// Compute cell volume.
		_atomicVolumes->setFloat(index, (FloatType)v.volume());

		// Iterate over the Voronoi faces and their edges.
		for(int i=1; i < v.p; i++) {
			for(int j = 0; j < v.nu[i]; j++) {
				int k = v.ed[i][j];
				if(k >= 0) {
					int faceOrder = 0;
					double dx = v.pts[3*k]   - v.pts[3*i];
					double dy = v.pts[3*k+1] - v.pts[3*i+1];
					double dz = v.pts[3*k+2] - v.pts[3*i+2];
					double edge_len = dx*dx + dy*dy + dz*dz;
					if(edge_len > _edgeThreshold) faceOrder++;
					v.ed[i][j] = -1-k;
					int l = v.cycle_up(v.ed[i][v.nu[i]+j], k);
					do {
						int m = v.ed[k][l];
						dx = v.pts[3*m]   - v.pts[3*k];
						dy = v.pts[3*m+1] - v.pts[3*k+1];
						dz = v.pts[3*m+2] - v.pts[3*k+2];
						edge_len = dx*dx + dy*dy + dz*dz;
						if(edge_len > _edgeThreshold) faceOrder++;
						v.ed[k][l] = -1-m;
						l = v.cycle_up(v.ed[k][v.nu[k]+l],m);
						k = m;
					}
					while(k != i);
					if(faceOrder >= 3 && faceOrder <= 6)
						signature[faceOrder-3]++;
				}
			}
		}

	});
}

/******************************************************************************
* Unpacks the computation results stored in the given engine object.
******************************************************************************/
void VoronoiAnalysisModifier::retrieveModifierResults(Engine* engine)
{
	VoronoiAnalysisEngine* eng = static_cast<VoronoiAnalysisEngine*>(engine);
	_coordinationNumbers = eng->coordinationNumbers();
	_atomicVolumes = eng->atomicVolumes();
	_voronoiIndices = eng->voronoiIndices();
}

/******************************************************************************
* Inserts the computed and cached modifier results into the modification pipeline.
******************************************************************************/
PipelineStatus VoronoiAnalysisModifier::applyModifierResults(TimePoint time, TimeInterval& validityInterval)
{
	if(!coordinationNumbers() || inputParticleCount() != coordinationNumbers()->size())
		throw Exception(tr("The number of input particles has changed. The stored results have become invalid."));

	outputStandardProperty(ParticleProperty::CoordinationProperty)->setStorage(coordinationNumbers());
	outputCustomProperty(atomicVolumes()->name(), atomicVolumes()->dataType(), atomicVolumes()->dataTypeSize(), atomicVolumes()->componentCount())->setStorage(atomicVolumes());
	if(voronoiIndices())
		outputCustomProperty(voronoiIndices()->name(), voronoiIndices()->dataType(), voronoiIndices()->dataTypeSize(), voronoiIndices()->componentCount())->setStorage(voronoiIndices());

	return PipelineStatus::Success;
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void VoronoiAnalysisModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	// Recompute modifier results when the parameters have been changed.
	if(autoUpdateEnabled()) {
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
