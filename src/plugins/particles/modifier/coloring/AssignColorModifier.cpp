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

#include <plugins/particles/Particles.h>
#include <core/animation/controller/Controller.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include "AssignColorModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Coloring)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, AssignColorModifier, ParticleModifier);
SET_OVITO_OBJECT_EDITOR(AssignColorModifier, AssignColorModifierEditor);
DEFINE_FLAGS_REFERENCE_FIELD(AssignColorModifier, _colorCtrl, "Color", Controller, PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(AssignColorModifier, _keepSelection, "KeepSelection");
SET_PROPERTY_FIELD_LABEL(AssignColorModifier, _colorCtrl, "Color");
SET_PROPERTY_FIELD_LABEL(AssignColorModifier, _keepSelection, "Keep selection");

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, AssignColorModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AssignColorModifier::AssignColorModifier(DataSet* dataset) : ParticleModifier(dataset), _keepSelection(false)
{
	INIT_PROPERTY_FIELD(AssignColorModifier::_colorCtrl);
	INIT_PROPERTY_FIELD(AssignColorModifier::_keepSelection);

	_colorCtrl = ControllerManager::instance().createColorController(dataset);
	_colorCtrl->setColorValue(0, Color(0.3f, 0.3f, 1.0f));
}

/******************************************************************************
* Asks the modifier for its validity interval at the given time.
******************************************************************************/
TimeInterval AssignColorModifier::modifierValidity(TimePoint time)
{
	TimeInterval interval = Modifier::modifierValidity(time);
	interval.intersect(_colorCtrl->validityInterval(time));
	return interval;
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
PipelineStatus AssignColorModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Get the selection property.
	ParticlePropertyObject* selProperty = inputStandardProperty(ParticleProperty::SelectionProperty);

	// Get the output color property.
	ParticlePropertyObject* colorProperty = outputStandardProperty(ParticleProperty::ColorProperty, selProperty != nullptr);

	// Get the color to be assigned.
	Color color(1,1,1);
	if(_colorCtrl)
		_colorCtrl->getColorValue(time, color, validityInterval);

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

		// Clear particle selection if requested.
		if(!keepSelection())
			output().removeObject(selProperty);
	}
	else {
		// Assign color to all particles.
		std::fill(colorProperty->dataColor(), colorProperty->dataColor() + colorProperty->size(), color);
	}
	colorProperty->changed();

	return PipelineStatus();
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void AssignColorModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Assign color"), rolloutParams, "particles.modifiers.assign_color.html");

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

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
