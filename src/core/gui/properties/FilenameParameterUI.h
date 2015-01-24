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

#ifndef __OVITO_FILENAME_PARAMETER_UI_H
#define __OVITO_FILENAME_PARAMETER_UI_H

#include <core/Core.h>
#include "ParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* This UI allows the user to select a filename as property value.
******************************************************************************/
class OVITO_CORE_EXPORT FilenameParameterUI : public PropertyParameterUI
{
public:

	/// Constructor for a Qt property.
	FilenameParameterUI(QObject* parentEditor, const char* propertyName);
	
	/// Constructor for a PropertyField property.
	FilenameParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField);
	
	/// Destructor.
	virtual ~FilenameParameterUI();

	/// This returns the button managed by this ParameterUI.
	QPushButton* selectorWidget() const { return _selectorButton; }
	
	/// This method is called when a new editable object has been assigned to the properties owner this
	/// parameter UI belongs to.  
	virtual void resetUI() override;

	/// This method updates the displayed value of the property UI.
	virtual void updateUI() override;
	
	/// Sets the enabled state of the UI.
	virtual void setEnabled(bool enabled) override;
	
	/// Sets the What's This helper text for the selector widget.
	void setWhatsThis(const QString& text) const { 
		if(selectorWidget()) selectorWidget()->setWhatsThis(text); 
	}
	
public:
	
	Q_PROPERTY(QPushButton selectorWidget READ selectorWidget);
	
Q_SIGNALS:

	/// This signal is emitted when the file selector should be shown to let the user select a new file.
	void showSelectionDialog();
	
protected:

	/// The selector control.
	QPointer<QPushButton> _selectorButton;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_FILENAME_PARAMETER_UI_H
