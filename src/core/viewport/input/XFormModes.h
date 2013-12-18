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

#ifndef __OVITO_XFORM_VIEWPORT_MODES_H
#define __OVITO_XFORM_VIEWPORT_MODES_H

#include <core/Core.h>
#include "ViewportInputMode.h"

namespace Ovito {

/******************************************************************************
* Base class for selection, move, rotate and scale modes.
******************************************************************************/
class OVITO_CORE_EXPORT XFormMode : public ViewportInputMode
{
	Q_OBJECT

public:

	/// \brief Returns the activation behavior of this input mode.
	virtual InputModeType modeType() override {
		return isTransformationMode() ? NormalMode : ExclusiveMode;
	}

	/// \brief Handles the mouse down event for the given viewport.
	virtual void mousePressEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Handles the mouse up event for the given viewport.
	virtual void mouseReleaseEvent(Viewport* vp, QMouseEvent* event) override;

	/// \brief Handles the mouse move event for the given viewport.
	virtual void mouseMoveEvent(Viewport* vp, QMouseEvent* event) override;

protected:

	/// Protected constructor.
	XFormMode(QObject* parent, const QString& cursorImagePath) : ViewportInputMode(parent), _viewport(nullptr), _xformCursor(QPixmap(cursorImagePath)) {}

	/// \brief This is called by the system after the input handler is
	///        no longer the active handler.
	virtual void deactivated(bool temporary) override;

protected:

	/// Is called when the transformation operation begins.
	virtual void startXForm() {}

	/// Is repeatedly called during the transformation operation.
	virtual void doXForm(Viewport* vp) {}

	/// Returns true if this is a move, rotate of scale mode.
	/// Returns false if it is a selection mode.
	virtual bool isTransformationMode() const { return true; }

	/// Returns the display name for undoable operations performed by this input mode.
	virtual QString undoDisplayName() = 0;

	/// Applies the current transformation to a set of nodes.
	virtual void applyXForm(const QVector<SceneNode*>& nodeSet, FloatType multiplier) {}

protected:

	/// Mouse position at first click.
	QPointF _startPoint;

	/// The current mouse position
	QPointF _currentPoint;

	/// Indicates that the selection should be cleared when the user releases the mouse button.
	bool _clearSelection;

	/// Indicates if we are currently selecting objects (instead of moving).
	bool _isSelecting;

	/// The current viewport we are working in.
	Viewport* _viewport;

	/// The cursor shown while the mouse cursor is over an object.
	QCursor _xformCursor;
};

/******************************************************************************
* The default input mode for the viewports. This mode lets the user
* select scene nodes.
******************************************************************************/
class OVITO_CORE_EXPORT SelectionMode : public XFormMode
{
	Q_OBJECT

public:

	/// Constructor.
	SelectionMode(QObject* parent) : XFormMode(parent, QStringLiteral(":/core/cursor/editing/cursor_mode_select.png")) {}

protected:

	/// Returns true if this a move, rotate of scale mode.
	/// Returns false if it is a selection mode.
	virtual bool isTransformationMode() const override { return false; }

	/// Returns the display name for undoable operations performed by this input mode.
	virtual QString undoDisplayName() override { return tr("Select"); }
};

/******************************************************************************
* This mode lets the user move scene nodes.
******************************************************************************/
class OVITO_CORE_EXPORT MoveMode : public XFormMode
{
	Q_OBJECT

public:

	/// Constructor.
	MoveMode(QObject* parent) : XFormMode(parent, QStringLiteral(":/core/cursor/editing/cursor_mode_move.png")) {}

protected:

	/// Returns the display name for undoable operations performed by this input mode.
	virtual QString undoDisplayName() override { return tr("Move"); }
};

/******************************************************************************
* This mode lets the user rotate scene nodes.
******************************************************************************/
class OVITO_CORE_EXPORT RotateMode : public XFormMode
{
	Q_OBJECT

public:

	/// Constructor.
	RotateMode(QObject* parent) : XFormMode(parent, QStringLiteral(":/core/cursor/editing/cursor_mode_rotate.png")) {}

protected:

	/// Returns the display name for undoable operations performed by this input mode.
	virtual QString undoDisplayName() override { return tr("Rotate"); }
};

};

#endif // __OVITO_XFORM_VIEWPORT_MODES_H
