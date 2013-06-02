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
#include <core/viewport/ViewportManager.h>
#include <core/viewport/Viewport.h>
#include <core/dataset/DataSetManager.h>

namespace Ovito {

/// The singleton instance of the class.
QScopedPointer<ViewportManager> ViewportManager::_instance;

IMPLEMENT_OVITO_OBJECT(ViewportManager, RefMaker)
DEFINE_FLAGS_REFERENCE_FIELD(ViewportManager, _viewportConfig, "ViewportConfiguration", ViewportConfiguration, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_NO_UNDO|PROPERTY_FIELD_NEVER_CLONE_TARGET)

/******************************************************************************
* Initializes the viewport manager.
******************************************************************************/
ViewportManager::ViewportManager() :
	_viewportSuspendCount(1), _viewportsNeedUpdate(false)
{
	OVITO_ASSERT_MSG(!_instance, "ViewportManager constructor", "Multiple instances of this singleton class have been created.");
	INIT_PROPERTY_FIELD(ViewportManager::_viewportConfig);

	// Listen for changes of the data set.
	connect(&DataSetManager::instance(), SIGNAL(dataSetReset(DataSet*)), this, SLOT(onDataSetReset(DataSet*)));
}

/******************************************************************************
* This is called when a new dataset has been loaded.
******************************************************************************/
void ViewportManager::onDataSetReset(DataSet* newDataSet)
{
	if(_viewportConfig) {
		disconnect(_viewportConfig, SIGNAL(activeViewportChanged(Viewport*)), this, SIGNAL(activeViewportChanged(Viewport*)));
		disconnect(_viewportConfig, SIGNAL(maximizedViewportChanged(Viewport*)), this, SIGNAL(maximizedViewportChanged(Viewport*)));
	}

	// Listen for changes of the current viewport configuration.
	_viewportConfig = newDataSet ? newDataSet->viewportConfig() : NULL;

	if(newDataSet) {
		connect(_viewportConfig, SIGNAL(activeViewportChanged(Viewport*)), this, SIGNAL(activeViewportChanged(Viewport*)));
		connect(_viewportConfig, SIGNAL(maximizedViewportChanged(Viewport*)), this, SIGNAL(maximizedViewportChanged(Viewport*)));
	}
}

#if 0
/******************************************************************************
* Sets the maximized viewport.
******************************************************************************/
void ViewportManager::setMaximizedViewport(Viewport* vp)
{
	if(vp == _maximizedViewport) return;
	_maximizedViewport = vp;

	// Adjust visibility of viewports.
    for(Viewport* viewport : viewports()) {
		if(_maximizedViewport == NULL || viewport == _maximizedViewport)
			viewport->widget()->show();
		else
			viewport->widget()->hide();
	}

	// Make it the active viewport.
	if(vp) setActiveViewport(vp);

	// Generate signal.
	maximizedViewportChanged(vp);

	// Redraw viewports.
	updateViewports();
}
#endif

/******************************************************************************
* This flags all viewports for redrawing.
******************************************************************************/
void ViewportManager::updateViewports()
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
void ViewportManager::processViewportUpdates()
{
	if(isSuspended())
		return;

#if 0
	if(Application::instance().guiMode())
		Window3D::processWindowUpdates();
#endif
}

/******************************************************************************
* Return true if there is currently a rendering operation going on.
* No new windows or dialogs should be shown during this phase
* to prevent an infinite update loop.
******************************************************************************/
bool ViewportManager::isRendering() const
{
	// Check if any of the viewport windows is rendering.
	for(Viewport* vp : viewports())
		if(vp->isRendering()) return true;

	return false;
}

/******************************************************************************
* This will resume redrawing of the viewports after a call to suspendViewportUpdates().
******************************************************************************/
void ViewportManager::resumeViewportUpdates()
{
	OVITO_ASSERT(_viewportSuspendCount > 0);
	_viewportSuspendCount--;
	if(_viewportSuspendCount == 0 && _viewportsNeedUpdate)
		updateViewports();
}

};
