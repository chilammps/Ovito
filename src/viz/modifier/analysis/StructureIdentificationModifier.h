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
 * \file StructureIdentificationModifier.h
 * \brief Contains the definition of the Viz::StructureIdentificationModifier class.
 */

#ifndef __OVITO_STRUCTURE_IDENTIFICATION_MODIFIER_H
#define __OVITO_STRUCTURE_IDENTIFICATION_MODIFIER_H

#include <core/Core.h>
#include <core/gui/properties/RefTargetListParameterUI.h>

#include <viz/modifier/AsynchronousParticleModifier.h>
#include <viz/data/ParticleTypeProperty.h>
#include <viz/data/ParticleType.h>

namespace Viz {

using namespace Ovito;

/**
 * \brief Base class for modifiers that assign a structure type to each particle.
 */
class StructureIdentificationModifier : public AsynchronousParticleModifier
{
public:

	/// Computes the modifier's results.
	class StructureIdentificationEngine : public AsynchronousParticleModifier::Engine
	{
	public:

		/// Constructor.
		StructureIdentificationEngine(ParticleProperty* positions, const SimulationCellData& simCell) :
			_positions(positions), _simCell(simCell),
			_structures(new ParticleProperty(positions->size(), ParticleProperty::StructureTypeProperty)) {}

		/// Returns the property storage that contains the input particle positions.
		ParticleProperty* positions() const { return _positions.data(); }

		/// Returns the property storage that contains the computed per-particle structure types.
		ParticleProperty* structures() const { return _structures.data(); }

		/// Returns the simulation cell data.
		const SimulationCellData& cell() const { return _simCell; }

	private:

		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<ParticleProperty> _structures;
		SimulationCellData _simCell;
	};

public:

	/// Default constructor.
	StructureIdentificationModifier();

	/// Returns the array of structure types that are assigned to the particles by this modifier.
	const ParticleTypeList& structureTypes() const { return _structureTypes; }

	/// Returns the computed per-particle structure types.
	const ParticleProperty& particleStructures() const { OVITO_CHECK_POINTER(_structureProperty.constData()); return *_structureProperty; }

	/// Returns a map that contains the number of matching particles for each structure type.
	const std::map<int,size_t>& structureCounts() const { return _structureCounts; }

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Create an instance of the ParticleType class to represent a structure type.
	void createStructureType(int id, const QString& name, const Color& color);

	/// Unpacks the computation results stored in the given engine object.
	virtual void retrieveModifierResults(Engine* engine) override;

	/// Inserts the computed and cached modifier results into the modification pipeline.
	virtual ObjectStatus applyModifierResults(TimePoint time, TimeInterval& validityInterval) override;

private:

	/// This stores the cached results of the modifier, i.e. the structures assigned to the particles.
	QExplicitlySharedDataPointer<ParticleProperty> _structureProperty;

	/// Contains the list of structure types recognized by this analysis modifier.
	VectorReferenceField<ParticleType> _structureTypes;

	/// The number of matching particles for each structure type.
	std::map<int,size_t> _structureCounts;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_structureTypes);
};

/**
 * List box that displays the structure types.
 */
class StructureListParameterUI : public RefTargetListParameterUI
{
public:

	StructureListParameterUI(PropertiesEditor* parentEditor)
		: RefTargetListParameterUI(parentEditor, PROPERTY_FIELD(StructureIdentificationModifier::_structureTypes), RolloutInsertionParameters(), nullptr) {
		connect(tableWidget(220), SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onDoubleClickStructureType(const QModelIndex&)));
		tableWidget()->setAutoScroll(false);
	}

protected:

	/// Returns a data item from the list data model.
	virtual QVariant getItemData(RefTarget* target, const QModelIndex& index, int role) override;

	/// Returns the number of columns for the table view.
	virtual int tableColumnCount() override { return 4; }

	/// Returns the header data under the given role for the given RefTarget.
	virtual QVariant getHorizontalHeaderData(int index, int role) override {
		if(index == 0)
			return QVariant(tr("Color"));
		else if(index == 1)
			return QVariant(tr("Name"));
		else if(index == 2)
			return QVariant(tr("Count"));
		else
			return QVariant(tr("Fraction"));
	}

	/// Do not open sub-editor for selected atom type.
	virtual void openSubEditor() override {}

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

protected Q_SLOTS:

	/// Is called when the user has double-clicked on one of the structure types in the list widget.
	void onDoubleClickStructureType(const QModelIndex& index);

private:

	Q_OBJECT
};

};	// End of namespace

#endif // __OVITO_STRUCTURE_IDENTIFICATION_MODIFIER_H
