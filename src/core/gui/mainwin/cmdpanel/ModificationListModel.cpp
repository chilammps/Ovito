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
#include <core/scene/objects/DataObject.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/SelectionSet.h>
#include <core/gui/actions/ActionManager.h>
#include <core/dataset/DataSetContainer.h>
#include "ModificationListModel.h"
#include "ModifyCommandPage.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Constructor.
******************************************************************************/
ModificationListModel::ModificationListModel(DataSetContainer& datasetContainer, QObject* parent) : QAbstractListModel(parent),
	_datasetContainer(datasetContainer),
	_nextToSelectObject(nullptr), _needListUpdate(false),
	_statusInfoIcon(":/core/mainwin/status/status_info.png"),
	_statusWarningIcon(":/core/mainwin/status/status_warning.png"),
	_statusErrorIcon(":/core/mainwin/status/status_error.png"),
	_statusNoneIcon(":/core/mainwin/status/status_none.png"),
	_statusPendingIcon(":/core/mainwin/status/status_pending.gif"),
	_sectionHeaderFont(QGuiApplication::font())
{
	connect(&_statusPendingIcon, &QMovie::frameChanged, this, &ModificationListModel::iconAnimationFrameChanged);
	_selectionModel = new QItemSelectionModel(this);
	connect(_selectionModel, &QItemSelectionModel::selectionChanged, this, &ModificationListModel::selectedItemChanged);
	connect(&_selectedNodes, &VectorRefTargetListener<ObjectNode>::notificationEvent, this, &ModificationListModel::onNodeEvent);
	if(_sectionHeaderFont.pixelSize() < 0)
		_sectionHeaderFont.setPointSize(_sectionHeaderFont.pointSize() * 4 / 5);
	else
		_sectionHeaderFont.setPixelSize(_sectionHeaderFont.pixelSize() * 4 / 5);
}

/******************************************************************************
* Populates the model with the given list items.
******************************************************************************/
void ModificationListModel::setItems(const QList<OORef<ModificationListItem>>& newItems, const QList<OORef<ModificationListItem>>& newHiddenItems)
{
	beginResetModel();
	_items = newItems;
	_hiddenItems = newHiddenItems;
	for(ModificationListItem* item : _items) {
		connect(item, &ModificationListItem::itemChanged, this, &ModificationListModel::refreshItem);
		connect(item, &ModificationListItem::subitemsChanged, this, &ModificationListModel::requestUpdate);
	}
	for(ModificationListItem* item : _hiddenItems) {
		connect(item, &ModificationListItem::itemChanged, this, &ModificationListModel::refreshItem);
		connect(item, &ModificationListItem::subitemsChanged, this, &ModificationListModel::requestUpdate);
	}
	endResetModel();
}

/******************************************************************************
* Returns the currently selected item in the modification list.
******************************************************************************/
ModificationListItem* ModificationListModel::selectedItem() const
{
	QModelIndexList selection = _selectionModel->selectedRows();
	if(selection.empty())
		return nullptr;
	else
		return item(selection.front().row());
}

/******************************************************************************
* Completely rebuilds the modifier list.
******************************************************************************/
void ModificationListModel::refreshList()
{
	_needListUpdate = false;

	// Determine the currently selected object and
	// select it again after the list has been rebuilt (and it is still there).
	// If _currentObject is already non-NULL then the caller
	// has specified an object to be selected.
	if(_nextToSelectObject == nullptr) {
		ModificationListItem* item = selectedItem();
		if(item)
			_nextToSelectObject = item->object();
	}
	RefTarget* defaultObjectToSelect = nullptr;

	// Collect all selected ObjectNodes.
	// Also check if all selected object nodes reference the same data object.
	_selectedNodes.clear();
    DataObject* cmnObject = nullptr;

    if(_datasetContainer.currentSet()) {
		for(SceneNode* node : _datasetContainer.currentSet()->selection()->nodes()) {
			if(ObjectNode* objNode = dynamic_object_cast<ObjectNode>(node)) {
				_selectedNodes.push_back(objNode);

				if(cmnObject == nullptr) cmnObject = objNode->dataProvider();
				else if(cmnObject != objNode->dataProvider()) {
					cmnObject = nullptr;
					break;	// The scene nodes are not compatible.
				}
			}
		}
    }

	QList<OORef<ModificationListItem>> items;
	QList<OORef<ModificationListItem>> hiddenItems;
	if(cmnObject) {

		// Create list items for display objects.
		for(ObjectNode* objNode : _selectedNodes.targets()) {
			for(DisplayObject* displayObj : objNode->displayObjects())
				items.push_back(new ModificationListItem(displayObj));
		}
		if(!items.empty())
			items.push_front(new ModificationListItem(nullptr, false, tr("Display")));

		// Walk up the pipeline.
		do {
			OVITO_CHECK_OBJECT_POINTER(cmnObject);

			// Create entries for the modifier applications if this is a PipelineObject.
			PipelineObject* modObj = dynamic_object_cast<PipelineObject>(cmnObject);
			if(modObj) {

				if(!modObj->modifierApplications().empty())
					items.push_back(new ModificationListItem(nullptr, false, tr("Modifications")));

				hiddenItems.push_back(new ModificationListItem(modObj));

				for(int i = modObj->modifierApplications().size(); i--; ) {
					ModifierApplication* app = modObj->modifierApplications()[i];
					ModificationListItem* item = new ModificationListItem(app->modifier());
					item->setModifierApplications({1, app});
					items.push_back(item);

					// Create list items for the modifier's editable sub-objects.
					for(int j = 0; j < app->modifier()->editableSubObjectCount(); j++) {
						RefTarget* subobject = app->modifier()->editableSubObject(j);
						if(subobject != NULL && subobject->isSubObjectEditable()) {
							items.push_back(new ModificationListItem(subobject, true));
						}
					}
				}

				cmnObject = modObj->sourceObject();
			}
			else {
				items.push_back(new ModificationListItem(nullptr, false, tr("Input")));

				// Create an entry for the data object.
				items.push_back(new ModificationListItem(cmnObject));
				if(defaultObjectToSelect == nullptr)
					defaultObjectToSelect = cmnObject;

				// Create list items for the object's editable sub-objects.
				for(int i = 0; i < cmnObject->editableSubObjectCount(); i++) {
					RefTarget* subobject = cmnObject->editableSubObject(i);
					if(subobject != NULL && subobject->isSubObjectEditable()) {
						items.push_back(new ModificationListItem(subobject, true));
					}
				}

				break;
			}
		}
		while(cmnObject != nullptr);
	}

	int selIndex = -1;
	int selDefaultIndex = -1;
	for(int i = 0; i < items.size(); i++) {
		if(_nextToSelectObject && _nextToSelectObject == items[i]->object())
			selIndex = i;
		if(defaultObjectToSelect && defaultObjectToSelect == items[i]->object())
			selDefaultIndex = i;
	}
	if(selIndex == -1)
		selIndex = selDefaultIndex;

	setItems(items, hiddenItems);
	_nextToSelectObject = nullptr;

	// Select the proper item in the list box.
	if(!items.empty()) {
		if(selIndex == -1) {
			for(int index = 0; index < items.size(); index++) {
				if(items[index]->object()) {
					selIndex = index;
					break;
				}
			}
		}
		_selectionModel->select(index(selIndex), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Clear);
	}
	else {
		Q_EMIT selectedItemChanged();
	}
}

/******************************************************************************
* Handles notification events generated by the selected object nodes.
******************************************************************************/
void ModificationListModel::onNodeEvent(RefTarget* source, ReferenceEvent* event)
{
	// Update the entire modification list if the ObjectNode has been assigned a new
	// data object, or if the list of display objects has changed.
	if(event->type() == ReferenceEvent::ReferenceChanged || event->type() == ReferenceEvent::ReferenceAdded || event->type() == ReferenceEvent::ReferenceRemoved) {
		requestUpdate();
	}
}

/******************************************************************************
* Updates the appearance of a single list item.
******************************************************************************/
void ModificationListModel::refreshItem(ModificationListItem* item)
{
	OVITO_CHECK_OBJECT_POINTER(item);
	int i = _items.indexOf(item);
	if(i != -1) {
		Q_EMIT dataChanged(index(i), index(i));

		// Also update available actions if the changed item is currently selected.
		if(selectedItem() == item)
			Q_EMIT selectedItemChanged();
	}
}

/******************************************************************************
* The the current modification stack contains a hidden pipeline object
* at the top, this function returns it.
******************************************************************************/
PipelineObject* ModificationListModel::hiddenPipelineObject()
{
	for(int i = _hiddenItems.size() - 1; i >= 0; i--) {
		PipelineObject* pipelineObj = dynamic_object_cast<PipelineObject>(_hiddenItems[i]->object());
		if(pipelineObj) {
			OVITO_CHECK_OBJECT_POINTER(pipelineObj);
			return pipelineObj;
		}
	}
	return nullptr;
}

/******************************************************************************
* Inserts the given modifier into the modification pipeline of the
* selected scene nodes.
******************************************************************************/
void ModificationListModel::applyModifier(Modifier* modifier)
{
	// Get the selected stack entry. The new modifier is inserted just behind it.
	ModificationListItem* currentItem = selectedItem();

	// On the next list update, the new modifier should be selected.
	_nextToSelectObject = modifier;

	if(currentItem) {
		if(dynamic_object_cast<Modifier>(currentItem->object())) {
			for(ModifierApplication* modApp : currentItem->modifierApplications()) {
				PipelineObject* pipelineObj = modApp->pipelineObject();
				OVITO_CHECK_OBJECT_POINTER(pipelineObj);
				pipelineObj->insertModifier(modifier, pipelineObj->modifierApplications().indexOf(modApp) + 1);
			}
			return;
		}
		else if(dynamic_object_cast<PipelineObject>(currentItem->object())) {
			PipelineObject* pipelineObj = static_object_cast<PipelineObject>(currentItem->object());
			OVITO_CHECK_OBJECT_POINTER(pipelineObj);
			pipelineObj->insertModifier(modifier, 0);
			return;
		}
		else if(dynamic_object_cast<DataObject>(currentItem->object())) {
			if(PipelineObject* pipelineObj = hiddenPipelineObject()) {
				pipelineObj->insertModifier(modifier, 0);
				return;
			}
		}
	}

	// Apply modifier to each selected node.
	for(ObjectNode* objNode : selectedNodes()) {
		objNode->applyModifier(modifier);
	}
}

/******************************************************************************
* Is called by the system when the animated status icon changed.
******************************************************************************/
void ModificationListModel::iconAnimationFrameChanged()
{
	bool stopMovie = true;
	for(int i = 0; i < _items.size(); i++) {
		if(_items[i]->status() == ModificationListItem::Pending) {
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
QVariant ModificationListModel::data(const QModelIndex& index, int role) const
{
	OVITO_ASSERT(index.row() >= 0 && index.row() < _items.size());

	ModificationListItem* item = this->item(index.row());

	if(role == Qt::DisplayRole) {
		if(item->object()) {
			if(item->isSubObject())
#ifdef Q_OS_LINUX
			return QStringLiteral("  ⇾ ") + item->object()->objectTitle();
#else
			return QStringLiteral("    ") + item->object()->objectTitle();
#endif
			else
				return item->object()->objectTitle();
		}
		else return item->title();
	}
	else if(role == Qt::DecorationRole) {
		if(item->object()) {
			switch(item->status()) {
			case ModificationListItem::Info: return qVariantFromValue(_statusInfoIcon);
			case ModificationListItem::Warning: return qVariantFromValue(_statusWarningIcon);
			case ModificationListItem::Error: return qVariantFromValue(_statusErrorIcon);
			case ModificationListItem::Pending:
				const_cast<QMovie&>(_statusPendingIcon).start();
				return qVariantFromValue(_statusPendingIcon.currentPixmap());
			case ModificationListItem::None: return qVariantFromValue(_statusNoneIcon);
			default: OVITO_ASSERT(false);
			}
		}
	}
	else if(role == Qt::CheckStateRole) {
		DisplayObject* displayObj = dynamic_object_cast<DisplayObject>(item->object());
		if(displayObj)
			return displayObj->isEnabled() ? Qt::Checked : Qt::Unchecked;
		Modifier* modifier = dynamic_object_cast<Modifier>(item->object());
		if(modifier)
			return modifier->isEnabled() ? Qt::Checked : Qt::Unchecked;
	}
	else if(role == Qt::TextAlignmentRole) {
		if(item->object() == nullptr) {
			return Qt::AlignCenter;
		}
	}
	else if(role == Qt::BackgroundRole) {
		if(item->object() == nullptr) {
			return QBrush(Qt::lightGray, Qt::Dense4Pattern);
		}
	}
	else if(role == Qt::ForegroundRole) {
		if(item->object() == nullptr) {
			return QBrush(Qt::blue);
		}
	}
	else if(role == Qt::FontRole) {
		if(item->object() == nullptr) {
			return _sectionHeaderFont;
		}
	}

	return QVariant();
}

/******************************************************************************
* Changes the data associated with a list entry.
******************************************************************************/
bool ModificationListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(role == Qt::CheckStateRole) {
		ModificationListItem* item = this->item(index.row());
		DisplayObject* displayObj = dynamic_object_cast<DisplayObject>(item->object());
		if(displayObj) {
			UndoableTransaction::handleExceptions(_datasetContainer.currentSet()->undoStack(),
					(value == Qt::Checked) ? tr("Enable display") : tr("Disable display"), [displayObj, &value]() {
				displayObj->setEnabled(value == Qt::Checked);
			});
		}
		else {
			Modifier* modifier = dynamic_object_cast<Modifier>(item->object());
			if(modifier) {
				UndoableTransaction::handleExceptions(_datasetContainer.currentSet()->undoStack(),
						(value == Qt::Checked) ? tr("Enable modifier") : tr("Disable modifier"), [modifier, &value]() {
					modifier->setEnabled(value == Qt::Checked);
				});
			}
		}
	}
	return QAbstractListModel::setData(index, value, role);
}

/******************************************************************************
* Returns the flags for an item.
******************************************************************************/
Qt::ItemFlags ModificationListModel::flags(const QModelIndex& index) const
{
	OVITO_ASSERT(index.row() >= 0 && index.row() < _items.size());
	ModificationListItem* item = this->item(index.row());
	if(item->object() == nullptr) {
		return Qt::NoItemFlags;
	}
	else {
		if(dynamic_object_cast<DisplayObject>(item->object()) || dynamic_object_cast<Modifier>(item->object())) {
			return QAbstractListModel::flags(index) | Qt::ItemIsUserCheckable;
		}
	}
	return QAbstractListModel::flags(index);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
