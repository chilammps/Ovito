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

#include <plugins/particles/Particles.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/AnimationSettings.h>
#include "../ParticleModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Coloring)

/**
 * \brief This modifier assigns a certain color to all selected particles.
 */
class OVITO_PARTICLES_EXPORT AssignColorModifier : public ParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE AssignColorModifier(DataSet* dataset);

	/// Asks the modifier for its validity interval at the given time.
	virtual TimeInterval modifierValidity(TimePoint time) override;

	/// Returns the color that is assigned to the selected atoms.
	Color color() const { return _colorCtrl ? _colorCtrl->currentColorValue() : Color(0,0,0); }

	/// Sets the color that is assigned to the selected atoms.
	void setColor(const Color& color) { if(_colorCtrl) _colorCtrl->setCurrentColorValue(color); }

	/// Returns the controller for the color that is assigned to the selected atoms.
	Controller* colorController() const { return _colorCtrl; }

	/// Sets the controller for the color that is assigned to the selected atoms.
	void setColorController(Controller* ctrl) { _colorCtrl = ctrl; }

	/// Returns whether the input particle selection is preserved.
	/// If false, the selection is cleared by the modifier.
	bool keepSelection() const { return _keepSelection; }

	/// Sets whether the input particle selection is preserved.
	/// If false, the selection is cleared by the modifier.
	void setKeepSelection(bool keep) { _keepSelection = keep; }

protected:

	/// Modifies the particles.
	virtual PipelineStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// This controller stores the constant color to be assigned to all atoms.
	ReferenceField<Controller> _colorCtrl;

	/// Controls whether the input particle selection is preserved.
	/// If false, the selection is cleared by the modifier.
	PropertyField<bool> _keepSelection;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Assign color");
	Q_CLASSINFO("ModifierCategory", "Coloring");

	DECLARE_REFERENCE_FIELD(_colorCtrl);
	DECLARE_PROPERTY_FIELD(_keepSelection);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* A properties editor for the AssignColorModifier class.
******************************************************************************/
class AssignColorModifierEditor : public ParticleModifierEditor
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

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_ASSIGN_COLOR_MODIFIER_H
