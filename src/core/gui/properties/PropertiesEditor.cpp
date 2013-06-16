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
#include <core/gui/properties/PropertiesEditor.h>

namespace Ovito {

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, PropertiesEditor, RefMaker)
DEFINE_FLAGS_REFERENCE_FIELD(PropertiesEditor, _editObject, "EditObject", RefTarget, PROPERTY_FIELD_NO_UNDO | PROPERTY_FIELD_NO_CHANGE_MESSAGE)

/******************************************************************************
* The constructor.
******************************************************************************/
PropertiesEditor::PropertiesEditor() : _container(nullptr)
{
	INIT_PROPERTY_FIELD(PropertiesEditor::_editObject);
}

/******************************************************************************
* Creates a new rollout in the rollout container and returns
* the empty widget that can then be filled with UI controls.
* The rollout is automatically deleted when the editor is deleted.
******************************************************************************/
QWidget* PropertiesEditor::createRollout(const QString& title, const RolloutInsertionParameters& params)
{
	OVITO_ASSERT_MSG(container(), "PropertiesEditor::createRollout()", "Editor has not been initialized properly.");
	QWidget* panel = new QWidget(params.intoThisContainer);
	_rollouts.add(panel);
	if(params.intoThisContainer == NULL)
		container()->addRollout(panel, title, params);
	else if(params.intoThisContainer->layout())
		params.intoThisContainer->layout()->addWidget(panel);
	return panel;
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool PropertiesEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject() && event->type() == ReferenceEvent::TargetChanged) {
		// Generate signal.
		contentsChanged(source);
	}
	return RefMaker::referenceEvent(source, event);
}

};
