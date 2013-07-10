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
#include <core/viewport/Viewport.h>

#include <viz/modifier/ParticleModifier.h>
#include <viz/data/ParticleTypeProperty.h>
#include <viz/data/ParticleType.h>

namespace Viz {

using namespace Ovito;

/**
 * \brief A modifier that performs the structure analysis developed by Ackland and Jones.
 *
 * See G. Ackland, PRB(2006)73:054104.
 */
class BondAngleAnalysisModifier : public ParticleModifier
{
public:

	/// Default constructor.
	Q_INVOKABLE BondAngleAnalysisModifier();

	/// Asks the modifier for its validity interval at the given time.
	virtual TimeInterval modifierValidity(TimePoint time) override;

	/// Returns the array of structure types that are assigned to the particles by this modifier.
	const ParticleTypeList& structureTypes() const { return _structureTypes; }

	/// Returns the property that holds the computed per-particle structure types.
	ParticleTypeProperty* outputProperty() const { return _outputProperty; }

	/// \brief Returns whether the analysis results are saved along with the scene.
	/// \return \c true if data is stored in the scene file; \c false if the data needs to be recomputed after loading the scene file.
	bool storeResultsWithScene() const { return outputProperty() ? outputProperty()->saveWithScene() : false; }

	/// \brief Returns whether analysis results are saved along with the scene.
	/// \param on \c true if data should be stored in the scene file; \c false if the data needs to be recomputed after loading the scene file.
	/// \undoable
	void setStoreResultsWithScene(bool on) { if(outputProperty()) outputProperty()->setSaveWithScene(on); }

public:

	Q_PROPERTY(bool storeResultsWithScene READ storeResultsWithScene WRITE setStoreResultsWithScene)

protected:

	/// Modifies the particle object. The time interval passed
	/// to the function is reduced to the interval where the modified object is valid/constant.
	virtual ObjectStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// This stores the computed per-particle structure types.
	ReferenceField<ParticleTypeProperty> _outputProperty;

	/// Contains the list of structure types recognized by this analysis modifier.
	VectorReferenceField<ParticleType> _structureTypes;

	/// Controls whether the analysis is performed every time the input data chanages.
	PropertyField<bool> _autoUpdate;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_structureTypes);
	DECLARE_REFERENCE_FIELD(_outputProperty);
	DECLARE_PROPERTY_FIELD(_autoUpdate);
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
