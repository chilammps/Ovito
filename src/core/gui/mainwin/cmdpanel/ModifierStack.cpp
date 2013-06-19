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
#include <core/viewport/ViewportManager.h>
#include <core/gui/undo/UndoManager.h>
#include <core/gui/actions/ActionManager.h>
#include <core/plugins/PluginManager.h>
#include "ModifierStack.h"
#include "ModifyCommandPage.h"

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, ModifierStackEntry, RefTarget)
DEFINE_FLAGS_REFERENCE_FIELD(ModifierStackEntry, _object, "Object", RefTarget, PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(ModifierStackEntry, _modApps, "ModifierApplications", ModifierApplication, PROPERTY_FIELD_NO_UNDO)

/******************************************************************************
* Constructor.
******************************************************************************/
ModifierStackEntry::ModifierStackEntry(ModifierStack* stack, RefTarget* commonObject, bool isSubObject) :
	_stack(stack),
	_isSubObject(isSubObject)
{
	INIT_PROPERTY_FIELD(ModifierStackEntry::_object);
	INIT_PROPERTY_FIELD(ModifierStackEntry::_modApps);

	this->_object = commonObject;
}


/******************************************************************************
* This method is called when an object referenced by the modifier
* stack list sends a message.
******************************************************************************/
bool ModifierStackEntry::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	// The modifier stack list must be updated if a modifier has been added or removed
	// from a PipelineObject.
	if((event->type() == ReferenceEvent::ReferenceAdded || event->type() == ReferenceEvent::ReferenceRemoved || event->type() == ReferenceEvent::ReferenceChanged)
		&& source == commonObject() && dynamic_object_cast<PipelineObject>(commonObject()))
	{
		_stack->invalidate();
	}
	/// Update a modifier entry if the modifier has been enabled or disabled.
	else if(event->type() == ReferenceEvent::TargetEnabledOrDisabled && source == commonObject() && event->sender() == commonObject()) {
		_stack->listModel()->refreshStackEntry(this);
	}
	/// Update an entry if the evaluation status of the modifier has changed.
	else if(event->type() == ReferenceEvent::StatusChanged) {
		_stack->listModel()->refreshStackEntry(this);
	}
	/// If the list of sub-objects changes for one of the entries, we need
	/// to update everything.
	else if(event->type() == ReferenceEvent::SubobjectListChanged && source == commonObject() && event->sender() == commonObject()) {
		_stack->invalidate();
	}

	return RefTarget::referenceEvent(source, event);
}

IMPLEMENT_OVITO_OBJECT(Core, ModifierStack, RefMaker)
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(ModifierStack, stackEntries, "StackEntries", ModifierStackEntry, PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(ModifierStack, selectedNodes, "SelectedNodes", ObjectNode, PROPERTY_FIELD_NO_UNDO)

/******************************************************************************
* Initializes the object.
******************************************************************************/
ModifierStack::ModifierStack(ModifyCommandPage* modifyPage)
	: page(modifyPage), nextObjectToSelect(nullptr), needStackUpdate(false)
{
	INIT_PROPERTY_FIELD(ModifierStack::stackEntries);
	INIT_PROPERTY_FIELD(ModifierStack::selectedNodes);

	loadModifierCategories();

	_listModel = new ModifierStackModel(this);
	setParent(modifyPage);
	connect(this, SIGNAL(internalStackUpdate()), this, SLOT(onInternalStackUpdate()), Qt::QueuedConnection);
}

/******************************************************************************
* Returns the currently selected modifier stack entry (or NULL).
******************************************************************************/
ModifierStackEntry* ModifierStack::selectedEntry() const
{
	QModelIndexList selection = page->stackBox->selectionModel()->selectedRows();
	if(selection.empty()) return NULL;
	return (ModifierStackEntry*)selection.front().data(Qt::UserRole).value<void*>();
}

/******************************************************************************
* Clears the displayed modification stack.
******************************************************************************/
void ModifierStack::clearStack()
{
	listModel()->clear();
	UndoSuspender noUndo;
	stackEntries.clear();
	selectedNodes.clear();
	updatePropertiesPanel();
}

/******************************************************************************
* Filters the given input list for ObjectNodes.
******************************************************************************/
void ModifierStack::collectObjectNodes(const QVector<SceneNode*>& in)
{
	Q_FOREACH(SceneNode* node, in) {
		if(node->isObjectNode()) {
			selectedNodes.push_back(static_object_cast<ObjectNode>(node));
		}
		else if(node->isGroupNode()) {
            // Step recursively into the group node.
			collectObjectNodes(node->children());
		}
	}
}

/******************************************************************************
* If all selected object nodes reference the same SceneObject
* then it is returned by this method; otherwise NULL is returned.
******************************************************************************/
SceneObject* ModifierStack::commonObject()
{
	SceneObject* obj = nullptr;
	Q_FOREACH(ObjectNode* objNode, selectedNodes.targets()) {
		if(obj == nullptr) obj = objNode->sceneObject();
		else if(obj != objNode->sceneObject()) return nullptr;	// The scene nodes are not compatible.
	}
	return obj;
}

/******************************************************************************
* Completely rebuilds the modifier stack list.
******************************************************************************/
void ModifierStack::refreshModifierStack()
{
	UndoSuspender noUndo;

	// Determine the currently selected object and
	// try to select it again after the stack has been rebuilt.
	// If nextObjectToSelect is already non-NULL then the caller
	// has specified an object to be selected.
	if(nextObjectToSelect == NULL) {
		ModifierStackEntry* entry = selectedEntry();
		if(entry) {
			OVITO_CHECK_OBJECT_POINTER(entry);
			nextObjectToSelect = entry->commonObject();
		}
	}

	// Remove old stuff.
	clearStack();

	// Collect all selected ObjectNodes.
	selectedNodes.clear();
	collectObjectNodes(DataSetManager::instance().currentSelection()->nodes());

    SceneObject* cmnObject = commonObject();
	if(cmnObject != NULL) {
		// The user has selected a set of identical instances.

		// Walk up the pipeline.
		do {
			OVITO_CHECK_OBJECT_POINTER(cmnObject);

			// Create entries for the modifier applications if this is a ModifiedObject.
			PipelineObject* modObj = dynamic_object_cast<PipelineObject>(cmnObject);
			if(modObj != NULL) {
				for(int i = modObj->modifierApplications().size(); i--; ) {
					ModifierApplication* app = modObj->modifierApplications()[i];
				    ModifierStackEntry* entry = new ModifierStackEntry(this, app->modifier());
					entry->addModifierApplication(app);
					stackEntries.push_back(entry);
				}
			}

			// Create an entry for the scene object.
            stackEntries.push_back(new ModifierStackEntry(this, cmnObject));

            // Create entries for the object's editable sub-objects.
            for(int i = 0; i < cmnObject->editableSubObjectCount(); i++) {
            	RefTarget* subobject = cmnObject->editableSubObject(i);
            	if(subobject != NULL && subobject->isSubObjectEditable()) {
            		stackEntries.push_back(new ModifierStackEntry(this, subobject, true));
            	}
            }

			// In case there are multiple input slots, determine if they all point to the same object.
			SceneObject* nextObj = NULL;
			for(int i = 0; i < cmnObject->inputObjectCount(); i++) {
				if(nextObj == NULL)
					nextObj = cmnObject->inputObject(i);
				else if(nextObj != cmnObject->inputObject(i)) {
					nextObj = NULL;  // The input objects do not match.
					break;
				}
			}
			cmnObject = nextObj;
		}
		while(cmnObject != NULL);
	}
	else {

		// Several different scene nodes are selected that are not cloned instances.
		// But maybe they have been applied the same modifier which should be
		// displayed in the modifier stack.

		// Collect the PipelineObject of all selected nodes.
        QVector<PipelineObject*> modObjs;
		Q_FOREACH(ObjectNode* objNode, selectedNodes.targets()) {
			PipelineObject* modObj = dynamic_object_cast<PipelineObject>(objNode->sceneObject());
			if(modObj == NULL) {
				modObjs.clear();
				break;
			}
			modObjs.push_back(modObj);
		}

		if(!modObjs.empty()) {

			// Each of the selected nodes has a ModifiedObject, now try to find
			// identical modifiers in them.

			bool done = false;
			for(size_t i=0; !done; i++) {
                QVector<ModifierApplication*> apps;
				Q_FOREACH(PipelineObject* modObj, modObjs) {
					if(modObj->modifierApplications().size() == i) {
						done = true;
						break;
					}
					ModifierApplication* app = modObj->modifierApplications()[modObj->modifierApplications().size() - i - 1];
					if(!apps.empty() && apps.front()->modifier() != app->modifier()) {
						done = true;
						break;
					}
					apps.push_back(app);
				}
				if(!done) {
					ModifierStackEntry* entry = new ModifierStackEntry(this, apps.front()->modifier());
					Q_FOREACH(ModifierApplication* app, apps)
						entry->addModifierApplication(app);
					stackEntries.push_back(entry);
				}
			}
		}
	}

	// The internal list of ModifierStackEntries is now complete.
	// Fill the list model with them.
	int selIndex = 0;
	QVector<ModifierStackEntry*> listBoxEntries;
	Q_FOREACH(ModifierStackEntry* entry, stackEntries.targets()) {
		if(nextObjectToSelect == entry->commonObject())
			selIndex = listBoxEntries.size();
		if(dynamic_object_cast<PipelineObject>(entry->commonObject())) {
			// Skip ModifiedObject if it is empty and on the top of the stack.
			if(listBoxEntries.empty()) continue;
		}
		listBoxEntries.push_back(entry);
	}
	listModel()->setEntries(listBoxEntries);
	nextObjectToSelect = NULL;
	page->stackBox->setEnabled(!listBoxEntries.empty());

	// Select the first entry in the list or the one given by "nextObjectToSelect".
	if(!listBoxEntries.empty())
		page->stackBox->selectionModel()->select(listModel()->index(selIndex), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Clear);

	// Show the properties of the selected object.
	updatePropertiesPanel();
}

/******************************************************************************
* Shows the properties of the selected item in the modifier stack box in the
* properties panel of the page.
******************************************************************************/
void ModifierStack::updatePropertiesPanel()
{
	ModifierStackEntry* selEntry = selectedEntry();

	if(selEntry == NULL) {
		page->propertiesPanel->setEditObject(NULL);
		updateAvailableModifiers(NULL);
		updateAvailableActions(NULL);
	}
	else {
		page->propertiesPanel->setEditObject(selEntry->commonObject());
		updateAvailableModifiers(selEntry);
		updateAvailableActions(selEntry);
	}

	ViewportManager::instance().updateViewports();
}

/******************************************************************************
* Defines a comparison operator for the modifier list.
* This ensures an alphabetical ordering.
******************************************************************************/
bool ModifierStack::modifierOrdering(const OvitoObjectType* a, const OvitoObjectType* b)
{
    return QString::compare(a->name(), b->name(), Qt::CaseInsensitive) < 0;
}

/******************************************************************************
* Defines a comparison operator for the modifier category list.
* This ensures an alphabetical ordering.
******************************************************************************/
bool ModifierStack::modifierCategoryOrdering(const ModifierStack::ModifierCategory& a, const ModifierStack::ModifierCategory& b)
{
    return QString::compare(a.label, b.label, Qt::CaseInsensitive) < 0;
}

/******************************************************************************
* Gathers all defined modifier categories from the plugin manifests.
******************************************************************************/
void ModifierStack::loadModifierCategories()
{
	// Sort category list alphabetically.
	std::sort(_modifierCategories.begin(), _modifierCategories.end(), modifierCategoryOrdering);

	// Assign modifiers to categories.
	ModifierCategory nullCategory;
	nullCategory.label = tr("Others");
	Q_FOREACH(const OvitoObjectType* clazz, page->modifierClasses) {
		nullCategory.modifierClasses.push_back(clazz);
	}
	if(nullCategory.modifierClasses.empty() == false)
		_modifierCategories.push_back(nullCategory);

	// Sort modifier sub-lists alphabetically.
	for(int i=0; i<_modifierCategories.size(); i++) {
		std::sort(_modifierCategories[i].modifierClasses.begin(), _modifierCategories[i].modifierClasses.end(), modifierOrdering);
	}
}

/******************************************************************************
* Updates the list box of modifier classes that can be applied to the current selected
* item in the modifier stack.
******************************************************************************/
void ModifierStack::updateAvailableModifiers(ModifierStackEntry* currentEntry)
{
	page->modifierSelector->clear();
	page->modifierSelector->addItem(tr("Modifier List"));
	page->modifierSelector->addItem("-------------");
	page->modifierSelector->setCurrentIndex(0);

	if(currentEntry == NULL) {
		if(selectedNodes.empty()) {
			// Empty node selection.
			page->modifierSelector->setEnabled(false);
			return;
		}
	}

	QFont categoryFont = page->modifierSelector->font();
	categoryFont.setBold(true);

	Q_FOREACH(const ModifierCategory& category, _modifierCategories) {
		page->modifierSelector->addItem(category.label);
		page->modifierSelector->setItemData(page->modifierSelector->count()-1, categoryFont, Qt::FontRole);
		Q_FOREACH(const OvitoObjectType* descriptor, category.modifierClasses) {
			page->modifierSelector->addItem("    " + descriptor->name(), qVariantFromValue((void*)descriptor));
		}
	}

    page->modifierSelector->setEnabled(true);
    page->modifierSelector->setMaxVisibleItems(page->modifierSelector->count());
}

/******************************************************************************
* Updates the state of the actions that can be invoked on the currently selected
* item in the modifier stack.
******************************************************************************/
void ModifierStack::updateAvailableActions(ModifierStackEntry* currentEntry)
{
	QAction* deleteModifierAction = ActionManager::instance().getAction(ACTION_MODIFIER_DELETE);
	QAction* moveModifierUpAction = ActionManager::instance().getAction(ACTION_MODIFIER_MOVE_UP);
	QAction* moveModifierDownAction = ActionManager::instance().getAction(ACTION_MODIFIER_MOVE_DOWN);
	QAction* toggleModifierStateAction = ActionManager::instance().getAction(ACTION_MODIFIER_TOGGLE_STATE);
	Modifier* modifier = currentEntry ? dynamic_object_cast<Modifier>(currentEntry->commonObject()) : nullptr;
	if(modifier) {
		deleteModifierAction->setEnabled(true);
		if(currentEntry->modifierApplications().size() == 1) {
			ModifierApplication* modApp = currentEntry->modifierApplications()[0];
			PipelineObject* modObj = modApp->pipelineObject();
			if(modObj) {
				OVITO_ASSERT(modObj->modifierApplications().contains(modApp));
				moveModifierUpAction->setEnabled(modApp != modObj->modifierApplications().back());
				moveModifierDownAction->setEnabled(modApp != modObj->modifierApplications().front());
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
* Inserts the given modifier into the modification stack of the selected scene nodes.
******************************************************************************/
void ModifierStack::applyModifier(Modifier* modifier)
{
	// Get the selected stack entry. The new modifier is inserted just behind it.
	ModifierStackEntry* selEntry = selectedEntry();

	// On the next stack update the new modifier should be selected.
	nextObjectToSelect = modifier;

	if(selEntry) {
		if(dynamic_object_cast<Modifier>(selEntry->commonObject())) {
			Q_FOREACH(ModifierApplication* modApp, selEntry->modifierApplications()) {
				PipelineObject* modObj = modApp->pipelineObject();
				OVITO_CHECK_OBJECT_POINTER(modObj);
				modObj->insertModifier(modifier, modObj->modifierApplications().indexOf(modApp)+1);
			}
			return;
		}
		else if(dynamic_object_cast<PipelineObject>(selEntry->commonObject())) {
			PipelineObject* modObj = static_object_cast<PipelineObject>(selEntry->commonObject());
			OVITO_CHECK_OBJECT_POINTER(modObj);
			modObj->insertModifier(modifier, 0);
			return;
		}
		else {
			OVITO_ASSERT(stackEntries.contains(selEntry));
			int index = stackEntries.indexOf(selEntry);
			if(index != 0) {
				selEntry = stackEntries[index-1];
				if(dynamic_object_cast<PipelineObject>(selEntry->commonObject())) {
					PipelineObject* modObj = static_object_cast<PipelineObject>(selEntry->commonObject());
					OVITO_CHECK_OBJECT_POINTER(modObj);
					modObj->insertModifier(modifier, 0);
					return;
				}
			}
		}
	}

	// Apply modifier to each node separately.
	Q_FOREACH(ObjectNode* objNode, selectedNodes.targets())
		objNode->applyModifier(modifier);
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool ModifierStack::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::ReferenceChanged) {
		ObjectNode* targetNode = dynamic_object_cast<ObjectNode>(source);
		if(targetNode) {
			OVITO_ASSERT(selectedNodes.contains(targetNode));
			invalidate();
		}
	}
	return RefMaker::referenceEvent(source, event);
}

/******************************************************************************
*  Updates the display of a single modifier stack entry.
******************************************************************************/
void ModifierStackModel::refreshStackEntry(ModifierStackEntry* entry)
{
	OVITO_CHECK_OBJECT_POINTER(entry);
	int i = _entries.indexOf(entry);
	if(i != -1) {
		dataChanged(index(i), index(i));

		// Also update available actions if the changed entry is selected.
		if(stack()->selectedEntry() == entry)
			stack()->updateAvailableActions(entry);
	}
}

/******************************************************************************
* Is called by the system when the animated status icon changed.
******************************************************************************/
void ModifierStackModel::iconFrameChanged()
{
	bool stopMovie = true;
	for(int i = 0; i < _entries.size(); i++) {
		if(getEntryStatus(_entries[i]) == Pending) {
			dataChanged(index(i), index(i), { Qt::DecorationRole });
			stopMovie = false;
		}
	}
	if(stopMovie)
		_statusPendingIcon.stop();
}

/******************************************************************************
* Returns the data for the QListView widget.
******************************************************************************/
QVariant ModifierStackModel::data(const QModelIndex& index, int role) const
{
	OVITO_ASSERT(index.row() >= 0 && index.row() < _entries.size());

	ModifierStackEntry* entry = _entries[index.row()];
	OVITO_CHECK_OBJECT_POINTER(entry);

	if(role == Qt::DisplayRole) {
		QString title;
		if(dynamic_object_cast<PipelineObject>(entry->commonObject())) {
			title = "---------------------";
		}
		else {
			if(entry->isSubObject())
				title = "   " + entry->commonObject()->objectTitle();
			else
				title = entry->commonObject()->objectTitle();
		}
		return title;
	}
	else if(role == Qt::UserRole) {
		return qVariantFromValue((void*)entry);
	}
	else if(role == Qt::DecorationRole) {
		switch(getEntryStatus(entry)) {
		case Enabled: return qVariantFromValue(_modifierEnabledIcon);
		case Disabled: return qVariantFromValue(_modifierDisabledIcon);
		case Info: return qVariantFromValue(_statusInfoIcon);
		case Warning: return qVariantFromValue(_statusWarningIcon);
		case Error: return qVariantFromValue(_statusErrorIcon);
		case Pending:
			const_cast<QMovie&>(_statusPendingIcon).start();
			return qVariantFromValue(_statusPendingIcon.currentImage());
		}
	}
	else if(role == Qt::ToolTipRole) {
		return getEntryToolTip(entry);
	}

	return QVariant();
}

/******************************************************************************
* Returns the status for a given entry of the modifier stack.
******************************************************************************/
ModifierStackModel::EntryStatus ModifierStackModel::getEntryStatus(ModifierStackEntry* entry) const
{
	Modifier* modifier = dynamic_object_cast<Modifier>(entry->commonObject());
	if(modifier) {
		if(!modifier->isEnabled()) {
			return Disabled;
		}
		else {
			ObjectStatus status;
			Q_FOREACH(ModifierApplication* modApp, entry->modifierApplications()) {
				status = modApp->status();
				if(status.type() == ObjectStatus::Error) break;
			}
			if(status.type() == ObjectStatus::Success) {
				if(status.shortText().isEmpty())
					return Enabled;
				else
					return Info;
			}
			else if(status.type() == ObjectStatus::Warning)
				return Warning;
			else if(status.type() == ObjectStatus::Error)
				return Error;
			else
				return Enabled;
		}
	}
	else {
		SceneObject* sceneObject = dynamic_object_cast<SceneObject>(entry->commonObject());
		if(sceneObject) {
			ObjectStatus status = sceneObject->status();
			if(status.type() == ObjectStatus::Warning)
				return Warning;
			else if(status.type() == ObjectStatus::Error)
				return Error;
			else if(status.type() == ObjectStatus::Pending)
				return Pending;
		}
	}
	return None;
}

/******************************************************************************
* Returns the tooltip text for a given entry of the modifier stack.
******************************************************************************/
QVariant ModifierStackModel::getEntryToolTip(ModifierStackEntry* entry) const
{
	Modifier* modifier = dynamic_object_cast<Modifier>(entry->commonObject());
	if(modifier && modifier->isEnabled()) {
		ObjectStatus status;
		Q_FOREACH(ModifierApplication* modApp, entry->modifierApplications()) {
			status = modApp->status();
			if(status.type() == ObjectStatus::Error) break;
		}
		if(status.shortText().isEmpty() == false)
			return qVariantFromValue(status.shortText());
	}
	return QVariant();
}


};
