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

#include <core/Core.h>
#include <core/animation/controller/StandardControllers.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include "AssignColorModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, AssignColorModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, AssignColorModifierEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(AssignColorModifier, AssignColorModifierEditor)
DEFINE_REFERENCE_FIELD(AssignColorModifier, _colorCtrl, "Color", VectorController)
DEFINE_PROPERTY_FIELD(AssignColorModifier, _keepSelection, "KeepSelection")
SET_PROPERTY_FIELD_LABEL(AssignColorModifier, _colorCtrl, "Color")
SET_PROPERTY_FIELD_LABEL(AssignColorModifier, _keepSelection, "Keep selection")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AssignColorModifier::AssignColorModifier() : _keepSelection(false)
{
	INIT_PROPERTY_FIELD(AssignColorModifier::_colorCtrl);
	INIT_PROPERTY_FIELD(AssignColorModifier::_keepSelection);

	_colorCtrl = ControllerManager::instance().createDefaultController<VectorController>();
	_colorCtrl->setValue(0, Vector3(0.3,0.3,1));
}

/******************************************************************************
* Asks the modifier for its validity interval at the given time.
******************************************************************************/
TimeInterval AssignColorModifier::modifierValidity(TimePoint time)
{
	// Return an empty validity interval if the modifier is currently being edited
	// to let the system create a pipeline cache point just before the modifier.
	// This will speed up re-evaluation of the pipeline if the user adjusts this modifier's parameters interactively.
	if(isBeingEdited())
		return TimeInterval::empty();

	if(_colorCtrl)
		return _colorCtrl->validityInterval(time);
	else
		return TimeInterval::forever();
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus AssignColorModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Get the selection property.
	ParticlePropertyObject* selProperty = inputStandardProperty(ParticleProperty::SelectionProperty);

	// Get the output color property.
	ParticlePropertyObject* colorProperty = outputStandardProperty(ParticleProperty::ColorProperty);

	// Get the color to be assigned.
	Color color(1,1,1);
	if(_colorCtrl)
		_colorCtrl->getValue(time, color, validityInterval);

	if(selProperty) {
		OVITO_ASSERT(colorProperty->size() == selProperty->size());
		const int* s = selProperty->constDataInt();
		Color* c = colorProperty->dataColor();
		Color* c_end = c + colorProperty->size();
		if(inputStandardProperty(ParticleProperty::ColorProperty) == nullptr) {
			std::vector<Color> existingColors = inputParticleColors(time, validityInterval);
			OVITO_ASSERT(existingColors.size() == colorProperty->size());
			auto ec = existingColors.cbegin();
			for(; c != c_end; ++c, ++s, ++ec) {
				if(*s)
					*c = color;
				else
					*c = *ec;
			}
		}
		else {
			for(; c != c_end; ++c, ++s)
				if(*s) *c = color;
		}

		if(!keepSelection()) {
			output().removeObject(selProperty);
		}
	}
	else {
		// Assign color to all particles.
		std::fill(colorProperty->dataColor(), colorProperty->dataColor() + colorProperty->size(), color);
	}

	return ObjectStatus();
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void AssignColorModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Assign color"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(0);
	layout->setColumnStretch(1, 1);

	// Color parameter.
	ColorParameterUI* constColorPUI = new ColorParameterUI(this, PROPERTY_FIELD(AssignColorModifier::_colorCtrl));
	layout->addWidget(constColorPUI->label(), 0, 0);
	layout->addWidget(constColorPUI->colorPicker(), 0, 1);

	// Keep selection parameter.
	BooleanParameterUI* keepSelectionPUI = new BooleanParameterUI(this, PROPERTY_FIELD(AssignColorModifier::_keepSelection));
	layout->addWidget(keepSelectionPUI->checkBox(), 1, 0, 1, 2);
}

};	// End of namespace
