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

#ifndef __OVITO_CA_STRUCTURE_PATTERN_H
#define __OVITO_CA_STRUCTURE_PATTERN_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/gui/properties/RefTargetListParameterUI.h>
#include <plugins/particles/objects/ParticleType.h>
#include "BurgersVectorFamily.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/**
 * \brief Encapsulates a super pattern from the CA lib.
 */
class OVITO_CRYSTALANALYSIS_EXPORT StructurePattern : public ParticleType
{
public:

	/// The types of structure described by a pattern:
	enum StructureType {
		Lattice,		// Three-dimensional crystal lattice.
		Interface,		// Two-dimensional coherent crystal interface, grain boundary, or stacking fault.
		PointDefect		// Zero-dimensional crystal defect.
	};
	Q_ENUMS(StructureType);

public:

	/// \brief Constructor.
	Q_INVOKABLE StructurePattern(DataSet* dataset);

	/// Returns the long name of this pattern.
	const QString& longName() const { return name(); }

	/// Assigns a long name to this pattern.
	void setLongName(const QString& name) { setName(name); }

	/// Returns the short name of this pattern.
	const QString& shortName() const { return _shortName; }

	/// Sets the short name to this pattern.
	void setShortName(const QString& name) { _shortName = name; }

	/// Returns the list of Burgers vector families defined for this lattice pattern.
	const QVector<BurgersVectorFamily*>& burgersVectorFamilies() const { return _burgersVectorFamilies; }

	/// Adds a new family to this lattice pattern's list of Burgers vector families.
	void addBurgersVectorFamily(BurgersVectorFamily* family) { _burgersVectorFamilies.push_back(family); }

	/// Removes a family from this lattice pattern's list of Burgers vector families.
	void removeBurgersVectorFamily(int index) { _burgersVectorFamilies.remove(index); }

	/// Returns the default Burgers vector family, which is assigned to dislocation segments that
	/// don't belong to any family.
	BurgersVectorFamily* defaultBurgersVectorFamily() const { return _burgersVectorFamilies.front(); }

	/// Returns the type of structure described by this pattern.
	StructureType structureType() const { return _structureType; }

	/// Changes the type of structure described by this pattern.
	void setStructureType(StructureType type) { _structureType = type; }

private:

	/// The short name of this pattern.
	PropertyField<QString> _shortName;

	/// The type of structure described by this pattern.
	PropertyField<StructureType, int> _structureType;

	/// List of Burgers vector families.
	VectorReferenceField<BurgersVectorFamily> _burgersVectorFamilies;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_shortName);
	DECLARE_PROPERTY_FIELD(_structureType);
	DECLARE_VECTOR_REFERENCE_FIELD(_burgersVectorFamilies);
};

/**
 * \brief A properties editor for the StructurePattern class.
 */
class OVITO_CRYSTALANALYSIS_EXPORT StructurePatternEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE StructurePatternEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

protected Q_SLOTS:

	/// Is called when the user has double-clicked on one of the entries in the list widget.
	void onDoubleClickBurgersFamily(const QModelIndex& index);

private:

	RefTargetListParameterUI* familiesListUI;

	Q_OBJECT
	OVITO_OBJECT
};

}	// End of namespace
}	// End of namespace
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Plugins::CrystalAnalysis::StructurePattern::StructureType);
Q_DECLARE_TYPEINFO(Ovito::Plugins::CrystalAnalysis::StructurePattern::StructureType, Q_PRIMITIVE_TYPE);

#endif // __OVITO_CA_STRUCTURE_PATTERN_H
