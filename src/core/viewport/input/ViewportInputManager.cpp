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
#include <core/viewport/input/ViewportInputMode.h>
#include <core/gui/mainwin/MainWindow.h>

namespace Ovito {

/******************************************************************************
* Initializes the viewport input manager.
******************************************************************************/
ViewportInputManager::ViewportInputManager(MainWindow* mainWindow) : QObject(mainWindow)
{
	class DefaultInputMode : public ViewportInputMode {
	public:
		DefaultInputMode(QObject* parent) : ViewportInputMode(parent) {}
		InputModeType modeType() override { return ExclusiveMode; }
	};
	_defaultMode = new DefaultInputMode(this);
	_zoomMode = new ZoomMode(this);
	_panMode = new PanMode(this);
	_orbitMode = new OrbitMode(this);
	_fovMode = new FOVMode(this);
	_pickOrbitCenterMode = new PickOrbitCenterMode(this);

	// Reset the viewport input manager when a new scene has been loaded.
	connect(&mainWindow->datasetContainer(), &DataSetContainer::dataSetChanged, this, &ViewportInputManager::reset);
}

/******************************************************************************
* Returns the currently active ViewportInputMode that handles the mouse events in viewports.
******************************************************************************/
ViewportInputMode* ViewportInputManager::activeMode()
{
	if(_inputModeStack.empty()) return nullptr;
	return _inputModeStack.back();
}

/******************************************************************************
* Pushes a handler onto the stack and makes it active.
******************************************************************************/
void ViewportInputManager::pushInputMode(ViewportInputMode* newMode)
{
    OVITO_CHECK_POINTER(newMode);

    ViewportInputMode* oldMode = activeMode();
	if(newMode == oldMode) return;

	if(oldMode) {
		if(newMode->modeType() == ViewportInputMode::ExclusiveMode) {
			// Remove all handlers from the stack
			_inputModeStack.clear();
		}
		else if(newMode->modeType() == ViewportInputMode::NormalMode) {
			// Remove all non-exclusive handlers from the stack
			for(int i = _inputModeStack.size(); i--; ) {
				if(_inputModeStack[i]->modeType() != ViewportInputMode::ExclusiveMode) {
					_inputModeStack.remove(i);
				}
			}
		}
		else if(newMode->modeType() == ViewportInputMode::TemporaryMode) {
			// Remove all temporary handlers from the stack.
			if(oldMode->modeType() == ViewportInputMode::TemporaryMode)
				_inputModeStack.pop_back();
		}
	}

	// Put new handler on the stack.
	_inputModeStack.push_back(newMode);
	if(oldMode) oldMode->deactivated();
	newMode->activated();

	Q_EMIT inputModeChanged(oldMode, newMode);
}

/******************************************************************************
* Removes a handler from the stack and deactivates it if
* it is currently active.
******************************************************************************/
void ViewportInputManager::removeInputMode(ViewportInputMode* mode)
{
	OVITO_CHECK_POINTER(mode);

	int index = _inputModeStack.indexOf(mode);
	if(index < 0) return;

	if(index == _inputModeStack.size() - 1) {
		_inputModeStack.remove(index);
		mode->deactivated();
		if(!_inputModeStack.empty())
			activeMode()->activated();
		Q_EMIT inputModeChanged(mode, activeMode());
	}
	else {
		_inputModeStack.remove(index);
	}
}

/******************************************************************************
* Resets the input mode stack to its initial state on application startup.
******************************************************************************/
void ViewportInputManager::reset()
{
	// Remove all input modes from the stack
	while(activeMode())
		removeInputMode(activeMode());

	// Activate default mode
	pushInputMode(_defaultMode);
}

};
