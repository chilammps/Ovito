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
#include <core/gui/widgets/SceneNodeSelectionBox.h>
#include <core/dataset/DataSetManager.h>
#include "SceneNodesListModel.h"

namespace Ovito {

/******************************************************************************
* Constructs the widget.
******************************************************************************/
SceneNodeSelectionBox::SceneNodeSelectionBox(QWidget* parent) : QComboBox(parent)
{
	// Set the list model, which tracks the scene nodes.
	setModel(new SceneNodesListModel(this));

	setInsertPolicy(QComboBox::NoInsert);
	setEditable(false);
	setMinimumContentsLength(25);
	setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	setToolTip(tr("Object Selector"));

	// Listen for selection changes.
	connect(&DataSetManager::instance(), SIGNAL(selectionChangeComplete(SelectionSet*)), this, SLOT(onSceneSelectionChanged()));
	connect(&DataSetManager::instance(), SIGNAL(dataSetReset(DataSet*)), this, SLOT(onSceneSelectionChanged()));

	connect(this, SIGNAL(activated(int)), this, SLOT(onItemActivated(int)));
}

/******************************************************************************
* This is called whenever the node selection has changed.
******************************************************************************/
void SceneNodeSelectionBox::onSceneSelectionChanged()
{
	SelectionSet* selection = DataSetManager::instance().currentSelection();
	if(!selection || selection->empty()) {
		setCurrentText(tr("No selection"));
	}
	else if(selection->count() > 1) {
		setCurrentText(tr("%i selected objects").arg(selection->count()));
	}
	else {
		int index = findData(QVariant::fromValue(selection->node(0)));
		setCurrentIndex(index);
	}
}

/******************************************************************************
* Is called when the user selected an item in the list box.
******************************************************************************/
void SceneNodeSelectionBox::onItemActivated(int index)
{
	SceneNode* node = qobject_cast<SceneNode*>(itemData(index).value<QObject*>());
	SelectionSet* selection = DataSetManager::instance().currentSelection();
	UndoableTransaction::handleExceptions(tr("Select object"), [node, selection]() {
		if(node)
			selection->setNode(node);
		else
			selection->clear();
	});
}

};
