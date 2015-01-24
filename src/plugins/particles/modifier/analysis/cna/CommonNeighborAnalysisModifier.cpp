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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include <plugins/particles/util/NearestNeighborFinder.h>
#include <plugins/particles/util/CutoffNeighborFinder.h>
#include <plugins/particles/util/CutoffRadiusPresetsUI.h>

#include "CommonNeighborAnalysisModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, CommonNeighborAnalysisModifier, StructureIdentificationModifier);
SET_OVITO_OBJECT_EDITOR(CommonNeighborAnalysisModifier, CommonNeighborAnalysisModifierEditor);
DEFINE_FLAGS_PROPERTY_FIELD(CommonNeighborAnalysisModifier, _cutoff, "Cutoff", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(CommonNeighborAnalysisModifier, _adaptiveMode, "AdaptiveMode", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(CommonNeighborAnalysisModifier, _cutoff, "Cutoff radius");
SET_PROPERTY_FIELD_LABEL(CommonNeighborAnalysisModifier, _adaptiveMode, "Adaptive CNA");
SET_PROPERTY_FIELD_UNITS(CommonNeighborAnalysisModifier, _cutoff, WorldParameterUnit);

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, CommonNeighborAnalysisModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
CommonNeighborAnalysisModifier::CommonNeighborAnalysisModifier(DataSet* dataset) : StructureIdentificationModifier(dataset),
	_cutoff(3.2), _adaptiveMode(true)
{
	INIT_PROPERTY_FIELD(CommonNeighborAnalysisModifier::_cutoff);
	INIT_PROPERTY_FIELD(CommonNeighborAnalysisModifier::_adaptiveMode);

	// Create the structure types.
	createStructureType(OTHER, tr("Other"));
	createStructureType(FCC, tr("FCC"));
	createStructureType(HCP, tr("HCP"));
	createStructureType(BCC, tr("BCC"));
	createStructureType(ICO, tr("ICO"));
	createStructureType(DIA, tr("DIA"));
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void CommonNeighborAnalysisModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	StructureIdentificationModifier::propertyChanged(field);

	// Recompute results when the parameters have been changed.
	if(field == PROPERTY_FIELD(CommonNeighborAnalysisModifier::_cutoff) ||
		field == PROPERTY_FIELD(CommonNeighborAnalysisModifier::_adaptiveMode))
		invalidateCachedResults();
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::ComputeEngine> CommonNeighborAnalysisModifier::createEngine(TimePoint time, TimeInterval validityInterval)
{
	if(structureTypes().size() != NUM_STRUCTURE_TYPES)
		throw Exception(tr("The number of structure types has changed. Please remove this modifier from the modification pipeline and insert it again."));

	// Get modifier input.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);
	SimulationCellObject* simCell = expectSimulationCell();

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	if(adaptiveMode())
		return std::make_shared<AdaptiveCNAEngine>(validityInterval, posProperty->storage(), simCell->data());
	else
		return std::make_shared<FixedCNAEngine>(validityInterval, posProperty->storage(), simCell->data(), cutoff());
}

/******************************************************************************
* Performs the actual analysis. This method is executed in a worker thread.
******************************************************************************/
void CommonNeighborAnalysisModifier::AdaptiveCNAEngine::perform()
{
	setProgressText(tr("Performing adaptive common neighbor analysis"));

	// Prepare the neighbor list.
	NearestNeighborFinder neighFinder(MAX_NEIGHBORS);
	if(!neighFinder.prepare(positions(), cell(), this))
		return;

	// Create output storage.
	ParticleProperty* output = structures();

	// Perform analysis on each particle.
	parallelFor(positions()->size(), *this, [&neighFinder, output](size_t index) {
		output->setInt(index, determineStructureAdaptive(neighFinder, index));
	});
}

/******************************************************************************
* Performs the actual analysis. This method is executed in a worker thread.
******************************************************************************/
void CommonNeighborAnalysisModifier::FixedCNAEngine::perform()
{
	setProgressText(tr("Performing common neighbor analysis"));

	// Prepare the neighbor list.
	CutoffNeighborFinder neighborListBuilder;
	if(!neighborListBuilder.prepare(_cutoff, positions(), cell(), this))
		return;

	// Create output storage.
	ParticleProperty* output = structures();

	// Perform analysis on each particle.
	parallelFor(positions()->size(), *this, [&neighborListBuilder, output](size_t index) {
		output->setInt(index, determineStructureFixed(neighborListBuilder, index));
	});
}

/******************************************************************************
* Find all atoms that are nearest neighbors of the given pair of atoms.
******************************************************************************/
int CommonNeighborAnalysisModifier::findCommonNeighbors(const NeighborBondArray& neighborArray, int neighborIndex, unsigned int& commonNeighbors, int numNeighbors)
{
	commonNeighbors = neighborArray.neighborArray[neighborIndex];
#ifndef Q_CC_MSVC
	// Count the number of bits set in neighbor bit field.
	return __builtin_popcount(commonNeighbors);
#else
	// Count the number of bits set in neighbor bit field.
	unsigned int v = commonNeighbors - ((commonNeighbors >> 1) & 0x55555555);
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
	return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
#endif
}

/******************************************************************************
* Finds all bonds between common nearest neighbors.
******************************************************************************/
int CommonNeighborAnalysisModifier::findNeighborBonds(const NeighborBondArray& neighborArray, unsigned int commonNeighbors, int numNeighbors, CNAPairBond* neighborBonds)
{
	int numBonds = 0;

	unsigned int nib[MAX_NEIGHBORS];
	int nibn = 0;
	unsigned int ni1b = 1;
	for(int ni1 = 0; ni1 < numNeighbors; ni1++, ni1b <<= 1) {
		if(commonNeighbors & ni1b) {
			unsigned int b = commonNeighbors & neighborArray.neighborArray[ni1];
			for(int n = 0; n < nibn; n++) {
				if(b & nib[n]) {
					OVITO_ASSERT(numBonds < MAX_NEIGHBORS*MAX_NEIGHBORS);
					neighborBonds[numBonds++] = ni1b | nib[n];
				}
			}

			nib[nibn++] = ni1b;
		}
	}
	return numBonds;
}

/******************************************************************************
* Find all chains of bonds.
******************************************************************************/
static int getAdjacentBonds(unsigned int atom, CommonNeighborAnalysisModifier::CNAPairBond* bondsToProcess, int& numBonds, unsigned int& atomsToProcess, unsigned int& atomsProcessed)
{
    int adjacentBonds = 0;
	for(int b = numBonds - 1; b >= 0; b--) {
		if(atom & *bondsToProcess) {
            ++adjacentBonds;
   			atomsToProcess |= *bondsToProcess & (~atomsProcessed);
   			memmove(bondsToProcess, bondsToProcess + 1, sizeof(CommonNeighborAnalysisModifier::CNAPairBond) * b);
   			numBonds--;
		}
		else ++bondsToProcess;
	}
	return adjacentBonds;
}

/******************************************************************************
* Find all chains of bonds between common neighbors and determine the length
* of the longest continuous chain.
******************************************************************************/
int CommonNeighborAnalysisModifier::calcMaxChainLength(CNAPairBond* neighborBonds, int numBonds)
{
    // Group the common bonds into clusters.
	int maxChainLength = 0;
	while(numBonds) {
        // Make a new cluster starting with the first remaining bond to be processed.
		numBonds--;
        unsigned int atomsToProcess = neighborBonds[numBonds];
        unsigned int atomsProcessed = 0;
		int clusterSize = 1;
        do {
#ifndef Q_CC_MSVC
        	// Determine the number of trailing 0-bits in atomsToProcess, starting at the least significant bit position.
			int nextAtomIndex = __builtin_ctz(atomsToProcess);
#else
			unsigned long nextAtomIndex;
			_BitScanForward(&nextAtomIndex, atomsToProcess);
			OVITO_ASSERT(nextAtomIndex >= 0 && nextAtomIndex < 32);
#endif
			unsigned int nextAtom = 1 << nextAtomIndex;
        	atomsProcessed |= nextAtom;
			atomsToProcess &= ~nextAtom;
			clusterSize += getAdjacentBonds(nextAtom, neighborBonds, numBonds, atomsToProcess, atomsProcessed);
		}
        while(atomsToProcess);
        if(clusterSize > maxChainLength)
        	maxChainLength = clusterSize;
	}
	return maxChainLength;
}

/******************************************************************************
* Determines the coordination structure of a single particle using the
* adaptive common neighbor analysis method.
******************************************************************************/
CommonNeighborAnalysisModifier::StructureType CommonNeighborAnalysisModifier::determineStructureAdaptive(NearestNeighborFinder& neighFinder, size_t particleIndex)
{
	// Create neighbor list finder.
	NearestNeighborFinder::Query<MAX_NEIGHBORS> neighQuery(neighFinder);

	// Find N nearest neighbor of current atom.
	neighQuery.findNeighbors(neighFinder.particlePos(particleIndex));
	int numNeighbors = neighQuery.results().size();

	{ /////////// 12 neighbors ///////////

	// Number of neighbors to analyze.
	int nn = 12; // For FCC, HCP and Icosahedral atoms

	// Early rejection of under-coordinated atoms:
	if(numNeighbors < nn)
		return OTHER;

	// Compute scaling factor.
	FloatType localScaling = 0;
	for(int n = 0; n < nn; n++)
		localScaling += sqrt(neighQuery.results()[n].distanceSq);
	FloatType localCutoff = localScaling / nn * (1.0 + sqrt(2.0)) / 2;
	FloatType localCutoffSquared =  localCutoff * localCutoff;

	// Compute common neighbor bit-flag array.
	NeighborBondArray neighborArray;
	for(int ni1 = 0; ni1 < nn; ni1++) {
		neighborArray.setNeighborBond(ni1, ni1, false);
		for(int ni2 = ni1+1; ni2 < nn; ni2++)
			neighborArray.setNeighborBond(ni1, ni2, (neighQuery.results()[ni1].delta - neighQuery.results()[ni2].delta).squaredLength() <= localCutoffSquared);
	}

	int n421 = 0;
	int n422 = 0;
	int n555 = 0;
	for(int ni = 0; ni < nn; ni++) {

		// Determine number of neighbors the two atoms have in common.
		unsigned int commonNeighbors;
		int numCommonNeighbors = findCommonNeighbors(neighborArray, ni, commonNeighbors, nn);
		if(numCommonNeighbors != 4 && numCommonNeighbors != 5)
			break;

		// Determine the number of bonds among the common neighbors.
		CNAPairBond neighborBonds[MAX_NEIGHBORS*MAX_NEIGHBORS];
		int numNeighborBonds = findNeighborBonds(neighborArray, commonNeighbors, nn, neighborBonds);
		if(numNeighborBonds != 2 && numNeighborBonds != 5)
			break;

		// Determine the number of bonds in the longest continuous chain.
		int maxChainLength = calcMaxChainLength(neighborBonds, numNeighborBonds);
		if(numCommonNeighbors == 4 && numNeighborBonds == 2) {
			if(maxChainLength == 1) n421++;
			else if(maxChainLength == 2) n422++;
			else break;
		}
		else if(numCommonNeighbors == 5 && numNeighborBonds == 5 && maxChainLength == 5) n555++;
		else break;
	}
	if(n421 == 12) return FCC;
	else if(n421 == 6 && n422 == 6) return HCP;
	else if(n555 == 12) return ICO;

	}

	{ /////////// 14 neighbors ///////////

	// Number of neighbors to analyze.
	int nn = 14; // For BCC atoms

	// Early rejection of under-coordinated atoms:
	if(numNeighbors < nn)
		return OTHER;

	// Compute scaling factor.
	FloatType localScaling = 0;
	for(int n = 0; n < 8; n++)
		localScaling += sqrt(neighQuery.results()[n].distanceSq / (3.0/4.0));
	for(int n = 8; n < 14; n++)
		localScaling += sqrt(neighQuery.results()[n].distanceSq);
	FloatType localCutoff = localScaling / nn * 1.207;
	FloatType localCutoffSquared =  localCutoff * localCutoff;

	// Compute common neighbor bit-flag array.
	NeighborBondArray neighborArray;
	for(int ni1 = 0; ni1 < nn; ni1++) {
		neighborArray.setNeighborBond(ni1, ni1, false);
		for(int ni2 = ni1+1; ni2 < nn; ni2++)
			neighborArray.setNeighborBond(ni1, ni2, (neighQuery.results()[ni1].delta - neighQuery.results()[ni2].delta).squaredLength() <= localCutoffSquared);
	}

	int n444 = 0;
	int n666 = 0;
	for(int ni = 0; ni < nn; ni++) {

		// Determine number of neighbors the two atoms have in common.
		unsigned int commonNeighbors;
		int numCommonNeighbors = findCommonNeighbors(neighborArray, ni, commonNeighbors, nn);
		if(numCommonNeighbors != 4 && numCommonNeighbors != 6)
			break;

		// Determine the number of bonds among the common neighbors.
		CNAPairBond neighborBonds[MAX_NEIGHBORS*MAX_NEIGHBORS];
		int numNeighborBonds = findNeighborBonds(neighborArray, commonNeighbors, nn, neighborBonds);
		if(numNeighborBonds != 4 && numNeighborBonds != 6)
			break;

		// Determine the number of bonds in the longest continuous chain.
		int maxChainLength = calcMaxChainLength(neighborBonds, numNeighborBonds);
		if(numCommonNeighbors == 4 && numNeighborBonds == 4 && maxChainLength == 4) n444++;
		else if(numCommonNeighbors == 6 && numNeighborBonds == 6 && maxChainLength == 6) n666++;
		else break;
	}
	if(n666 == 8 && n444 == 6) return BCC;

	}

	{ /////////// 16 neighbors ///////////

	// Detect DIA atoms having 16 NN. Detection according to http://arxiv.org/pdf/1202.5005.pdf

	// Number of neighbors to analyze.
	int nn = 16;

	// Early rejection of under-coordinated atoms:
	if(numNeighbors < nn)
		return OTHER;

	// Compute scaling factor.
	FloatType localScaling = 0;
	for(int n = 0; n < 4; n++)
		localScaling += sqrt(neighQuery.results()[n].distanceSq / (3.0/16.0));
	for(int n = 4; n < 16; n++)
		localScaling += sqrt(neighQuery.results()[n].distanceSq / (2.0/4.0));
	FloatType localCutoff = localScaling / nn * 0.7681;
	FloatType localCutoffSquared =  localCutoff * localCutoff;

	// Compute common neighbor bit-flag array.
	NeighborBondArray neighborArray;
	for(int ni1 = 0; ni1 < nn; ni1++) {
		neighborArray.setNeighborBond(ni1, ni1, false);
		for(int ni2 = ni1+1; ni2 < nn; ni2++)
			neighborArray.setNeighborBond(ni1, ni2, (neighQuery.results()[ni1].delta - neighQuery.results()[ni2].delta).squaredLength() <= localCutoffSquared);
	}

	int n543 = 0;
	int n663 = 0;
	for(int ni = 0; ni < nn; ni++) {

		// Determine number of neighbors the two atoms have in common.
		unsigned int commonNeighbors;
		int numCommonNeighbors = findCommonNeighbors(neighborArray, ni, commonNeighbors, nn);
		if(numCommonNeighbors != 5 && numCommonNeighbors != 6)
			break;

		// Determine the number of bonds among the common neighbors.
		CNAPairBond neighborBonds[MAX_NEIGHBORS*MAX_NEIGHBORS];
		int numNeighborBonds = findNeighborBonds(neighborArray, commonNeighbors, nn, neighborBonds);
		if(numNeighborBonds != 4 && numNeighborBonds != 6)
			break;

		// Determine the number of bonds in the longest continuous chain.
		int maxChainLength = calcMaxChainLength(neighborBonds, numNeighborBonds);
		if(numCommonNeighbors == 5 && numNeighborBonds == 4 && maxChainLength == 3) n543++;
		else if(numCommonNeighbors == 6 && numNeighborBonds == 6 && maxChainLength == 3) n663++;
		else break;
	}
	if(n543 == 12 && n663 == 4) return DIA;

	}

	return OTHER;
}

/******************************************************************************
* Determines the coordination structure of a single particle using the
* conventional common neighbor analysis method.
******************************************************************************/
CommonNeighborAnalysisModifier::StructureType CommonNeighborAnalysisModifier::determineStructureFixed(CutoffNeighborFinder& neighList, size_t particleIndex)
{
	// Store neighbor vectors in a local array.
	int numNeighbors = 0;
	Vector3 neighborVectors[MAX_NEIGHBORS];
	for(CutoffNeighborFinder::Query neighborQuery(neighList, particleIndex); !neighborQuery.atEnd(); neighborQuery.next()) {
		if(numNeighbors == MAX_NEIGHBORS) return OTHER;
		neighborVectors[numNeighbors] = neighborQuery.delta();
		numNeighbors++;
	}

	if(numNeighbors != 12 && numNeighbors != 14 && numNeighbors != 16)
		return OTHER;

	// Compute bond bit-flag array.
	NeighborBondArray neighborArray;
	for(int ni1 = 0; ni1 < numNeighbors; ni1++) {
		neighborArray.setNeighborBond(ni1, ni1, false);
		for(int ni2 = ni1+1; ni2 < numNeighbors; ni2++)
			neighborArray.setNeighborBond(ni1, ni2, (neighborVectors[ni1] - neighborVectors[ni2]).squaredLength() <= neighList.cutoffRadiusSquared());
	}

	if(numNeighbors == 12) { // Detect FCC and HCP atoms each having 12 NN.
		int n421 = 0;
		int n422 = 0;
		int n555 = 0;
		for(int ni = 0; ni < 12; ni++) {

			// Determine number of neighbors the two atoms have in common.
			unsigned int commonNeighbors;
			int numCommonNeighbors = findCommonNeighbors(neighborArray, ni, commonNeighbors, 12);
			if(numCommonNeighbors != 4 && numCommonNeighbors != 5)
				return OTHER;

			// Determine the number of bonds among the common neighbors.
			CNAPairBond neighborBonds[MAX_NEIGHBORS*MAX_NEIGHBORS];
			int numNeighborBonds = findNeighborBonds(neighborArray, commonNeighbors, 12, neighborBonds);
			if(numNeighborBonds != 2 && numNeighborBonds != 5)
				break;

			// Determine the number of bonds in the longest continuous chain.
			int maxChainLength = calcMaxChainLength(neighborBonds, numNeighborBonds);
			if(numCommonNeighbors == 4 && numNeighborBonds == 2) {
				if(maxChainLength == 1) n421++;
				else if(maxChainLength == 2) n422++;
				else return OTHER;
			}
			else if(numCommonNeighbors == 5 && numNeighborBonds == 5 && maxChainLength == 5) n555++;
			else return OTHER;
		}
		if(n421 == 12) return FCC;
		else if(n421 == 6 && n422 == 6) return HCP;
		else if(n555 == 12) return ICO;
	}
	else if(numNeighbors == 14) { // Detect BCC atoms having 14 NN (in 1st and 2nd shell).
		int n444 = 0;
		int n666 = 0;
		for(int ni = 0; ni < 14; ni++) {

			// Determine number of neighbors the two atoms have in common.
			unsigned int commonNeighbors;
			int numCommonNeighbors = findCommonNeighbors(neighborArray, ni, commonNeighbors, 14);
			if(numCommonNeighbors != 4 && numCommonNeighbors != 6)
				return OTHER;

			// Determine the number of bonds among the common neighbors.
			CNAPairBond neighborBonds[MAX_NEIGHBORS*MAX_NEIGHBORS];
			int numNeighborBonds = findNeighborBonds(neighborArray, commonNeighbors, 14, neighborBonds);
			if(numNeighborBonds != 4 && numNeighborBonds != 6)
				break;

			// Determine the number of bonds in the longest continuous chain.
			int maxChainLength = calcMaxChainLength(neighborBonds, numNeighborBonds);
			if(numCommonNeighbors == 4 && numNeighborBonds == 4 && maxChainLength == 4) n444++;
			else if(numCommonNeighbors == 6 && numNeighborBonds == 6 && maxChainLength == 6) n666++;
			else return OTHER;
		}
		if(n666 == 8 && n444 == 6) return BCC;
	}
	else if(numNeighbors == 16) { // Detect DIA atoms having 16 NN. Detection according to http://arxiv.org/pdf/1202.5005.pdf
		int n543 = 0;
		int n663 = 0;
		for(int ni = 0; ni < numNeighbors; ni++) {

			// Determine number of neighbors the two atoms have in common.
			unsigned int commonNeighbors;
			int numCommonNeighbors = findCommonNeighbors(neighborArray, ni, commonNeighbors, numNeighbors);
			if(numCommonNeighbors != 5 && numCommonNeighbors != 6)
				return OTHER;

			// Determine the number of bonds among the common neighbors.
			CNAPairBond neighborBonds[MAX_NEIGHBORS*MAX_NEIGHBORS];
			int numNeighborBonds = findNeighborBonds(neighborArray, commonNeighbors, numNeighbors, neighborBonds);
			if(numNeighborBonds != 4 && numNeighborBonds != 6)
				break;

			// Determine the number of bonds in the longest continuous chain.
			int maxChainLength = calcMaxChainLength(neighborBonds, numNeighborBonds);
			if(numCommonNeighbors == 5 && numNeighborBonds == 4 && maxChainLength == 3) n543++;
			else if(numCommonNeighbors == 6 && numNeighborBonds == 6 && maxChainLength == 3) n663++;
			else return OTHER;
		}
		if(n543 == 12 && n663 == 4) return DIA;
	}

	return OTHER;
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CommonNeighborAnalysisModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Common neighbor analysis"), rolloutParams, "particles.modifiers.common_neighbor_analysis.html");

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(6);

	BooleanRadioButtonParameterUI* adaptiveModeUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(CommonNeighborAnalysisModifier::_adaptiveMode));
	adaptiveModeUI->buttonTrue()->setText(tr("Adaptive CNA (variable cutoff)"));
	adaptiveModeUI->buttonFalse()->setText(tr("Conventional CNA (fixed cutoff)"));
	layout1->addWidget(adaptiveModeUI->buttonTrue());
	layout1->addWidget(adaptiveModeUI->buttonFalse());

	QGridLayout* gridlayout = new QGridLayout();
	gridlayout->setContentsMargins(0,0,0,0);
	gridlayout->setColumnStretch(2, 1);
	gridlayout->setColumnMinimumWidth(0, 20);

	// Cutoff parameter.
	FloatParameterUI* cutoffRadiusPUI = new FloatParameterUI(this, PROPERTY_FIELD(CommonNeighborAnalysisModifier::_cutoff));
	gridlayout->addWidget(cutoffRadiusPUI->label(), 0, 1);
	gridlayout->addLayout(cutoffRadiusPUI->createFieldLayout(), 0, 2);
	cutoffRadiusPUI->setMinValue(0);

	CutoffRadiusPresetsUI* cutoffPresetsPUI = new CutoffRadiusPresetsUI(this, PROPERTY_FIELD(CommonNeighborAnalysisModifier::_cutoff));
	gridlayout->addWidget(cutoffPresetsPUI->comboBox(), 1, 1, 1, 2);
	layout1->addLayout(gridlayout);

	connect(adaptiveModeUI->buttonFalse(), &QRadioButton::toggled, cutoffRadiusPUI, &FloatParameterUI::setEnabled);
	connect(adaptiveModeUI->buttonFalse(), &QRadioButton::toggled, cutoffPresetsPUI, &CutoffRadiusPresetsUI::setEnabled);
	cutoffRadiusPUI->setEnabled(false);
	cutoffPresetsPUI->setEnabled(false);

	// Status label.
	layout1->addSpacing(10);
	layout1->addWidget(statusLabel());

	StructureListParameterUI* structureTypesPUI = new StructureListParameterUI(this);
	layout1->addSpacing(10);
	layout1->addWidget(new QLabel(tr("Structure types:")));
	layout1->addWidget(structureTypesPUI->tableWidget());
	layout1->addWidget(new QLabel(tr("(Double-click to change colors)")));
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
