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
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <core/dataset/UndoStack.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, BooleanRadioButtonParameterUI, PropertyParameterUI);

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
BooleanRadioButtonParameterUI::BooleanRadioButtonParameterUI(QObject* parentEditor, const char* propertyName) :
	PropertyParameterUI(parentEditor, propertyName)
{
	_buttonGroup = new QButtonGroup(this);
	connect(_buttonGroup.data(), (void (QButtonGroup::*)(int))&QButtonGroup::buttonClicked, this, &BooleanRadioButtonParameterUI::updatePropertyValue);

	QRadioButton* buttonNo = new QRadioButton();
	QRadioButton* buttonYes = new QRadioButton();
	_buttonGroup->addButton(buttonNo, 0);
	_buttonGroup->addButton(buttonYes, 1);
}

/******************************************************************************
* Constructor for a PropertyField property.
******************************************************************************/
BooleanRadioButtonParameterUI::BooleanRadioButtonParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField) :
	PropertyParameterUI(parentEditor, propField)
{
	_buttonGroup = new QButtonGroup(this);
	connect(_buttonGroup.data(), (void (QButtonGroup::*)(int))&QButtonGroup::buttonClicked, this, &BooleanRadioButtonParameterUI::updatePropertyValue);

	QRadioButton* buttonNo = new QRadioButton();
	QRadioButton* buttonYes = new QRadioButton();
	_buttonGroup->addButton(buttonNo, 0);
	_buttonGroup->addButton(buttonYes, 1);
}

/******************************************************************************
* Destructor.
******************************************************************************/
BooleanRadioButtonParameterUI::~BooleanRadioButtonParameterUI()
{
	// Release GUI controls. 
	delete buttonTrue(); 
	delete buttonFalse();
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void BooleanRadioButtonParameterUI::resetUI()
{
	PropertyParameterUI::resetUI();	
	
	if(buttonGroup()) {
		Q_FOREACH(QAbstractButton* button, buttonGroup()->buttons()) 
			button->setEnabled(editObject() != NULL && isEnabled());
	}
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void BooleanRadioButtonParameterUI::updateUI()
{
	PropertyParameterUI::updateUI();	
	
	if(buttonGroup() && editObject()) {
		QVariant val;
		if(propertyName()) {		
			val = editObject()->property(propertyName());
			OVITO_ASSERT_MSG(val.isValid(), "BooleanRadioButtonParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2 that can be cast to boolean type.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
			if(!val.isValid()) {
				throw Exception(tr("The object class %1 does not define a property with the name %2 that can be cast to boolean type.").arg(editObject()->metaObject()->className(), QString(propertyName())));
			}
		}
		else if(propertyField()) {
			val = editObject()->getPropertyFieldValue(*propertyField());
			OVITO_ASSERT(val.isValid());
		}
		bool state = val.toBool();
		if(state && buttonTrue())
			buttonTrue()->setChecked(true);
		else if(!state && buttonFalse())
			buttonFalse()->setChecked(true);
	}
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void BooleanRadioButtonParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(buttonGroup()) {
		Q_FOREACH(QAbstractButton* button, buttonGroup()->buttons()) 
			button->setEnabled(editObject() != NULL && isEnabled());
	}
}

/******************************************************************************
* Takes the value entered by the user and stores it in the property field 
* this property UI is bound to.
******************************************************************************/
void BooleanRadioButtonParameterUI::updatePropertyValue()
{
	if(buttonGroup() && editObject()) {
		undoableTransaction(tr("Change parameter"), [this]() {
			int id = buttonGroup()->checkedId();
			if(id != -1) {
				if(propertyName()) {
					if(!editObject()->setProperty(propertyName(), (bool)id)) {
						OVITO_ASSERT_MSG(false, "BooleanRadioButtonParameterUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
					}
				}
				else if(propertyField()) {
					editObject()->setPropertyFieldValue(*propertyField(), (bool)id);
				}
			}
			Q_EMIT valueEntered();
		});
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
