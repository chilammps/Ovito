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
#include <core/scene/ObjectNode.h>
#include <core/scene/SelectionSet.h>
#include "DislocationNetwork.h"
#include "DislocationDisplay.h"
#include "DislocationInspector.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, DislocationNetwork, DataObject);
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, DislocationNetworkEditor, PropertiesEditor);
SET_OVITO_OBJECT_EDITOR(DislocationNetwork, DislocationNetworkEditor);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(DislocationNetwork, _segments, "DislocationSegments", DislocationSegment, PROPERTY_FIELD_ALWAYS_CLONE);
SET_PROPERTY_FIELD_LABEL(DislocationNetwork, _segments, "Dislocation segments");

/******************************************************************************
* Constructor.
******************************************************************************/
DislocationNetwork::DislocationNetwork(DataSet* dataset) : DataObject(dataset)
{
	INIT_PROPERTY_FIELD(DislocationNetwork::_segments);
	addDisplayObject(new DislocationDisplay(dataset));
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void DislocationNetworkEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Dislocations"), rolloutParams);
	QVBoxLayout* rolloutLayout = new QVBoxLayout(rollout);

	QPushButton* openInspectorButton = new QPushButton(tr("Open Dislocation Inspector"), rollout);
	rolloutLayout->addWidget(openInspectorButton);
	connect(openInspectorButton, &QPushButton::clicked, this, &DislocationNetworkEditor::onOpenInspector);
}

/******************************************************************************
* Is called when the user presses the "Open Inspector" button.
******************************************************************************/
void DislocationNetworkEditor::onOpenInspector()
{
	DislocationNetwork* dislocationsObj = static_object_cast<DislocationNetwork>(editObject());
	if(!dislocationsObj) return;

	QMainWindow* inspectorWindow = new QMainWindow(container()->window(), (Qt::WindowFlags)(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint));
	inspectorWindow->setWindowTitle(tr("Dislocation Inspector"));
	PropertiesPanel* propertiesPanel = new PropertiesPanel(inspectorWindow);
	propertiesPanel->hide();

	QWidget* mainPanel = new QWidget(inspectorWindow);
	QVBoxLayout* mainPanelLayout = new QVBoxLayout(mainPanel);
	mainPanelLayout->setStretch(0,1);
	mainPanelLayout->setContentsMargins(0,0,0,0);
	inspectorWindow->setCentralWidget(mainPanel);

	ObjectNode* node = dynamic_object_cast<ObjectNode>(dataset()->selection()->front());
	DislocationInspector* inspector = new DislocationInspector(node);
	connect(inspector, &QObject::destroyed, inspectorWindow, &QMainWindow::close);
	inspector->setParent(propertiesPanel);
	inspector->initialize(propertiesPanel, mainWindow(), RolloutInsertionParameters().insertInto(mainPanel));
	inspector->setEditObject(dislocationsObj);
	inspectorWindow->setAttribute(Qt::WA_DeleteOnClose);
	inspectorWindow->resize(1000, 350);
	inspectorWindow->show();
}

}	// End of namespace
}	// End of namespace
}	// End of namespace
