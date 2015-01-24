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
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include <plugins/particles/util/NearestNeighborFinder.h>
#include "CentroSymmetryModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

/// The maximum number of neighbors that can be taken into account to compute the CSP.
enum { MAX_CSP_NEIGHBORS = 32 };

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, CentroSymmetryModifier, AsynchronousParticleModifier);
SET_OVITO_OBJECT_EDITOR(CentroSymmetryModifier, CentroSymmetryModifierEditor);
DEFINE_FLAGS_PROPERTY_FIELD(CentroSymmetryModifier, _numNeighbors, "NumNeighbors", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(CentroSymmetryModifier, _numNeighbors, "Number of neighbors");

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, CentroSymmetryModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
CentroSymmetryModifier::CentroSymmetryModifier(DataSet* dataset) : AsynchronousParticleModifier(dataset),
	_numNeighbors(12)
{
	INIT_PROPERTY_FIELD(CentroSymmetryModifier::_numNeighbors);
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::ComputeEngine> CentroSymmetryModifier::createEngine(TimePoint time, TimeInterval validityInterval)
{
	// Get modifier input.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);
	SimulationCellObject* simCell = expectSimulationCell();

	if(numNeighbors() < 2)
		throw Exception(tr("The selected number of neighbors to take into account for the centrosymmetry calculation is invalid."));

	if(numNeighbors() % 2)
		throw Exception(tr("The number of neighbors to take into account for the centrosymmetry calculation must be a positive, even integer."));

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<CentroSymmetryEngine>(validityInterval, posProperty->storage(), simCell->data(), numNeighbors());
}

/******************************************************************************
* Performs the actual computation. This method is executed in a worker thread.
******************************************************************************/
void CentroSymmetryModifier::CentroSymmetryEngine::perform()
{
	setProgressText(tr("Computing centrosymmetry parameters"));

	// Prepare the neighbor list.
	NearestNeighborFinder neighFinder(_nneighbors);
	if(!neighFinder.prepare(positions(), cell(), this)) {
		return;
	}

	// Output storage.
	ParticleProperty* output = csp();

	// Perform analysis on each particle.
	parallelFor(positions()->size(), *this, [&neighFinder, output](size_t index) {
		output->setFloat(index, computeCSP(neighFinder, index));
	});
}

/******************************************************************************
* Computes the centrosymmetry parameter of a single particle.
******************************************************************************/
FloatType CentroSymmetryModifier::computeCSP(NearestNeighborFinder& neighFinder, size_t particleIndex)
{
	// Find k nearest neighbor of current atom.
	NearestNeighborFinder::Query<MAX_CSP_NEIGHBORS> neighQuery(neighFinder);
	neighQuery.findNeighbors(neighFinder.particlePos(particleIndex));

	int numNN = neighQuery.results().size();

    // R = Ri + Rj for each of npairs i,j pairs among numNN neighbors.
	FloatType pairs[MAX_CSP_NEIGHBORS*MAX_CSP_NEIGHBORS/2];
	FloatType* p = pairs;
	for(auto ij = neighQuery.results().begin(); ij != neighQuery.results().end(); ++ij) {
		for(auto ik = ij + 1; ik != neighQuery.results().end(); ++ik) {
			*p++ = (ik->delta + ij->delta).squaredLength();
		}
	}

    // Find NN/2 smallest pair distances from the list.
	std::partial_sort(pairs, pairs + (numNN/2), p);

    // Centrosymmetry = sum of numNN/2 smallest squared values.
    return std::accumulate(pairs, pairs + (numNN/2), FloatType(0), std::plus<FloatType>());
}

/******************************************************************************
* Unpacks the results of the computation engine and stores them in the modifier.
******************************************************************************/
void CentroSymmetryModifier::transferComputationResults(ComputeEngine* engine)
{
	_cspValues = static_cast<CentroSymmetryEngine*>(engine)->csp();
}

/******************************************************************************
* Lets the modifier insert the cached computation results into the
* modification pipeline.
******************************************************************************/
PipelineStatus CentroSymmetryModifier::applyComputationResults(TimePoint time, TimeInterval& validityInterval)
{
	if(!_cspValues)
		throw Exception(tr("No computation results available."));

	if(inputParticleCount() != _cspValues->size())
		throw Exception(tr("The number of input particles has changed. The stored results have become invalid."));

	outputStandardProperty(_cspValues.data());
	return PipelineStatus::Success;
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void CentroSymmetryModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	AsynchronousParticleModifier::propertyChanged(field);

	// Recompute brightness values when the parameters have been changed.
	if(field == PROPERTY_FIELD(CentroSymmetryModifier::_numNeighbors))
		invalidateCachedResults();
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CentroSymmetryModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Centrosymmetry parameter"), rolloutParams, "particles.modifiers.centrosymmetry.html");

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(4);

	QGridLayout* layout2 = new QGridLayout();
	layout2->setContentsMargins(0,0,0,0);
	layout2->setSpacing(4);
	layout2->setColumnStretch(1, 1);
	layout1->addLayout(layout2);

	// Num neighbors parameter.
	IntegerParameterUI* numNeighborsPUI = new IntegerParameterUI(this, PROPERTY_FIELD(CentroSymmetryModifier::_numNeighbors));
	layout2->addWidget(numNeighborsPUI->label(), 0, 0);
	layout2->addLayout(numNeighborsPUI->createFieldLayout(), 0, 1);
	numNeighborsPUI->setMinValue(2);
	numNeighborsPUI->setMaxValue(MAX_CSP_NEIGHBORS);

	QLabel* infoLabel = new QLabel(tr("This parameter specifies the number of nearest neighbors in the underlying lattice of atoms. For FCC and BCC lattices, set this to 12 and 8 respectively. More generally, it must be a positive, even integer."));
	infoLabel->setWordWrap(true);
	layout1->addWidget(infoLabel);

	// Status label.
	layout1->addSpacing(10);
	layout1->addWidget(statusLabel());
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
