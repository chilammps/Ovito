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

#ifndef __OVITO_SCENE_NODE_SELECTION_BOX_H
#define __OVITO_SCENE_NODE_SELECTION_BOX_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A combo-box widget that displays the current scene node selection
 * and allows to select scene nodes.
 */
class SceneNodeSelectionBox : public QComboBox
{
	Q_OBJECT
	
public:
	
	/// Constructs the widget.
	SceneNodeSelectionBox(DataSetContainer& datasetContainer, QWidget* parent = 0);

protected Q_SLOTS:

	/// This is called whenever the node selection has changed.
	void onSceneSelectionChanged();

	/// Is called when the user selected an item in the list box.
	void onItemActivated(int index);

	/// This is called whenever the number of nodes changes.
	void onNodeCountChanged();

private:

	/// The container of the dataset.
	DataSetContainer& _datasetContainer;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_ANIMATION_FRAMES_TOOL_BUTTON_H
