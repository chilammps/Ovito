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
#include <core/gui/properties/ParameterUI.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/AnimationSettings.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, ParameterUI, RefMaker);
IMPLEMENT_OVITO_OBJECT(Core, PropertyParameterUI, ParameterUI);
DEFINE_FLAGS_REFERENCE_FIELD(ParameterUI, _editObject, "EditObject", RefTarget, PROPERTY_FIELD_NO_UNDO | PROPERTY_FIELD_WEAK_REF | PROPERTY_FIELD_NO_CHANGE_MESSAGE);
DEFINE_FLAGS_REFERENCE_FIELD(PropertyParameterUI, _parameterObject, "ParameterObject", RefTarget, PROPERTY_FIELD_NO_UNDO | PROPERTY_FIELD_WEAK_REF | PROPERTY_FIELD_NO_CHANGE_MESSAGE);

///////////////////////////////////// ParameterUI /////////////////////////////////////////

/******************************************************************************
* The constructor.
******************************************************************************/
ParameterUI::ParameterUI(QObject* parent) : RefMaker(nullptr), _enabled(true)
{
	INIT_PROPERTY_FIELD(ParameterUI::_editObject);
	
	setParent(parent);

	PropertiesEditor* editor = this->editor();
	if(editor) {
		if(editor->editObject())
			setEditObject(editor->editObject());

		// Connect to the contentsReplaced() signal of the editor to synchronize the
		// parameter UI's edit object with the editor's edit object.
		connect(editor, &PropertiesEditor::contentsReplaced, this, &ParameterUI::setEditObject);
	}
}

///////////////////////////////////// PropertyParameterUI /////////////////////////////////////////

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
PropertyParameterUI::PropertyParameterUI(QObject* parent, const char* propertyName) :
	ParameterUI(parent), _propertyName(propertyName), _propField(nullptr)
{
	OVITO_ASSERT(propertyName != NULL);
	INIT_PROPERTY_FIELD(PropertyParameterUI::_parameterObject);
}

/******************************************************************************
* Constructor for a PropertyField or ReferenceField property.
******************************************************************************/
PropertyParameterUI::PropertyParameterUI(QObject* parent, const PropertyFieldDescriptor& propField) :
	ParameterUI(parent), _propertyName(nullptr), _propField(&propField)
{
	INIT_PROPERTY_FIELD(PropertyParameterUI::_parameterObject);

	// If requested, save parameter value to application's settings store each time the user changes it.
	if(propField.flags().testFlag(PROPERTY_FIELD_MEMORIZE))
		connect(this, &PropertyParameterUI::valueEntered, this, &PropertyParameterUI::memorizeDefaultParameterValue);
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool PropertyParameterUI::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(isReferenceFieldUI()) {
		if(source == editObject() && event->type() == ReferenceEvent::ReferenceChanged) {
			if(*propertyField() == static_cast<ReferenceFieldEvent*>(event)->field()) {
				// The parameter value object stored in the reference field of the edited object
				// has been replaced by another one, so update our own reference to the parameter value object.
				if(editObject()->getReferenceField(*propertyField()) != parameterObject())
					resetUI();
			}
		}
		else if(source == parameterObject() && event->type() == ReferenceEvent::TargetChanged) {
			// The parameter value object has changed -> update value shown in UI.
			updateUI();
		}
	}
	else if(source == editObject() && event->type() == ReferenceEvent::TargetChanged) {
		// The edited object has changed -> update value shown in UI.
		updateUI();
	}
	return ParameterUI::referenceEvent(source, event);
}

/******************************************************************************
* This method is called when parameter object has been assigned to the reference field of the editable object
* this parameter UI is bound to. It is also called when the editable object itself has
* been replaced in the editor.
******************************************************************************/
void PropertyParameterUI::resetUI()
{
	if(editObject() && isReferenceFieldUI()) {
		OVITO_CHECK_OBJECT_POINTER(editObject());
		OVITO_ASSERT(editObject() == NULL || editObject()->getOOType().isDerivedFrom(*propertyField()->definingClass()));

		// Bind this parameter UI to the parameter object of the new edited object.
		_parameterObject = editObject()->getReferenceField(*propertyField());
	}
	else {
		_parameterObject = nullptr;
	}

	ParameterUI::resetUI();
}

/******************************************************************************
* This slot is called when the user has changed the value of the parameter.
* It stores the new value in the application's settings store so that it can be used
* as the default initialization value next time when a new object of the same class is created.
******************************************************************************/
void PropertyParameterUI::memorizeDefaultParameterValue()
{
	if(!editObject())
		return;

	if(isPropertyFieldUI()) {
		propertyField()->memorizeDefaultValue(editObject());
	}
	else if(isReferenceFieldUI() && !propertyField()->isVector()) {
		Controller* ctrl = dynamic_object_cast<Controller>(parameterObject());
		if(ctrl) {
			QSettings settings;
			settings.beginGroup(propertyField()->definingClass()->plugin()->pluginId());
			settings.beginGroup(propertyField()->definingClass()->name());
			if(ctrl->controllerType() == Controller::ControllerTypeFloat) {
				settings.setValue(propertyField()->identifier(), QVariant::fromValue(ctrl->currentFloatValue()));
			}
			else if(ctrl->controllerType() == Controller::ControllerTypeInt) {
				settings.setValue(propertyField()->identifier(), QVariant::fromValue(ctrl->currentIntValue()));
			}
			else if(ctrl->controllerType() == Controller::ControllerTypeVector3) {
				settings.setValue(propertyField()->identifier(), QVariant::fromValue(ctrl->currentVector3Value()));
			}
		}
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
