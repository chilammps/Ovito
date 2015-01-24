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

#include <plugins/particles/Particles.h>
#include <core/dataset/UndoStack.h>
#include <core/scene/pipeline/Modifier.h>
#include "ParticlePropertyParameterUI.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Particles, ParticlePropertyParameterUI, PropertyParameterUI);

/******************************************************************************
* Constructor.
******************************************************************************/
ParticlePropertyParameterUI::ParticlePropertyParameterUI(QObject* parentEditor, const char* propertyName, bool showComponents, bool inputProperty) :
	PropertyParameterUI(parentEditor, propertyName), _comboBox(new ParticlePropertyComboBox()), _showComponents(showComponents), _inputProperty(inputProperty)
{
	connect(comboBox(), (void (QComboBox::*)(int))&QComboBox::activated, this, &ParticlePropertyParameterUI::updatePropertyValue);

	if(!inputProperty)
		comboBox()->setEditable(true);
}

/******************************************************************************
* Constructor.
******************************************************************************/
ParticlePropertyParameterUI::ParticlePropertyParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField, bool showComponents, bool inputProperty) :
	PropertyParameterUI(parentEditor, propField), _comboBox(new ParticlePropertyComboBox()), _showComponents(showComponents), _inputProperty(inputProperty)
{
	connect(comboBox(), (void (QComboBox::*)(int))&QComboBox::activated, this, &ParticlePropertyParameterUI::updatePropertyValue);

	if(!inputProperty)
		comboBox()->setEditable(true);
}

/******************************************************************************
* Destructor.
******************************************************************************/
ParticlePropertyParameterUI::~ParticlePropertyParameterUI()
{
	// Release GUI controls. 
	delete comboBox(); 
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void ParticlePropertyParameterUI::resetUI()
{
	PropertyParameterUI::resetUI();	
	
	if(comboBox()) 
		comboBox()->setEnabled(editObject() != NULL && isEnabled());
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void ParticlePropertyParameterUI::updateUI()
{
	PropertyParameterUI::updateUI();	
	
	if(comboBox() && editObject()) {

		ParticlePropertyReference pref;
		if(isQtPropertyUI()) {
			QVariant val = editObject()->property(propertyName());
			OVITO_ASSERT_MSG(val.isValid() && val.canConvert<ParticlePropertyReference>(), "ParticlePropertyParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2 of type ParticlePropertyReference.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
			if(!val.isValid() || !val.canConvert<ParticlePropertyReference>()) {
				throw Exception(tr("The object class %1 does not define a property with the name %2 that can be cast to a ParticlePropertyReference.").arg(editObject()->metaObject()->className(), QString(propertyName())));
			}
			pref = val.value<ParticlePropertyReference>();
		}
		else if(isPropertyFieldUI()) {
			QVariant val = editObject()->getPropertyFieldValue(*propertyField());
			OVITO_ASSERT_MSG(val.isValid() && val.canConvert<ParticlePropertyReference>(), "ParticlePropertyParameterUI::updateUI()", QString("The property field of object class %1 is not of type ParticlePropertyReference.").arg(editObject()->metaObject()->className()).toLocal8Bit().constData());
			pref = val.value<ParticlePropertyReference>();
		}

		if(_inputProperty) {
			_comboBox->clear();

			// Obtain the list of input particle properties.
			Modifier* mod = dynamic_object_cast<Modifier>(editObject());
			if(mod) {
				PipelineFlowState inputState = mod->getModifierInput();

				// Populate property list from input object.
				int initialIndex = -1;
				for(DataObject* o : inputState.objects()) {
					ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o);
					if(!property) continue;

					// Properties with a non-numeric data type cannot be used as source properties.
					if(property->dataType() != qMetaTypeId<int>() && property->dataType() != qMetaTypeId<FloatType>())
						continue;

					if(property->componentNames().empty() || !_showComponents) {
						// Scalar property:
						_comboBox->addItem(property);
					}
					else {
						// Vector property:
						for(int vectorComponent = 0; vectorComponent < (int)property->componentCount(); vectorComponent++) {
							_comboBox->addItem(property, vectorComponent);
						}
					}
				}
			}

			// Select the right item in the list box.
			int selIndex = _comboBox->propertyIndex(pref);
			if(selIndex < 0 && !pref.isNull()) {
				// Add a place-holder item if the selected property does not exist anymore.
				_comboBox->addItem(pref, tr("%1 (no longer available)").arg(pref.name()));
				selIndex = _comboBox->count() - 1;
			}
			_comboBox->setCurrentIndex(selIndex);
		}
		else {
			if(_comboBox->count() == 0) {
				for(auto type : ParticleProperty::standardPropertyList())
					_comboBox->addItem(type);
			}
			_comboBox->setCurrentProperty(pref);
		}
	}
	else if(comboBox())
		comboBox()->clear();
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void ParticlePropertyParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(comboBox()) comboBox()->setEnabled(editObject() != NULL && isEnabled());
}

/******************************************************************************
* Takes the value entered by the user and stores it in the property field 
* this property UI is bound to.
******************************************************************************/
void ParticlePropertyParameterUI::updatePropertyValue()
{
	if(comboBox() && editObject() && comboBox()->currentIndex() >= 0) {
		undoableTransaction(tr("Change parameter"), [this]() {
			ParticlePropertyReference pref = _comboBox->currentProperty();
			if(isQtPropertyUI()) {
				if(!editObject()->setProperty(propertyName(), QVariant::fromValue(pref))) {
					OVITO_ASSERT_MSG(false, "ParticlePropertyParameterUI::updatePropertyValue()", QString("The value of property %1 of object class %2 could not be set.").arg(QString(propertyName()), editObject()->metaObject()->className()).toLocal8Bit().constData());
				}
			}
			else if(isPropertyFieldUI()) {
				editObject()->setPropertyFieldValue(*propertyField(), QVariant::fromValue(pref));
			}

			Q_EMIT valueEntered();
		});
	}
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

