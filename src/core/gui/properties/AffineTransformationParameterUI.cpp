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
#include <core/gui/properties/AffineTransformationParameterUI.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, AffineTransformationParameterUI, FloatParameterUI);

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
AffineTransformationParameterUI::AffineTransformationParameterUI(QObject* parentEditor, const char* propertyName, size_t _row, size_t _column, const QString& labelText, const QMetaObject* parameterUnitType)
	: FloatParameterUI(parentEditor, propertyName, labelText, parameterUnitType), row(_row), column(_column)
{
	OVITO_ASSERT_MSG(row >= 0 && row < 3, "AffineTransformationParameterUI constructor", "The row must be in the range 0-2.");
	OVITO_ASSERT_MSG(column >= 0 && column < 4, "AffineTransformationParameterUI constructor", "The column must be in the range 0-3.");
}

/******************************************************************************
* Constructor for a PropertyField property.
******************************************************************************/
AffineTransformationParameterUI::AffineTransformationParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField, size_t _row, size_t _column)
	: FloatParameterUI(parentEditor, propField), row(_row), column(_column)
{
	OVITO_ASSERT_MSG(row >= 0 && row < 3, "AffineTransformationParameterUI constructor", "The row must be in the range 0-2.");
	OVITO_ASSERT_MSG(column >= 0 && column < 4, "AffineTransformationParameterUI constructor", "The column must be in the range 0-3.");
}

/******************************************************************************
* Takes the value entered by the user and stores it in the parameter object
* this parameter UI is bound to.
******************************************************************************/
void AffineTransformationParameterUI::updatePropertyValue()
{
	if(editObject() && spinner()) {
		try {
			if(isQtPropertyUI()) {
				QVariant currentValue = editObject()->property(propertyName());
				if(currentValue.canConvert<AffineTransformation>()) {
					AffineTransformation val = currentValue.value<AffineTransformation>();
					val(row, column) = spinner()->floatValue();
					currentValue.setValue(val);
				}
				if(!editObject()->setProperty(propertyName(), currentValue)) {
					OVITO_ASSERT_MSG(false, "AffineTransformationParameterUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
				}
			}
			else if(isPropertyFieldUI()) {
				QVariant currentValue = editObject()->getPropertyFieldValue(*propertyField());
				if(currentValue.canConvert<AffineTransformation>()) {
					AffineTransformation val = currentValue.value<AffineTransformation>();
					val(row, column) = spinner()->floatValue();
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
void AffineTransformationParameterUI::updateUI()
{
	if(editObject() && spinner() && !spinner()->isDragging()) {
		QVariant val;
		if(isQtPropertyUI()) {
			val = editObject()->property(propertyName());
			OVITO_ASSERT_MSG(val.isValid() && (val.canConvert<AffineTransformation>()), "AffineTransformationParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2 that can be cast to an AffineTransformation type.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
			if(!val.isValid() || !(val.canConvert<AffineTransformation>())) {
				throw Exception(tr("The object class %1 does not define a property with the name %2 that can be cast to an AffineTransformation type.").arg(editObject()->metaObject()->className(), QString(propertyName())));
			}
		}
		else if(isPropertyFieldUI()) {
			val = editObject()->getPropertyFieldValue(*propertyField());
			OVITO_ASSERT(val.isValid() && (val.canConvert<AffineTransformation>()));
		}
		else return;
		
		if(val.canConvert<AffineTransformation>())
			spinner()->setFloatValue(val.value<AffineTransformation>()(row, column));
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

