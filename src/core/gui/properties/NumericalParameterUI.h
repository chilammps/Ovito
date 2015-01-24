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

#ifndef __OVITO_NUMERICAL_PARAMETER_UI_H
#define __OVITO_NUMERICAL_PARAMETER_UI_H

#include <core/Core.h>
#include <core/gui/widgets/general/SpinnerWidget.h>
#include "ParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* Base class for UI components that allow the user to edit a numerical
* property of an object via a spinner widget and a text box.
******************************************************************************/
class OVITO_CORE_EXPORT NumericalParameterUI : public PropertyParameterUI
{
public:

	/// Constructor for a Qt property.
	NumericalParameterUI(QObject* parentEditor, const char* propertyName, const QMetaObject* defaultParameterUnitType, const QString& labelText = QString());

	/// Constructor for a PropertyField or ReferenceField property.
	NumericalParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField, const QMetaObject* defaultParameterUnitType);
	
	/// Destructor.
	virtual ~NumericalParameterUI();

	/// This returns a label for the text box managed by this ParameterUI.
	QLabel* label() const { return _label; }

	/// This returns the spinner managed by this ParameterUI.
	SpinnerWidget* spinner() const { return _spinner; }

	/// This returns the text box managed by this ParameterUI.
	QLineEdit* textBox() const { return _textBox; }

	/// Creates a QLayout that contains the text box and the spinner widget.
	QLayout* createFieldLayout() const;

	/// Returns the type of unit conversion service, which is used to format the parameter value as a text string.
	const QMetaObject* parameterUnitType() const { return _parameterUnitType; }

	/// This method is called when a new editable object has been assigned to the properties owner this
	/// parameter UI belongs to.  
	virtual void resetUI() override;

	/// Sets the enabled state of the UI.
	virtual void setEnabled(bool enabled) override;
	
	/// Sets the tooltip text for the text box and the label widget.
	void setToolTip(const QString& text) const { 
		if(label()) label()->setToolTip(text); 
		if(textBox()) textBox()->setToolTip(text); 
	}
	
	/// Sets the What's This helper text for the label, textbox, and the spinner.
	void setWhatsThis(const QString& text) const { 
		if(label()) label()->setWhatsThis(text); 
		if(textBox()) textBox()->setWhatsThis(text); 
		if(spinner()) spinner()->setWhatsThis(text); 
	}
	
public:
	
	Q_PROPERTY(SpinnerWidget spinner READ spinner)		
	Q_PROPERTY(QLineEdit textBox READ textBox)		
	Q_PROPERTY(QLabel label READ label)
	
public Q_SLOTS:
	
	/// Takes the value entered by the user and stores it in the property field 
	/// this property UI is bound to. This method must be implemented by derived classes.
	virtual void updatePropertyValue() = 0;
	
protected Q_SLOTS:
	
	/// Is called when the spinner value has changed.
	void onSpinnerValueChanged();
	
	/// Is called when the user begins dragging the spinner interactively.
	void onSpinnerDragStart();

	/// Is called when the user stops dragging the spinner interactively.
	void onSpinnerDragStop();
	
	/// Is called when the user aborts dragging the spinner interactively.
	void onSpinnerDragAbort();
	
protected:

	/// the spinner control of the UI component.
	QPointer<SpinnerWidget> _spinner;

	/// The text box of the UI component.
	QPointer<QLineEdit> _textBox;

	/// The label of the UI component.
	QPointer<QLabel> _label;

	/// The type of unit conversion service, which is used to format the parameter value as a text string.
	const QMetaObject* _parameterUnitType;

private:

	/// Creates the widgets for this property UI.
	void initUIControls(const QString& labelText);

	/// The signal/slot connection that informs the parameter UI about animation time changes.
	QMetaObject::Connection _animationTimeChangedConnection;
	
	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_NUMERICAL_PARAMETER_UI_H
