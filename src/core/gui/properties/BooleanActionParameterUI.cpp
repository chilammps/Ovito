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
#include <core/gui/properties/BooleanActionParameterUI.h>
#include <core/dataset/UndoStack.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, BooleanActionParameterUI, PropertyParameterUI);

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
BooleanActionParameterUI::BooleanActionParameterUI(QObject* parentEditor, const char* propertyName, QAction* action) :
	PropertyParameterUI(parentEditor, propertyName), _action(action)
{
	OVITO_ASSERT(action != nullptr);
	action->setCheckable(true);
	connect(action, &QAction::triggered, this, &BooleanActionParameterUI::updatePropertyValue);
}

/******************************************************************************
* Constructor for a PropertyField property.
******************************************************************************/
BooleanActionParameterUI::BooleanActionParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField, QAction* action) :
	PropertyParameterUI(parentEditor, propField), _action(action)
{
	OVITO_ASSERT(isPropertyFieldUI());
	OVITO_ASSERT(action != nullptr);
	action->setCheckable(true);
	connect(action, &QAction::triggered, this, &BooleanActionParameterUI::updatePropertyValue);
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void BooleanActionParameterUI::resetUI()
{
	PropertyParameterUI::resetUI();	
	
	if(action())
		action()->setEnabled(editObject() != NULL && isEnabled());
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void BooleanActionParameterUI::updateUI()
{
	PropertyParameterUI::updateUI();	
	
	if(action() && editObject()) {
		QVariant val(false);
		if(isQtPropertyUI()) {
			val = editObject()->property(propertyName());
			OVITO_ASSERT_MSG(val.isValid(), "BooleanActionParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2 that can be cast to bool type.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
			if(!val.isValid()) {
				throw Exception(tr("The object class %1 does not define a property with the name %2 that can be cast to bool type.").arg(editObject()->metaObject()->className(), QString(propertyName())));
			}
		}
		else if(isPropertyFieldUI()) {
			val = editObject()->getPropertyFieldValue(*propertyField());
			OVITO_ASSERT(val.isValid());
		}
		action()->setChecked(val.toBool());
	}
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void BooleanActionParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(action()) action()->setEnabled(editObject() != NULL && isEnabled());
}

/******************************************************************************
* Takes the value entered by the user and stores it in the property field 
* this property UI is bound to.
******************************************************************************/
void BooleanActionParameterUI::updatePropertyValue()
{
	if(action() && editObject()) {
		undoableTransaction(tr("Change parameter"), [this]() {
			if(isQtPropertyUI()) {
				if(!editObject()->setProperty(propertyName(), action()->isChecked())) {
					OVITO_ASSERT_MSG(false, "BooleanActionParameterUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
				}
			}
			else if(isPropertyFieldUI()) {
				editObject()->setPropertyFieldValue(*propertyField(), action()->isChecked());
			}
			Q_EMIT valueEntered();
		});
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
