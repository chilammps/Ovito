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

IMPLEMENT_OVITO_OBJECT(Core, ViewportInputHandler, OvitoObject)

/******************************************************************************
* This is called by the system after the input handler has become the active handler.
******************************************************************************/
void ViewportInputHandler::activated()
{
	_showOrbitCenter = false;
}

/******************************************************************************
* This is called by the system after the input handler is no longer the active handler.
******************************************************************************/
void ViewportInputHandler::deactivated()
{
	if(_temporaryNavMode) {
		_temporaryNavMode->deactivated();
		_temporaryNavMode = nullptr;
	}
}

/******************************************************************************
* Activates the given temporary navigation mode.
******************************************************************************/
void ViewportInputHandler::activateTemporaryNavigationMode(ViewportInputHandler* mode)
{
	OVITO_ASSERT(mode != nullptr);
	if(this->hasOverlay())
		ViewportManager::instance().updateViewports();
	_showOrbitCenter = false;
	_temporaryNavMode = mode;
	_temporaryNavMode->activated();
	if(_temporaryNavMode->hasOverlay())
		ViewportManager::instance().updateViewports();
	ViewportInputManager::instance().updateViewportCursor();
}

/******************************************************************************
* Sets the mouse cursor shown in the viewport windows
* while this input handler is active.
******************************************************************************/
void ViewportInputHandler::setCursor(const QCursor& cursor)
{
	_cursor = cursor;
	if(ViewportInputManager::isInitialized())
		ViewportInputManager::instance().updateViewportCursor();
}

/******************************************************************************
* Handles the mouse down event for the given viewport.
******************************************************************************/
void ViewportInputHandler::mousePressEvent(Viewport* vp, QMouseEvent* event)
{
	_lastMousePressEvent.reset();
	if(ViewportInputManager::instance().currentHandler() == this) {
		if(event->button() == Qt::RightButton) {
			if(handlerType() != EXCLUSIVE) {
				ViewportInputManager::instance().removeInputHandler(this);
			}
			else {
				activateTemporaryNavigationMode(PanMode::instance());
				temporaryNavigationMode()->mousePressEvent(vp, event);
			}
			event->accept();
		}
		else if(event->button() == Qt::LeftButton) {
			_lastMousePressEvent.reset(new QMouseEvent(event->type(), event->localPos(), event->windowPos(), event->screenPos(), event->button(), event->buttons(), event->modifiers()));
			event->accept();
		}
		else if(event->button() == Qt::MidButton) {
			activateTemporaryNavigationMode(PanMode::instance());
			temporaryNavigationMode()->mousePressEvent(vp, event);
			event->accept();
		}
	}
}

/******************************************************************************
* Handles the mouse up event for the given viewport.
******************************************************************************/
void ViewportInputHandler::mouseReleaseEvent(Viewport* vp, QMouseEvent* event)
{
	_lastMousePressEvent.reset();
	if(temporaryNavigationMode()) {
		temporaryNavigationMode()->mouseReleaseEvent(vp, event);
		temporaryNavigationMode()->deactivated();
		if(temporaryNavigationMode()->hasOverlay())
			ViewportManager::instance().updateViewports();
		_temporaryNavMode = nullptr;
		if(this->hasOverlay())
			ViewportManager::instance().updateViewports();
		ViewportInputManager::instance().updateViewportCursor();
		event->accept();
	}
}

/******************************************************************************
* Handles the mouse move event for the given viewport.
******************************************************************************/
void ViewportInputHandler::mouseMoveEvent(Viewport* vp, QMouseEvent* event)
{
	if(_lastMousePressEvent && (event->pos() - _lastMousePressEvent->pos()).manhattanLength() > 2) {
		if(!temporaryNavigationMode() && this != OrbitMode::instance()) {
			activateTemporaryNavigationMode(OrbitMode::instance());
			temporaryNavigationMode()->mousePressEvent(vp, _lastMousePressEvent.get());
		}
		_lastMousePressEvent.reset();
	}
	if(temporaryNavigationMode()) {
		temporaryNavigationMode()->mouseMoveEvent(vp, event);
		event->accept();
	}
}

/******************************************************************************
* Handles the mouse wheel event for the given viewport.
******************************************************************************/
void ViewportInputHandler::wheelEvent(Viewport* vp, QWheelEvent* event)
{
	_lastMousePressEvent.reset();
	ZoomMode::instance()->zoom(vp, (FloatType)event->delta());
	event->accept();
}

/******************************************************************************
* Handles the mouse double-click events for the given viewport.
******************************************************************************/
void ViewportInputHandler::mouseDoubleClickEvent(Viewport* vp, QMouseEvent* event)
{
	_lastMousePressEvent.reset();
	if(ViewportInputManager::instance().currentHandler() == this) {
		if(event->button() == Qt::LeftButton) {
			PickOrbitCenterMode::instance()->pickOrbitCenter(vp, event->pos());
			event->accept();
			_showOrbitCenter = true;
			ViewportManager::instance().updateViewports();
		}
	}
}

/******************************************************************************
* Lets the input mode render its overlay content in a viewport.
******************************************************************************/
void ViewportInputHandler::renderOverlay3D(Viewport* vp, ViewportSceneRenderer* renderer, bool isActive)
{
	if(_temporaryNavMode)
		_temporaryNavMode->renderOverlay3D(vp, renderer, isActive);
	else if(_showOrbitCenter)
		OrbitMode::instance()->renderOverlay3D(vp, renderer, isActive);
}

/******************************************************************************
* Computes the bounding box of the visual viewport overlay rendered by the input mode.
******************************************************************************/
Box3 ViewportInputHandler::overlayBoundingBox(Viewport* vp, ViewportSceneRenderer* renderer, bool isActive)
{
	Box3 bb;
	if(_temporaryNavMode)
		bb.addBox(_temporaryNavMode->overlayBoundingBox(vp, renderer, isActive));
	else if(_showOrbitCenter)
		bb.addBox(OrbitMode::instance()->overlayBoundingBox(vp, renderer, isActive));
	return bb;
}

/******************************************************************************
* Lets the input mode render its overlay content in a viewport.
******************************************************************************/
void ViewportInputHandler::renderOverlay2D(Viewport* vp, ViewportSceneRenderer* renderer, bool isActive)
{
	if(_temporaryNavMode)
		_temporaryNavMode->renderOverlay2D(vp, renderer, isActive);
}

};
