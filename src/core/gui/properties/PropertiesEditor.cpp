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
	OVITO_ASSERT_MSG(container(), "PropertiesEditor::createRollout()", "Editor has not been properly initialized.");
	QWidget* panel = new QWidget(params.container());
	_rollouts.add(panel);
	if(params.container() == nullptr) {

		// Let the rollout-insertion parameters set the rollout title prefix.
		QString titlePrefix;
		if(!params.title().isEmpty())
			titlePrefix = QString("%1: ").arg(params.title());

		// Create a new rollout in the rollout container.
		QPointer<Rollout> rollout = container()->addRollout(panel, titlePrefix + title, params);

		// Check if a title for the rollout has been specified. If not,
		// automatically set the rollout title to the title of the object being edited.
		if(title.isEmpty()) {

			if(editObject())
				rollout->setTitle(titlePrefix + editObject()->objectTitle());

			// Automatically update rollout title each time a new object is loaded into the editor.
			connect(this, &PropertiesEditor::contentsReplaced, [rollout, titlePrefix](RefTarget* target) {
				if(rollout && target)
					rollout->setTitle(titlePrefix + target->objectTitle());
			});

		}
	}
	else if(params.container()->layout()) {

		// Instead of creating a new rollout for the widget, insert widget into a prescribed parent widget.
		params.container()->layout()->addWidget(panel);
	}
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
