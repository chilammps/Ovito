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

#ifndef __OVITO_FLOAT_PARAMETER_UI_H
#define __OVITO_FLOAT_PARAMETER_UI_H

#include <core/Core.h>
#include "NumericalParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* A parameter UI for floating-point properties.
******************************************************************************/
class OVITO_CORE_EXPORT FloatParameterUI : public NumericalParameterUI
{
public:

	/// Constructor for a Qt property.
	FloatParameterUI(QObject* parentEditor, const char* propertyName, const QString& labelText = QString(), const QMetaObject* parameterUnitType = nullptr);

	/// Constructor for a PropertyField property.
	FloatParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField);
	
	/// Gets the minimum value to be entered.
	/// This value is in native controller units.
	FloatType minValue() const;

	/// Sets the minimum value to be entered.
	/// This value must be specified in native controller units.
	void setMinValue(FloatType minValue);

	/// Gets the maximum value to be entered.
	/// This value is in native controller units.
	FloatType maxValue() const;

	/// Sets the maximum value to be entered.
	/// This value must be specified in native controller units.
	void setMaxValue(FloatType maxValue);

	/// This method updates the displayed value of the parameter UI.
	virtual void updateUI() override;
	
	/// Takes the value entered by the user and stores it in the property field 
	/// this property UI is bound to. 
	virtual void updatePropertyValue() override;
	
public:
	
	Q_PROPERTY(FloatType minValue READ minValue WRITE setMinValue)	
	Q_PROPERTY(FloatType maxValue READ maxValue WRITE setMaxValue)	
	
private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_FLOAT_PARAMETER_UI_H
