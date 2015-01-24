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
#include <core/gui/properties/RefTargetListParameterUI.h>
#include "ClusterGraph.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, ClusterGraph, DataObject);
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, ClusterGraphEditor, PropertiesEditor);
SET_OVITO_OBJECT_EDITOR(ClusterGraph, ClusterGraphEditor);
DEFINE_VECTOR_REFERENCE_FIELD(ClusterGraph, _clusters, "Clusters", Cluster);
SET_PROPERTY_FIELD_LABEL(ClusterGraph, _clusters, "Clusters");

/******************************************************************************
* Constructs a cluster graph object.
******************************************************************************/
ClusterGraph::ClusterGraph(DataSet* dataset) : DataObject(dataset)
{
	INIT_PROPERTY_FIELD(ClusterGraph::_clusters);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ClusterGraphEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Clusters"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);

	// Derive a custom class from the list parameter UI to
	// give the items a color.
	class CustomRefTargetListParameterUI : public RefTargetListParameterUI {
	public:
		CustomRefTargetListParameterUI(PropertiesEditor* parentEditor, const PropertyFieldDescriptor& refField)
			: RefTargetListParameterUI(parentEditor, refField) {}
	protected:
		virtual QVariant getItemData(RefTarget* target, const QModelIndex& index, int role) override {
			if(target != NULL && role == Qt::DisplayRole) {
				if(index.column() == 0)
					return static_object_cast<Cluster>(target)->id();
				else if(index.column() == 1)
					return static_object_cast<Cluster>(target)->pattern()->name();
				else
					return static_object_cast<Cluster>(target)->atomCount();
			}
			return QVariant();
		}

		/// Returns the number of columns for the table view.
		virtual int tableColumnCount() override { return 3; }

		/// Returns the header data under the given role for the given RefTarget.
		/// This method is part of the data model used by the list widget and can be overriden
		/// by sub-classes.
		virtual QVariant getHorizontalHeaderData(int index, int role) override {
			if(role != Qt::DisplayRole) return QVariant();
			if(index == 0)
				return QVariant(ClusterGraphEditor::tr("Id"));
			else if(index == 1)
				return QVariant(ClusterGraphEditor::tr("Structure"));
			else
				return QVariant(ClusterGraphEditor::tr("#Atoms"));
		}

		/// Do not open sub-editor for selected item.
		virtual void openSubEditor() override {}
	};

	layout1->addWidget(new QLabel(tr("Clusters:")));
	RefTargetListParameterUI* clusterListUI = new CustomRefTargetListParameterUI(this, PROPERTY_FIELD(ClusterGraph::_clusters));
	layout1->addWidget(clusterListUI->tableWidget(300));
	clusterListUI->tableWidget()->setAutoScroll(false);
	clusterListUI->tableWidget()->setShowGrid(true);
	clusterListUI->tableWidget()->horizontalHeader()->setVisible(true);
	clusterListUI->tableWidget()->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

}	// End of namespace
}	// End of namespace
}	// End of namespace
