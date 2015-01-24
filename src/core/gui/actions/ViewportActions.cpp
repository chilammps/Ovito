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
#include <core/viewport/ViewportConfiguration.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui)

/******************************************************************************
* Handles the ACTION_VIEWPORT_MAXIMIZE command.
******************************************************************************/
void ActionManager::on_ViewportMaximize_triggered()
{
	ViewportConfiguration* vpconf = _dataset->viewportConfig();
	if(vpconf->maximizedViewport())
		vpconf->setMaximizedViewport(nullptr);
	else if(vpconf->activeViewport())
		vpconf->setMaximizedViewport(vpconf->activeViewport());
}

/******************************************************************************
* Handles the ACTION_VIEWPORT_ZOOM_SCENE_EXTENTS command.
******************************************************************************/
void ActionManager::on_ViewportZoomSceneExtents_triggered()
{
	ViewportConfiguration* vpconf = _dataset->viewportConfig();
	if(vpconf->activeViewport())
		vpconf->activeViewport()->zoomToSceneExtents();
}

/******************************************************************************
* Handles the ACTION_VIEWPORT_ZOOM_SCENE_EXTENTS_ALL command.
******************************************************************************/
void ActionManager::on_ViewportZoomSceneExtentsAll_triggered()
{
	_dataset->viewportConfig()->zoomToSceneExtents();
}

/******************************************************************************
* Handles the ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS command.
******************************************************************************/
void ActionManager::on_ViewportZoomSelectionExtents_triggered()
{
	ViewportConfiguration* vpconf = _dataset->viewportConfig();
	if(vpconf->activeViewport())
		vpconf->activeViewport()->zoomToSelectionExtents();
}

/******************************************************************************
* Handles the ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS_ALL command.
******************************************************************************/
void ActionManager::on_ViewportZoomSelectionExtentsAll_triggered()
{
	_dataset->viewportConfig()->zoomToSelectionExtents();
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
