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
#include <core/viewport/input/ViewportInputManager.h>
#include <core/viewport/input/ViewportInputHandler.h>
#include <core/viewport/ViewportManager.h>
#include <core/dataset/DataSetManager.h>

namespace Ovito {

/// The singleton instance of the class.
ViewportInputManager* ViewportInputManager::_instance = nullptr;

/******************************************************************************
* Initializes the viewport input manager.
******************************************************************************/
ViewportInputManager::ViewportInputManager()
{
	// Reset the viewport input manager when a new scene has been loaded.
	connect(&DataSetManager::instance(), SIGNAL(dataSetReset(DataSet*)), this, SLOT(reset()));
}

/******************************************************************************
* Returns the currently active ViewportInputHandler that
* handles the mouse events in viewports.
******************************************************************************/
ViewportInputHandler* ViewportInputManager::currentHandler()
{
	if(_inputHandlerStack.empty()) return NULL;
	return _inputHandlerStack.back().get();
}

/******************************************************************************
* Pushes a handler onto the stack and makes it active.
******************************************************************************/
void ViewportInputManager::pushInputHandler(const OORef<ViewportInputHandler>& handler)
{
    OVITO_CHECK_OBJECT_POINTER(handler);

    OORef<ViewportInputHandler> oldHandler = currentHandler();
	if(handler == oldHandler) return;

	if(oldHandler != NULL) {
		if(handler->handlerType() == ViewportInputHandler::EXCLUSIVE) {
			// Remove all handlers from the stack
			_inputHandlerStack.clear();
		}
		else if(handler->handlerType() == ViewportInputHandler::NORMAL) {
			// Remove all non-exclusive handlers from the stack
			for(int i = _inputHandlerStack.size(); i--; ) {
				OVITO_CHECK_OBJECT_POINTER(_inputHandlerStack[i]);
				if(_inputHandlerStack[i]->handlerType() != ViewportInputHandler::EXCLUSIVE) {
					_inputHandlerStack.remove(i);
				}
			}
		}
		else if(handler->handlerType() == ViewportInputHandler::TEMPORARY) {
			// Remove all temporary handlers from the stack.
			if(oldHandler->handlerType() == ViewportInputHandler::TEMPORARY)
				_inputHandlerStack.pop_back();
		}
	}

	// Put new handler on the stack.
	_inputHandlerStack.push_back(handler);
	if(oldHandler != NULL) oldHandler->deactivated();
	handler->activated();
	inputModeChanged(oldHandler.get(), handler.get());

	// Redraw viewports if the old or the new handler uses overlays.
	if((oldHandler && oldHandler->hasOverlay()) || (handler && handler->hasOverlay()))
		ViewportManager::instance().updateViewports();
}

/******************************************************************************
* Removes a handler from the stack and deactivates it if
* it is currently active.
******************************************************************************/
void ViewportInputManager::removeInputHandler(ViewportInputHandler* handler)
{
	int index = _inputHandlerStack.indexOf(handler);
	if(index < 0) return;
	OVITO_CHECK_OBJECT_POINTER(handler);
	if(index == _inputHandlerStack.size() - 1) {
		OORef<ViewportInputHandler> oldHandler = handler;
		_inputHandlerStack.remove(index);
		handler->deactivated();
		if(!_inputHandlerStack.empty())
			currentHandler()->activated();
		inputModeChanged(handler, currentHandler());

		// Redraw viewports if the old or the new handler uses overlays.
		if((oldHandler && oldHandler->hasOverlay()) || (currentHandler() && currentHandler()->hasOverlay()))
			ViewportManager::instance().updateViewports();
	}
	else {
		// Redraw viewports if the removed handler used overlays.
		if(handler->hasOverlay())
			ViewportManager::instance().updateViewports();

		_inputHandlerStack.remove(index);
	}
}

/******************************************************************************
* Resets the input mode stack to its initial state on application startup.
******************************************************************************/
void ViewportInputManager::reset()
{
	// Remove all input modes from the stack
	while(currentHandler() != NULL)
		removeInputHandler(currentHandler());

	class DefaultInputMode : public ViewportInputHandler {
	public:
		InputHandlerType handlerType() { return EXCLUSIVE; }
	};

	// Activate default mode
	pushInputHandler(new DefaultInputMode());
}

};
