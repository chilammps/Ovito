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
 * \file ViewportInputHandler.h
 * \brief Contains the definition of the Ovito::ViewportInputHandler.
 */

#ifndef __OVITO_VIEWPORT_INPUT_HANDLER_H
#define __OVITO_VIEWPORT_INPUT_HANDLER_H

#include <core/Core.h>

namespace Ovito {

class Viewport;					// defined in Viewport.h
class ViewportSceneRenderer;	// defined in ViewportSceneRenderer.h

/**
 * \brief Abstract base class for viewport input handlers that process mouse events
 *        in the viewport windows.
 *
 * The ViewportInputManager holds a stack of ViewportInputHandler derived objects.
 * The topmost handler on the stack handles the mouse messages for the viewport
 * windows.
 */
class ViewportInputHandler : public OvitoObject
{
public:

	/// \brief These are the activation behavior types for input handlers.
	enum InputHandlerType {
		NORMAL,				///< The handler is temporarily suspended when another handler becomes active.
		TEMPORARY,			///< The handler is completely removed from the stack when another handler becomes active.
		EXCLUSIVE			///< The stack is cleared before the handler becomes active.
	};

	/// \brief Default constructor.
	ViewportInputHandler() : _temporaryNavMode(NULL) {}

	/// \brief Returns the activation behavior of this input handler.
	/// \return The activation type controls what happens when the handler is activated and deactivated.
	///         The returned value is interpreted by the ViewportInputManager.
	virtual InputHandlerType handlerType() = 0;

	/// \brief Handles double click events for a Viewport.
	/// \param vp The viewport in which the mouse event occurred.
	/// \param event The mouse event.
	///
	/// The default implementation does nothing.
	virtual void mouseDoubleClickEvent(Viewport* vp, QMouseEvent* event) {}

	/// \brief Handles mouse press events for a Viewport.
	/// \param vp The viewport in which the mouse event occurred.
	/// \param event The mouse event.
	///
	/// The default implementation of this method deactivates the
	/// input handler when the user presses the right mouse button.
	/// It also activates temporary viewport navigation modes like
	/// pan, zoom and orbit when the user uses the corresponding
	/// mouse+key combination.
	virtual void mousePressEvent(Viewport* vp, QMouseEvent* event);

	/// \brief Handles mouse release events for a Viewport.
	/// \param vp The viewport in which the mouse event occurred.
	/// \param event The mouse event.
	///
	/// The default implementation deactivates any
	/// temporary viewport navigation mode like pan, zoom and orbit
	/// when they have been activated by the mousePressEvent() method.
	virtual void mouseReleaseEvent(Viewport* vp, QMouseEvent* event);

	/// \brief Handles mouse move events for a Viewport.
	/// \param vp The viewport in which the mouse event occurred.
	/// \param event The mouse event.
	///
	/// The default implementation delegates the event to the
	/// temporary viewport navigation mode like pan, zoom and orbit
	/// when it has been activated in the mousePressEvent() method.
	virtual void mouseMoveEvent(Viewport* vp, QMouseEvent* event);

	/// \brief Handles mouse wheel events for a Viewport.
	/// \param vp The viewport in which the mouse event occurred.
	/// \param event The mouse event.
	///
	/// The default implementation zooms in or out according to the wheel rotation.
	virtual void wheelEvent(Viewport* vp, QWheelEvent* event);

	/// \brief Return the cursor to be used for the viewport windows
	///        while this input handler is active.
	/// \return A cursor.
	///
	/// The default implementation returns the standard arrow cursor.
	///
	/// An input handler can update the current dynamically by calling
	/// updateCursor().
	///
	/// \sa updateCursor()
	virtual QCursor getCursor() {
		return QCursor(Qt::ArrowCursor);
	}

	/// \brief Return the temporary navigation mode if the user is currently using the
	///        middle button or the mouse wheel.
	/// \return The viewport navigation mode that temporarily overrides this
	///         input mode or \c NULL if it is not active.
	///
	/// The default implementation of the onMouseDown() event handler method activates
	/// a temporary navigation mode like zoom, pan or orbit on special mouse/key combinations.
	/// This temporary navigation mode then handles all mouse events instead of this
	/// input handler as long as it is active.
	ViewportInputHandler* temporaryNavigationMode() const { return _temporaryNavMode; }

	/// \brief Activates the given temporary navigation mode.
	void activateTemporaryNavigationMode(ViewportInputHandler* mode);

	/// \brief Indicates whether this input mode renders into the viewports.
	/// \return \c true if the renderOverlay() method has been overridden for this class; \c false otherwise.
	///
	/// Subclasses should override this method to return \c true if they also override the renderOverlay() method.
	/// The default implementation returns \c false.
	///
	/// \sa renderOverlay()
	virtual bool hasOverlay() {
		return (_temporaryNavMode != NULL) ? _temporaryNavMode->hasOverlay() : false;
	}

	/// \brief Lets the input mode render its overlay content in a viewport.
	/// \param vp The viewport into which the mode should render its specific overlay content.
	/// \param renderer The renderer that should be used to display the overlay.
	/// \param isActive Indicates whether this input is currently active. The renderOverlay()
	///                 method is also called for an inactive input mode if it is suspended due to
	///                 one or more modes on top of it on the mode stack.
	///
	/// This method is called by the system every time the viewports are redrawn and this input
	/// mode is on the input mode stack.
	///
	/// The default implementation of this method does nothing. If a subclasses implements this
	/// method then it should also override the hasOverlay() function.
	///
	/// \sa hasOverlay()
	virtual void renderOverlay(Viewport* vp, ViewportSceneRenderer* renderer, bool isActive) {
		if(_temporaryNavMode != NULL)
			_temporaryNavMode->renderOverlay(vp, renderer, isActive);
	}

protected:

	/// \brief This is called by the system after the input handler has
	///        become the active handler.
	///
	/// Implementations of this virtual method in sub-classes should call the base implementation.
	///
	/// \sa ViewportInputManager::pushInputHandler()
	virtual void activated();

	/// \brief This is called by the system after the input handler is
	///        no longer the active handler.
	///
	/// Implementations of this virtual method in sub-classes should call the base implementation.
	///
	/// \sa ViewportInputManager::removeInputHandler()
	virtual void deactivated();

	/// \brief Updates the cursor in the viewport.
	///
	/// This method can be called by an input handler to change the current viewport
	/// cursor dynamically. After calling updateCursor() the system will query
	/// the handler for the new cursor through the getCursor() method.
	///
	/// \sa getCursor()
	void updateCursor();

private:

	/// Contains one of the temporary navigation modes if the user is using the
	/// middle button or the mouse wheel.
	ViewportInputHandler* _temporaryNavMode;

	Q_OBJECT
	OVITO_OBJECT

	friend class ViewportInputManager;
};

};

#endif // __OVITO_VIEWPORT_INPUT_HANDLER_H
