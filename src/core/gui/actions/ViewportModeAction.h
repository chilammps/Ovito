////////////////////////////////////////////////////////////////////////////////
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

#ifndef __OVITO_VIEWPORT_MODE_ACTION_H
#define __OVITO_VIEWPORT_MODE_ACTION_H

#include <core/Core.h>
#include <core/viewport/input/ViewportInputManager.h>

namespace Ovito {

/******************************************************************************
* An action that activates a ViewportInputHandler.
******************************************************************************/
class OVITO_CORE_EXPORT ViewportModeAction : public QAction
{
	Q_OBJECT

public:

	/// \brief Initializes the action object.
	/// \param id The unique identifier string of this action object.
	ViewportModeAction(const QString& text, QObject* parent, const OORef<ViewportInputHandler>& inputHandler, const QColor& highlightColor = QColor());

protected Q_SLOTS:

	/// Is called when the active viewport input handler has changed.
	void onInputModeChanged(ViewportInputHandler* oldMode, ViewportInputHandler* newMode);

	/// Is called when the user has triggered the action's state.
	void onActionTriggered(bool checked);

private:

	/// The viewport input handler activated by this action.
	OORef<ViewportInputHandler> _inputHandler;

	/// The highlight color for the button controls.
	QColor _highlightColor;

};

};

#endif // __OVITO_VIEWPORT_MODE_ACTION_H
