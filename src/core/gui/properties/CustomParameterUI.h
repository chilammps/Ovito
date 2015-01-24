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

#ifndef __OVITO_CUSTOM_PARAMETER_UI_H
#define __OVITO_CUSTOM_PARAMETER_UI_H

#include <core/Core.h>
#include "ParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* Utility class for creating UIs for custom parameter types.
******************************************************************************/
class OVITO_CORE_EXPORT CustomParameterUI : public PropertyParameterUI
{
public:

	/// Constructor.
	CustomParameterUI(QObject* parentEditor, const char* propertyName, QWidget* widget,
			const std::function<void(const QVariant&)>& updateWidgetFunction,
			const std::function<QVariant()>& updatePropertyFunction,
			const std::function<void(RefTarget*)>& resetUIFunction = std::function<void(RefTarget*)>());

	/// Constructor for a PropertyField property.
	CustomParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField, QWidget* widget,
			const std::function<void(const QVariant&)>& updateWidgetFunction,
			const std::function<QVariant()>& updatePropertyFunction,
			const std::function<void(RefTarget*)>& resetUIFunction = std::function<void(RefTarget*)>());
	
	/// Destructor.
	virtual ~CustomParameterUI();
	
	/// This returns the widget managed by this ParameterUI.
	QWidget* widget() const { return _widget; }
	
	/// This method is called when a new editable object has been assigned to the properties owner this
	/// parameter UI belongs to.  
	virtual void resetUI() override;

	/// This method updates the displayed value of the property UI.
	virtual void updateUI() override;
	
	/// Sets the enabled state of the UI.
	virtual void setEnabled(bool enabled) override;
	
	/// Sets the tooltip text for the widget.
	void setToolTip(const QString& text) const { 
		if(widget()) widget()->setToolTip(text);
	}
	
	/// Sets the What's This helper text for the widget.
	void setWhatsThis(const QString& text) const { 
		if(widget()) widget()->setWhatsThis(text);
	}
	
public:
	
	Q_PROPERTY(QWidget widget READ widget);
	
public Q_SLOTS:
	
	/// Takes the value entered by the user and stores it in the property field this property UI is bound to.
	void updatePropertyValue();
	
protected:

	/// The widget managed by this UI component.
	QPointer<QWidget> _widget;

	/// This function is called when the property value has changed and the UI widget needs to be updated.
	std::function<void(const QVariant&)> _updateWidgetFunction;

	/// This function is called when the user has manipulated the UI widget and the property needs to be set accordingly.
	std::function<QVariant()> _updatePropertyFunction;

	/// This function is called when a new object is loaded into the editor.
	std::function<void(RefTarget*)> _resetUIFunction;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_VARIANT_COMBO_BOX_PARAMETER_UI_H
