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
#include <core/gui/properties/FontParameterUI.h>
#include <core/dataset/UndoStack.h>
#include <core/dataset/DataSetContainer.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, FontParameterUI, PropertyParameterUI);

/******************************************************************************
* The constructor.
******************************************************************************/
FontParameterUI::FontParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField)
	: PropertyParameterUI(parentEditor, propField)
{
	_label = new QLabel(propField.displayName() + ":");
	_fontPicker = new QPushButton();
	connect(_fontPicker.data(), &QPushButton::clicked, this, &FontParameterUI::onButtonClicked);
}

/******************************************************************************
* Destructor.
******************************************************************************/
FontParameterUI::~FontParameterUI()
{
	// Release GUI controls. 
	delete label();
	delete fontPicker();
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to.
******************************************************************************/
void FontParameterUI::resetUI()
{
	PropertyParameterUI::resetUI();
	
	if(fontPicker())  {
		if(editObject() && (!isReferenceFieldUI() || parameterObject())) {
			fontPicker()->setEnabled(isEnabled());
		}
		else {
			fontPicker()->setEnabled(false);
			fontPicker()->setText(QString());
		}
	}
}

/******************************************************************************
* This method updates the displayed value of the parameter UI.
******************************************************************************/
void FontParameterUI::updateUI()
{
	if(editObject() && fontPicker()) {
		if(isPropertyFieldUI()) {
			QVariant currentValue = editObject()->getPropertyFieldValue(*propertyField());
			OVITO_ASSERT(currentValue.isValid());
			if(currentValue.canConvert<QFont>())
				fontPicker()->setText(currentValue.value<QFont>().family());
			else
				fontPicker()->setText(QString());
		}
	}
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void FontParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(fontPicker()) {
		if(isReferenceFieldUI())
			fontPicker()->setEnabled(parameterObject() != NULL && isEnabled());
		else
			fontPicker()->setEnabled(editObject() != NULL && isEnabled());
	}
}

/******************************************************************************
* Is called when the user has pressed the font picker button.
******************************************************************************/
void FontParameterUI::onButtonClicked()
{
	if(fontPicker() && editObject() && isPropertyFieldUI()) {
		QVariant currentValue = editObject()->getPropertyFieldValue(*propertyField());
		OVITO_ASSERT(currentValue.isValid());
		QFont currentFont;
		if(currentValue.canConvert<QFont>())
			currentFont = currentValue.value<QFont>();
		bool ok;
		QFont font = QFontDialog::getFont(&ok, currentFont, fontPicker()->window());
		if(ok && font != currentFont) {
			undoableTransaction(tr("Change font"), [this, &font]() {
				editObject()->setPropertyFieldValue(*propertyField(), qVariantFromValue(font));
				Q_EMIT valueEntered();
			});
		}
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
