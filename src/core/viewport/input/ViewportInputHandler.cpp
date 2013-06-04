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
#include <core/gui/app/Application.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/viewport/input/ViewportInputHandler.h>
#include <core/viewport/input/NavigationModes.h>

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(ViewportInputHandler, OvitoObject)

/******************************************************************************
* Updates the cursor in the viewport.
******************************************************************************/
void ViewportInputHandler::updateCursor()
{
	if(Application::instance().guiMode()) {
	//	if(ViewportInputManager::instance().currentHandler() == NULL) return;
	//	if(ViewportInputManager::instance().currentHandler() == this || ViewportInputManager::instance().currentHandler() == _temporaryNavMode) {
	}
}

/******************************************************************************
* This is called by the system after the input handler has become the active handler.
******************************************************************************/
void ViewportInputHandler::activated()
{
}

/******************************************************************************
* This is called by the system after the input handler is no longer the active handler.
******************************************************************************/
void ViewportInputHandler::deactivated()
{
	if(_temporaryNavMode) {
		_temporaryNavMode->deactivated();
		_temporaryNavMode = NULL;
	}
}

/******************************************************************************
* Activates the given temporary navigation mode.
******************************************************************************/
void ViewportInputHandler::activateTemporaryNavigationMode(ViewportInputHandler* mode)
{
	OVITO_ASSERT(mode != NULL);
	_temporaryNavMode = mode;
	_temporaryNavMode->activated();
	updateCursor();
}

/******************************************************************************
* Handles the mouse down event for the given viewport.
******************************************************************************/
void ViewportInputHandler::mousePressEvent(Viewport* vp, QMouseEvent* event)
{
	ViewportManager::instance().setActiveViewport(vp);
	if(ViewportInputManager::instance().currentHandler() == this) {
		if(event->button() == Qt::RightButton) {
			if(handlerType() != EXCLUSIVE)
				ViewportInputManager::instance().removeInputHandler(this);
			else {
				activateTemporaryNavigationMode(PanMode::instance());
				temporaryNavigationMode()->mousePressEvent(vp, event);
			}
		}
		else if(event->button() == Qt::LeftButton) {
			activateTemporaryNavigationMode(OrbitMode::instance());
			temporaryNavigationMode()->mousePressEvent(vp, event);
		}
		else if(event->button() == Qt::MidButton) {
			activateTemporaryNavigationMode(PanMode::instance());
			temporaryNavigationMode()->mousePressEvent(vp, event);
		}
	}
}

/******************************************************************************
* Handles the mouse up event for the given viewport.
******************************************************************************/
void ViewportInputHandler::mouseReleaseEvent(Viewport* vp, QMouseEvent* event)
{
	if(_temporaryNavMode) {
		_temporaryNavMode->mouseReleaseEvent(vp, event);
		_temporaryNavMode->deactivated();
		_temporaryNavMode = NULL;
		updateCursor();
	}
}

/******************************************************************************
* Handles the mouse move event for the given viewport.
******************************************************************************/
void ViewportInputHandler::mouseMoveEvent(Viewport* vp, QMouseEvent* event)
{
	if(_temporaryNavMode) {
		_temporaryNavMode->mouseMoveEvent(vp, event);
	}
}

/******************************************************************************
* Handles the mouse wheel event for the given viewport.
******************************************************************************/
void ViewportInputHandler::wheelEvent(Viewport* vp, QWheelEvent* event)
{
	ZoomMode::instance()->zoom(vp, (FloatType)event->delta());
}

};
