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
#include <core/gui/properties/BooleanGroupBoxParameterUI.h>
#include <core/dataset/UndoStack.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/AnimationSettings.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, BooleanGroupBoxParameterUI, PropertyParameterUI);

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
BooleanGroupBoxParameterUI::BooleanGroupBoxParameterUI(QObject* parentEditor, const char* propertyName, const QString& label) :
	PropertyParameterUI(parentEditor, propertyName)
{
	// Create UI widget.
	_groupBox = new QGroupBox(label);
	_groupBox->setCheckable(true);
	_childContainer = new QWidget(_groupBox);
	QVBoxLayout* layout = new QVBoxLayout(_groupBox);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	layout->addWidget(_childContainer, 1);
	connect(_groupBox.data(), &QGroupBox::clicked, this, &BooleanGroupBoxParameterUI::updatePropertyValue);
}

/******************************************************************************
* Constructor for a PropertyField property.
******************************************************************************/
BooleanGroupBoxParameterUI::BooleanGroupBoxParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField) :
	PropertyParameterUI(parentEditor, propField)
{
	// Create UI widget.
	_groupBox = new QGroupBox(propField.displayName());
	_groupBox->setCheckable(true);
	_childContainer = new QWidget(_groupBox);
	QVBoxLayout* layout = new QVBoxLayout(_groupBox);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);
	layout->addWidget(_childContainer, 1);
	connect(_groupBox.data(), &QGroupBox::clicked, this, &BooleanGroupBoxParameterUI::updatePropertyValue);
}

/******************************************************************************
* Destructor.
******************************************************************************/
BooleanGroupBoxParameterUI::~BooleanGroupBoxParameterUI()
{
	// Release GUI controls. 
	delete groupBox(); 
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to.
******************************************************************************/
void BooleanGroupBoxParameterUI::resetUI()
{
	PropertyParameterUI::resetUI();
	
	if(groupBox()) {
		if(isReferenceFieldUI())
			groupBox()->setEnabled(parameterObject() != NULL && isEnabled());
		else
			groupBox()->setEnabled(editObject() != NULL && isEnabled());
	}
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to.
******************************************************************************/
void BooleanGroupBoxParameterUI::updateUI()
{
	PropertyParameterUI::updateUI();

	if(groupBox() && editObject()) {
		if(isReferenceFieldUI()) {
#if 0
			BooleanController* ctrl = dynamic_object_cast<BooleanController>(parameterObject());
			if(ctrl != NULL) {
				bool val = ctrl->currentValue();
				groupBox()->setChecked(val);
			}
#endif
		}
		else {
			QVariant val(false);
			if(isQtPropertyUI()) {
				val = editObject()->property(propertyName());
				OVITO_ASSERT_MSG(val.isValid(), "BooleanGroupBoxParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2 that can be cast to bool type.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
				if(!val.isValid()) {
					throw Exception(tr("The object class %1 does not define a property with the name %2 that can be cast to bool type.").arg(editObject()->metaObject()->className(), QString(propertyName())));
				}
			}
			else if(isPropertyFieldUI()) {
				val = editObject()->getPropertyFieldValue(*propertyField());
				OVITO_ASSERT(val.isValid());
			}
			groupBox()->setChecked(val.toBool());
		}
	}
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void BooleanGroupBoxParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(groupBox()) {
		if(isReferenceFieldUI())
			groupBox()->setEnabled(parameterObject() != NULL && isEnabled());
		else
			groupBox()->setEnabled(editObject() != NULL && isEnabled());
	}
}

/******************************************************************************
* Takes the value entered by the user and stores it in the property field
* this property UI is bound to.
******************************************************************************/
void BooleanGroupBoxParameterUI::updatePropertyValue()
{
	if(groupBox() && editObject()) {
		undoableTransaction(tr("Change parameter"), [this]() {
			if(isReferenceFieldUI()) {
#if 0
				if(BooleanController* ctrl = dynamic_object_cast<BooleanController>(parameterObject())) {
					ctrl->setCurrentValue(groupBox()->isChecked());
					updateUI();
				}
#endif
			}
			else if(isQtPropertyUI()) {
				if(!editObject()->setProperty(propertyName(), groupBox()->isChecked())) {
					OVITO_ASSERT_MSG(false, "BooleanGroupBoxParameterUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
				}
			}
			else if(isPropertyFieldUI()) {
				editObject()->setPropertyFieldValue(*propertyField(), groupBox()->isChecked());
			}
			Q_EMIT valueEntered();
		});
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
