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

#ifndef __OVITO_FILE_SOURCE_OBJECT_EDITOR_H
#define __OVITO_FILE_SOURCE_OBJECT_EDITOR_H

#include <core/Core.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/gui/properties/PropertiesPanel.h>
#include <core/gui/widgets/general/ElidedTextLabel.h>
#include <core/gui/widgets/display/StatusWidget.h>

#include "FileSource.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(DataIO) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A properties editor for the FileSource object.
 */
class FileSourceEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE FileSourceEditor() {
		connect(this, &PropertiesEditor::contentsReplaced, this, &FileSourceEditor::onEditorContentsReplaced);
	}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

protected Q_SLOTS:

	/// Is called when a new object has been loaded into the editor.
	void onEditorContentsReplaced(RefTarget* newObject);

	/// Is called when the user presses the "Pick local input file" button.
	void onPickLocalInputFile();

	/// Is called when the user presses the "Pick remote input file" button.
	void onPickRemoteInputFile();

	/// Is called when the user presses the Reload frame button.
	void onReloadFrame();

	/// Is called when the user presses the Reload animation button.
	void onReloadAnimation();

	/// Updates the displayed status information.
	void updateInformationLabel();

	/// This is called when the user has changed the source URL.
	void onWildcardPatternEntered();

	/// Is called when the user has selected a certain frame in the frame list box.
	void onFrameSelected(int index);

private:

	QLineEdit* _filenameLabel;
	QLineEdit* _sourcePathLabel;
	QLineEdit* _wildcardPatternTextbox;
	StatusWidget* _statusLabel;
	QComboBox* _framesListBox;

	std::vector<OORef<PropertiesEditor>> _subEditors;
	RolloutInsertionParameters _subEditorRolloutParams;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_FILE_SOURCE_OBJECT_EDITOR_H
