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

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/gui/undo/UndoManager.h>
#include <core/gui/app/Application.h>
#include "DislocationInspector.h"
#include "DislocationSegment.h"
#include "DislocationNetwork.h"

namespace CrystalAnalysis {

IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, DislocationInspector, PropertiesEditor);
DEFINE_FLAGS_REFERENCE_FIELD(DislocationInspector, _sceneNode, "SceneNode", ObjectNode, PROPERTY_FIELD_NO_UNDO);

/// List of column indices used by the table.
enum DislocationInspectorColumns {
	VISIBLE_COLUMN = 0,
	INDEX_COLUMN = 1,
	TRUE_BURGERS_VECTOR_COLUMN = 2,
	TRANSFORMED_BURGERS_VECTOR_COLUMN = 3,
	BURGERS_VECTOR_FAMILY_COLUMN = 4,
	CLUSTER_COLUMN = 5,
	LENGTH_COLUMN = 6
};

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void DislocationInspector::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Dislocation inspector"), rolloutParams);
	rollout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	QVBoxLayout* rolloutLayout = new QVBoxLayout(rollout);
	rolloutLayout->setContentsMargins(0,0,0,0);

	QToolBar* toolbar = new QToolBar(rollout);
	toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
	toolbar->addAction(tr("Hide unselected"), this, SLOT(onHideUnselected()));
	toolbar->addAction(tr("Show all"), this, SLOT(onShowAll()));
	for(QToolButton* button : toolbar->findChildren<QToolButton*>())
		button->setAutoRaise(false);
	rolloutLayout->addWidget(toolbar);

	class CustomRefTargetListParameterUI : public RefTargetListParameterUI {
	public:

		CustomRefTargetListParameterUI(PropertiesEditor* parentEditor, const PropertyFieldDescriptor& refField)
			: RefTargetListParameterUI(parentEditor, refField) {}

	protected:

		virtual QVariant getItemData(RefTarget* target, const QModelIndex& index, int role) override {
			DislocationSegment* segment = static_object_cast<DislocationSegment>(target);
			if((role == Qt::DisplayRole || role == Qt::UserRole) && segment != NULL) {
				if(index.column() == TRUE_BURGERS_VECTOR_COLUMN) {
					return DislocationSegment::formatBurgersVector(segment->burgersVector());
				}
				else if(index.column() == INDEX_COLUMN) {
					return (index.row() + 1);
				}
				else if(index.column() == TRANSFORMED_BURGERS_VECTOR_COLUMN) {
					Vector3 transformedVector = segment->cluster()->orientation() * segment->burgersVector();
					return QString("%1 %2 %3")
							.arg(QLocale::c().toString(transformedVector.x(), 'f', 4), 7)
							.arg(QLocale::c().toString(transformedVector.y(), 'f', 4), 7)
							.arg(QLocale::c().toString(transformedVector.z(), 'f', 4), 7);
				}
				else if(index.column() == BURGERS_VECTOR_FAMILY_COLUMN) {
					return segment->burgersVectorFamily()->name();
				}
				else if(index.column() == CLUSTER_COLUMN) {
					return QString("%1 (%2 atoms, id:%3)").arg(segment->cluster()->pattern()->shortName()).arg(segment->cluster()->atomCount()).arg(segment->cluster()->id());
				}
				else if(index.column() == LENGTH_COLUMN) {
					// Determine the segment length from the output of the modification pipeline and not from the input.
					DislocationInspector* inspector = static_object_cast<DislocationInspector>(editor());
					if(!inspector->_sceneNode)
						return tr("N/A");
					const PipelineFlowState& state = inspector->_sceneNode->evalPipeline(AnimManager::instance().time());
					DislocationNetwork* dislocations = state.findObject<DislocationNetwork>();
					if(!dislocations || dislocations->segments().size() != _model->rowCount())
						return tr("N/A");
					if(role == Qt::DisplayRole)
						return QLocale::c().toString(dislocations->segments()[index.row()]->length(), 'f', 3);
					else
						return dislocations->segments()[index.row()]->length();
				}
			}
			else if(role == Qt::EditRole && index.column() == CLUSTER_COLUMN && segment != NULL) {
				//SegmentCluster data = { segment, NULL, IDENTITY };
				//return QVariant::fromValue(data);
			}
			else if(role == Qt::CheckStateRole) {
				if(index.column() == VISIBLE_COLUMN) {
					return QVariant(segment->isVisible() ? Qt::Checked : Qt::Unchecked);
				}
			}
			return QVariant();
		}

		/// Sets the role data for the item at index to value.
		virtual bool setItemData(RefTarget* target, const QModelIndex& index, const QVariant& value, int role) override {
			if(index.isValid() && index.column() == CLUSTER_COLUMN) {
#if 0
				SegmentCluster data = qvariant_cast<SegmentCluster>(value);
				DislocationSegment* segment = data.segment;
				OVITO_ASSERT(segment == target);
				if(data.cluster != NULL) {
					UNDO_MANAGER.beginCompoundOperation(tr("Change dislocation cluster"));
					segment->setBurgersVector(data.transitionTM * segment->burgersVector(), data.cluster);
					UNDO_MANAGER.endCompoundOperation();
				}
				return true;
#endif
			}
			else if(index.isValid() && index.column() == VISIBLE_COLUMN) {
				UndoableTransaction::handleExceptions(tr("Show/hide dislocation segment"), [&value, target]() {
					static_object_cast<DislocationSegment>(target)->setVisible(value.value<int>() == Qt::Checked);
				});
				return true;
			}
			return false;
		}

		/// Returns the number of columns for the table view.
		virtual int tableColumnCount() override { return 7; }

		/// Returns the model/view item flags for the given entry.
		virtual Qt::ItemFlags getItemFlags(RefTarget* target, const QModelIndex& index) override {
			if(index.column() == VISIBLE_COLUMN)
				return RefTargetListParameterUI::getItemFlags(target, index) | Qt::ItemIsUserCheckable;
			//else if(index.column() == CLUSTER_COLUMN)
			//	return RefTargetListParameterUI::getItemFlags(target, index) | Qt::ItemIsEditable;
			else
				return RefTargetListParameterUI::getItemFlags(target, index);
		}

		/// Returns the header data under the given role for the given RefTarget.
		/// This method is part of the data model used by the list widget and can be overridden
		/// by sub-classes.
		virtual QVariant getHorizontalHeaderData(int index, int role) override {
			if(role != Qt::DisplayRole) return QVariant();
			if(index == TRUE_BURGERS_VECTOR_COLUMN) return QVariant(DislocationInspector::tr("True Burgers vector"));
			else if(index == TRANSFORMED_BURGERS_VECTOR_COLUMN) return QVariant(DislocationInspector::tr("Transformed Burgers vector"));
			else if(index == BURGERS_VECTOR_FAMILY_COLUMN) return QVariant(DislocationInspector::tr("Burgers vector family"));
			else if(index == CLUSTER_COLUMN) return QVariant(DislocationInspector::tr("Cluster"));
			else if(index == LENGTH_COLUMN) return QVariant(DislocationInspector::tr("Length"));
			else if(index == VISIBLE_COLUMN) return QVariant(DislocationInspector::tr("Vis."));
			else if(index == INDEX_COLUMN) return QVariant(DislocationInspector::tr("ID"));
			else return QVariant();
		}

		/// Do not open sub-editor for selected item.
		virtual void openSubEditor() override {}
	};

	_dislocationListUI = new CustomRefTargetListParameterUI(this, PROPERTY_FIELD(DislocationNetwork::_segments));
	rolloutLayout->addWidget(_dislocationListUI->tableWidget(300));
	_dislocationListUI->tableWidget()->setAutoScroll(true);
	_dislocationListUI->tableWidget()->setShowGrid(true);
	_dislocationListUI->tableWidget()->horizontalHeader()->setVisible(true);
	_dislocationListUI->tableWidget()->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	_dislocationListUI->tableWidget()->horizontalHeader()->setSectionResizeMode(VISIBLE_COLUMN, QHeaderView::ResizeToContents);
	_dislocationListUI->tableWidget()->horizontalHeader()->setSectionResizeMode(INDEX_COLUMN, QHeaderView::ResizeToContents);
	_dislocationListUI->tableWidget()->horizontalHeader()->resizeSection(TRUE_BURGERS_VECTOR_COLUMN, 150);
	_dislocationListUI->tableWidget()->horizontalHeader()->resizeSection(TRANSFORMED_BURGERS_VECTOR_COLUMN, 200);
	_dislocationListUI->tableWidget()->horizontalHeader()->resizeSection(BURGERS_VECTOR_FAMILY_COLUMN, 210);
	_dislocationListUI->tableWidget()->horizontalHeader()->resizeSection(CLUSTER_COLUMN, 200);
	_dislocationListUI->tableWidget()->horizontalHeader()->resizeSection(LENGTH_COLUMN, 80);
	//_dislocationListUI->tableWidget()->setItemDelegateForColumn(CLUSTER_COLUMN, new ClusterItemDelegate(this));
	_dislocationListUI->tableWidget()->setEditTriggers(QAbstractItemView::AllEditTriggers);
	_dislocationListUI->tableWidget()->setSelectionBehavior(QAbstractItemView::SelectRows);
	_dislocationListUI->tableWidget()->setSelectionMode(QAbstractItemView::ExtendedSelection);

	// Make the table sortable.
	_sortedModel = new QSortFilterProxyModel(_dislocationListUI->tableWidget());
	_sortedModel->setSourceModel(_dislocationListUI->tableWidget()->model());
	_sortedModel->setSortRole(Qt::UserRole);
	_sortedModel->setDynamicSortFilter(false);
	_dislocationListUI->tableWidget()->setModel(_sortedModel);
	_dislocationListUI->tableWidget()->setSortingEnabled(true);
	_dislocationListUI->tableWidget()->sortByColumn(INDEX_COLUMN, Qt::AscendingOrder);
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool DislocationInspector::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == _sceneNode && event->type() == ReferenceEvent::TargetChanged) {
		// Update display of segment lengths.
		Application::instance().runOnceLater(this, [this]() {
			_dislocationListUI->updateColumns(LENGTH_COLUMN, LENGTH_COLUMN);
		});
	}
	return PropertiesEditor::referenceEvent(source, event);
}

/******************************************************************************
* Hides all dislocation segments.
******************************************************************************/
void DislocationInspector::onHideAll()
{
	DislocationNetwork* dislocationsObj = static_object_cast<DislocationNetwork>(editObject());
	if(!dislocationsObj) return;

	UndoableTransaction::handleExceptions(tr("Hide all dislocations"), [dislocationsObj]() {
		for(DislocationSegment* segment : dislocationsObj->segments())
			segment->setVisible(false);
	});
}

/******************************************************************************
* Shows all dislocation segments.
******************************************************************************/
void DislocationInspector::onShowAll()
{
	DislocationNetwork* dislocationsObj = static_object_cast<DislocationNetwork>(editObject());
	if(!dislocationsObj) return;

	UndoableTransaction::handleExceptions(tr("Show all dislocations"), [dislocationsObj]() {
		for(DislocationSegment* segment : dislocationsObj->segments())
			segment->setVisible(true);
	});
}

/******************************************************************************
* Hides all selected dislocation segments.
******************************************************************************/
void DislocationInspector::onHideSelected()
{
	DislocationNetwork* dislocationsObj = static_object_cast<DislocationNetwork>(editObject());
	if(!dislocationsObj) return;

	QItemSelectionModel* selectionModel = _dislocationListUI->tableWidget()->selectionModel();
	UndoableTransaction::handleExceptions(tr("Hide selected dislocations"), [dislocationsObj, selectionModel]() {
		int row = 0;
		for(DislocationSegment* segment : dislocationsObj->segments()) {
			if(selectionModel->isRowSelected(row++, QModelIndex()))
				segment->setVisible(false);
		}
	});
}

/******************************************************************************
* Hides all unselected dislocation segments.
******************************************************************************/
void DislocationInspector::onHideUnselected()
{
	DislocationNetwork* dislocationsObj = static_object_cast<DislocationNetwork>(editObject());
	if(!dislocationsObj) return;

	UndoableTransaction::handleExceptions(tr("Hide unselected dislocations"), [this, dislocationsObj]() {
		QItemSelectionModel* selectionModel = _dislocationListUI->tableWidget()->selectionModel();
		QBitArray segmentSelection(dislocationsObj->segments().size());
		for(const QModelIndex& index : selectionModel->selectedRows())
			segmentSelection.setBit(_sortedModel->mapToSource(index).row());
		int index = 0;
		for(DislocationSegment* segment : dislocationsObj->segments()) {
			if(!segmentSelection.testBit(index++))
				segment->setVisible(false);
		}
	});
}

/******************************************************************************
* Shows all selected dislocation segments.
******************************************************************************/
void DislocationInspector::onShowSelected()
{
	DislocationNetwork* dislocationsObj = static_object_cast<DislocationNetwork>(editObject());
	if(!dislocationsObj) return;

	UndoableTransaction::handleExceptions(tr("Show selected dislocations"), [this, dislocationsObj]() {
		QItemSelectionModel* selectionModel = _dislocationListUI->tableWidget()->selectionModel();
		QBitArray segmentSelection(dislocationsObj->segments().size());
		for(const QModelIndex& index : selectionModel->selectedRows())
			segmentSelection.setBit(_sortedModel->mapToSource(index).row());
		int index = 0;
		for(DislocationSegment* segment : dislocationsObj->segments()) {
			if(segmentSelection.testBit(index++))
				segment->setVisible(true);
		}
	});
}

};
