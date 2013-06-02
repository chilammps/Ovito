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
#include <core/gui/actions/ViewportModeAction.h>

namespace Ovito {

/******************************************************************************
* Initializes the action object.
******************************************************************************/
ViewportModeAction::ViewportModeAction(const QString& text, QObject* parent, const OORef<ViewportInputHandler>& inputHandler, const QColor& highlightColor)
	: QAction(text, parent), _inputHandler(inputHandler), _highlightColor(highlightColor)
{
	OVITO_CHECK_OBJECT_POINTER(inputHandler);

	setCheckable(true);
	setChecked(ViewportInputManager::instance().currentHandler() == inputHandler);

	connect(&ViewportInputManager::instance(), SIGNAL(inputModeChanged(ViewportInputHandler*, ViewportInputHandler*)), this, SLOT(onInputModeChanged(ViewportInputHandler*, ViewportInputHandler*)));
	connect(this, SIGNAL(triggered(bool)), this, SLOT(onActionTriggered(bool)));
}

/******************************************************************************
* Is called when the user has triggered the action's state.
******************************************************************************/
void ViewportModeAction::onActionTriggered(bool checked)
{
	// Activate/deactivate the input mode.
	OVITO_CHECK_OBJECT_POINTER(_inputHandler);
	if(checked && ViewportInputManager::instance().currentHandler() != _inputHandler) {
		ViewportInputManager::instance().pushInputHandler(_inputHandler);
	}
	else if(!checked) {
		if(_inputHandler->handlerType() != ViewportInputHandler::EXCLUSIVE)
			ViewportInputManager::instance().removeInputHandler(_inputHandler.get());
		else
			// Make sure that an exclusive input mode cannot be deactivated by the user.
			setChecked(true);
	}
}

/******************************************************************************
* Is called when the active viewport input handler has changed.
******************************************************************************/
void ViewportModeAction::onInputModeChanged(ViewportInputHandler* oldMode, ViewportInputHandler* newMode)
{
	if(oldMode == _inputHandler || newMode == _inputHandler) {
		setChecked(ViewportInputManager::instance().currentHandler() == _inputHandler);
	}
}

};
