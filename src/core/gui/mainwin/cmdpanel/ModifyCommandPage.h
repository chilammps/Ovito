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

#ifndef __OVITO_MODIFY_COMMAND_PAGE_H
#define __OVITO_MODIFY_COMMAND_PAGE_H

#include <core/Core.h>
#include <core/gui/properties/PropertiesPanel.h>
#include <core/gui/widgets/general/RolloutContainer.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/reference/RefTargetListener.h>

// QtNetwork module
#include <QtNetwork>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

class ModificationListModel;	// defined in ModificationListModel.h
class ModificationListItem;		// defined in ModificationListModel.h
class ModifierListBox;			// defined in ModifierListBox.h

/**
 * The command panel tab lets the user modify the selected object.
 */
class OVITO_CORE_EXPORT ModifyCommandPage : public QWidget
{
	Q_OBJECT

public:

	/// Initializes the modify page.
    ModifyCommandPage(MainWindow* mainWindow, QWidget* parent);

	/// Returns the object that is currently being edited in the properties panel.
	RefTarget* editObject() const { return _propertiesPanel->editObject(); }

	/// Returns the list model that encapsulates the modification pipeline of the selected node(s).
	ModificationListModel* modificationListModel() const { return _modificationListModel; }

protected Q_SLOTS:

	/// This is called after all changes to the selection set have been completed.
	void onSelectionChangeComplete(SelectionSet* newSelection);

	/// Is called when a new modification list item has been selected, or if the currently
	/// selected item has changed.
	void onSelectedItemChanged();

	/// Handles the ACTION_MODIFIER_DELETE command, which deleted the selected modifier from the stack.
	void onDeleteModifier();

	/// Is called when the user has selected an item in the modifier class list.
	void onModifierAdd(int index);

	/// This called when the user double clicks on an item in the modifier stack.
	void onModifierStackDoubleClicked(const QModelIndex& index);

	/// Handles the ACTION_MODIFIER_MOVE_UP command, which moves the selected modifier up one entry in the stack.
	void onModifierMoveUp();

	/// Handles the ACTION_MODIFIER_MOVE_DOWN command, which moves the selected modifier down one entry in the stack.
	void onModifierMoveDown();

	/// Handles the ACTION_MODIFIER_TOGGLE_STATE command, which toggles the enabled/disable state of the selected modifier.
	void onModifierToggleState(bool newState);

	/// Is called by the system when fetching the news web page from the server is completed.
	void onWebRequestFinished(QNetworkReply* reply);

private:

	/// Updates the state of the actions that can be invoked on the currently selected item.
	void updateActions(ModificationListItem* currentItem);

	/// Creates the rollout panel that shows information about the application whenever no object is selected.
	void createAboutPanel();

private:

	/// The container of the current dataset being edited.
	DataSetContainer& _datasetContainer;

	/// The action manager of the main window.
	ActionManager* _actionManager;

	/// This list box shows the modifier stack of the selected scene node(s).
	QListView* _modificationListWidget;

	/// The visual representation of the modification pipeline of the selected node(s).
	ModificationListModel* _modificationListModel;

	/// This control displays the list of available modifier classes and allows the user to apply a modifier.
	ModifierListBox* _modifierSelector;

	/// This panel shows the properties of the selected modifier stack entry
	PropertiesPanel* _propertiesPanel;

	/// The panel displaying information about the application when no object is selected.
	Rollout* _aboutRollout;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif	// __OVITO_MODIFY_COMMAND_PAGE_H
