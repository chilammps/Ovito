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

#include <core/Core.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/AnimationSettings.h>
#include <core/utilities/units/UnitsManager.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, IntegerParameterUI, NumericalParameterUI);

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
IntegerParameterUI::IntegerParameterUI(QObject* parentEditor, const char* propertyName, const QString& labelText, const QMetaObject* parameterUnitType) :
	NumericalParameterUI(parentEditor, propertyName, parameterUnitType ? parameterUnitType : &IntegerParameterUnit::staticMetaObject, labelText)
{
}

/******************************************************************************
* Constructor for a PropertyField property.
******************************************************************************/
IntegerParameterUI::IntegerParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField) :
		NumericalParameterUI(parentEditor, propField, &IntegerParameterUnit::staticMetaObject)
{
}

/******************************************************************************
* Takes the value entered by the user and stores it in the property field 
* this property UI is bound to.
******************************************************************************/
void IntegerParameterUI::updatePropertyValue()
{
	if(editObject() && spinner()) {
		if(isReferenceFieldUI()) {
			if(Controller* ctrl = dynamic_object_cast<Controller>(parameterObject()))
				ctrl->setCurrentIntValue(spinner()->intValue());
		}
		else if(isQtPropertyUI()) {
			if(!editObject()->setProperty(propertyName(), spinner()->intValue())) {
				OVITO_ASSERT_MSG(false, "IntegerParameterUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
			}
		}
		else if(isPropertyFieldUI()) {
			editObject()->setPropertyFieldValue(*propertyField(), spinner()->intValue());						
		}
	}
}

/******************************************************************************
* This method updates the displayed value of the parameter UI.
******************************************************************************/
void IntegerParameterUI::updateUI()
{
	if(editObject() && spinner() && !spinner()->isDragging()) {
		try {
			if(isReferenceFieldUI()) {
				if(Controller* ctrl = dynamic_object_cast<Controller>(parameterObject()))
					spinner()->setIntValue(ctrl->currentIntValue());
			}
			else {
				QVariant val(0);
				if(isQtPropertyUI()) {
					val = editObject()->property(propertyName());
					OVITO_ASSERT_MSG(val.isValid() && val.canConvert(QVariant::Int), "IntegerParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2 that can be cast to integer type.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
					if(!val.isValid() || !val.canConvert(QVariant::Int)) {
						throw Exception(tr("The object class %1 does not define a property with the name %2 that can be cast to integer type.").arg(editObject()->metaObject()->className(), QString(propertyName())));
					}
				}
				else if(isPropertyFieldUI()) {
					val = editObject()->getPropertyFieldValue(*propertyField());
					OVITO_ASSERT(val.isValid());
				}
				spinner()->setIntValue(val.toInt());
			}
			Q_EMIT valueEntered();
		}
		catch(const Exception& ex) {
			ex.showError();
		}
	}
}

/******************************************************************************
* Gets the minimum value to be entered.
* This value is in native controller units.
******************************************************************************/
int IntegerParameterUI::minValue() const
{
	return (spinner() ? (int)spinner()->minValue() : std::numeric_limits<int>::min());
}

/******************************************************************************
* Sets the minimum value to be entered.
* This value must be specified in native controller units.
******************************************************************************/
void IntegerParameterUI::setMinValue(int minValue)
{
	if(spinner()) spinner()->setMinValue(minValue);
}

/******************************************************************************
* Gets the maximum value to be entered.
* This value is in native controller units.
******************************************************************************/
int IntegerParameterUI::maxValue() const
{
	return (spinner() ? (int)spinner()->maxValue() : std::numeric_limits<int>::max());
}

/******************************************************************************
* Sets the maximum value to be entered.
* This value must be specified in native controller units.
******************************************************************************/
void IntegerParameterUI::setMaxValue(int maxValue)
{
	if(spinner()) spinner()->setMaxValue(maxValue);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
