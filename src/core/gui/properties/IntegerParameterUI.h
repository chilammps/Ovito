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

#ifndef __OVITO_INTEGER_PARAMETER_UI_H
#define __OVITO_INTEGER_PARAMETER_UI_H

#include <core/Core.h>
#include "NumericalParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* A parameter UI for integer properties.
******************************************************************************/
class OVITO_CORE_EXPORT IntegerParameterUI : public NumericalParameterUI
{
public:

	/// Constructor for a Qt property.
	IntegerParameterUI(QObject* parentEditor, const char* propertyName, const QString& labelText = QString(), const QMetaObject* parameterUnitType = nullptr);

	/// Constructor for a PropertyField property.
	IntegerParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField);
	
	/// Gets the minimum value to be entered.
	/// This value is in native controller units.
	int minValue() const;

	/// Sets the minimum value to be entered.
	/// This value must be specified in native controller units.
	void setMinValue(int minValue);

	/// Gets the maximum value to be entered.
	/// This value is in native controller units.
	int maxValue() const;

	/// Sets the maximum value to be entered.
	/// This value must be specified in native controller units.
	void setMaxValue(int maxValue);

	/// This method updates the displayed value of the parameter UI.
	virtual void updateUI() override;
	
	/// Takes the value entered by the user and stores it in the property field 
	/// this property UI is bound to. 
	virtual void updatePropertyValue() override;
	
public:
	
	Q_PROPERTY(int minValue READ minValue WRITE setMinValue)	
	Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue)	
	
private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_INTEGER_PARAMETER_UI_H
