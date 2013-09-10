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

/**
 * \file ViewportInputManager.h
 * \brief Contains the definition of the Ovito::ViewportInputManager class.
 */

#ifndef __OVITO_VIEWPORT_INPUT_MANAGER_H
#define __OVITO_VIEWPORT_INPUT_MANAGER_H

#include <core/Core.h>
#include "ViewportInputHandler.h"

namespace Ovito {

class Viewport; // defined in Viewport.h

/**
 * \brief Manages a stack of viewport input handlers.
 *
 * This is a singleton class with only one predefined instance of this class.
 * You can access the instance of this class using the VIEWPORT_INPUT_MANAGER macro.
 *
 * \author Alexander Stukowski
 * \sa ViewportInputHandler
 */
class OVITO_CORE_EXPORT ViewportInputManager : public QObject
{
	Q_OBJECT

public:

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the ViewportInputManager singleton class.
	inline static ViewportInputManager& instance() {
		OVITO_ASSERT_MSG(_instance != nullptr, "ViewportInputManager::instance", "Singleton object is not initialized yet.");
		return *_instance;
	}

	/// \brief Returns the currently active ViewportInputHandler that handles the mouse events in viewports.
	/// \return The handler implementation that is responsible for mouse event processing. Can be \c NULL when the handler stack is empty.
	ViewportInputHandler* currentHandler();

	/// \brief Returns the current stack of input handlers.
	/// \return The stack of input handlers. The topmost handler is the active one.
	const QVector<OORef<ViewportInputHandler>>& stack() { return _inputHandlerStack; }

	/// \brief Pushes a handler onto the stack and makes it active.
	/// \param handler The handler to be made active.
	/// \sa removeInputHandler()
	void pushInputHandler(const OORef<ViewportInputHandler>& handler);

	/// \brief Removes a handler from the stack and deactivates it if it is currently active.
	/// \param handler The handler to remove from the stack. It is not deleted by this method.
	/// \sa pushInputHandler()
	void removeInputHandler(ViewportInputHandler* handler);

	/// Returns true if the singleton instance of this class has been created and initialized.
	static bool isInitialized() { return _instance != nullptr; }

public Q_SLOTS:

	/// \brief Resets the input mode stack to its initial state on application startup.
	///
	/// All input handlers are removed from the handler stack and a default input handler
	/// is activated.
	void reset();

	/// \brief Updates the mouse cursor displayed in the viewports.
	void updateViewportCursor();

Q_SIGNALS:

	/// \brief This signal is sent when the active viewport input mode has changed.
	/// \param oldMode The previous input handler (can be \c NULL).
	/// \param newMode The new input handler that is now active (can be \c NULL).
	void inputModeChanged(ViewportInputHandler* oldMode, ViewportInputHandler* newMode);

private:

	/// Stack of input handlers. The topmost entry is the active one.
	QVector<OORef<ViewportInputHandler>> _inputHandlerStack;

private:

	/// Private constructor.
	/// This is a singleton class; no public instances are allowed.
	ViewportInputManager();

	/// Create the singleton instance of this class.
	static void initialize() { _instance = new ViewportInputManager(); }

	/// Deletes the singleton instance of this class.
	static void shutdown() { delete _instance; _instance = nullptr; }

	/// The singleton instance of this class.
	static ViewportInputManager* _instance;

	friend class Application;
};

};

#endif // __OVITO_VIEWPORT_INPUT_MANAGER_H
