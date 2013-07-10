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

#ifndef __OVITO_ASSIGN_COLOR_MODIFIER_H
#define __OVITO_ASSIGN_COLOR_MODIFIER_H

#include <core/Core.h>
#include <core/animation/controller/Controller.h>
#include "../ParticleModifier.h"

namespace Viz {

using namespace Ovito;

/******************************************************************************
* This modifier assigns a single color to the selected atoms of the input object.
******************************************************************************/
class AssignColorModifier : public ParticleModifier
{
public:

	/// Default constructor.
	Q_INVOKABLE AssignColorModifier();

	/// Asks the modifier for its validity interval at the given time.
	virtual TimeInterval modifierValidity(TimePoint time) override;

	/// Returns the color that is assigned to the selected atoms.
	Color color() const { return _colorCtrl ? (Color)_colorCtrl->currentValue() : Color(0,0,0); }

	/// Sets the color that is assigned to the selected atoms.
	void setColor(const Color& color) { if(_colorCtrl) _colorCtrl->setCurrentValue((Vector3)color); }

	/// Returns the controller for the color that is assigned to the selected atoms.
	VectorController* colorController() const { return _colorCtrl; }

	/// Sets the controller for the color that is assigned to the selected atoms.
	void setColorController(const OORef<VectorController>& ctrl) { _colorCtrl = ctrl; }

	/// Returns whether the input particle selection is preserved.
	/// If false, the selection is cleared by the modifier.
	bool keepSelection() const { return _keepSelection; }

	/// Sets whether the input particle selection is preserved.
	/// If false, the selection is cleared by the modifier.
	void setKeepSelection(bool keep) { _keepSelection = keep; }

public:

	Q_PROPERTY(bool keepSelection READ keepSelection WRITE setKeepSelection)
	Q_PROPERTY(Ovito::Color color READ color WRITE setColor)

protected:

	/// Modifies the particle object. The time interval passed
	/// to the function is reduced to the interval where the modified object is valid/constant.
	virtual ObjectStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// This controller stores the constant color to be assigned to all atoms.
	ReferenceField<VectorController> _colorCtrl;

	/// Controls whether the input particle selection is preserved.
	/// If false, the selection is cleared by the modifier.
	PropertyField<bool> _keepSelection;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Assign Color");
	Q_CLASSINFO("ModifierCategory", "Coloring");

	DECLARE_REFERENCE_FIELD(_colorCtrl);
	DECLARE_PROPERTY_FIELD(_keepSelection);
};

/******************************************************************************
* A properties editor for the AssignColorModifier class.
******************************************************************************/
class AssignColorModifierEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE AssignColorModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_ASSIGN_COLOR_MODIFIER_H
