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

#ifndef __OVITO_CALCULATE_DISPLACEMENTS_MODIFIER_H
#define __OVITO_CALCULATE_DISPLACEMENTS_MODIFIER_H

#include <core/Core.h>
#include <viz/data/VectorDisplay.h>
#include "../../ParticleModifier.h"

namespace Viz {

/******************************************************************************
* Calculates the per-particle displacement vectors by comparing the current
* positions to a reference configuration.
******************************************************************************/
class CalculateDisplacementsModifier : public ParticleModifier
{
public:

	/// Default constructor.
	Q_INVOKABLE CalculateDisplacementsModifier();

	/// Asks the modifier for its validity interval at the given time.
	virtual TimeInterval modifierValidity(TimePoint time) override;

	/// Returns the object that contains the reference configuration of the particles
	/// used for calculating the displacement vectors.
	SceneObject* referenceConfiguration() const { return _referenceObject; }

	/// Sets the object that contains the reference configuration of the particles
	/// used for calculating the displacement vectors.
	void setReferenceConfiguration(const OORef<SceneObject>& refConf) { _referenceObject = refConf; }

	/// Returns true if the homogeneous deformation of the simulation cell is eliminated from the calculated displacement vectors.
	bool eliminateCellDeformation() const { return _eliminateCellDeformation; }

	/// Sets whether the homogeneous deformation of the simulation cell is eliminated from the calculated displacement vectors.
	void setEliminateCellDeformation(bool enable) { _eliminateCellDeformation = enable; }

	/// Returns true if we assume the particle coordinates are unwrapped when calculating the displacement vectors.
	bool assumeUnwrappedCoordinates() const { return _assumeUnwrappedCoordinates; }

	/// Sets we assume the particle coordinates are unwrapped when calculating the displacement vectors.
	void setAssumeUnwrappedCoordinates(bool enable) { _assumeUnwrappedCoordinates = enable; }

public:

	Q_PROPERTY(bool eliminateCellDeformation READ eliminateCellDeformation WRITE setEliminateCellDeformation)
	Q_PROPERTY(bool assumeUnwrappedCoordinates READ assumeUnwrappedCoordinates WRITE setAssumeUnwrappedCoordinates)

protected:

	/// Handles reference events sent by reference targets of this object.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Modifies the particle object.
	virtual ObjectStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// The reference configuration.
	ReferenceField<SceneObject> _referenceObject;

	/// Controls the whether the reference configuration is shown instead of the current configuration.
	PropertyField<bool> _referenceShown;

	/// Controls the whether the homogeneous deformation of the simulation cell is eliminated from the calculated displacement vectors.
	PropertyField<bool> _eliminateCellDeformation;

	/// Controls the whether we assume the particle coordinates are unwrapped when calculating the displacement vectors.
	PropertyField<bool> _assumeUnwrappedCoordinates;

	/// Specify reference frame relative to current frame.
	PropertyField<bool> _useReferenceFrameOffset;

	/// Absolute frame number from reference file to use when calculating displacement vectors.
	PropertyField<int> _referenceFrameNumber;

	/// Relative frame offset for reference coordinates.
	PropertyField<int> _referenceFrameOffset;

	/// The vector display object for rendering the displacement vectors.
	ReferenceField<VectorDisplay> _vectorDisplay;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Displacement vectors");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_REFERENCE_FIELD(_referenceObject);
	DECLARE_PROPERTY_FIELD(_referenceShown);
	DECLARE_PROPERTY_FIELD(_eliminateCellDeformation);
	DECLARE_PROPERTY_FIELD(_assumeUnwrappedCoordinates);
	DECLARE_PROPERTY_FIELD(_useReferenceFrameOffset);
	DECLARE_PROPERTY_FIELD(_referenceFrameNumber);
	DECLARE_PROPERTY_FIELD(_referenceFrameOffset);
	DECLARE_REFERENCE_FIELD(_vectorDisplay);
};

/******************************************************************************
* A properties editor for the CalculateDisplacementsModifier class.
******************************************************************************/
class CalculateDisplacementsModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE CalculateDisplacementsModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_CALCULATE_DISPLACEMENTS_MODIFIER_H
