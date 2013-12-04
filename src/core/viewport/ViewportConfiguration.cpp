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
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/Viewport.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ViewportConfiguration, RefTarget);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(ViewportConfiguration, _viewports, "Viewports", Viewport, PROPERTY_FIELD_NO_UNDO|PROPERTY_FIELD_ALWAYS_CLONE)
DEFINE_FLAGS_REFERENCE_FIELD(ViewportConfiguration, _activeViewport, "ActiveViewport", Viewport, PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_REFERENCE_FIELD(ViewportConfiguration, _maximizedViewport, "MaximizedViewport", Viewport, PROPERTY_FIELD_NO_UNDO)

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportConfiguration::ViewportConfiguration(DataSet* dataset) : RefTarget(dataset)
{
	INIT_PROPERTY_FIELD(ViewportConfiguration::_viewports);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_activeViewport);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_maximizedViewport);
}

/******************************************************************************
* Is called when the value of a reference field of this RefMaker changes.
******************************************************************************/
void ViewportConfiguration::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(ViewportConfiguration::_activeViewport)) {
		activeViewportChanged(_activeViewport);
	}
	else if(field == PROPERTY_FIELD(ViewportConfiguration::_maximizedViewport)) {
		maximizedViewportChanged(_maximizedViewport);
	}
	RefTarget::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* This flags all viewports for redrawing.
******************************************************************************/
void ViewportConfiguration::updateViewports()
{
	// Ignore update request that are made during an update.
	if(isRendering())
		return;

	// Check if viewport updates are suppressed.
	if(_viewportSuspendCount > 0) {
		_viewportsNeedUpdate = true;
		return;
	}
	_viewportsNeedUpdate = false;

	for(Viewport* vp : viewports())
		vp->updateViewport();
}

/******************************************************************************
* This immediately redraws the viewports reflecting all
* changes made to the scene.
******************************************************************************/
void ViewportConfiguration::processViewportUpdates()
{
	if(isSuspended())
		return;

	for(Viewport* vp : viewports())
		vp->processUpdateRequest();
}

/******************************************************************************
* Return true if there is currently a rendering operation going on.
* No new windows or dialogs should be shown during this phase
* to prevent an infinite update loop.
******************************************************************************/
bool ViewportConfiguration::isRendering() const
{
	// Check if any of the viewport windows is rendering.
	for(Viewport* vp : viewports())
		if(vp->isRendering()) return true;

	return false;
}

/******************************************************************************
* This will resume redrawing of the viewports after a call to suspendViewportUpdates().
******************************************************************************/
void ViewportConfiguration::resumeViewportUpdates()
{
	OVITO_ASSERT(_viewportSuspendCount > 0);
	_viewportSuspendCount--;
	if(_viewportSuspendCount == 0 && _viewportsNeedUpdate)
		updateViewports();
}

/******************************************************************************
* Returns the renderer to be used for rendering the interactive viewports.
******************************************************************************/
ViewportSceneRenderer* ViewportConfiguration::viewportRenderer()
{
	if(!_viewportRenderer)
		_viewportRenderer = new ViewportSceneRenderer();
	return _viewportRenderer.get();
}


};
