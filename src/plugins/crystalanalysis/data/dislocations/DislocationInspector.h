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

#ifndef __OVITO_CA_DISLOCATION_INSPECTOR_H
#define __OVITO_CA_DISLOCATION_INSPECTOR_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/viewport/input/ViewportInputMode.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/gui/properties/RefTargetListParameterUI.h>
#include <core/scene/ObjectNode.h>
#include "DislocationSegment.h"
#include "DislocationDisplay.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/******************************************************************************
* A special properties editor for the Dislocations class.
******************************************************************************/
class OVITO_CRYSTALANALYSIS_EXPORT DislocationInspector : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE DislocationInspector(ObjectNode* sceneNode) {
		INIT_PROPERTY_FIELD(DislocationInspector::_sceneNode);
		_sceneNode = sceneNode;
	}

	/// Destructor.
	virtual ~DislocationInspector() { clearAllReferences(); }

	RefTargetListParameterUI* dislocationListUI() const { return _dislocationListUI; }
	QSortFilterProxyModel* sortedModel() const { return _sortedModel; }

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private Q_SLOTS:

	void onHideAll();
	void onShowAll();
	void onHideSelected();
	void onHideUnselected();
	void onShowSelected();

private:

	RefTargetListParameterUI* _dislocationListUI;
	QSortFilterProxyModel* _sortedModel;
	ViewportInputMode* _pickDislocationMode;
	ViewportModeAction* _pickDislocationAction;

	/// The scene node being loaded in the editor.
	ReferenceField<ObjectNode> _sceneNode;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_sceneNode);
};

/******************************************************************************
* This class belongs to the DislocationInspector and allows the user to pick
* a dislocation segment in the viewports.
******************************************************************************/
class OVITO_CRYSTALANALYSIS_EXPORT DislocationPickMode : public ViewportInputMode
{
public:

	/// Constructor.
	DislocationPickMode(DislocationInspector* inspector) : ViewportInputMode(inspector), _inspector(inspector) {
		_hoverSegment.segmentIndex = -1;
	}

	/// Handles the mouse button up events for a Viewport.
	virtual void mouseReleaseEvent(Viewport* vp, QMouseEvent* event) override;

	/// Handles the mouse move events for a Viewport.
	virtual void mouseMoveEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Lets the input mode render its overlay content in a viewport.
	virtual void renderOverlay3D(Viewport* vp, ViewportSceneRenderer* renderer) override;

	/// \brief Indicates whether this input mode renders into the viewports.
	virtual bool hasOverlay() override { return true; }

private:

	struct DislocationPickResult {

		/// The index of the picked dislocation segment.
		size_t segmentIndex;

		/// The picked dislocation segment.
		OORef<DislocationSegment> segment;

		/// The scene node that contains the picked segment.
		OORef<ObjectNode> objNode;

		/// The display object used to render the picked segment.
		OORef<DislocationDisplay> displayObj;
	};

	bool pickDislocationSegment(Viewport* vp, const QPoint& pos, DislocationPickResult& result) const;

	DislocationInspector* _inspector;
	DislocationPickResult _hoverSegment;

	Q_OBJECT
};

/******************************************************************************
* This helper class is used by the DislocationsInspector.
******************************************************************************/
struct SegmentCluster {
	DislocationSegment* segment;
	Cluster* cluster;
	Matrix3 transitionTM;

	bool operator<(const SegmentCluster& other) const {
		if(cluster == other.cluster) {
			for(size_t i = 0; i < 3; i++)
				for(size_t j = 0; j < 3; j++) {
					FloatType d = transitionTM(i,j) - other.transitionTM(i,j);
					if(d < FloatType(-1e-4)) return true;
					else if(d > FloatType(1e-4)) return false;
				}
			return false;
		}
		return cluster->atomCount() > other.cluster->atomCount();
	}
};

/******************************************************************************
* This helper class is used by the DislocationInspector.
******************************************************************************/
class ClusterItemDelegate : public QStyledItemDelegate
{
public:

	/// Constructor.
	ClusterItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

	/// Create the editor for the table item.
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
		if(index.data(Qt::EditRole).canConvert<SegmentCluster>()) {
			QComboBox* combobox = new QComboBox(parent);
			connect(combobox, (void (QComboBox::*)(int))&QComboBox::activated, this, &ClusterItemDelegate::commitAndCloseEditor);
			combobox->view()->setTextElideMode(Qt::ElideNone);
			return combobox;
		}
		return QStyledItemDelegate::createEditor(parent, option, index);
	}

	void setEditorData(QWidget* editor, const QModelIndex& index) const override;
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private Q_SLOTS:

	void commitAndCloseEditor() {
		QWidget* editor = qobject_cast<QWidget*>(sender());
		Q_EMIT commitData(editor);
		Q_EMIT closeEditor(editor);
	}

private:
	Q_OBJECT
};

}	// End of namespace
}	// End of namespace
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Plugins::CrystalAnalysis::SegmentCluster);

#endif // __OVITO_CA_DISLOCATIONS_EDITOR_H
