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

#ifndef __OVITO_STRING_PARAMETER_UI_H
#define __OVITO_STRING_PARAMETER_UI_H

#include <core/Core.h>
#include "ParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* This UI allows the user to edit a string property of the object being edited.
******************************************************************************/
class OVITO_CORE_EXPORT StringParameterUI : public PropertyParameterUI
{
public:
	/// Constructor for a Qt property.
	StringParameterUI(QObject* parentEditor, const char* propertyName);
	
	/// Constructor for a PropertyField property.
	StringParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField);
	
	/// Destructor.
	virtual ~StringParameterUI();

	/// This returns the text box managed by this ParameterUI.
	QLineEdit* textBox() const { return _textBox; }
	
	/// Replaces the text box managed by this ParameterUI. The ParameterUI becomes the owner of the new text box.
	void setTextBox(QLineEdit* textBox);

	/// This method is called when a new editable object has been assigned to the properties owner this
	/// parameter UI belongs to.  
	virtual void resetUI() override;

	/// This method updates the displayed value of the property UI.
	virtual void updateUI() override;
	
	/// Sets the enabled state of the UI.
	virtual void setEnabled(bool enabled) override;
	
	/// Sets the tooltip text for the text box.
	void setToolTip(const QString& text) const { 
		if(textBox()) textBox()->setToolTip(text); 
	}
	
	/// Sets the What's This helper text for the textbox.
	void setWhatsThis(const QString& text) const { 
		if(textBox()) textBox()->setWhatsThis(text); 
	}
	
public:
	
	Q_PROPERTY(QLineEdit textBox READ textBox)		
	
public Q_SLOTS:
	
	/// Takes the value entered by the user and stores it in the property field 
	/// this property UI is bound to. 
	void updatePropertyValue();
	
protected:

	/// The text box of the UI component.
	QPointer<QLineEdit> _textBox;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_STRING_PARAMETER_UI_H
