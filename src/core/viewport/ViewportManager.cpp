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

namespace Ovito {

/// The singleton instance of the class.
QScopedPointer<ViewportManager> ViewportManager::_instance;

/******************************************************************************
* Initializes the viewport manager.
******************************************************************************/
ViewportManager::ViewportManager() :
	_activeViewport(NULL), _maximizedViewport(NULL),
	_viewportSuspendCount(1), _viewportsNeedUpdate(false)
{
}

/******************************************************************************
* Sets the active viewport.
******************************************************************************/
void ViewportManager::setActiveViewport(Viewport* vp)
{
	if(vp == _activeViewport) return;
	_activeViewport = vp;

	// Generate signal.
	activeViewportChanged(vp);

	// Redraw viewports.
	updateViewports();
}

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

/******************************************************************************
* This gets a viewport configuration that should be
* used as template for new scene files.
******************************************************************************/
OORef<ViewportConfigurationSet> ViewportManager::defaultConfiguration()
{
	// Make sure the default configuration is initialized.
	if(!_defaultConfig) {
		_defaultConfig = new ViewportConfigurationSet();

		OORef<ViewportConfiguration> topView = new ViewportConfiguration();
		topView->setViewType(ViewportConfiguration::VIEW_TOP);
		_defaultConfig->addViewport(topView);

		OORef<ViewportConfiguration> frontView = new ViewportConfiguration();
		frontView->setViewType(ViewportConfiguration::VIEW_FRONT);
		_defaultConfig->addViewport(frontView);

		OORef<ViewportConfiguration> leftView = new ViewportConfiguration();
		leftView->setViewType(ViewportConfiguration::VIEW_LEFT);
		_defaultConfig->addViewport(leftView);

		OORef<ViewportConfiguration> perspectiveView = new ViewportConfiguration();
		perspectiveView->setViewType(ViewportConfiguration::VIEW_PERSPECTIVE);
		_defaultConfig->addViewport(perspectiveView);

		_defaultConfig->setActiveViewport(0);
		_defaultConfig->setMaximizedViewport(-1);
	}
	return _defaultConfig;
}

};
