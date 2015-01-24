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
#include <core/gui/properties/FilenameParameterUI.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, FilenameParameterUI, PropertyParameterUI);

/******************************************************************************
* Constructor for a Qt property.
******************************************************************************/
FilenameParameterUI::FilenameParameterUI(QObject* parentEditor, const char* propertyName) :
	PropertyParameterUI(parentEditor, propertyName)
{
	// Create UI widget.
	_selectorButton = new QPushButton(QStringLiteral(" "));
	connect(_selectorButton.data(), &QPushButton::clicked, this, &FilenameParameterUI::showSelectionDialog);
}

/******************************************************************************
* Constructor for a PropertyField property.
******************************************************************************/
FilenameParameterUI::FilenameParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField) :
	PropertyParameterUI(parentEditor, propField)
{
	// Create UI widget.
	_selectorButton = new QPushButton(QStringLiteral(" "));
	connect(_selectorButton.data(), &QPushButton::clicked, this, &FilenameParameterUI::showSelectionDialog);
}

/******************************************************************************
* Destructor.
******************************************************************************/
FilenameParameterUI::~FilenameParameterUI()
{
	// Release GUI controls. 
	delete selectorWidget();
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void FilenameParameterUI::resetUI()
{
	PropertyParameterUI::resetUI();	
	
	if(selectorWidget()) 
		selectorWidget()->setEnabled(editObject() && isEnabled());
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. 
******************************************************************************/
void FilenameParameterUI::updateUI()
{
	PropertyParameterUI::updateUI();	

	if(selectorWidget() && editObject()) {
		QVariant val;
		if(propertyName()) {
			val = editObject()->property(propertyName());
			OVITO_ASSERT_MSG(val.isValid() && val.canConvert(QVariant::String), "FilenameParameterUI::updateUI()", QString("The object class %1 does not define a property with the name %2 that can be cast to string type.").arg(editObject()->metaObject()->className(), QString(propertyName())).toLocal8Bit().constData());
			if(!val.isValid() || !val.canConvert(QVariant::String)) {
				throw Exception(tr("The object class %1 does not define a property with the name %2 that can be cast to string type.").arg(editObject()->metaObject()->className(), QString(propertyName())));
			}
		}
		else if(propertyField()) {
			val = editObject()->getPropertyFieldValue(*propertyField());
			OVITO_ASSERT(val.isValid());
		}
		
		QString filename = val.toString();
		if(filename.isEmpty() == false) {
			selectorWidget()->setText(QFileInfo(filename).fileName());
		}
		else {
			selectorWidget()->setText(tr("[Choose File...]"));
		}
	}
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void FilenameParameterUI::setEnabled(bool enabled)
{
	if(enabled == isEnabled()) return;
	PropertyParameterUI::setEnabled(enabled);
	if(selectorWidget()) selectorWidget()->setEnabled(editObject() && isEnabled());
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
