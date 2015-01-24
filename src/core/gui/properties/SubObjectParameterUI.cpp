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
#include <core/gui/properties/SubObjectParameterUI.h>
#include <core/dataset/UndoStack.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, SubObjectParameterUI, PropertyParameterUI);

/******************************************************************************
* The constructor.
******************************************************************************/
SubObjectParameterUI::SubObjectParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& refField, const RolloutInsertionParameters& rolloutParams)
	: PropertyParameterUI(parentEditor, refField), _rolloutParams(rolloutParams)
{
}

/******************************************************************************
* This method is called when a new sub-object has been assigned to the reference field of the editable object 
* this parameter UI is bound to. It is also called when the editable object itself has
* been replaced in the editor. 
******************************************************************************/
void SubObjectParameterUI::resetUI()
{
	PropertyParameterUI::resetUI();
	
	try {

		// Close editor if it is no longer needed.
		if(subEditor()) {
			if(!parameterObject() || subEditor()->editObject() == NULL ||
					subEditor()->editObject()->getOOType() != parameterObject()->getOOType() ||
					!isEnabled()) {

				_subEditor = nullptr;
			}
		}
		if(!parameterObject() || !isEnabled()) return;
		if(!subEditor()) {
			_subEditor = parameterObject()->createPropertiesEditor();
			if(_subEditor)
				_subEditor->initialize(editor()->container(), editor()->mainWindow(), _rolloutParams);
		}

		if(subEditor())
			subEditor()->setEditObject(parameterObject());
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Sets the enabled state of the UI.
******************************************************************************/
void SubObjectParameterUI::setEnabled(bool enabled)
{
	if(enabled != isEnabled()) {
		PropertyParameterUI::setEnabled(enabled);
		if(editObject())
			resetUI();
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
