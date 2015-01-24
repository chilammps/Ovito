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

#ifndef __OVITO_BOOLEAN_PARAMETER_UI_H
#define __OVITO_BOOLEAN_PARAMETER_UI_H

#include <core/Core.h>
#include "ParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* This UI allows the user to change a boolean property of the object being edited.
******************************************************************************/
class OVITO_CORE_EXPORT BooleanParameterUI : public PropertyParameterUI
{
public:

	/// Constructor for a Qt property.
	BooleanParameterUI(QObject* parentEditor, const char* propertyName, const QString& checkBoxLabel);

	/// Constructor for a PropertyField property.
	BooleanParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField);
	
	/// Destructor.
	virtual ~BooleanParameterUI();
	
	/// This returns the checkbox managed by this parameter UI.
	QCheckBox* checkBox() const { return _checkBox; }
	
	/// This method is called when a new editable object has been assigned to the properties owner this
	/// parameter UI belongs to.
	virtual void resetUI() override;

	/// This method updates the displayed value of the property UI.
	virtual void updateUI() override;
	
	/// Sets the enabled state of the UI.
	virtual void setEnabled(bool enabled) override;
	
	/// Sets the tooltip text for the check box.
	void setToolTip(const QString& text) const { if(checkBox()) checkBox()->setToolTip(text); }
	
	/// Sets the What's This helper text for the check box.
	void setWhatsThis(const QString& text) const { if(checkBox()) checkBox()->setWhatsThis(text); }
	
public:
	
	Q_PROPERTY(QCheckBox checkBox READ checkBox);
	
public Q_SLOTS:
	
	/// Takes the value entered by the user and stores it in the property field 
	/// this property UI is bound to. 
	void updatePropertyValue();
	
protected:

	/// The check box of the UI component.
	QPointer<QCheckBox> _checkBox;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_BOOLEAN_PARAMETER_UI_H
