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

#ifndef __OVITO_BOOLEAN_RADIO_BUTTON_PARAMETER_UI_H
#define __OVITO_BOOLEAN_RADIO_BUTTON_PARAMETER_UI_H

#include <core/Core.h>
#include "ParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* This UI allows the user to change a boolean-value property of the object being edited
* using two radio buttons.
******************************************************************************/
class OVITO_CORE_EXPORT BooleanRadioButtonParameterUI : public PropertyParameterUI
{
public:

	/// Constructor for a Qt property.
	BooleanRadioButtonParameterUI(QObject* parentEditor, const char* propertyName);

	/// Constructor for a PropertyField property.
	BooleanRadioButtonParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField);
	
	/// Destructor.
	virtual ~BooleanRadioButtonParameterUI();
	
	/// This returns the radio button group managed by this ParameterUI.
	QButtonGroup* buttonGroup() const { return _buttonGroup; }
	
	/// This returns the radio button for the "False" state. 
	QRadioButton* buttonFalse() const { return buttonGroup() ? (QRadioButton*)buttonGroup()->button(0) : nullptr; }

	/// This returns the radio button for the "True" state. 
	QRadioButton* buttonTrue() const { return buttonGroup() ? (QRadioButton*)buttonGroup()->button(1) : nullptr; }
	
	/// This method is called when a new editable object has been assigned to the properties owner this
	/// parameter UI belongs to.  
	virtual void resetUI() override;

	/// This method updates the displayed value of the property UI.
	virtual void updateUI() override;
	
	/// Sets the enabled state of the UI.
	virtual void setEnabled(bool enabled) override;
		
public:
	
	Q_PROPERTY(QButtonGroup buttonGroup READ buttonGroup)		
	
public Q_SLOTS:
	
	/// Takes the value entered by the user and stores it in the property field 
	/// this property UI is bound to. 
	void updatePropertyValue();
	
protected:

	/// The radio button group.
	QPointer<QButtonGroup> _buttonGroup;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_BOOLEAN_RADIO_BUTTON_PARAMETER_UI_H
