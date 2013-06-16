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
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/undo/UndoManager.h>
#include <core/viewport/ViewportManager.h>
#include <core/animation/AnimManager.h>

namespace Ovito {

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, ColorParameterUI, PropertyParameterUI)

/******************************************************************************
* The constructor.
******************************************************************************/
ColorParameterUI::ColorParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField)
	: PropertyParameterUI(parentEditor, propField)
{
	_label = new QLabel(propField.displayName() + ":");
	_colorPicker = new ColorPickerWidget();
	_colorPicker->setObjectName("colorButton");
	connect(_colorPicker, SIGNAL(colorChanged()), this, SLOT(onColorPickerChanged()));

	if(isReferenceFieldUI()) {
		// Update the displayed color when the animation time has changed.
		connect(&AnimManager::instance(), SIGNAL(timeChanged(TimePoint)), this, SLOT(updateUI()));
	}
}

/******************************************************************************
* Destructor, that releases all GUI controls.
******************************************************************************/
ColorParameterUI::~ColorParameterUI()
{
	// Release GUI controls. 
	delete label();
	delete colorPicker();
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to.
******************************************************************************/
void ColorParameterUI::resetUI()
{
	PropertyParameterUI::resetUI();
	
	if(colorPicker())  {
		if(editObject() && (!isReferenceFieldUI() || parameterObject())) {
			colorPicker()->setEnabled(isEnabled());
		}
		else {
			colorPicker()->setEnabled(false);
			colorPicker()->setColor(Color(1,1,1));
		}
	}
}

/******************************************************************************
* This method updates the displayed value of the parameter UI.
******************************************************************************/
void ColorParameterUI::updateUI()
{
	if(editObject() && colorPicker()) {
		if(isReferenceFieldUI()) {
			VectorController* ctrl = dynamic_object_cast<VectorController>(parameterObject());
			if(ctrl) {
				Vector3 val = ctrl->currentValue();
				colorPicker()->setColor(Color(val));
			}
		}
		else if(isPropertyFieldUI()) {
			QVariant currentValue = editObject()->getPropertyFieldValue(*propertyField());
			OVITO_ASSERT(currentValue.isValid());
			if(currentValue.canConvert<Color>()) {
				Color val = currentValue.value<Color>();
				colorPicker()->setColor(val);
			}
		}
	}
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void ColorParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(colorPicker()) {
		if(isReferenceFieldUI())
			colorPicker()->setEnabled(parameterObject() != NULL && isEnabled());
		else
			colorPicker()->setEnabled(editObject() != NULL && isEnabled());
	}
}

/******************************************************************************
* Is called when the user has changed the color.
******************************************************************************/
void ColorParameterUI::onColorPickerChanged()
{
	if(colorPicker() && editObject()) {
		ViewportSuspender noVPUpdate;
		UndoManager::instance().beginCompoundOperation(tr("Change color"));
		if(isReferenceFieldUI()) {
			VectorController* ctrl = dynamic_object_cast<VectorController>(parameterObject());
			if(ctrl)
				ctrl->setCurrentValue((Vector3)colorPicker()->color());
		}
		else if(isPropertyFieldUI()) {
			QVariant newValue;
			newValue.setValue(colorPicker()->color());
			editObject()->setPropertyFieldValue(*propertyField(), newValue);
		}
		UndoManager::instance().endCompoundOperation();
	}
}

};

