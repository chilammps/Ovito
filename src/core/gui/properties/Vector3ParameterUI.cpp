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
#include <core/gui/properties/Vector3ParameterUI.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/AnimationSettings.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, Vector3ParameterUI, FloatParameterUI);

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
Vector3ParameterUI::Vector3ParameterUI(QObject* parentEditor, const char* propertyName, size_t vectorComponent, const QString& labelText, const QMetaObject* parameterUnitType)
	: FloatParameterUI(parentEditor, propertyName, labelText, parameterUnitType), _component(vectorComponent)
{
	OVITO_ASSERT_MSG(vectorComponent >= 0 && vectorComponent < 3, "Vector3ParameterUI constructor", "The vector component must be in the range 0-2.");
}

/******************************************************************************
* Constructor for a PropertyField property.
******************************************************************************/
Vector3ParameterUI::Vector3ParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField, size_t vectorComponent)
	: FloatParameterUI(parentEditor, propField), _component(vectorComponent)
{
	OVITO_ASSERT_MSG(vectorComponent >= 0 && vectorComponent < 3, "Vector3ParameterUI constructor", "The vector component must be in the range 0-2.");

	switch(_component) {
		case 0: label()->setText(propField.displayName() + " (X):"); break;
		case 1: label()->setText(propField.displayName() + " (Y):"); break;
		case 2: label()->setText(propField.displayName() + " (Z):"); break;
	}
}

/******************************************************************************
* Takes the value entered by the user and stores it in the parameter object
* this parameter UI is bound to.
******************************************************************************/
void Vector3ParameterUI::updatePropertyValue()
{
	if(editObject() && spinner()) {
		try {
			if(isReferenceFieldUI()) {
				if(Controller* ctrl = dynamic_object_cast<Controller>(parameterObject())) {
					Vector3 val = ctrl->currentVector3Value();
					val[_component] = spinner()->floatValue();
					ctrl->setCurrentVector3Value(val);
				}
			}
			else if(isQtPropertyUI()) {
				QVariant currentValue = editObject()->property(propertyName());
				if(currentValue.canConvert<Vector3>()) {
					Vector3 val = currentValue.value<Vector3>();
					val[_component] = spinner()->floatValue();
					currentValue.setValue(val);
				}
				else if(currentValue.canConvert<Point3>()) {
					Point3 val = currentValue.value<Point3>();
					val[_component] = spinner()->floatValue();
					currentValue.setValue(val);
				}
				if(!editObject()->setProperty(propertyName(), currentValue)) {
					OVITO_ASSERT_MSG(false, "Vector3ParameterUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
				}
			}
			else if(isPropertyFieldUI()) {
				QVariant currentValue = editObject()->getPropertyFieldValue(*propertyField());
				if(currentValue.canConvert<Vector3>()) {
					Vector3 val = currentValue.value<Vector3>();
					val[_component] = spinner()->floatValue();
					currentValue.setValue(val);
				}
				else if(currentValue.canConvert<Point3>()) {
					Point3 val = currentValue.value<Point3>();
					val[_component] = spinner()->floatValue();
					currentValue.setValue(val);
				}
				editObject()->setPropertyFieldValue(*propertyField(), currentValue);
			}

			Q_EMIT valueEntered();
		}
		catch(const Exception& ex) {
			ex.showError();
		}
	}
}

/******************************************************************************
* This method updates the displayed value of the parameter UI.
******************************************************************************/
void Vector3ParameterUI::updateUI()
{
	if(editObject() && spinner() && !spinner()->isDragging()) {
		if(isReferenceFieldUI()) {
			if(Controller* ctrl = dynamic_object_cast<Controller>(parameterObject())) {
				spinner()->setFloatValue(ctrl->currentVector3Value()[_component]);
			}
		}
		else {
			QVariant val;
			if(isQtPropertyUI()) {
				val = editObject()->property(propertyName());
				OVITO_ASSERT_MSG(val.isValid() && (val.canConvert<Vector3>() || val.canConvert<Point3>()), "Vector3ParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2 that can be cast to Vector3/Point3 type.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
				if(!val.isValid() || !(val.canConvert<Vector3>() || val.canConvert<Point3>())) {
					throw Exception(tr("The object class %1 does not define a property with the name %2 that can be cast to Vector3/Point3 type.").arg(editObject()->metaObject()->className(), QString(propertyName())));
				}
			}
			else if(isPropertyFieldUI()) {
				val = editObject()->getPropertyFieldValue(*propertyField());
				OVITO_ASSERT(val.isValid() && (val.canConvert<Vector3>() || val.canConvert<Point3>()));
			}
			else return;

			if(val.canConvert<Vector3>())
				spinner()->setFloatValue(val.value<Vector3>()[_component]);
			else if(val.canConvert<Point3>())
				spinner()->setFloatValue(val.value<Point3>()[_component]);
		}
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

