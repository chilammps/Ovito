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
 * \brief Contains the definition of the Viz::BondAngleAnalysisModifier class.
 */

#ifndef __OVITO_BOND_ANGLE_ANALYSIS_MODIFIER_H
#define __OVITO_BOND_ANGLE_ANALYSIS_MODIFIER_H

#include <core/Core.h>
#include <core/gui/properties/RefTargetListParameterUI.h>

#include <viz/modifier/AsynchronousParticleModifier.h>
#include <viz/data/ParticleTypeProperty.h>
#include <viz/data/ParticleType.h>

namespace Viz {

using namespace Ovito;

/**
 * \brief A modifier that performs the structure analysis developed by Ackland and Jones.
 *
 * See G. Ackland, PRB(2006)73:054104.
 */
class BondAngleAnalysisModifier : public AsynchronousParticleModifier
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
	class BondAngleAnalysisEngine : public AsynchronousParticleModifier::Engine
	{
	public:

		/// Constructor.
		BondAngleAnalysisEngine(ParticleProperty* positions, const SimulationCellData simCell) :
			_positions(positions), _simCell(simCell) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void compute(FutureInterfaceBase& futureInterface) override;

		/// Returns the property storage that contains the computed per-particle structure types.
		ParticleProperty* structures() const { return _structures.data(); }

	private:

		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<ParticleProperty> _structures;
		SimulationCellData _simCell;
	};

public:

	/// Default constructor.
	Q_INVOKABLE BondAngleAnalysisModifier();

	/// Returns the array of structure types that are assigned to the particles by this modifier.
	const ParticleTypeList& structureTypes() const { return _structureTypes; }

	/// Returns the computed per-particle structure types.
	const ParticleProperty& particleStructures() const { OVITO_CHECK_POINTER(_structureProperty.constData()); return *_structureProperty; }

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Create an instance of the ParticleType class to represent a structure type.
	void createStructureType(StructureType id, const QString& name, const Color& color);

	/// Creates and initializes a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<Engine> createEngine(TimePoint time) override;

	/// Unpacks the computation results stored in the given engine object.
	virtual void retrieveResults(Engine* engine) override;

private:

	/// This stores the cached results of the modifier, i.e. the structures assigned to the particles.
	QExplicitlySharedDataPointer<ParticleProperty> _structureProperty;

	/// Contains the list of structure types recognized by this analysis modifier.
	VectorReferenceField<ParticleType> _structureTypes;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Bond Angle Analysis");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_VECTOR_REFERENCE_FIELD(_structureTypes);
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

protected Q_SLOTS:

	/// Is called when the user has double-clicked on one of the structure types in the list widget.
	void onDoubleClickStructureType(const QModelIndex& index);

private:

	/// The parameter UI for the list of structure types.
	RefTargetListParameterUI* _structureTypesPUI;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_BOND_ANGLE_ANALYSIS_MODIFIER_H
