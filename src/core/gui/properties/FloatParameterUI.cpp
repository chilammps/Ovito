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
#include <core/gui/properties/FloatParameterUI.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/AnimationSettings.h>
#include <core/utilities/units/UnitsManager.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, FloatParameterUI, NumericalParameterUI);

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
FloatParameterUI::FloatParameterUI(QObject* parentEditor, const char* propertyName, const QString& labelText, const QMetaObject* parameterUnitType) :
	NumericalParameterUI(parentEditor, propertyName, parameterUnitType ? parameterUnitType : &FloatParameterUnit::staticMetaObject, labelText)
{
}

/******************************************************************************
* Constructor for a PropertyField or ReferenceField property.
******************************************************************************/
FloatParameterUI::FloatParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField) :
	NumericalParameterUI(parentEditor, propField, &FloatParameterUnit::staticMetaObject)
{
}

/******************************************************************************
* Takes the value entered by the user and stores it in the property field 
* this property UI is bound to.
******************************************************************************/
void FloatParameterUI::updatePropertyValue()
{
	if(editObject() && spinner()) {
		if(isReferenceFieldUI()) {
			if(Controller* ctrl = dynamic_object_cast<Controller>(parameterObject()))
				ctrl->setCurrentFloatValue(spinner()->floatValue());
		}
		else if(isQtPropertyUI()) {
			if(!editObject()->setProperty(propertyName(), spinner()->floatValue())) {
				OVITO_ASSERT_MSG(false, "FloatParameterUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
			}
		}
		else if(isPropertyFieldUI()) {
			editObject()->setPropertyFieldValue(*propertyField(), spinner()->floatValue());						
		}
		Q_EMIT valueEntered();
	}
}

/******************************************************************************
* This method updates the displayed value of the parameter UI.
******************************************************************************/
void FloatParameterUI::updateUI()
{
	if(editObject() && spinner() && !spinner()->isDragging()) {
		try {
			if(isReferenceFieldUI()) {
				Controller* ctrl = dynamic_object_cast<Controller>(parameterObject());
				if(ctrl != NULL && spinner() && !spinner()->isDragging()) {
					spinner()->setFloatValue(ctrl->currentFloatValue());
				}
			}
			else {
				QVariant val(0.0);
				if(isQtPropertyUI()) {
					val = editObject()->property(propertyName());
					OVITO_ASSERT_MSG(val.isValid() && val.canConvert(QVariant::Double), "FloatParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2 that can be cast to float type.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
					if(!val.isValid() || !val.canConvert(QVariant::Double)) {
						throw Exception(tr("The object class %1 does not define a property with the name %2 that can be cast to float type.").arg(editObject()->metaObject()->className(), QString(propertyName())));
					}
				}
				else if(isPropertyFieldUI()) {
					val = editObject()->getPropertyFieldValue(*propertyField());
					OVITO_ASSERT(val.isValid());
				}
				spinner()->setFloatValue(val.value<FloatType>());
			}
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
FloatType FloatParameterUI::minValue() const
{
	return (spinner() ? spinner()->minValue() : FLOATTYPE_MIN);
}

/******************************************************************************
* Sets the minimum value to be entered.
* This value must be specified in native controller units.
******************************************************************************/
void FloatParameterUI::setMinValue(FloatType minValue)
{
	if(spinner()) spinner()->setMinValue(minValue);
}

/******************************************************************************
* Gets the maximum value to be entered.
* This value is in native controller units.
******************************************************************************/
FloatType FloatParameterUI::maxValue() const
{
	return (spinner() ? spinner()->maxValue() : FLOATTYPE_MAX);
}

/******************************************************************************
* Sets the maximum value to be entered.
* This value must be specified in native controller units.
******************************************************************************/
void FloatParameterUI::setMaxValue(FloatType maxValue)
{
	if(spinner()) spinner()->setMaxValue(maxValue);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
