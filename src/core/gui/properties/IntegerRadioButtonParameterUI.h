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

#ifndef __OVITO_INTEGER_RADIO_BUTTON_PARAMETER_UI_H
#define __OVITO_INTEGER_RADIO_BUTTON_PARAMETER_UI_H

#include <core/Core.h>
#include "ParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* This UI lets the user change an integer-value property of the object 
* being edited using a set of radio buttons.
******************************************************************************/
class OVITO_CORE_EXPORT IntegerRadioButtonParameterUI : public PropertyParameterUI
{
public:

	/// Constructor.
	IntegerRadioButtonParameterUI(QObject* parentEditor, const char* propertyName);

	/// Constructor for a PropertyField property.
	IntegerRadioButtonParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField);

	/// This returns the radio button group managed by this ParameterUI.
	QButtonGroup* buttonGroup() const { return _buttonGroup; }
	
	/// Creates a new radio button widget that can be selected by the user
	/// to set the property value to the given value.
	QRadioButton* addRadioButton(int value, const QString& caption = QString());
	
	/// This method is called when a new editable object has been assigned to the properties owner this
	/// parameter UI belongs to.  
	virtual void resetUI();

	/// This method updates the displayed value of the property UI.
	virtual void updateUI();
	
	/// Sets the enabled state of the UI.
	virtual void setEnabled(bool enabled);
	
	/// Sets the tooltip text for the radio button widgets.
	void setToolTip(const QString& text) const { 
		if(buttonGroup()) {
			Q_FOREACH(QAbstractButton* button, buttonGroup()->buttons()) 
				button->setToolTip(text);
		} 
	}
	
	/// Sets the What's This helper text for the radio button widgets.
	void setWhatsThis(const QString& text) const { 
		if(buttonGroup()) {
			Q_FOREACH(QAbstractButton* button, buttonGroup()->buttons()) 
				button->setWhatsThis(text);
		} 
	}
	
public:
	
	Q_PROPERTY(QButtonGroup buttonGroup READ buttonGroup)		
	
public Q_SLOTS:
	
	/// Takes the value entered by the user and stores it in the property field 
	/// this property UI is bound to. 
	void updatePropertyValue();
	
protected:

	/// The radio button group.
	QPointer<QButtonGroup> _buttonGroup;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_INTEGER_RADIO_BUTTON_PARAMETER_UI_H
