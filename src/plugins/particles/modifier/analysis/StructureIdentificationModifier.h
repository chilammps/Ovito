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

#ifndef __OVITO_STRUCTURE_IDENTIFICATION_MODIFIER_H
#define __OVITO_STRUCTURE_IDENTIFICATION_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <core/gui/properties/RefTargetListParameterUI.h>

#include <plugins/particles/modifier/AsynchronousParticleModifier.h>
#include <plugins/particles/objects/ParticleTypeProperty.h>
#include <plugins/particles/objects/ParticleType.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

/**
 * \brief Base class for modifiers that assign a structure type to each particle.
 */
class OVITO_PARTICLES_EXPORT StructureIdentificationModifier : public AsynchronousParticleModifier
{
public:

	/// Computes the modifier's results.
	class StructureIdentificationEngine : public ComputeEngine
	{
	public:

		/// Constructor.
		StructureIdentificationEngine(const TimeInterval& validityInterval, ParticleProperty* positions, const SimulationCell& simCell) :
			ComputeEngine(validityInterval),
			_positions(positions), _simCell(simCell),
			_structures(new ParticleProperty(positions->size(), ParticleProperty::StructureTypeProperty, 0, false)) {}

		/// Returns the property storage that contains the input particle positions.
		ParticleProperty* positions() const { return _positions.data(); }

		/// Returns the property storage that contains the computed per-particle structure types.
		ParticleProperty* structures() const { return _structures.data(); }

		/// Returns the simulation cell data.
		const SimulationCell& cell() const { return _simCell; }

	private:

		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<ParticleProperty> _structures;
		SimulationCell _simCell;
	};

public:

	/// Constructor.
	StructureIdentificationModifier(DataSet* dataset);

	/// Returns the array of structure types that are assigned to the particles by this modifier.
	const QVector<ParticleType*>& structureTypes() const { return _structureTypes; }

	/// Returns an array that contains the number of matching particles for each structure type.
	const QList<int>& structureCounts() const { return _structureCounts; }

	//////////////////////////////////// Default settings ////////////////////////////////

	/// Returns the default color for a structure type.
	static Color getDefaultStructureColor(const QString& structureName);

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Create an instance of the ParticleType class to represent a structure type.
	void createStructureType(int id, const QString& name) {
		createStructureType(id, name, getDefaultStructureColor(name));
	}

	/// Create an instance of the ParticleType class to represent a structure type.
	void createStructureType(int id, const QString& name, const Color& color);

	/// Unpacks the results of the computation engine and stores them in the modifier.
	virtual void transferComputationResults(ComputeEngine* engine) override;

	/// Lets the modifier insert the cached computation results into the modification pipeline.
	virtual PipelineStatus applyComputationResults(TimePoint time, TimeInterval& validityInterval) override;

private:

	/// This stores the cached results of the modifier, i.e. the structures assigned to the particles.
	QExplicitlySharedDataPointer<ParticleProperty> _structureData;

	/// Contains the list of structure types recognized by this analysis modifier.
	VectorReferenceField<ParticleType> _structureTypes;

	/// The number of matching particles for each structure type.
	QList<int> _structureCounts;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_structureTypes);
};

/**
 * List box that displays the structure types.
 */
class OVITO_PARTICLES_EXPORT StructureListParameterUI : public RefTargetListParameterUI
{
public:

	/// Constructor.
	StructureListParameterUI(PropertiesEditor* parentEditor);

	/// This method is called when a new editable object has been activated.
	virtual void resetUI() override {
		RefTargetListParameterUI::resetUI();
		// Clear initial selection by default.
		tableWidget()->selectionModel()->clear();
	}

protected:

	/// Returns a data item from the list data model.
	virtual QVariant getItemData(RefTarget* target, const QModelIndex& index, int role) override;

	/// Returns the number of columns for the table view.
	virtual int tableColumnCount() override { return 5; }

	/// Returns the header data under the given role for the given RefTarget.
	virtual QVariant getHorizontalHeaderData(int index, int role) override {
		if(role == Qt::DisplayRole) {
			if(index == 0)
				return qVariantFromValue(tr("Color"));
			else if(index == 1)
				return qVariantFromValue(tr("Name"));
			else if(index == 2)
				return qVariantFromValue(tr("Count"));
			else if(index == 3)
				return qVariantFromValue(tr("Fraction"));
			else
				return qVariantFromValue(tr("Id"));
		}
		else return RefTargetListParameterUI::getHorizontalHeaderData(index, role);
	}

	/// Do not open sub-editor for selected structure type.
	virtual void openSubEditor() override {}

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

protected Q_SLOTS:

	/// Is called when the user has double-clicked on one of the structure types in the list widget.
	void onDoubleClickStructureType(const QModelIndex& index);

private:

	Q_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_STRUCTURE_IDENTIFICATION_MODIFIER_H
