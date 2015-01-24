///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2014) Alexander Stukowski
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
#include <core/gui/properties/CustomParameterUI.h>
#include <core/dataset/UndoStack.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, CustomParameterUI, PropertyParameterUI);

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
CustomParameterUI::CustomParameterUI(QObject* parentEditor, const char* propertyName, QWidget* widget,
		const std::function<void(const QVariant&)>& updateWidgetFunction,
		const std::function<QVariant()>& updatePropertyFunction,
		const std::function<void(RefTarget*)>& resetUIFunction) :
	PropertyParameterUI(parentEditor, propertyName), _widget(widget), _updateWidgetFunction(updateWidgetFunction), _updatePropertyFunction(updatePropertyFunction), _resetUIFunction(resetUIFunction)
{
}

/******************************************************************************
* Constructor for a PropertyField property.
******************************************************************************/
CustomParameterUI::CustomParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField, QWidget* widget,
		const std::function<void(const QVariant&)>& updateWidgetFunction,
		const std::function<QVariant()>& updatePropertyFunction,
		const std::function<void(RefTarget*)>& resetUIFunction) :
	PropertyParameterUI(parentEditor, propField), _widget(widget), _updateWidgetFunction(updateWidgetFunction), _updatePropertyFunction(updatePropertyFunction), _resetUIFunction(resetUIFunction)
{
}

/******************************************************************************
* Destructor.
******************************************************************************/
CustomParameterUI::~CustomParameterUI()
{
	// Release widget.
	delete widget();
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void CustomParameterUI::resetUI()
{
	if(widget()) {
		widget()->setEnabled(editObject() != NULL && isEnabled());
		if(_resetUIFunction)
			_resetUIFunction(editObject());
	}

	PropertyParameterUI::resetUI();
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void CustomParameterUI::updateUI()
{
	PropertyParameterUI::updateUI();	
	
	if(widget() && editObject()) {
		QVariant val;
		if(isQtPropertyUI()) {
			val = editObject()->property(propertyName());
			if(!val.isValid())
				throw Exception(tr("The object class %1 does not define a property with the name %2.").arg(editObject()->metaObject()->className(), QString(propertyName())));
		}
		else if(isPropertyFieldUI()) {
			val = editObject()->getPropertyFieldValue(*propertyField());
			OVITO_ASSERT_MSG(val.isValid(), "CustomParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
		}
		else return;

		_updateWidgetFunction(val);
	}
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void CustomParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(widget()) widget()->setEnabled(editObject() != NULL && isEnabled());
}

/******************************************************************************
* Takes the value entered by the user and stores it in the property field 
* this property UI is bound to.
******************************************************************************/
void CustomParameterUI::updatePropertyValue()
{
	if(widget() && editObject()) {
		undoableTransaction(tr("Change parameter"), [this]() {
			QVariant newValue = _updatePropertyFunction();

            if(isQtPropertyUI()) {
                if(!editObject()->setProperty(propertyName(), newValue)) {
                    OVITO_ASSERT_MSG(false, "CustomParameterUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
                }
            }
            else if(isPropertyFieldUI()) {
                editObject()->setPropertyFieldValue(*propertyField(), newValue);
            }

			Q_EMIT valueEntered();
		});
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
