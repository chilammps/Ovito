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

#ifndef __OVITO_LINKED_FILE_OBJECT_EDITOR_H
#define __OVITO_LINKED_FILE_OBJECT_EDITOR_H

#include <core/Core.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/gui/properties/PropertiesPanel.h>
#include <core/gui/widgets/ElidedTextLabel.h>

#include "LinkedFileObject.h"

namespace Ovito {

/**
 * A properties editor for the LinkedFileObject class.
 */
class LinkedFileObjectEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE LinkedFileObjectEditor() {}

	/// Sets the object being edited in this editor.
	virtual void setEditObject(RefTarget* newObject) override;

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

protected Q_SLOTS:

	/// Is called when the user presses the "Pick remote input file" button.
	void onPickRemoteInputFile();

	/// Is called when the user presses the Reload frame button.
	void onReloadFrame();

	/// Is called when the user presses the Reload animation button.
	void onReloadAnimation();

	/// Is called when the user presses the Parser Settings button.
	void onParserSettings();

	/// Updates the contents of the status label.
	void updateInformationLabel();

	/// This is called when the user has changed the source URL.
	void onWildcardPatternEntered();

private:

	QAction* _parserSettingsAction;

	ElidedTextLabel* _sourcePathLabel;
	ElidedTextLabel* _filenameLabel;
	QGroupBox* _wildcardBox;
	QLineEdit* _wildcardPatternTextbox;
	QLabel* _statusTextLabel;
	QLabel* _statusIconLabel;

	QPixmap _statusWarningIcon;
	QPixmap _statusErrorIcon;

	std::vector<OORef<PropertiesEditor>> _subEditors;
	RolloutInsertionParameters _subEditorRolloutParams;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_LINKED_FILE_OBJECT_EDITOR_H
