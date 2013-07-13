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

/**
 * \file CommonNeighborAnalysisModifier.h
 * \brief Contains the definition of the Viz::CommonNeighborAnalysisModifier class.
 */

#ifndef __OVITO_COMMON_NEIGHBOR_ANALYSIS_MODIFIER_H
#define __OVITO_COMMON_NEIGHBOR_ANALYSIS_MODIFIER_H

#include <core/Core.h>
#include <core/gui/properties/RefTargetListParameterUI.h>
#include <viz/modifier/analysis/StructureIdentificationModifier.h>

namespace Viz {

using namespace Ovito;

class TreeNeighborListBuilder;

/**
 * \brief A modifier that performs the common neighbor analysis (CNA) to identify
 *        local coordination structure.
 */
class CommonNeighborAnalysisModifier : public StructureIdentificationModifier
{
public:

	/// The structure types recognized by the common neighbor analysis.
	enum StructureType {
		OTHER = 0,				//< Unidentified structure
		FCC,					//< Face-centered cubic
		HCP,					//< Hexagonal close-packed
		BCC,					//< Body-centered cubic
		ICO,					//< Icosahedral structure
		DIA,					//< Cubic diamond structure

		NUM_STRUCTURE_TYPES 	//< This just counts the number of defined structure types.
	};

	/// Computes the modifier's results.
	class CommonNeighborAnalysisEngine : public StructureIdentificationModifier::StructureIdentificationEngine
	{
	public:

		/// Constructor.
		CommonNeighborAnalysisEngine(ParticleProperty* positions, const SimulationCellData simCell) :
			StructureIdentificationModifier::StructureIdentificationEngine(positions, simCell) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void compute(FutureInterfaceBase& futureInterface) override;
	};

public:

	/// Default constructor.
	Q_INVOKABLE CommonNeighborAnalysisModifier();

protected:

	/// Creates and initializes a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<Engine> createEngine(TimePoint time) override;

	/// Determines the coordination structure of a single particle using the common neighbor analysis method.
	static StructureType determineStructure(TreeNeighborListBuilder& neighList, size_t particleIndex);

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Common Neighbor Analysis");
	Q_CLASSINFO("ModifierCategory", "Analysis");
};

/**
 * \brief A properties editor for the CommonNeighborAnalysisModifier class.
 */
class CommonNeighborAnalysisModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE CommonNeighborAnalysisModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_COMMON_NEIGHBOR_ANALYSIS_MODIFIER_H
