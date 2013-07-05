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
#include "AssignColorModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, AssignColorModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, AssignColorModifierEditor, PropertiesEditor)
DEFINE_REFERENCE_FIELD(AssignColorModifier, _colorCtrl, "Color", VectorController)
SET_PROPERTY_FIELD_LABEL(AssignColorModifier, _colorCtrl, "Color")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AssignColorModifier::AssignColorModifier()
{
	INIT_PROPERTY_FIELD(AssignColorModifier::_colorCtrl);

	_colorCtrl = ControllerManager::instance().createDefaultController<VectorController>();
	_colorCtrl->setValue(0, Vector3(0.3,0.3,1));
}

/******************************************************************************
* Asks the modifier for its validity interval at the given time.
******************************************************************************/
TimeInterval AssignColorModifier::modifierValidity(TimePoint time)
{
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
#if 0
	// Get the selection channel.
	DataChannel* selChannel = inputStandardChannel(DataChannel::SelectionChannel);

	// Get the output color channel.
	DataChannel* colorChannel = outputStandardChannel(DataChannel::ColorChannel);
	colorChannel->setVisible(true);

	// Get the color to be assigned.
	Vector3 selColor = Vector3(1,0,0);
	if(colorCtrl) colorCtrl->getValue(time, selColor, validityInterval);

	if(selChannel != NULL) {
		OVITO_ASSERT(colorChannel->size() == selChannel->size());
		const int* s = selChannel->constDataInt();
		Vector3* c = colorChannel->dataVector3();

		if(inputStandardChannel(DataChannel::ColorChannel) == NULL) {
			QVector<Color> oldColors = input()->getAtomColors(time, validityInterval);
			QVector<Color>::const_iterator oldc = oldColors.constBegin();
			for(size_t i = selChannel->size(); i != 0; i--, ++c, ++s, ++oldc) {
				if(*s) *c = selColor; else *c = *oldc;
			}
		}
		else {
			for(size_t i = selChannel->size(); i != 0; i--, ++c, ++s)
				if(*s) *c = selColor;
		}

		// Hide selection channel to make assigned color visible.
		if(selChannel->isVisible())
			outputStandardChannel(DataChannel::SelectionChannel)->setVisible(false);
	}
	else {
		Vector3* c = colorChannel->dataVector3();
		for(size_t i = colorChannel->size(); i != 0; i--)
			*c++ = selColor;
	}

#endif
	return ObjectStatus();
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void AssignColorModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Color"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(0);
	layout->setColumnStretch(1, 1);

	// Constant color parameter.
	ColorParameterUI* constColorPUI = new ColorParameterUI(this, PROPERTY_FIELD(AssignColorModifier::_colorCtrl));
	layout->addWidget(constColorPUI->label(), 0, 0);
	layout->addWidget(constColorPUI->colorPicker(), 0, 1);
}

};	// End of namespace
