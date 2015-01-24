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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/dataset/UndoStack.h>
#include <core/dataset/DataSetContainer.h>
#include <core/animation/AnimationSettings.h>
#include <core/animation/controller/Controller.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, BooleanParameterUI, PropertyParameterUI);

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
BooleanParameterUI::BooleanParameterUI(QObject* parentEditor, const char* propertyName, const QString& checkBoxLabel) :
	PropertyParameterUI(parentEditor, propertyName)
{
	// Create UI widget.
	_checkBox = new QCheckBox(checkBoxLabel);
	connect(_checkBox.data(), &QCheckBox::clicked, this, &BooleanParameterUI::updatePropertyValue);
}

/******************************************************************************
* Constructor for a PropertyField property.
******************************************************************************/
BooleanParameterUI::BooleanParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField) :
	PropertyParameterUI(parentEditor, propField) 
{
	// Create UI widget.
	_checkBox = new QCheckBox(propField.displayName());
	connect(_checkBox.data(), &QCheckBox::clicked, this, &BooleanParameterUI::updatePropertyValue);
}

/******************************************************************************
* Destructor.
******************************************************************************/
BooleanParameterUI::~BooleanParameterUI()
{
	// Release widget.
	delete checkBox(); 
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void BooleanParameterUI::resetUI()
{
	PropertyParameterUI::resetUI();	
	
	if(checkBox()) {
		if(isReferenceFieldUI())
			checkBox()->setEnabled(parameterObject() != NULL && isEnabled());
		else
			checkBox()->setEnabled(editObject() != NULL && isEnabled());
	}

	if(isReferenceFieldUI() && editObject()) {
		// Update the displayed value when the animation time has changed.
		connect(dataset()->container(), &DataSetContainer::timeChanged, this, &BooleanParameterUI::updateUI, Qt::UniqueConnection);
	}
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void BooleanParameterUI::updateUI()
{
	PropertyParameterUI::updateUI();	
	
	if(checkBox() && editObject()) {
		if(isReferenceFieldUI()) {
#if 0
			BooleanController* ctrl = dynamic_object_cast<BooleanController>(parameterObject());
			if(ctrl) {
				bool val = ctrl->currentValue();
				checkBox()->setChecked(val);
			}
#endif
		}
		else {
			QVariant val(false);
			if(propertyName()) {
				val = editObject()->property(propertyName());
				OVITO_ASSERT_MSG(val.isValid(), "BooleanParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2 that can be cast to bool type.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
				if(!val.isValid()) {
					throw Exception(tr("The object class %1 does not define a property with the name %2 that can be cast to bool type.").arg(editObject()->metaObject()->className(), QString(propertyName())));
				}
			}
			else if(propertyField()) {
				val = editObject()->getPropertyFieldValue(*propertyField());
				OVITO_ASSERT(val.isValid());
			}
			checkBox()->setChecked(val.toBool());
		}
	}
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void BooleanParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(checkBox()) {
		if(isReferenceFieldUI())
			checkBox()->setEnabled(parameterObject() != NULL && isEnabled());
		else
			checkBox()->setEnabled(editObject() != NULL && isEnabled());
	}
}

/******************************************************************************
* Takes the value entered by the user and stores it in the property field 
* this property UI is bound to.
******************************************************************************/
void BooleanParameterUI::updatePropertyValue()
{
	if(checkBox() && editObject()) {
		undoableTransaction(tr("Change parameter"), [this]() {
			if(isReferenceFieldUI()) {
#if 0
				if(BooleanController* ctrl = dynamic_object_cast<BooleanController>(parameterObject())) {
					ctrl->setCurrentValue(checkBox()->isChecked());
					updateUI();
				}
#endif
			}
			else if(isQtPropertyUI()) {
				if(!editObject()->setProperty(propertyName(), checkBox()->isChecked())) {
					OVITO_ASSERT_MSG(false, "BooleanParameterUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
				}
			}
			else if(isPropertyFieldUI()) {
				editObject()->setPropertyFieldValue(*propertyField(), checkBox()->isChecked());
			}
			Q_EMIT valueEntered();
		});
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
