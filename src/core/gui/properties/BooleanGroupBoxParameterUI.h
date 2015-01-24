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

#ifndef __OVITO_BOOLEAN_GROUP_BOX_PARAMETER_UI_H
#define __OVITO_BOOLEAN_GROUP_BOX_PARAMETER_UI_H

#include <core/Core.h>
#include "ParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* Allows the user to edit a boolean parameter.
* The check box is displayed in the title bar of a QGroupBox.
******************************************************************************/
class OVITO_CORE_EXPORT BooleanGroupBoxParameterUI : public PropertyParameterUI
{
public:

	/// Constructor for a Qt property.
	BooleanGroupBoxParameterUI(QObject* parentEditor, const char* propertyName, const QString& label);

	/// Constructor for a PropertyField property.
	BooleanGroupBoxParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField);

	/// Destructor, which releases all GUI controls.
	virtual ~BooleanGroupBoxParameterUI();
    
	/// This returns the group box managed by this parameter UI.
	QGroupBox* groupBox() const { return _groupBox; }

	/// This returns the container managed by this parameter UI where child widgets should be inserted into.
	QWidget* childContainer() const { return _childContainer; }

	/// This method is called when a new editable object has been assigned to the properties owner this
	/// parameter UI belongs to.
	virtual void resetUI() override;

	/// This method updates the displayed value of the property UI.
	virtual void updateUI() override;
	
	/// Sets the enabled state of the UI.
	virtual void setEnabled(bool enabled) override;

	/// Sets the tooltip text for the check box.
	void setToolTip(const QString& text) const { if(groupBox()) groupBox()->setToolTip(text); }

	/// Sets the What's This helper text for the check box.
	void setWhatsThis(const QString& text) const { if(groupBox()) groupBox()->setWhatsThis(text); }
	
public:
	
	Q_PROPERTY(QGroupBox groupBox READ groupBox)			
	
public Q_SLOTS:
	
	/// Takes the value entered by the user and stores it in the property field
	/// this property UI is bound to.
	void updatePropertyValue();

protected:

	/// The QGroupBox control of the UI component.
	QPointer<QGroupBox> _groupBox;

	/// A container widget where child widgets should be inserted into.
	QWidget* _childContainer;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_BOOLEAN_GROUP_BOX_PARAMETER_UI_H
