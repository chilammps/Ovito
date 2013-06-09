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
#include <core/plugins/PluginManager.h>
#include <core/scene/objects/SceneObject.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/ObjectNode.h>
#include <core/gui/undo/UndoManager.h>
#include <core/gui/actions/ActionManager.h>
#include "ModifyCommandPage.h"
#include "ModifierStack.h"

namespace Ovito {

/******************************************************************************
* Initializes the modify page.
******************************************************************************/
ModifyCommandPage::ModifyCommandPage()
{
	scanInstalledModifierClasses();

	stack = new ModifierStack(this);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(2,2,2,2);
	layout->setSpacing(0);

	modifierSelector = new QComboBox();
	layout->addSpacing(4);
    layout->addWidget(modifierSelector);
    connect(modifierSelector, SIGNAL(activated(int)), this, SLOT(onModifierAdd(int)));

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
	subLayout->setSpacing(0);

	stackBox = new ModifierStackListView(upperContainer);
	stackBox->setModel(stack->listModel());
	connect(stackBox->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(onModifierStackSelectionChanged()));
	connect(stackBox, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onModifierStackDoubleClicked(const QModelIndex&)));
	layout->addSpacing(4);
	subLayout->addWidget(stackBox);

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
	propertiesPanel = new PropertiesPanel(NULL);
	propertiesPanel->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	splitter->addWidget(propertiesPanel);
	splitter->setStretchFactor(1,1);

	connect(&selectionSetListener, SIGNAL(notificationEvent(ReferenceEvent*)), this, SLOT(onSelectionSetEvent(ReferenceEvent*)));
}

/******************************************************************************
* Finds all modifier classes provided by the installed plugins.
******************************************************************************/
void ModifyCommandPage::scanInstalledModifierClasses()
{
	// Create an iterator that retrieves all available modifiers.
	Q_FOREACH(const OvitoObjectType* clazz, PluginManager::instance().listClasses(Modifier::OOType)) {
		modifierClasses.push_back(clazz);
	}
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
	onSelectionChangeComplete(DataSetManager::instance().currentSelection());
}

/******************************************************************************
* Is called when the user selects another page.
******************************************************************************/
void ModifyCommandPage::onLeave()
{
	CommandPanelPage::onLeave();
	stack->clearStack();
	stack->validate();
	selectionSetListener.setTarget(NULL);
}

/******************************************************************************
* This is called after all changes to the selection set have been completed.
******************************************************************************/
void ModifyCommandPage::onSelectionChangeComplete(SelectionSet* newSelection)
{
	selectionSetListener.setTarget(newSelection);
	stack->validate();
	stack->refreshModifierStack();
}

/******************************************************************************
* This is called by the RefTargetListener that listens to notification messages sent by the
* current selection set.
******************************************************************************/
void ModifyCommandPage::onSelectionSetEvent(ReferenceEvent* event)
{
}

/******************************************************************************
* Is called when the user has selected another item in the modifier stack list box.
******************************************************************************/
void ModifyCommandPage::onModifierStackSelectionChanged()
{
	stack->updatePropertiesPanel();
}

/******************************************************************************
* Is called when the user has selected an item in the modifier class list.
******************************************************************************/
void ModifyCommandPage::onModifierAdd(int index)
{
	if(index >= 0 && stack->isValid()) {
		OvitoObjectType* descriptor = (OvitoObjectType*)modifierSelector->itemData(index).value<void*>();
		if(descriptor) {
			UndoManager::instance().beginCompoundOperation(tr("Apply modifier"));
			try {
				// Create an instance of the modifier...
				OORef<Modifier> modifier = static_object_cast<Modifier>(descriptor->createInstance());
				OVITO_CHECK_OBJECT_POINTER(modifier);
				// .. and apply it.
				stack->applyModifier(modifier.get());
			}
			catch(const Exception& ex) {
				ex.showError();
				UndoManager::instance().currentCompoundOperation()->clear();
			}
			UndoManager::instance().endCompoundOperation();
			stack->invalidate();
		}
		modifierSelector->setCurrentIndex(0);
	}
}

/******************************************************************************
* Handles the ACTION_MODIFIER_DELETE command.
******************************************************************************/
void ModifyCommandPage::onDeleteModifier()
{
	// Get the selected modifier from the modifier stack box.
	QModelIndexList selection = stackBox->selectionModel()->selectedRows();
	if(selection.empty()) return;
	ModifierStackEntry* selEntry = (ModifierStackEntry*)selection.front().data(Qt::UserRole).value<void*>();
	OVITO_CHECK_OBJECT_POINTER(selEntry);

	Modifier* modifier = dynamic_object_cast<Modifier>(selEntry->commonObject());
	if(!modifier) return;

	UndoManager::instance().beginCompoundOperation(tr("Delete modifier"));
	try {
		// Remove each ModifierApplication from the ModifiedObject it belongs to.
		Q_FOREACH(ModifierApplication* modApp, selEntry->modifierApplications()) {
			OVITO_ASSERT(modApp->modifier() == modifier);
			modApp->pipelineObject()->removeModifier(modApp);
		}
	}
	catch(const Exception& ex) {
		ex.showError();
		UndoManager::instance().currentCompoundOperation()->clear();
	}
	UndoManager::instance().endCompoundOperation();
	stack->invalidate();
}

/******************************************************************************
* This called when the user double clicks on an item in the modifier stack.
******************************************************************************/
void ModifyCommandPage::onModifierStackDoubleClicked(const QModelIndex& index)
{
	ModifierStackEntry* entry = (ModifierStackEntry*)index.data(Qt::UserRole).value<void*>();
	OVITO_CHECK_OBJECT_POINTER(entry);

	Modifier* modifier = dynamic_object_cast<Modifier>(entry->commonObject());
	if(modifier) {
		// Toggle enabled state of modifier.
		UndoManager::instance().beginCompoundOperation(tr("Toggle modifier state"));
		try {
#if 0
			modifier->setModifierEnabled(!modifier->isModifierEnabled());
#endif
		}
		catch(const Exception& ex) {
			ex.showError();
			UndoManager::instance().currentCompoundOperation()->clear();
		}
		UndoManager::instance().endCompoundOperation();
	}
}

/******************************************************************************
* Handles the ACTION_MODIFIER_MOVE_UP command, which moves the selected modifier up one entry in the stack.
******************************************************************************/
void ModifyCommandPage::onModifierMoveUp()
{
	// Get the selected modifier from the modifier stack box.
	QModelIndexList selection = stackBox->selectionModel()->selectedRows();
	if(selection.empty()) return;

	ModifierStackEntry* selectedEntry = (ModifierStackEntry*)selection.front().data(Qt::UserRole).value<void*>();
	OVITO_CHECK_OBJECT_POINTER(selectedEntry);

	if(selectedEntry->modifierApplications().size() != 1) return;

	OORef<ModifierApplication> modApp = selectedEntry->modifierApplications()[0];
	OORef<PipelineObject> modObj = modApp->pipelineObject();
	if(modObj == NULL) return;

	OVITO_ASSERT(modObj->modifierApplications().contains(modApp.get()));
	if(modApp == modObj->modifierApplications().back()) return;

	UndoManager::instance().beginCompoundOperation(tr("Move modifier up"));
	try {
		// Determine old position in stack.
		int index = modObj->modifierApplications().indexOf(modApp.get());
		// Remove ModifierApplication from the ModifiedObject.
		modObj->removeModifier(modApp.get());
		// Re-insert ModifierApplication into the ModifiedObject.
		modObj->insertModifierApplication(modApp.get(), index+1);
	}
	catch(const Exception& ex) {
		ex.showError();
		UndoManager::instance().currentCompoundOperation()->clear();
	}
	UndoManager::instance().endCompoundOperation();
	stack->invalidate();
}

/******************************************************************************
* Handles the ACTION_MODIFIER_MOVE_DOWN command, which moves the selected modifier down one entry in the stack.
******************************************************************************/
void ModifyCommandPage::onModifierMoveDown()
{
	// Get the selected modifier from the modifier stack box.
	QModelIndexList selection = stackBox->selectionModel()->selectedRows();
	if(selection.empty()) return;

	ModifierStackEntry* selectedEntry = (ModifierStackEntry*)selection.front().data(Qt::UserRole).value<void*>();
	OVITO_CHECK_OBJECT_POINTER(selectedEntry);

	if(selectedEntry->modifierApplications().size() != 1) return;

	OORef<ModifierApplication> modApp = selectedEntry->modifierApplications()[0];
	OORef<PipelineObject> modObj = modApp->pipelineObject();
	if(modObj == NULL) return;

	OVITO_ASSERT(modObj->modifierApplications().contains(modApp.get()));
	if(modApp == modObj->modifierApplications().front()) return;

	UndoManager::instance().beginCompoundOperation(tr("Move modifier down"));
	try {
		// Determine old position in stack.
		int index = modObj->modifierApplications().indexOf(modApp.get());
		// Remove ModifierApplication from the ModifiedObject.
		modObj->removeModifier(modApp.get());
		// Re-insert ModifierApplication into the ModifiedObject.
		modObj->insertModifierApplication(modApp.get(), index-1);
	}
	catch(const Exception& ex) {
		ex.showError();
		UndoManager::instance().currentCompoundOperation()->clear();
	}
	UndoManager::instance().endCompoundOperation();
	stack->invalidate();
}

/******************************************************************************
* Handles the ACTION_MODIFIER_TOGGLE_STATE command, which toggles the enabled/disable state of the selected modifier.
******************************************************************************/
void ModifyCommandPage::onModifierToggleState(bool newState)
{
	// Get the selected modifier from the modifier stack box.
	QModelIndexList selection = stackBox->selectionModel()->selectedRows();
	if(selection.empty()) return;

	onModifierStackDoubleClicked(selection.front());
}

};
