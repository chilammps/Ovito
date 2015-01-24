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

#ifndef __OVITO_VIEWPORT_INPUT_MODE_H
#define __OVITO_VIEWPORT_INPUT_MODE_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(ViewportInput)

/**
 * \brief Abstract base class for viewport input modes that handle mouse input
 *        in the viewports.
 *
 * The ViewportInputManager keeps a stack of ViewportInputMode objects.
 * The topmost handler is the active one and handles all mouse events for the viewports.
 */
class OVITO_CORE_EXPORT ViewportInputMode : public QObject
{
public:

	/// \brief These are the activation behavior types for input modes.
	enum InputModeType {
		NormalMode,				///< The mode is temporarily suspended when another mode becomes active.
		TemporaryMode,			///< The mode is completely removed from the stack when another mode becomes active.
		ExclusiveMode			///< The stack is cleared before the mode becomes active.
	};

	/// \brief Constructor.
	ViewportInputMode(QObject* parent = nullptr) : QObject(parent), _manager(nullptr), _showOrbitCenter(false) {}

	/// \brief Destructor.
	virtual ~ViewportInputMode();

	/// \brief Returns a pointer to the viewport input manager that has a reference to this mode.
	ViewportInputManager* inputManager() const {
		OVITO_ASSERT_MSG(_manager != nullptr, "ViewportInputMode::inputManager()", "Cannot access input manager while mode is not on the input stack.");
		return _manager;
	}

	/// \brief Checks whether this mode is currently active.
	bool isActive() const;

	/// \brief Returns the activation behavior of this input mode.
	/// \return The activation type controls what happens when the mode is activated and deactivated.
	///         The returned value is used by the ViewportInputManager when managing the stack of modes.
	///
	/// The default implementation returns InputModeType::NormalMode.
	virtual InputModeType modeType() { return NormalMode; }

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

	/// \brief Handles double click events for a Viewport.
	/// \param vp The viewport in which the mouse event occurred.
	/// \param event The mouse event.
	virtual void mouseDoubleClickEvent(Viewport* vp, QMouseEvent* event);

	/// \brief Return the mouse cursor shown in the viewport windows
	///        while this input handler is active.
	const QCursor& cursor() { return _cursor; }

	/// \brief Sets the mouse cursor shown in the viewport windows
	///        while this input handler is active.
	void setCursor(const QCursor& cursor);

	/// \brief Activates the given temporary navigation mode.
	///
	/// This method can be overridden by subclasses to prevent the activation of temporary navigation modes.
	virtual void activateTemporaryNavigationMode(ViewportInputMode* navigationMode);

	/// \brief Indicates whether this input mode renders 3d geometry into the viewports.
	/// \return \c true if the renderOverlay3D() method has been overridden for this class; \c false otherwise.
	///
	/// Subclasses should override this method to return \c true if they also override the renderOverlay3D() method.
	/// The default implementation returns \c false.
	virtual bool hasOverlay() { return _showOrbitCenter; }

	/// \brief Lets the input mode render its 3d overlay content in a viewport.
	/// \param vp The viewport into which the mode should render its specific overlay content.
	/// \param renderer The renderer that should be used to display the overlay.
	///
	/// This method is called by the system every time the viewports are redrawn and this input
	/// mode is on the input mode stack.
	///
	/// The default implementation of this method does nothing. If a subclasses implements this
	/// method then it should also override the hasOverlay() function.
	virtual void renderOverlay3D(Viewport* vp, ViewportSceneRenderer* renderer);

	/// \brief Computes the bounding box of the 3d visual viewport overlay rendered by the input mode.
	/// \return The bounding box of the geometry in world coordinates.
	virtual Box3 overlayBoundingBox(Viewport* vp, ViewportSceneRenderer* renderer);

	/// \brief Lets the input mode render its 2d overlay content in a viewport.
	/// \param vp The viewport into which the mode should render its specific overlay content.
	/// \param renderer The renderer that should be used to display the overlay.
	///
	/// This method is called by the system every time the viewports are redrawn and this input
	/// mode is on the input mode stack.
	///
	/// The default implementation of this method does nothing. If a subclasses implements this
	/// method then it should also override the hasOverlay() function.
	virtual void renderOverlay2D(Viewport* vp, ViewportSceneRenderer* renderer) {}

Q_SIGNALS:

	/// \brief This signal is emitted when the input mode has become the active mode or is no longer the active mode.
	void statusChanged(bool isActive);

	/// \brief This signal is emitted when the current curser of this mode has changed.
	void curserChanged(const QCursor& cursor);

protected:

	/// \brief This is called by the system after the input handler has
	///        become the active handler.
	///
	/// Implementations of this virtual method in sub-classes should call the base implementation.
	virtual void activated(bool temporaryActivation);

	/// \brief This is called by the system after the input handler is
	///        no longer the active handler.
	///
	/// Implementations of this virtual method in sub-classes should call the base implementation.
	virtual void deactivated(bool temporary);

private:

	/// Stores a copy of the last mouse-press event.
	std::unique_ptr<QMouseEvent> _lastMousePressEvent;

	/// The cursor shown while this mode is active.
	QCursor _cursor;

	/// The viewport input manager that has a reference to this mode.
	ViewportInputManager* _manager;

	/// This flag indicates that the current camera orbit should be shown in the viewports.
	bool _showOrbitCenter;

	Q_OBJECT

	friend class ViewportInputManager;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_VIEWPORT_INPUT_MODE_H
