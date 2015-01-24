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

#ifndef __OVITO_VIEWPORT_INPUT_MANAGER_H
#define __OVITO_VIEWPORT_INPUT_MANAGER_H

#include <core/Core.h>
#include "ViewportInputMode.h"
#include "NavigationModes.h"
#include "XFormModes.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(ViewportInput)

/**
 * \brief Manages a stack of viewport input handlers.
 */
class OVITO_CORE_EXPORT ViewportInputManager : public QObject
{
	Q_OBJECT

public:

	/// \brief Constructor.
	ViewportInputManager(MainWindow* mainWindow);

	/// Destructor.
	virtual ~ViewportInputManager();

	/// Returns the associated main window.
	MainWindow* mainWindow() const { return reinterpret_cast<MainWindow*>(parent()); }

	/// \brief Returns the currently active ViewportInputMode that handles the mouse events in viewports.
	/// \return The mode that is responsible for mouse event handling. Can be \c NULL when the stack is empty.
	ViewportInputMode* activeMode();

	/// \brief Returns the stack of input modes.
	/// \return The stack of input modes. The topmost mode is the active one.
	const QVector<ViewportInputMode*>& stack() { return _inputModeStack; }

	/// \brief Pushes an input mode onto the stack and makes it active.
	/// \param mode The mode to be made active.
	/// \param temporary A flag passed to the input mode that indicates whether the activation is only temporary.
	void pushInputMode(ViewportInputMode* mode, bool temporary = false);

	/// \brief Removes an input mode from the stack and deactivates it if it is currently active.
	/// \param mode The mode to remove from the stack.
	void removeInputMode(ViewportInputMode* mode);

	/// \brief Returns the zoom input mode.
	ZoomMode* zoomMode() const { return _zoomMode; }

	/// \brief Returns the pan input mode.
	PanMode* panMode() const { return _panMode; }

	/// \brief Returns the orbit input mode.
	OrbitMode* orbitMode() const { return _orbitMode; }

	/// \brief Returns the FOV input mode.
	FOVMode* fovMode() const { return _fovMode; }

	/// \brief Returns the pick orbit center input mode.
	PickOrbitCenterMode* pickOrbitCenterMode() const { return _pickOrbitCenterMode; }

	/// \brief Returns the scene node selection mode.
	SelectionMode* selectionMode() const { return _selectionMode; }

	/// \brief Returns the scene node translation mode.
	MoveMode* moveMode() const { return _moveMode; }

	/// \brief Returns the scene node rotation mode.
	RotateMode* rotateMode() const { return _rotateMode; }

public Q_SLOTS:

	/// \brief Resets the input mode stack to its default state.
	///
	/// All input mode are removed from the stack and a default input mode
	/// is activated.
	void reset();

Q_SIGNALS:

	/// \brief This signal is sent when the active viewport input mode has changed.
	/// \param oldMode The previous input handler (can be \c NULL).
	/// \param newMode The new input handler that is now active (can be \c NULL).
	void inputModeChanged(ViewportInputMode* oldMode, ViewportInputMode* newMode);

private:

	/// Stack of input modes. The topmost entry is the active one.
	QVector<ViewportInputMode*> _inputModeStack;

	/// The default viewport input mode.
	ViewportInputMode* _defaultMode;

	/// The zoom input mode.
	ZoomMode* _zoomMode;

	/// The pan input mode.
	PanMode* _panMode;

	/// The orbit input mode.
	OrbitMode* _orbitMode;

	/// The FOV input mode.
	FOVMode* _fovMode;

	/// The pick orbit center input mode.
	PickOrbitCenterMode* _pickOrbitCenterMode;

	/// The default scene node selection mode.
	SelectionMode* _selectionMode;

	/// The scene node translation mode.
	MoveMode* _moveMode;

	/// The scene node rotation mode.
	RotateMode* _rotateMode;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_VIEWPORT_INPUT_MANAGER_H
