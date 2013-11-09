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
 * \file BondAngleAnalysisModifier.h
 * \brief Contains the definition of the Particles::BondAngleAnalysisModifier class.
 */

#ifndef __OVITO_BOND_ANGLE_ANALYSIS_MODIFIER_H
#define __OVITO_BOND_ANGLE_ANALYSIS_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <core/gui/properties/RefTargetListParameterUI.h>
#include <plugins/particles/modifier/analysis/StructureIdentificationModifier.h>

namespace Particles {

using namespace Ovito;

class TreeNeighborListBuilder;

/**
 * \brief A modifier that performs the structure analysis developed by Ackland and Jones.
 *
 * See G. Ackland, PRB(2006)73:054104.
 */
class OVITO_PARTICLES_EXPORT BondAngleAnalysisModifier : public StructureIdentificationModifier
{
public:

	/// The structure types recognized by the bond angle analysis.
	enum StructureType {
		OTHER = 0,				//< Unidentified structure
		FCC,					//< Face-centered cubic
		HCP,					//< Hexagonal close-packed
		BCC,					//< Body-centered cubic
		ICO,					//< Icosahedral structure

		NUM_STRUCTURE_TYPES 	//< This just counts the number of defined structure types.
	};

	/// Computes the modifier's results.
	class BondAngleAnalysisEngine : public StructureIdentificationModifier::StructureIdentificationEngine
	{
	public:

		/// Constructor.
		BondAngleAnalysisEngine(ParticleProperty* positions, const SimulationCellData& simCell) :
			StructureIdentificationModifier::StructureIdentificationEngine(positions, simCell) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void compute(FutureInterfaceBase& futureInterface) override;
	};

public:

	/// Default constructor.
	Q_INVOKABLE BondAngleAnalysisModifier();

protected:

	/// Creates and initializes a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<Engine> createEngine(TimePoint time) override;

	/// Determines the coordination structure of a single particle using the bond-angle analysis method.
	static StructureType determineStructure(TreeNeighborListBuilder& neighList, size_t particleIndex);

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Bond-angle analysis");
	Q_CLASSINFO("ModifierCategory", "Analysis");
};

/**
 * \brief A properties editor for the BondAngleAnalysisModifier class.
 */
class BondAngleAnalysisModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE BondAngleAnalysisModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_BOND_ANGLE_ANALYSIS_MODIFIER_H
