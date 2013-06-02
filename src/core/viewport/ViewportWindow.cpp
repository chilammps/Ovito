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
#include <core/viewport/ViewportWindow.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/viewport/input/ViewportInputManager.h>

namespace Ovito {

/******************************************************************************
* Puts an update request event for this viewport on the event loop.
******************************************************************************/
void ViewportWindow::renderLater()
{
	// If not already done so, put an update request event on the event loop,
	// which leads to renderNow() being called once the event gets processed.
	if(!_updatePending) {
		_updatePending = true;
		QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
	}
}

/******************************************************************************
* This internal method receives events to the viewport window.
******************************************************************************/
bool ViewportWindow::event(QEvent* event)
{
	// Handle update request events creates by renderLater().
	if(event->type() == QEvent::UpdateRequest) {
		if(_updatePending)
			renderNow();
		return true;
	}
	return QWindow::event(event);
}

/******************************************************************************
* Handles the expose events.
******************************************************************************/
void ViewportWindow::exposeEvent(QExposeEvent*)
{
	if(isExposed()) {
		renderNow();
	}
}

/******************************************************************************
* Handles the resize events.
******************************************************************************/
void ViewportWindow::resizeEvent(QResizeEvent*)
{
	if(isExposed()) {
		renderNow();
	}
}

/******************************************************************************
* Handles double click events.
******************************************************************************/
void ViewportWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
	ViewportInputHandler* handler = ViewportInputManager::instance().currentHandler();
	if(handler)
		handler->mouseDoubleClickEvent(_viewport, event);
}

/******************************************************************************
* Handles mouse press events.
******************************************************************************/
void ViewportWindow::mousePressEvent(QMouseEvent* event)
{
	ViewportInputHandler* handler = ViewportInputManager::instance().currentHandler();
	if(handler)
		handler->mousePressEvent(_viewport, event);
}

/******************************************************************************
* Handles mouse release events.
******************************************************************************/
void ViewportWindow::mouseReleaseEvent(QMouseEvent* event)
{
	ViewportInputHandler* handler = ViewportInputManager::instance().currentHandler();
	if(handler)
		handler->mouseReleaseEvent(_viewport, event);
}

/******************************************************************************
* Handles mouse move events.
******************************************************************************/
void ViewportWindow::mouseMoveEvent(QMouseEvent* event)
{
	ViewportInputHandler* handler = ViewportInputManager::instance().currentHandler();
	if(handler)
		handler->mouseMoveEvent(_viewport, event);
}

/******************************************************************************
* Handles mouse wheel events.
******************************************************************************/
void ViewportWindow::wheelEvent(QWheelEvent* event)
{
	ViewportInputHandler* handler = ViewportInputManager::instance().currentHandler();
	if(handler)
		handler->wheelEvent(_viewport, event);
}


/******************************************************************************
* Immediately redraws the contents of this window.
******************************************************************************/
void ViewportWindow::renderNow()
{
	if(!isExposed())
		return;

	if(ViewportManager::instance().isSuspended())
		return;

	_updatePending = false;

	// Create OpenGL context on first redraw.
	if(!_context) {
		_context = new QOpenGLContext(this);
		_context->setFormat(requestedFormat());
		if(!_context->create())
			throw Exception(tr("Failed to create OpenGL context."));
	}

	if(_context->isValid()) {
		_context->makeCurrent(this);

		if(!_paintDevice)
			_paintDevice.reset(new QOpenGLPaintDevice());
		_paintDevice->setSize(size());
		_viewport->render(_context, _paintDevice.data());
		_context->swapBuffers(this);
		_context->doneCurrent();
	}
}

};
