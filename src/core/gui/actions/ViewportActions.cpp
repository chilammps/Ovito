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
#include <core/gui/actions/ActionManager.h>
#include <core/viewport/ViewportManager.h>

namespace Ovito {

/******************************************************************************
* Handles the ACTION_VIEWPORT_MAXIMIZE command.
******************************************************************************/
void ActionManager::on_ViewportMaximize_triggered()
{
	if(ViewportManager::instance().maximizedViewport())
		ViewportManager::instance().setMaximizedViewport(nullptr);
	else if(ViewportManager::instance().activeViewport())
		ViewportManager::instance().setMaximizedViewport(ViewportManager::instance().activeViewport());
}

/******************************************************************************
* Handles the ACTION_VIEWPORT_ZOOM_SCENE_EXTENTS command.
******************************************************************************/
void ActionManager::on_ViewportZoomSceneExtents_triggered()
{
	if(ViewportManager::instance().activeViewport())
		ViewportManager::instance().activeViewport()->zoomToSceneExtents();
}

/******************************************************************************
* Handles the ACTION_VIEWPORT_ZOOM_SCENE_EXTENTS_ALL command.
******************************************************************************/
void ActionManager::on_ViewportZoomSceneExtentsAll_triggered()
{
	for(Viewport* viewport : ViewportManager::instance().viewports())
		viewport->zoomToSceneExtents();
}

/******************************************************************************
* Handles the ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS command.
******************************************************************************/
void ActionManager::on_ViewportZoomSelectionExtents_triggered()
{
	if(ViewportManager::instance().activeViewport())
		ViewportManager::instance().activeViewport()->zoomToSelectionExtents();
}

/******************************************************************************
* Handles the ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS_ALL command.
******************************************************************************/
void ActionManager::on_ViewportZoomSelectionExtentsAll_triggered()
{
	for(Viewport* viewport : ViewportManager::instance().viewports())
		viewport->zoomToSelectionExtents();
}

};
