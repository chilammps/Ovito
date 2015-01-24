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
#include <core/gui/properties/RefTargetListParameterUI.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

// Gives the class run-time type information.
IMPLEMENT_OVITO_OBJECT(Core, RefTargetListParameterUI, ParameterUI);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(RefTargetListParameterUI, _targets, "Targets", RefTarget, PROPERTY_FIELD_NO_UNDO | PROPERTY_FIELD_WEAK_REF | PROPERTY_FIELD_NO_CHANGE_MESSAGE);

/******************************************************************************
* The constructor.
******************************************************************************/
RefTargetListParameterUI::RefTargetListParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& refField, const RolloutInsertionParameters& rolloutParams, const OvitoObjectType* defaultEditorClass)
	: ParameterUI(parentEditor), _refField(refField), _rolloutParams(rolloutParams), _defaultEditorClass(defaultEditorClass)
{
	INIT_PROPERTY_FIELD(RefTargetListParameterUI::_targets);
	OVITO_ASSERT_MSG(refField.isVector(), "RefTargetListParameterUI constructor", "The reference field bound to this parameter UI must be a vector reference field.");
	
	_model = new ListViewModel(this);

	if(_defaultEditorClass)
		openSubEditor();
}

/******************************************************************************
* Destructor.
******************************************************************************/
RefTargetListParameterUI::~RefTargetListParameterUI()
{
	_subEditor = nullptr;
	clearAllReferences();
	 	
	// Release GUI controls. 
	delete _viewWidget;
}

/******************************************************************************
* Returns the list view managed by this ParameterUI.
******************************************************************************/
QListView* RefTargetListParameterUI::listWidget(int listWidgetHeight)
{
	OVITO_ASSERT(!_viewWidget || qobject_cast<QListView*>(_viewWidget));
	if(!_viewWidget) {
		class MyListView : public QListView {
		private:
			int _listWidgetHeight;
		public:
			MyListView(int listWidgetHeight) : QListView(), _listWidgetHeight(listWidgetHeight) {}
			virtual QSize sizeHint() const { return QSize(320, _listWidgetHeight); }
		};

		_viewWidget = new MyListView(listWidgetHeight);
		_viewWidget->setModel(_model);
		connect(_viewWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RefTargetListParameterUI::onSelectionChanged);
	}
	return qobject_cast<QListView*>(_viewWidget);
}

/******************************************************************************
* Returns the table view managed by this ParameterUI.
******************************************************************************/
QTableView* RefTargetListParameterUI::tableWidget(int tableWidgetHeight)
{
	OVITO_ASSERT(!_viewWidget || qobject_cast<QTableView*>(_viewWidget));
	if(!_viewWidget) {
		class MyTableView : public QTableView {
		private:
			int _tableWidgetHeight;
		public:
			MyTableView(int tableWidgetHeight) : QTableView(), _tableWidgetHeight(tableWidgetHeight) {}
			virtual QSize sizeHint() const override { return QSize(320, _tableWidgetHeight); }
		};
		MyTableView* tableView = new MyTableView(tableWidgetHeight);
		tableView->setShowGrid(false);
		tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
		tableView->setCornerButtonEnabled(false);
		tableView->verticalHeader()->hide();
		tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
		tableView->setSelectionMode(QAbstractItemView::SingleSelection);
		tableView->setWordWrap(false);
		tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

		_viewWidget = tableView;
		_viewWidget->setModel(_model);
		connect(_viewWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RefTargetListParameterUI::onSelectionChanged);
	}
	return qobject_cast<QTableView*>(_viewWidget);
}

/******************************************************************************
* This method is called when a new editable object has been assigned to the properties owner this
* parameter UI belongs to. The parameter UI should react to this change appropriately and
* show the properties value for the new edit object in the UI.
******************************************************************************/
void RefTargetListParameterUI::resetUI()
{
	ParameterUI::resetUI();	
	
	if(_viewWidget) {
		_viewWidget->setEnabled(editObject() != nullptr);
	
		_targets.clear();
		_targetToRow.clear();
		_rowToTarget.clear();
		
		if(editObject()) {
			// Create a local copy of the list of ref targets.
			const QVector<RefTarget*>& reflist = editObject()->getVectorReferenceField(referenceField());
			Q_FOREACH(RefTarget* t, reflist) {
				_targetToRow.push_back(_rowToTarget.size());
				if(t != nullptr)
					_rowToTarget.push_back(_targets.size());
				_targets.push_back(t);
			}
		}
		
		_model->resetList();

		if(editObject() && _targets.size() > 0)
			setSelectedObject(_targets.targets().front());
	}
	openSubEditor();
}

/******************************************************************************
* Is called when the user has selected an item in the list/table view.
******************************************************************************/
void RefTargetListParameterUI::onSelectionChanged()
{
	openSubEditor();
}

/******************************************************************************
* Opens a sub-editor for the object that is selected in the list view.
******************************************************************************/
void RefTargetListParameterUI::openSubEditor()
{
	try {
		RefTarget* selection = selectedObject();

		if(subEditor()) {
			// Close old editor if it is no longer needed.
			if(!selection || subEditor()->editObject() == nullptr ||
					subEditor()->editObject()->getOOType() != selection->getOOType()) {

				if(selection || &subEditor()->getOOType() != _defaultEditorClass)
					_subEditor = nullptr;
			}
		}
		if(!subEditor()) {
			if(selection) {
				_subEditor = selection->createPropertiesEditor();
				if(_subEditor)
					_subEditor->initialize(editor()->container(), editor()->mainWindow(), _rolloutParams);
			}
			else if(_defaultEditorClass) {
				_subEditor = dynamic_object_cast<PropertiesEditor>(_defaultEditorClass->createInstance(nullptr));
				if(_subEditor)
					_subEditor->initialize(editor()->container(), editor()->mainWindow(), _rolloutParams);
			}
			else return;
		}
		if(subEditor()) {
			subEditor()->setEditObject(selection);
		}
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Returns the RefTarget that is currently selected in the UI.
******************************************************************************/
RefTarget* RefTargetListParameterUI::selectedObject() const
{
	if(!_viewWidget) return nullptr;
	QModelIndexList selection = _viewWidget->selectionModel()->selectedRows();
	if(selection.empty()) return nullptr;
	if(selection.front().row() >= _rowToTarget.size()) return nullptr;
	int targetIndex = _rowToTarget[selection.front().row()];
	OVITO_ASSERT(targetIndex < _targets.size());
	OVITO_CHECK_OBJECT_POINTER(_targets[targetIndex]);
	return _targets[targetIndex];
}

/******************************************************************************
* Selects the given sub-object in the list.
******************************************************************************/
int RefTargetListParameterUI::setSelectedObject(RefTarget* selObj)
{
	if(!_viewWidget) return -1;
	OVITO_ASSERT(_targetToRow.size() == _targets.size());
	if(selObj != nullptr) {
		for(int i = 0; i<_targets.size(); i++) {
			if(_targets[i] == selObj) {
				int rowIndex = _targetToRow[i];
				_viewWidget->selectionModel()->select(_model->index(rowIndex, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
				return rowIndex;
			}
		}
	}
	_viewWidget->selectionModel()->clear();
	return -1;
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool RefTargetListParameterUI::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject()) {
		if(event->type() == ReferenceEvent::ReferenceAdded) {
			ReferenceFieldEvent* refevent = static_cast<ReferenceFieldEvent*>(event);
			if(refevent->field() == referenceField()) {
				int rowIndex;
				if(refevent->index() < _targetToRow.size())
					rowIndex = _targetToRow[refevent->index()];
				else
					rowIndex = _rowToTarget.size();
				if(refevent->newTarget() != nullptr)
					_model->beginInsert(rowIndex);
				_targets.insert(refevent->index(), refevent->newTarget());
				_targetToRow.insert(refevent->index(), rowIndex);
				for(int i=rowIndex; i<_rowToTarget.size(); i++)
					_rowToTarget[i]++;
				if(refevent->newTarget() != NULL) {
					_rowToTarget.insert(rowIndex, refevent->index());
					for(int i=refevent->index()+1; i<_targetToRow.size(); i++)
						_targetToRow[i]++;
					_model->endInsert();
				}
#ifdef OVITO_DEBUG
				// Check internal list structures.
				int numRows = 0;
				int numTargets = 0;
				const QVector<RefTarget*>& reflist = editObject()->getVectorReferenceField(referenceField());
				Q_FOREACH(RefTarget* t, reflist) {
					OVITO_ASSERT(_targets[numTargets] == t);
					OVITO_ASSERT(_targetToRow[numTargets] == numRows);
					if(t != nullptr) {
						OVITO_ASSERT(_rowToTarget[numRows] == numTargets);
						numRows++;
					}
					numTargets++;
				}
#endif
			}
		}
		else if(event->type() == ReferenceEvent::ReferenceRemoved) {
			ReferenceFieldEvent* refevent = static_cast<ReferenceFieldEvent*>(event);
			if(refevent->field() == referenceField()) {
				int rowIndex = _targetToRow[refevent->index()];
				if(refevent->oldTarget())
					_model->beginRemove(rowIndex);
				OVITO_ASSERT(refevent->oldTarget() == _targets[refevent->index()]);
				_targets.remove(refevent->index());
				_targetToRow.remove(refevent->index());
				for(int i=rowIndex; i<_rowToTarget.size(); i++)
					_rowToTarget[i]--;
				if(refevent->oldTarget()) {
					_rowToTarget.remove(rowIndex);
					for(int i=refevent->index(); i<_targetToRow.size(); i++)
						_targetToRow[i]--;
					_model->endRemove();
				}
#ifdef OVITO_DEBUG
				// Check internal list structures.
				int numRows = 0;
				int numTargets = 0;
				const QVector<RefTarget*>& reflist = editObject()->getVectorReferenceField(referenceField());
				Q_FOREACH(RefTarget* t, reflist) {
					OVITO_ASSERT(_targets[numTargets] == t);
					OVITO_ASSERT(_targetToRow[numTargets] == numRows);
					if(t != NULL) {
						OVITO_ASSERT(_rowToTarget[numRows] == numTargets);
						numRows++;
					}
					numTargets++;
				}
#endif
			}
		}
	} 
	else if(event->type() == ReferenceEvent::TitleChanged || event->type() == ReferenceEvent::TargetChanged) {
		OVITO_ASSERT(_targetToRow.size() == _targets.size());
		for(int i = 0; i < _targets.size(); i++) {
			if(_targets[i] == source) {
				// Update a single item.
				_model->updateItem(_targetToRow[i]);
			}
		}
	}
	return ParameterUI::referenceEvent(source, event);
}

/******************************************************************************
* Returns the data stored under the given role for the given RefTarget.
* Calls the RefTargetListParameterUI::getItemData() virtual method to obtain
* the display data.
******************************************************************************/
QVariant RefTargetListParameterUI::ListViewModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
		return QVariant();

	if(index.row() >= owner()->_rowToTarget.size())
		return QVariant();

	int targetIndex = owner()->_rowToTarget[index.row()];
	OVITO_ASSERT(targetIndex < owner()->_targets.size());

	RefTarget* t = owner()->_targets[targetIndex];
	return owner()->getItemData(t, index, role);
}

/******************************************************************************
* Returns the header data under the given role for the given RefTarget.
* Calls the RefTargetListParameterUI::getHeaderData() virtual method to obtain
* the data from the parameter UI.
******************************************************************************/
QVariant RefTargetListParameterUI::ListViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Vertical) {

		if(section >= owner()->_rowToTarget.size())
			return QVariant();

		int targetIndex = owner()->_rowToTarget[section];
		OVITO_ASSERT(targetIndex < owner()->_targets.size());

		RefTarget* t = owner()->_targets[targetIndex];
		return owner()->getVerticalHeaderData(t, section, role);
	}
	else {
		return owner()->getHorizontalHeaderData(section, role);
	}
}

/******************************************************************************
* Returns the item flags for the given index.
******************************************************************************/
Qt::ItemFlags RefTargetListParameterUI::ListViewModel::flags(const QModelIndex& index) const
{
	if(!index.isValid() || index.row() >= owner()->_rowToTarget.size())
		return QAbstractItemModel::flags(index);

	int targetIndex = owner()->_rowToTarget[index.row()];
	OVITO_ASSERT(targetIndex < owner()->_targets.size());

	RefTarget* t = owner()->_targets[targetIndex];
	return owner()->getItemFlags(t, index);
}

/******************************************************************************
* Sets the role data for the item at index to value.
******************************************************************************/
bool RefTargetListParameterUI::ListViewModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(!index.isValid() || index.row() >= owner()->_rowToTarget.size())
		return QAbstractItemModel::setData(index, value, role);

	int targetIndex = owner()->_rowToTarget[index.row()];
	OVITO_ASSERT(targetIndex < owner()->_targets.size());

	RefTarget* t = owner()->_targets[targetIndex];
	return owner()->setItemData(t, index, value, role);
}

/******************************************************************************
* Returns the data stored under the given role for the given RefTarget.
******************************************************************************/
QVariant RefTargetListParameterUI::getItemData(RefTarget* target, const QModelIndex& index, int role)
{
	if(role == Qt::DisplayRole) {
		if(target == nullptr) return QVariant();
		return target->objectTitle();
	}
	else return QVariant();
}

/******************************************************************************
* Returns the header data under the given role.
******************************************************************************/
QVariant RefTargetListParameterUI::getVerticalHeaderData(RefTarget* target, int index, int role)
{
	if(role == Qt::DisplayRole) {
		return QVariant(index);
	}
	else return QVariant();
}

/******************************************************************************
* Returns the header data under the given role.
******************************************************************************/
QVariant RefTargetListParameterUI::getHorizontalHeaderData(int index, int role)
{
	if(role == Qt::DisplayRole) {
		return QVariant(index);
	}
	else return QVariant();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
