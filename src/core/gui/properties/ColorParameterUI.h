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

#ifndef __OVITO_COLOR_PARAMETER_UI_H
#define __OVITO_COLOR_PARAMETER_UI_H

#include <core/Core.h>
#include <core/gui/widgets/general/ColorPickerWidget.h>
#include "ParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* Allows the user to edit a color parameter in the properties panel.
******************************************************************************/
class OVITO_CORE_EXPORT ColorParameterUI : public PropertyParameterUI
{
public:

	/// Constructor.
	ColorParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField);
	
	/// Destructor.
	virtual ~ColorParameterUI();
	    
	/// This returns the color picker widget managed by this parameter UI.
	ColorPickerWidget* colorPicker() const { return _colorPicker; }

	/// This returns a label for the color picker managed by this ColorPropertyUI.
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
		if(colorPicker()) colorPicker()->setWhatsThis(text); 
	}
	
public:
	
	Q_PROPERTY(QLabel* label READ label)		
	Q_PROPERTY(QWidget* colorPicker READ colorPicker)		
	
public Q_SLOTS:
	
	/// Is called when the user has changed the color.
	void onColorPickerChanged();

protected:

	/// The color picker control of the UI component.
	QPointer<ColorPickerWidget> _colorPicker;

	/// The label of the UI component.
	QPointer<QLabel> _label;
	
	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_COLOR_PARAMETER_UI_H
