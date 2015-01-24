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
#include <core/gui/properties/IntegerRadioButtonParameterUI.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/AnimationSettings.h>
#include <core/dataset/UndoStack.h>
#include <core/dataset/DataSetContainer.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, IntegerRadioButtonParameterUI, PropertyParameterUI);

/******************************************************************************
* The constructor.
******************************************************************************/
IntegerRadioButtonParameterUI::IntegerRadioButtonParameterUI(QObject* parentEditor, const char* propertyName) :
	PropertyParameterUI(parentEditor, propertyName)
{
	_buttonGroup = new QButtonGroup(this);
	connect(_buttonGroup.data(), (void (QButtonGroup::*)(int))&QButtonGroup::buttonClicked, this, &IntegerRadioButtonParameterUI::updatePropertyValue);
}

/******************************************************************************
* Constructor for a PropertyField property.
******************************************************************************/
IntegerRadioButtonParameterUI::IntegerRadioButtonParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField) :
	PropertyParameterUI(parentEditor, propField)
{
	_buttonGroup = new QButtonGroup(this);
	connect(_buttonGroup.data(), (void (QButtonGroup::*)(int))&QButtonGroup::buttonClicked, this, &IntegerRadioButtonParameterUI::updatePropertyValue);
}

/******************************************************************************
* Creates a new radio button widget that can be selected by the user
* to set the property value to the given value. 
******************************************************************************/
QRadioButton* IntegerRadioButtonParameterUI::addRadioButton(int value, const QString& caption)
{
	QRadioButton* button = new QRadioButton(caption);
	if(buttonGroup()) {
		button->setEnabled(editObject() && isEnabled());
		buttonGroup()->addButton(button, value);
	}
	return button;
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void IntegerRadioButtonParameterUI::resetUI()
{
	PropertyParameterUI::resetUI();	
	
	if(buttonGroup()) {
		Q_FOREACH(QAbstractButton* button, buttonGroup()->buttons()) {
			if(isReferenceFieldUI())
				button->setEnabled(parameterObject() && isEnabled());
			else
				button->setEnabled(editObject() && isEnabled());
		}
	}

	if(isReferenceFieldUI() && editObject()) {
		// Update the displayed value when the animation time has changed.
		connect(dataset()->container(), &DataSetContainer::timeChanged, this, &IntegerRadioButtonParameterUI::updateUI, Qt::UniqueConnection);
	}
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void IntegerRadioButtonParameterUI::updateUI()
{
	PropertyParameterUI::updateUI();	
	
	if(buttonGroup() && editObject()) {
		int id = buttonGroup()->checkedId();
		if(isReferenceFieldUI()) {
			if(Controller* ctrl = dynamic_object_cast<Controller>(parameterObject()))
				id = ctrl->currentIntValue();
		}
		else {
			if(isQtPropertyUI()) {
				QVariant val = editObject()->property(propertyName());
				OVITO_ASSERT_MSG(val.isValid() && val.canConvert(QVariant::Int), "IntegerRadioButtonParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2 that can be cast to integer type.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
				if(!val.isValid() || !val.canConvert(QVariant::Int)) {
					throw Exception(tr("The object class %1 does not define a property with the name %2 that can be cast to integer type.").arg(editObject()->metaObject()->className(), QString(propertyName())));
				}
				id = val.toInt();
			}
			else if(isPropertyFieldUI()) {
				QVariant val = editObject()->getPropertyFieldValue(*propertyField());
				OVITO_ASSERT(val.isValid());
				id = val.toInt();
			}
		}
		QAbstractButton* btn = buttonGroup()->button(id);
		if(btn != NULL) 
			btn->setChecked(true);
		else {
			btn = buttonGroup()->checkedButton();
			if(btn) btn->setChecked(false);
		}
	}
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void IntegerRadioButtonParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(buttonGroup()) {
		Q_FOREACH(QAbstractButton* button, buttonGroup()->buttons()) {
			if(isReferenceFieldUI())
				button->setEnabled(parameterObject() && isEnabled());
			else
				button->setEnabled(editObject() && isEnabled());
		}
	}
}

/******************************************************************************
* Takes the value entered by the user and stores it in the property field 
* this property UI is bound to.
******************************************************************************/
void IntegerRadioButtonParameterUI::updatePropertyValue()
{
	if(buttonGroup() && editObject()) {
		int id = buttonGroup()->checkedId();
		if(id != -1) {
			undoableTransaction(tr("Change parameter"), [this, id]() {
				if(isReferenceFieldUI()) {
					if(Controller* ctrl = dynamic_object_cast<Controller>(parameterObject())) {
						ctrl->setCurrentIntValue(id);
						updateUI();
					}
				}
				else if(isQtPropertyUI()) {
					if(!editObject()->setProperty(propertyName(), id)) {
						OVITO_ASSERT_MSG(false, "IntegerRadioButtonPropertyUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
					}
				}
				else if(isPropertyFieldUI()) {
					editObject()->setPropertyFieldValue(*propertyField(), id);
				}
				Q_EMIT valueEntered();
			});
		}
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
