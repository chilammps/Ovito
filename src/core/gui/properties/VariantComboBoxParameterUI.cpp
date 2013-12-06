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
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include <core/dataset/UndoStack.h>

namespace Ovito {

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, VariantComboBoxParameterUI, PropertyParameterUI)

/******************************************************************************
* The constructor.
******************************************************************************/
VariantComboBoxParameterUI::VariantComboBoxParameterUI(QObject* parentEditor, const char* propertyName) :
	PropertyParameterUI(parentEditor, propertyName), _comboBox(new QComboBox())
{
	connect(_comboBox, SIGNAL(activated(int)), this, SLOT(updatePropertyValue()));	
}

/******************************************************************************
* Destructor, that releases all GUI controls.
******************************************************************************/
VariantComboBoxParameterUI::~VariantComboBoxParameterUI()
{
	// Release GUI controls. 
	delete comboBox(); 
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void VariantComboBoxParameterUI::resetUI()
{
	PropertyParameterUI::resetUI();	
	
	if(comboBox()) 
		comboBox()->setEnabled(editObject() != NULL && isEnabled());
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void VariantComboBoxParameterUI::updateUI()
{
	PropertyParameterUI::updateUI();	
	
	if(comboBox() && editObject()) {
		QVariant val = editObject()->property(propertyName());
		OVITO_ASSERT_MSG(val.isValid(), "VariantComboBoxParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
		if(!val.isValid()) {
			throw Exception(tr("The object class %1 does not define a property with the name %2.").arg(editObject()->metaObject()->className(), QString(propertyName())));
		}
		comboBox()->setCurrentIndex(comboBox()->findData(val));
		if(comboBox()->isEditable())
			comboBox()->setEditText(val.toString());
	}
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void VariantComboBoxParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(comboBox()) comboBox()->setEnabled(editObject() != NULL && isEnabled());
}

/******************************************************************************
* Takes the value entered by the user and stores it in the property field 
* this property UI is bound to.
******************************************************************************/
void VariantComboBoxParameterUI::updatePropertyValue()
{
	if(comboBox() && editObject() && comboBox()->currentIndex() >= 0) {
		undoableTransaction(tr("Change parameter"), [this]() {
			QVariant newValue;
			if(comboBox()->isEditable())
				newValue = comboBox()->currentText();
			else
				newValue = comboBox()->itemData(comboBox()->currentIndex());

			if(!editObject()->setProperty(propertyName(), newValue)) {
				OVITO_ASSERT_MSG(false, "VariantComboBoxParameterUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
			}

			Q_EMIT valueEntered();
		});
	}
}

};

