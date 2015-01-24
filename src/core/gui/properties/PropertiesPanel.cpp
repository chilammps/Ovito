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
#include <core/gui/properties/PropertiesPanel.h>
#include <core/dataset/UndoStack.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/******************************************************************************
* Constructs the panel.
******************************************************************************/
PropertiesPanel::PropertiesPanel(QWidget* parent) : 
	RolloutContainer(parent)
{
}

/******************************************************************************
* Destructs the panel.
******************************************************************************/
PropertiesPanel::~PropertiesPanel()
{
}

/******************************************************************************
* Sets the target object being edited in the panel.
******************************************************************************/
void PropertiesPanel::setEditObject(RefTarget* newEditObject)
{
	if(newEditObject == editObject() && (newEditObject != nullptr) == (editor() != nullptr))
		return;

	if(editor()) {
		OVITO_CHECK_OBJECT_POINTER(editor());
		
		// Can we re-use the old editor?
		if(newEditObject != nullptr && editor()->editObject() != nullptr
			&& editor()->editObject()->getOOType() == newEditObject->getOOType()) {
			
			editor()->setEditObject(newEditObject);
			return;
		}
		else {
			// Close previous editor.	
			_editor = nullptr;
		}
	}
	
	if(newEditObject) {
		// Open new properties editor.
		_editor = newEditObject->createPropertiesEditor();
		if(editor()) {
			editor()->initialize(this, newEditObject->dataset()->mainWindow(), RolloutInsertionParameters());
			editor()->setEditObject(newEditObject);
		}
	}
}

/******************************************************************************
* Returns the target object being edited in the panel
******************************************************************************/
RefTarget* PropertiesPanel::editObject() const
{
	if(!editor()) return nullptr;
	return editor()->editObject();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
