///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_FONT_PARAMETER_UI_H
#define __OVITO_FONT_PARAMETER_UI_H

#include <core/Core.h>
#include "ParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* Allows the user to pick a font.
******************************************************************************/
class OVITO_CORE_EXPORT FontParameterUI : public PropertyParameterUI
{
public:

	/// Constructor.
	FontParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField);
	
	/// Destructor.
	virtual ~FontParameterUI();
	    
	/// This returns the font picker widget managed by this parameter UI.
	QPushButton* fontPicker() const { return _fontPicker; }

	/// This returns a label for the widget managed by this FontParameterUI.
	/// The text of the label widget is taken from the description text stored along 
	/// with the property field.
	QLabel* label() const { return _label; }

	/// This method is called when a new editable object has been assigned to the properties owner this
	/// parameter UI belongs to.
	virtual void resetUI() override;
	
	/// This method updates the displayed value of the property UI.
	virtual void updateUI() override;
	
	/// Sets the enabled state of the UI.
	virtual void setEnabled(bool enabled) override;
	
	/// Sets the What's This helper text for the label and the color picker.
	void setWhatsThis(const QString& text) const { 
		if(label()) label()->setWhatsThis(text); 
		if(fontPicker()) fontPicker()->setWhatsThis(text);
	}
	
public Q_SLOTS:
	
	/// Is called when the user has pressed the font picker button.
	void onButtonClicked();

protected:

	/// The font picker widget of the UI component.
	QPointer<QPushButton> _fontPicker;

	/// The label of the UI component.
	QPointer<QLabel> _label;
	
	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_FONT_PARAMETER_UI_H
