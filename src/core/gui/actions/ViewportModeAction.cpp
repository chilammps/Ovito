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
#include <core/gui/mainwin/MainWindow.h>
#include <core/viewport/input/ViewportInputManager.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(ViewportInput)

/******************************************************************************
* Initializes the action object.
******************************************************************************/
ViewportModeAction::ViewportModeAction(MainWindow* mainWindow, const QString& text, QObject* parent, ViewportInputMode* inputMode, const QColor& highlightColor)
	: QAction(text, parent), _inputMode(inputMode), _highlightColor(highlightColor), _viewportInputManager(*mainWindow->viewportInputManager())
{
	OVITO_CHECK_POINTER(inputMode);

	setCheckable(true);
	setChecked(inputMode->isActive());

	connect(inputMode, &ViewportInputMode::statusChanged, this, &ViewportModeAction::setChecked);
	connect(this, &ViewportModeAction::triggered, this, &ViewportModeAction::onActionTriggered);
}

/******************************************************************************
* Is called when the user has triggered the action's state.
******************************************************************************/
void ViewportModeAction::onActionTriggered(bool checked)
{
	// Activate/deactivate the input mode.
	if(checked && !_inputMode->isActive()) {
		_viewportInputManager.pushInputMode(_inputMode);
	}
	else if(!checked) {
		if(_inputMode->modeType() != ViewportInputMode::ExclusiveMode)
			_viewportInputManager.removeInputMode(_inputMode);
		else
			// Make sure that an exclusive input mode cannot be deactivated by the user.
			setChecked(true);
	}
}

/******************************************************************************
* Create a push button that activates this action.
******************************************************************************/
QPushButton* ViewportModeAction::createPushButton(QWidget* parent)
{
	QPushButton* button = new QPushButton(text(), parent);
	button->setCheckable(true);
	button->setChecked(isChecked());

#ifndef Q_OS_MACX
	if(_highlightColor.isValid())
		button->setStyleSheet("QPushButton:checked { background-color: " + _highlightColor.name() + " }");
	else
		button->setStyleSheet("QPushButton:checked { background-color: moccasin; }");
#endif

	connect(this, &ViewportModeAction::toggled, button, &QPushButton::setChecked);
	connect(button, &QPushButton::clicked, this, &ViewportModeAction::trigger);
	return button;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
