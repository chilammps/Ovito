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

#include <core/Core.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/animation/AnimManager.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include <viz/util/TreeNeighborListBuilder.h>
#include <viz/util/OnTheFlyNeighborListBuilder.h>

#include "CommonNeighborAnalysisModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, CommonNeighborAnalysisModifier, StructureIdentificationModifier)
IMPLEMENT_OVITO_OBJECT(Viz, CommonNeighborAnalysisModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(CommonNeighborAnalysisModifier, CommonNeighborAnalysisModifierEditor)
DEFINE_PROPERTY_FIELD(CommonNeighborAnalysisModifier, _cutoff, "Cutoff")
DEFINE_PROPERTY_FIELD(CommonNeighborAnalysisModifier, _adaptiveMode, "AdaptiveMode")
SET_PROPERTY_FIELD_LABEL(CommonNeighborAnalysisModifier, _cutoff, "Cutoff radius")
SET_PROPERTY_FIELD_LABEL(CommonNeighborAnalysisModifier, _adaptiveMode, "Adaptive CNA")
SET_PROPERTY_FIELD_UNITS(CommonNeighborAnalysisModifier, _cutoff, WorldParameterUnit)

// The maximum number of neighbor atoms taken into account for the common neighbor analysis.
#define CNA_MAX_PATTERN_NEIGHBORS 16

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
CommonNeighborAnalysisModifier::CommonNeighborAnalysisModifier() :
	_cutoff(3), _adaptiveMode(true)
{
	INIT_PROPERTY_FIELD(CommonNeighborAnalysisModifier::_cutoff);
	INIT_PROPERTY_FIELD(CommonNeighborAnalysisModifier::_adaptiveMode);

	// Create the structure types.
	createStructureType(OTHER, tr("Other"), Color(0.95f, 0.95f, 0.95f));
	createStructureType(FCC, tr("FCC"), Color(0.4f, 1.0f, 0.4f));
	createStructureType(HCP, tr("HCP"), Color(1.0f, 0.4f, 0.4f));
	createStructureType(BCC, tr("BCC"), Color(0.4f, 0.4f, 1.0f));
	createStructureType(ICO, tr("ICO"), Color(0.95f, 0.8f, 0.2f));
	createStructureType(DIA, tr("DIA"), Color(0.2f, 0.95f, 0.8f));

	// Load the default cutoff radius stored in the application settings.
	QSettings settings;
	settings.beginGroup("viz/cna");
	setCutoff(settings.value("DefaultCutoff", 0.0).value<FloatType>());
	settings.endGroup();
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void CommonNeighborAnalysisModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	// Recompute results when the parameters have been changed.
	if(autoUpdateEnabled()) {
		if(field == PROPERTY_FIELD(CommonNeighborAnalysisModifier::_cutoff) ||
			field == PROPERTY_FIELD(CommonNeighborAnalysisModifier::_adaptiveMode))
			invalidateCachedResults();
	}

	StructureIdentificationModifier::propertyChanged(field);
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::Engine> CommonNeighborAnalysisModifier::createEngine(TimePoint time)
{
	if(structureTypes().size() != NUM_STRUCTURE_TYPES)
		throw Exception(tr("The number of structure types has changed. Please remove this modifier from the modification pipeline and insert it again."));

	// Get modifier input.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);
	SimulationCell* simCell = expectSimulationCell();

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	if(adaptiveMode())
		return std::make_shared<AdaptiveCommonNeighborAnalysisEngine>(posProperty->storage(), simCell->data());
	else
		return std::make_shared<FixedCommonNeighborAnalysisEngine>(posProperty->storage(), simCell->data(), cutoff());
}

/******************************************************************************
* Performs the actual analysis. This method is executed in a worker thread.
******************************************************************************/
void CommonNeighborAnalysisModifier::AdaptiveCommonNeighborAnalysisEngine::compute(FutureInterfaceBase& futureInterface)
{
	size_t particleCount = positions()->size();
	futureInterface.setProgressText(tr("Performing adaptive common neighbor analysis"));

	// Prepare the neighbor list.
	TreeNeighborListBuilder neighborListBuilder(14);
	if(!neighborListBuilder.prepare(positions(), cell()) || futureInterface.isCanceled())
		return;

	// Create output storage.
	ParticleProperty* output = structures();

	// Perform analysis on each particle.
	parallelFor(particleCount, futureInterface, [&neighborListBuilder, output](size_t index) {
		output->setInt(index, determineStructureAdaptive(neighborListBuilder, index));
	});
}

/******************************************************************************
* Performs the actual analysis. This method is executed in a worker thread.
******************************************************************************/
void CommonNeighborAnalysisModifier::FixedCommonNeighborAnalysisEngine::compute(FutureInterfaceBase& futureInterface)
{
	size_t particleCount = positions()->size();
	futureInterface.setProgressText(tr("Performing common neighbor analysis"));

	// Prepare the neighbor list.
	OnTheFlyNeighborListBuilder neighborListBuilder(_cutoff);
	if(!neighborListBuilder.prepare(positions(), cell()) || futureInterface.isCanceled())
		return;

	// Create output storage.
	ParticleProperty* output = structures();

	// Perform analysis on each particle.
	parallelFor(particleCount, futureInterface, [&neighborListBuilder, output](size_t index) {
		output->setInt(index, determineStructureFixed(neighborListBuilder, index));
	});
}

/// Pair of neighbor atoms that form a bond (bit-wise storage).
typedef unsigned int CNAPairBond;

/**
 * A bit-flag array indicating which pairs of neighbors are bonded
 * and which are not.
 */
struct NeighborBondArray
{
	/// Default constructor.
	NeighborBondArray() {
		memset(neighborArray, 0, sizeof(neighborArray));
	}

	/// Two-dimensional bit array that stores the bonds between neighbors.
	unsigned int neighborArray[CNA_MAX_PATTERN_NEIGHBORS];

	/// Returns whether two nearest neighbors have a bond between them.
	inline bool neighborBond(int neighborIndex1, int neighborIndex2) const {
		OVITO_ASSERT(neighborIndex1 < CNA_MAX_PATTERN_NEIGHBORS);
		OVITO_ASSERT(neighborIndex2 < CNA_MAX_PATTERN_NEIGHBORS);
		return (neighborArray[neighborIndex1] & (1<<neighborIndex2));
	}

	/// Sets whether two nearest neighbors have a bond between them.
	void setNeighborBond(int neighborIndex1, int neighborIndex2, bool bonded) {
		OVITO_ASSERT(neighborIndex1 < CNA_MAX_PATTERN_NEIGHBORS);
		OVITO_ASSERT(neighborIndex2 < CNA_MAX_PATTERN_NEIGHBORS);
		if(bonded) {
			neighborArray[neighborIndex1] |= (1<<neighborIndex2);
			neighborArray[neighborIndex2] |= (1<<neighborIndex1);
		}
		else {
			neighborArray[neighborIndex1] &= ~(1<<neighborIndex2);
			neighborArray[neighborIndex2] &= ~(1<<neighborIndex1);
		}
	}
};

/******************************************************************************
* Find all atoms that are nearest neighbors of the given pair of atoms.
******************************************************************************/
static int findCommonNeighbors(const NeighborBondArray& neighborArray, int neighborIndex, unsigned int& commonNeighbors, int numNeighbors)
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
static int findNeighborBonds(const NeighborBondArray& neighborArray, unsigned int commonNeighbors, int numNeighbors, CNAPairBond* neighborBonds)
{
	int numBonds = 0;

	unsigned int nib[CNA_MAX_PATTERN_NEIGHBORS];
	int nibn = 0;
	unsigned int ni1b = 1;
	for(int ni1 = 0; ni1 < numNeighbors; ni1++, ni1b <<= 1) {
		if(commonNeighbors & ni1b) {
			unsigned int b = commonNeighbors & neighborArray.neighborArray[ni1];
			for(int n = 0; n < nibn; n++) {
				if(b & nib[n]) {
					OVITO_ASSERT(numBonds < CNA_MAX_PATTERN_NEIGHBORS*CNA_MAX_PATTERN_NEIGHBORS);
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
static int getAdjacentBonds(unsigned int atom, CNAPairBond* bondsToProcess, int& numBonds, unsigned int& atomsToProcess, unsigned int& atomsProcessed)
{
    int adjacentBonds = 0;
	for(int b = numBonds - 1; b >= 0; b--) {
		if(atom & *bondsToProcess) {
            ++adjacentBonds;
   			atomsToProcess |= *bondsToProcess & (~atomsProcessed);
   			memmove(bondsToProcess, bondsToProcess + 1, sizeof(CNAPairBond) * b);
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
static int calcMaxChainLength(CNAPairBond* neighborBonds, int numBonds)
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
CommonNeighborAnalysisModifier::StructureType CommonNeighborAnalysisModifier::determineStructureAdaptive(TreeNeighborListBuilder& neighList, size_t particleIndex)
{
	// Create neighbor list finder.
	TreeNeighborListBuilder::Locator<CNA_MAX_PATTERN_NEIGHBORS> loc(neighList);

	// Find N nearest neighbor of current atom.
	loc.findNeighbors(neighList.particlePos(particleIndex));

	// Early rejection of under-coordinated atoms:
	int numNeighbors = loc.results().size();

	{ /////////// 12 neighbors ///////////

	// Number of neighbors to analyze.
	int nn = 12; // For FCC, HCP and Icosahedral atoms

	// Early rejection of under-coordinated atoms:
	if(numNeighbors < nn)
		return OTHER;

	// Compute scaling factor.
	FloatType localScaling = 0;
	for(int n = 0; n < nn; n++)
		localScaling += sqrt(loc.results()[n].distanceSq);
	FloatType localCutoff = localScaling / nn * (1.0 + sqrt(2.0)) / 2;
	FloatType localCutoffSquared =  localCutoff * localCutoff;

	// Compute common neighbor bit-flag array.
	NeighborBondArray neighborArray;
	for(int ni1 = 0; ni1 < nn; ni1++) {
		neighborArray.setNeighborBond(ni1, ni1, false);
		for(int ni2 = ni1+1; ni2 < nn; ni2++)
			neighborArray.setNeighborBond(ni1, ni2, (loc.results()[ni1].delta - loc.results()[ni2].delta).squaredLength() <= localCutoffSquared);
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
		CNAPairBond neighborBonds[CNA_MAX_PATTERN_NEIGHBORS*CNA_MAX_PATTERN_NEIGHBORS];
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
		localScaling += sqrt(loc.results()[n].distanceSq / (3.0/4.0));
	for(int n = 8; n < 14; n++)
		localScaling += sqrt(loc.results()[n].distanceSq);
	FloatType localCutoff = localScaling / nn * 1.207;
	FloatType localCutoffSquared =  localCutoff * localCutoff;

	// Compute common neighbor bit-flag array.
	NeighborBondArray neighborArray;
	for(int ni1 = 0; ni1 < nn; ni1++) {
		neighborArray.setNeighborBond(ni1, ni1, false);
		for(int ni2 = ni1+1; ni2 < nn; ni2++)
			neighborArray.setNeighborBond(ni1, ni2, (loc.results()[ni1].delta - loc.results()[ni2].delta).squaredLength() <= localCutoffSquared);
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
		CNAPairBond neighborBonds[CNA_MAX_PATTERN_NEIGHBORS*CNA_MAX_PATTERN_NEIGHBORS];
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

	// Number of neighbors to analyze.
	int nn = 16; // For BCC atoms

	// Early rejection of under-coordinated atoms:
	if(numNeighbors < nn)
		return OTHER;

	// Compute scaling factor.
	FloatType localScaling = 0;
	for(int n = 0; n < 4; n++)
		localScaling += sqrt(loc.results()[n].distanceSq / (3.0/16.0));
	for(int n = 4; n < 16; n++)
		localScaling += sqrt(loc.results()[n].distanceSq / (2.0/4.0));
	FloatType localCutoff = localScaling / nn * 0.7681;
	FloatType localCutoffSquared =  localCutoff * localCutoff;

	// Compute common neighbor bit-flag array.
	NeighborBondArray neighborArray;
	for(int ni1 = 0; ni1 < nn; ni1++) {
		neighborArray.setNeighborBond(ni1, ni1, false);
		for(int ni2 = ni1+1; ni2 < nn; ni2++)
			neighborArray.setNeighborBond(ni1, ni2, (loc.results()[ni1].delta - loc.results()[ni2].delta).squaredLength() <= localCutoffSquared);
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
		CNAPairBond neighborBonds[CNA_MAX_PATTERN_NEIGHBORS*CNA_MAX_PATTERN_NEIGHBORS];
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
CommonNeighborAnalysisModifier::StructureType CommonNeighborAnalysisModifier::determineStructureFixed(OnTheFlyNeighborListBuilder& neighList, size_t particleIndex)
{
	// Store neighbor vectors in a local array.
	int numNeighbors = 0;
	Vector3 neighborVectors[CNA_MAX_PATTERN_NEIGHBORS];
	for(OnTheFlyNeighborListBuilder::iterator neighborIter(neighList, particleIndex); !neighborIter.atEnd(); neighborIter.next()) {
		if(numNeighbors == CNA_MAX_PATTERN_NEIGHBORS) return OTHER;
		neighborVectors[numNeighbors] = neighborIter.delta();
		numNeighbors++;
	}

	if(numNeighbors == 12) { // Detect FCC and HCP atoms each having 12 NN.

		// Compute bond bit-flag array.
		NeighborBondArray neighborArray;
		for(int ni1 = 0; ni1 < 12; ni1++) {
			neighborArray.setNeighborBond(ni1, ni1, false);
			for(int ni2 = ni1+1; ni2 < 12; ni2++)
				neighborArray.setNeighborBond(ni1, ni2, (neighborVectors[ni1] - neighborVectors[ni2]).squaredLength() <= neighList.cutoffRadiusSquared());
		}

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
			CNAPairBond neighborBonds[CNA_MAX_PATTERN_NEIGHBORS*CNA_MAX_PATTERN_NEIGHBORS];
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

		// Compute bond bit-flag array.
		NeighborBondArray neighborArray;
		for(int ni1 = 0; ni1 < 14; ni1++) {
			neighborArray.setNeighborBond(ni1, ni1, false);
			for(int ni2 = ni1+1; ni2 < 14; ni2++)
				neighborArray.setNeighborBond(ni1, ni2, (neighborVectors[ni1] - neighborVectors[ni2]).squaredLength() <= neighList.cutoffRadiusSquared());
		}

		int n444 = 0;
		int n666 = 0;
		for(int ni = 0; ni < 14; ni++) {

			// Determine number of neighbors the two atoms have in common.
			unsigned int commonNeighbors;
			int numCommonNeighbors = findCommonNeighbors(neighborArray, ni, commonNeighbors, 14);
			if(numCommonNeighbors != 4 && numCommonNeighbors != 6)
				return OTHER;

			// Determine the number of bonds among the common neighbors.
			CNAPairBond neighborBonds[CNA_MAX_PATTERN_NEIGHBORS*CNA_MAX_PATTERN_NEIGHBORS];
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

		// Compute bond bit-flag array.
		NeighborBondArray neighborArray;
		for(int ni1 = 0; ni1 < numNeighbors; ni1++) {
			neighborArray.setNeighborBond(ni1, ni1, false);
			for(int ni2 = ni1+1; ni2 < numNeighbors; ni2++)
				neighborArray.setNeighborBond(ni1, ni2, (neighborVectors[ni1] - neighborVectors[ni2]).squaredLength() <= neighList.cutoffRadiusSquared());
		}

		int n543 = 0;
		int n663 = 0;
		for(int ni = 0; ni < numNeighbors; ni++) {

			// Determine number of neighbors the two atoms have in common.
			unsigned int commonNeighbors;
			int numCommonNeighbors = findCommonNeighbors(neighborArray, ni, commonNeighbors, numNeighbors);
			if(numCommonNeighbors != 5 && numCommonNeighbors != 6)
				return OTHER;

			// Determine the number of bonds among the common neighbors.
			CNAPairBond neighborBonds[CNA_MAX_PATTERN_NEIGHBORS*CNA_MAX_PATTERN_NEIGHBORS];
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

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CommonNeighborAnalysisModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Common neighbor analysis"), rolloutParams);

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
	connect(cutoffRadiusPUI->spinner(), SIGNAL(spinnerValueChanged()), this, SLOT(memorizeCutoff()));

#if 0
	CutoffPresetsUI* cutoffPresetsPUI = new CutoffPresetsUI(this, PROPERTY_FIELD_DESCRIPTOR(CommonNeighborAnalysisModifier, _neighborCutoff));
	gridlayout->addWidget(cutoffPresetsPUI->comboBox(), 1, 1, 1, 2);
#endif
	layout1->addLayout(gridlayout);

	connect(adaptiveModeUI->buttonFalse(), SIGNAL(toggled(bool)), cutoffRadiusPUI, SLOT(setEnabled(bool)));
//	connect(adaptiveModeUI->buttonFalse(), SIGNAL(toggled(bool)), cutoffPresetsPUI, SLOT(setEnabled(bool)));
	cutoffRadiusPUI->setEnabled(false);
//	cutoffPresetsPUI->setEnabled(false);

#if 0
	BooleanParameterUI* autoUpdateUI = new BooleanParameterUI(this, PROPERTY_FIELD(AsynchronousParticleModifier::_autoUpdate));
	layout1->addWidget(autoUpdateUI->checkBox());
#endif

	// Status label.
	layout1->addSpacing(10);
	layout1->addWidget(statusLabel());

	StructureListParameterUI* structureTypesPUI = new StructureListParameterUI(this);
	layout1->addSpacing(10);
	layout1->addWidget(new QLabel(tr("Structure types:")));
	layout1->addWidget(structureTypesPUI->tableWidget());
	layout1->addWidget(new QLabel(tr("(Double-click to change colors)")));
}

/******************************************************************************
* Stores the current cutoff radius in the application settings
* so it can be used as default value for new modifiers in the future.
******************************************************************************/
void CommonNeighborAnalysisModifierEditor::memorizeCutoff()
{
	if(!editObject()) return;
	CommonNeighborAnalysisModifier* modifier = static_object_cast<CommonNeighborAnalysisModifier>(editObject());

	QSettings settings;
	settings.beginGroup("viz/cna");
	settings.setValue("DefaultCutoff", modifier->cutoff());
	settings.endGroup();
}

};	// End of namespace
