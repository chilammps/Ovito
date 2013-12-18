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
#include <core/dataset/UndoStack.h>
#include <core/scene/SelectionSet.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/viewport/input/XFormModes.h>

namespace Ovito {

/******************************************************************************
* This is called by the system after the input handler is
* no longer the active handler.
******************************************************************************/
void XFormMode::deactivated(bool temporary)
{
	if(_viewport) {
		// Restore old state if change has not been committed.
		_viewport->dataset()->undoStack().endCompoundOperation(false);
		if(isTransformationMode())
			_viewport->dataset()->undoStack().endCompoundOperation(false);
		_viewport = nullptr;
	}
	ViewportInputMode::deactivated(temporary);
}

/******************************************************************************
* Handles the mouse down event for the given viewport.
******************************************************************************/
void XFormMode::mousePressEvent(Viewport* vp, QMouseEvent* event)
{
	event->setAccepted(false);
	if(event->button() == Qt::LeftButton) {
		if(_viewport == nullptr) {

			// Select object under mouse cursor.
			ViewportPickResult pickResult = vp->pick(event->localPos());
			if(pickResult.valid && pickResult.objectNode) {
				_viewport = vp;
				_startPoint = event->localPos();
				_viewport->dataset()->undoStack().beginCompoundOperation(undoDisplayName());
				_viewport->dataset()->selection()->setNode(pickResult.objectNode.get());
				if(isTransformationMode()) {
					_viewport->dataset()->undoStack().beginCompoundOperation(undoDisplayName());
					startXForm();
					return;
				}
			}
		}
	}
	else if(event->button() == Qt::RightButton) {
		if(_viewport != nullptr) {
			// Restore old state when aborting the operation.
			_viewport->dataset()->undoStack().endCompoundOperation(false);
			if(isTransformationMode())
				_viewport->dataset()->undoStack().endCompoundOperation(false);
			_viewport = nullptr;
		}
	}
	ViewportInputMode::mousePressEvent(vp, event);
}

/******************************************************************************
* Handles the mouse up event for the given viewport.
******************************************************************************/
void XFormMode::mouseReleaseEvent(Viewport* vp, QMouseEvent* event)
{
	if(_viewport) {
		// Commit change.
		_viewport->dataset()->undoStack().endCompoundOperation();
		if(isTransformationMode())
			_viewport->dataset()->undoStack().endCompoundOperation();
		_viewport = nullptr;
	}
	ViewportInputMode::mouseReleaseEvent(vp, event);
}

/******************************************************************************
* Handles the mouse move event for the given viewport.
******************************************************************************/
void XFormMode::mouseMoveEvent(Viewport* vp, QMouseEvent* event)
{
	if(_viewport == vp) {
#if 1
		// Take the current mouse cursor position to make the input mode
		// look more responsive. The cursor position recorded when the mouse event was
		// generates may be too old.
		_currentPoint = vp->widget()->mapFromGlobal(QCursor::pos());
#else
		_currentPoint = event->localPos();
#endif

		if(isTransformationMode()) {
			vp->dataset()->undoStack().resetCurrentCompoundOperation();
			doXForm(vp);

			// Force immediate viewport repaints.
			vp->dataset()->mainWindow()->processViewportUpdates();
		}
	}
	else {

		// Change mouse cursor while hovering over an object.
		ViewportPickResult pickResult = vp->pick(event->localPos());
		setCursor(pickResult.valid ? _xformCursor : QCursor());

	}
	ViewportInputMode::mouseMoveEvent(vp, event);
}

};
