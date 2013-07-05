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
#include <core/scene/objects/SceneObject.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/ObjectNode.h>
#include <core/gui/undo/UndoManager.h>
#include <core/gui/actions/ActionManager.h>
#include "ModifyCommandPage.h"
#include "ModificationListModel.h"
#include "ModifierListModel.h"

namespace Ovito {

/******************************************************************************
* Initializes the modify page.
******************************************************************************/
ModifyCommandPage::ModifyCommandPage()
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(2,2,2,2);
	layout->setSpacing(0);

	_modifierSelector = new QComboBox();
	layout->addSpacing(4);
    layout->addWidget(_modifierSelector);
    connect(_modifierSelector, SIGNAL(activated(int)), this, SLOT(onModifierAdd(int)));

	class ModifierStackListView : public QListView {
	public:
		ModifierStackListView(QWidget* parent) : QListView(parent) {}
		virtual QSize sizeHint() const { return QSize(256, 130); }
	};

	QSplitter* splitter = new QSplitter(Qt::Vertical);
	splitter->setChildrenCollapsible(false);

	QWidget* upperContainer = new QWidget();
	splitter->addWidget(upperContainer);
	QHBoxLayout* subLayout = new QHBoxLayout(upperContainer);
	subLayout->setContentsMargins(0,0,0,0);
	subLayout->setSpacing(2);

	_modificationListModel = new ModificationListModel(this);
	_modificationListWidget = new ModifierStackListView(upperContainer);
	_modificationListWidget->setModel(_modificationListModel);
	_modificationListWidget->setSelectionModel(_modificationListModel->selectionModel());
	connect(_modificationListModel, SIGNAL(selectedItemChanged()), this, SLOT(onSelectedItemChanged()));
	connect(_modificationListWidget, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onModifierStackDoubleClicked(const QModelIndex&)));
	layout->addSpacing(4);
	subLayout->addWidget(_modificationListWidget);

	_modifierSelector->setModel(new ModifierListModel(this, _modificationListModel, _modifierSelector));

	QToolBar* editToolbar = new QToolBar(this);
	editToolbar->setOrientation(Qt::Vertical);
#ifndef Q_WS_MAC
	editToolbar->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; }");
#endif
	subLayout->addWidget(editToolbar);

	QAction* deleteModifierAction = ActionManager::instance().createCommandAction(ACTION_MODIFIER_DELETE, tr("Delete Modifier"), ":/core/actions/modify/delete_modifier.png");
	connect(deleteModifierAction, SIGNAL(triggered(bool)), this, SLOT(onDeleteModifier()));
	editToolbar->addAction(deleteModifierAction);

	editToolbar->addSeparator();

	QAction* moveModifierUpAction = ActionManager::instance().createCommandAction(ACTION_MODIFIER_MOVE_UP, tr("Move Modifier Up"), ":/core/actions/modify/modifier_move_up.png");
	connect(moveModifierUpAction, SIGNAL(triggered(bool)), this, SLOT(onModifierMoveUp()));
	editToolbar->addAction(moveModifierUpAction);
	QAction* moveModifierDownAction = ActionManager::instance().createCommandAction(ACTION_MODIFIER_MOVE_DOWN, tr("Move Modifier Down"), ":/core/actions/modify/modifier_move_down.png");
	connect(moveModifierDownAction, SIGNAL(triggered(bool)), this, SLOT(onModifierMoveDown()));
	editToolbar->addAction(moveModifierDownAction);

	editToolbar->addSeparator();

	QAction* toggleModifierStateAction = ActionManager::instance().createCommandAction(ACTION_MODIFIER_TOGGLE_STATE, tr("Enable/Disable Modifier"));
	toggleModifierStateAction->setCheckable(true);
	QIcon toggleStateActionIcon(QString(":/core/actions/modify/modifier_enabled_large.png"));
	toggleStateActionIcon.addFile(QString(":/core/actions/modify/modifier_disabled_large.png"), QSize(), QIcon::Normal, QIcon::On);
	toggleModifierStateAction->setIcon(toggleStateActionIcon);
	connect(toggleModifierStateAction, SIGNAL(triggered(bool)), this, SLOT(onModifierToggleState(bool)));
	editToolbar->addAction(toggleModifierStateAction);

	layout->addWidget(splitter);
	layout->addSpacing(4);

	// Create the properties panel.
	_propertiesPanel = new PropertiesPanel(nullptr);
	_propertiesPanel->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	splitter->addWidget(_propertiesPanel);
	splitter->setStretchFactor(1,1);

	connect(&_selectionSetListener, SIGNAL(notificationEvent(ReferenceEvent*)), this, SLOT(onSelectionSetEvent(ReferenceEvent*)));
	updateActions(nullptr);
}

/******************************************************************************
* Resets the modify page to the initial state.
******************************************************************************/
void ModifyCommandPage::reset()
{
	CommandPanelPage::reset();
}

/******************************************************************************
* Is called when the user selects the page.
******************************************************************************/
void ModifyCommandPage::onEnter()
{
	CommandPanelPage::onEnter();

	// Update everything.
	updateActions(nullptr);
	onSelectionChangeComplete(DataSetManager::instance().currentSelection());
}

/******************************************************************************
* Is called when the user selects another page.
******************************************************************************/
void ModifyCommandPage::onLeave()
{
	CommandPanelPage::onLeave();
	_modificationListModel->clear();
	_selectionSetListener.setTarget(nullptr);
}

/******************************************************************************
* This is called after all changes to the selection set have been completed.
******************************************************************************/
void ModifyCommandPage::onSelectionChangeComplete(SelectionSet* newSelection)
{
	// Make sure we get informed about any future changes of the selection set.
	_selectionSetListener.setTarget(newSelection);

	_modificationListModel->refreshList();
}

/******************************************************************************
* This is called by the RefTargetListener that listens to notification messages sent by the
* current selection set.
******************************************************************************/
void ModifyCommandPage::onSelectionSetEvent(ReferenceEvent* event)
{
}

/******************************************************************************
* Is called when a new modification list item has been selected, or if the currently
* selected item has changed.
******************************************************************************/
void ModifyCommandPage::onSelectedItemChanged()
{
	ModificationListItem* currentItem = _modificationListModel->selectedItem();
	RefTarget* object = currentItem ? currentItem->object() : nullptr;

	_propertiesPanel->setEditObject(object);
	updateActions(currentItem);
}

/******************************************************************************
* Updates the state of the actions that can be invoked on the currently selected item.
******************************************************************************/
void ModifyCommandPage::updateActions(ModificationListItem* currentItem)
{
	QAction* deleteModifierAction = ActionManager::instance().getAction(ACTION_MODIFIER_DELETE);
	QAction* moveModifierUpAction = ActionManager::instance().getAction(ACTION_MODIFIER_MOVE_UP);
	QAction* moveModifierDownAction = ActionManager::instance().getAction(ACTION_MODIFIER_MOVE_DOWN);
	QAction* toggleModifierStateAction = ActionManager::instance().getAction(ACTION_MODIFIER_TOGGLE_STATE);

	Modifier* modifier = currentItem ? dynamic_object_cast<Modifier>(currentItem->object()) : nullptr;
	if(modifier) {
		deleteModifierAction->setEnabled(true);
		if(currentItem->modifierApplications().size() == 1) {
			ModifierApplication* modApp = currentItem->modifierApplications()[0];
			PipelineObject* pipelineObj = modApp->pipelineObject();
			if(pipelineObj) {
				OVITO_ASSERT(pipelineObj->modifierApplications().contains(modApp));
				moveModifierUpAction->setEnabled(modApp != pipelineObj->modifierApplications().back());
				moveModifierDownAction->setEnabled(modApp != pipelineObj->modifierApplications().front());
			}
		}
		else {
			moveModifierUpAction->setEnabled(false);
			moveModifierDownAction->setEnabled(false);
		}
		if(modifier) {
			toggleModifierStateAction->setEnabled(true);
			toggleModifierStateAction->setChecked(modifier->isEnabled() == false);
		}
		else {
			toggleModifierStateAction->setChecked(false);
			toggleModifierStateAction->setEnabled(false);
		}
	}
	else {
		deleteModifierAction->setEnabled(false);
		moveModifierUpAction->setEnabled(false);
		moveModifierDownAction->setEnabled(false);
		toggleModifierStateAction->setChecked(false);
		toggleModifierStateAction->setEnabled(false);
	}
}

/******************************************************************************
* Is called when the user has selected an item in the modifier class list.
******************************************************************************/
void ModifyCommandPage::onModifierAdd(int index)
{
	if(index >= 0 && _modificationListModel->isUpToDate()) {
		const OvitoObjectType* descriptor = static_cast<const OvitoObjectType*>(_modifierSelector->itemData(index).value<void*>());
		if(descriptor) {
			UndoManager::instance().beginCompoundOperation(tr("Apply modifier"));
			try {
				// Create an instance of the modifier...
				OORef<Modifier> modifier = static_object_cast<Modifier>(descriptor->createInstance());
				OVITO_CHECK_OBJECT_POINTER(modifier);
				// .. and apply it.
				_modificationListModel->applyModifier(modifier.get());
			}
			catch(const Exception& ex) {
				ex.showError();
				UndoManager::instance().currentCompoundOperation()->clear();
			}
			UndoManager::instance().endCompoundOperation();
			_modificationListModel->requestUpdate();
		}
		_modifierSelector->setCurrentIndex(0);
	}
}

/******************************************************************************
* Handles the ACTION_MODIFIER_DELETE command.
******************************************************************************/
void ModifyCommandPage::onDeleteModifier()
{
	// Get the currently selected modifier.
	ModificationListItem* selectedItem = _modificationListModel->selectedItem();
	if(!selectedItem) return;

	Modifier* modifier = dynamic_object_cast<Modifier>(selectedItem->object());
	if(!modifier) return;

	UndoManager::instance().beginCompoundOperation(tr("Delete modifier"));
	try {
		// Remove each ModifierApplication from the ModifiedObject it belongs to.
		Q_FOREACH(ModifierApplication* modApp, selectedItem->modifierApplications()) {
			OVITO_ASSERT(modApp->modifier() == modifier);
			OVITO_CHECK_OBJECT_POINTER(modApp->pipelineObject());
			modApp->pipelineObject()->removeModifier(modApp);
		}
	}
	catch(const Exception& ex) {
		ex.showError();
		UndoManager::instance().currentCompoundOperation()->clear();
	}
	UndoManager::instance().endCompoundOperation();
}

/******************************************************************************
* This called when the user double clicks on an item in the modifier stack.
******************************************************************************/
void ModifyCommandPage::onModifierStackDoubleClicked(const QModelIndex& index)
{
	ModificationListItem* item = _modificationListModel->item(index.row());
	OVITO_CHECK_OBJECT_POINTER(item);

	Modifier* modifier = dynamic_object_cast<Modifier>(item->object());
	if(modifier) {
		// Toggle enabled state of modifier.
		UndoManager::instance().beginCompoundOperation(tr("Toggle modifier state"));
		try {
			modifier->setEnabled(!modifier->isEnabled());
		}
		catch(const Exception& ex) {
			ex.showError();
			UndoManager::instance().currentCompoundOperation()->clear();
		}
		UndoManager::instance().endCompoundOperation();
	}
}

/******************************************************************************
* Handles the ACTION_MODIFIER_MOVE_UP command, which moves the selected
* modifier up one position in the stack.
******************************************************************************/
void ModifyCommandPage::onModifierMoveUp()
{
	// Get the currently selected modifier.
	ModificationListItem* selectedItem = _modificationListModel->selectedItem();
	if(!selectedItem) return;

	if(selectedItem->modifierApplications().size() != 1)
		return;

	OORef<ModifierApplication> modApp = selectedItem->modifierApplications()[0];
	OORef<PipelineObject> pipelineObj = modApp->pipelineObject();
	if(!pipelineObj) return;

	OVITO_ASSERT(pipelineObj->modifierApplications().contains(modApp.get()));
	if(modApp == pipelineObj->modifierApplications().back())
		return;

	UndoManager::instance().beginCompoundOperation(tr("Move modifier up"));
	try {
		// Determine old position in stack.
		int index = pipelineObj->modifierApplications().indexOf(modApp.get());
		// Remove ModifierApplication from the ModifiedObject.
		pipelineObj->removeModifier(modApp.get());
		// Re-insert ModifierApplication into the ModifiedObject.
		pipelineObj->insertModifierApplication(modApp.get(), index+1);
	}
	catch(const Exception& ex) {
		ex.showError();
		UndoManager::instance().currentCompoundOperation()->clear();
	}
	UndoManager::instance().endCompoundOperation();
}

/******************************************************************************
* Handles the ACTION_MODIFIER_MOVE_DOWN command, which moves the selected
* modifier down one position in the stack.
******************************************************************************/
void ModifyCommandPage::onModifierMoveDown()
{
	// Get the currently selected modifier.
	ModificationListItem* selectedItem = _modificationListModel->selectedItem();
	if(!selectedItem) return;

	if(selectedItem->modifierApplications().size() != 1)
		return;

	OORef<ModifierApplication> modApp = selectedItem->modifierApplications()[0];
	OORef<PipelineObject> pipelineObj = modApp->pipelineObject();
	if(!pipelineObj)
		return;

	OVITO_ASSERT(pipelineObj->modifierApplications().contains(modApp.get()));
	if(modApp == pipelineObj->modifierApplications().front())
		return;

	UndoManager::instance().beginCompoundOperation(tr("Move modifier down"));
	try {
		// Determine old position in stack.
		int index = pipelineObj->modifierApplications().indexOf(modApp.get());
		// Remove ModifierApplication from the ModifiedObject.
		pipelineObj->removeModifier(modApp.get());
		// Re-insert ModifierApplication into the ModifiedObject.
		pipelineObj->insertModifierApplication(modApp.get(), index-1);
	}
	catch(const Exception& ex) {
		ex.showError();
		UndoManager::instance().currentCompoundOperation()->clear();
	}
	UndoManager::instance().endCompoundOperation();
}

/******************************************************************************
* Handles the ACTION_MODIFIER_TOGGLE_STATE command, which toggles the
* enabled/disable state of the selected modifier.
******************************************************************************/
void ModifyCommandPage::onModifierToggleState(bool newState)
{
	// Get the selected modifier from the modifier stack box.
	QModelIndexList selection = _modificationListWidget->selectionModel()->selectedRows();
	if(selection.empty())
		return;

	onModifierStackDoubleClicked(selection.front());
}

};
