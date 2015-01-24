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

#ifndef __OVITO_REFTARGET_LIST_PARAMETER_UI_H
#define __OVITO_REFTARGET_LIST_PARAMETER_UI_H

#include <core/Core.h>
#include "ParameterUI.h"
#include "PropertiesEditor.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* A list view that shows the RefTarget items contained in a vector reference field.
******************************************************************************/
class OVITO_CORE_EXPORT RefTargetListParameterUI : public ParameterUI
{
public:

	/// Constructor.
	RefTargetListParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& refField,
			const RolloutInsertionParameters& rolloutParams = RolloutInsertionParameters(), const OvitoObjectType* defaultEditorClass = nullptr);
	
	/// Destructor.
	virtual ~RefTargetListParameterUI();
	
	/// The reference field that specifies the vector reference field of the edited object that
	/// is bound to this parameter UI.
	const PropertyFieldDescriptor& referenceField() const { return _refField; }
	
	/// This returns the list view managed by this ParameterUI.
	QListView* listWidget(int listWidgetHeight = 92);

	/// This returns the table view managed by this ParameterUI.
	QTableView* tableWidget(int tableWidgetHeight = 92);
	
	/// Returns the RefTarget that is currently selected in the UI.
	RefTarget* selectedObject() const;
	
	/// Selects the given sub-object in the list.
	int setSelectedObject(RefTarget* selObj);

	/// This method is called when a new editable object has been assigned to the properties owner this
	/// parameter UI belongs to. The parameter UI should react to this change appropriately and
	/// show the properties value for the new edit object in the UI.
	virtual void resetUI() override;

	/// Returns the current sub-editor for the selected sub-object or NULL if there is none.
	PropertiesEditor* subEditor() const { return _subEditor; }

	/// Informs the parameter UI that the given columns of all items have changed.
	void updateColumns(int columnStartIndex, int columnEndIndex) { _model->updateColumns(columnStartIndex, columnEndIndex); }

	/// Returns the internal model used to populate the list view or table view widget.
	QAbstractTableModel* model() const { return _model; }

public:
	
	Q_PROPERTY(RefTarget selectedObject READ selectedObject);
	
protected:

	class ListViewModel : public QAbstractTableModel {
	public:

		/// Constructor that takes a pointer to the owning parameter UI object.
		ListViewModel(RefTargetListParameterUI* owner) : QAbstractTableModel(owner) {}

		/// Returns the parameter UI that owns this table model.
		RefTargetListParameterUI* owner() const { return reinterpret_cast<RefTargetListParameterUI*>(QObject::parent()); }

		/// Returns the number of rows in the model.
		virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override { return owner()->_rowToTarget.size(); }
		
		/// Returns the data stored under the given role for the item referred to by the index.
		virtual QVariant data(const QModelIndex &index, int role) const override;
		
		/// Returns the data for the given role and section in the header with the specified orientation.
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

		/// Returns the item flags for the given index.
		virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

		/// Sets the role data for the item at index to value.
		virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;

		/// Notifies the system that the given item has changed and the display needs to be updated.
		void updateItem(int itemIndex) {
			// Update all columns of that item.
			dataChanged(index(itemIndex, 0), index(itemIndex, columnCount() - 1));
		}

		/// Notifies the system that the given columns of all items have changed and the display needs to be updated.
		void updateColumns(int columnStartIndex, int columnEndIndex) {
			// Update the columns of all items.
			dataChanged(index(0, columnStartIndex), index(rowCount() - 1, columnEndIndex));
		}

		/// Returns the number of columns of the table model. Default is 1.
		int columnCount(const QModelIndex& parent = QModelIndex()) const { return owner()->tableColumnCount(); }

		/// Updates the entire list model.
		void resetList() { beginResetModel(); endResetModel(); }
		
		void beginInsert(int atIndex) { beginInsertRows(QModelIndex(), atIndex, atIndex); }
		void endInsert() { endInsertRows(); }

		void beginRemove(int atIndex) { beginRemoveRows(QModelIndex(), atIndex, atIndex); }
		void endRemove() { endRemoveRows(); }
	};
	
protected Q_SLOTS:

	/// Is called when the user has selected an item in the list/table view.
	void onSelectionChanged();
	
protected:

	/// This method is called when a reference target changes. 
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Returns the data stored under the given role for the given RefTarget.
	/// This method is part of the data model used by the list widget and can be overriden
	/// by sub-classes. The default implementation returns the title of the RefTarget 
	/// for the Qt::DisplayRole.
	virtual QVariant getItemData(RefTarget* target, const QModelIndex& index, int role);

	/// Sets the role data for the item at index to value.
	virtual bool setItemData(RefTarget* target, const QModelIndex& index, const QVariant& value, int role) { return false; }

	/// Returns the model/view item flags for the given entry.
	virtual Qt::ItemFlags getItemFlags(RefTarget* target, const QModelIndex& index) { return Qt::ItemFlags(Qt::ItemIsSelectable) | Qt::ItemIsEnabled; }

	/// Returns the number of columns for the table view. The default is 1.
	virtual int tableColumnCount() { return 1; }

	/// Returns the header data under the given role for the given RefTarget.
	/// This method is part of the data model used by the list widget and can be overriden
	/// by sub-classes.
	virtual QVariant getHorizontalHeaderData(int index, int role);

	/// Returns the header data under the given role for the given RefTarget.
	/// This method is part of the data model used by the list widget and can be overriden
	/// by sub-classes.
	virtual QVariant getVerticalHeaderData(RefTarget* target, int index, int role);
	
	/// Opens a sub-editor for the object that is selected in the list view.
	virtual void openSubEditor();

	/// The reference field that specifies the parameter of the edited object that
	/// is bound to this parameter UI.
	const PropertyFieldDescriptor& _refField;
	
	/// The view widget.
	QPointer<QAbstractItemView> _viewWidget;
	
	/// This editor type is show if no entry is selected in the list box.
	const OvitoObjectType* _defaultEditorClass;

	/// The internal model used for the list view widget.
	ListViewModel* _model;

	/// The list of items in the list view.
	VectorReferenceField<RefTarget> _targets;

	/// Maps reference field indices to row indices.
	QVector<int> _targetToRow;

	/// Maps row indices to reference field indices.
	QVector<int> _rowToTarget;
	
	/// The editor for the selected sub-object.
	OORef<PropertiesEditor> _subEditor;

	/// Controls where the sub-editor is opened and whether the sub-editor is opened in a collapsed state.
	RolloutInsertionParameters _rolloutParams;
	
private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_targets);
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_REFTARGET_LIST_PARAMETER_UI_H
