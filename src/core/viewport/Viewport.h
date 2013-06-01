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
 * \file Viewport.h
 * \brief Contains the definition of the Ovito::Viewport class.
 */

#ifndef __OVITO_VIEWPORT_H
#define __OVITO_VIEWPORT_H

#include <core/Core.h>
#include <core/reference/RefMaker.h>
#include "ViewportSettings.h"

namespace Ovito {

class ViewportConfiguration;	// defined in ViewportConfiguration.h

/**
 * \brief A viewport window that displays the current scene.
 */
class Viewport : public RefMaker
{
public:

	/// \brief Constructs a new viewport in the given container widget.
	/// \param parent The parent for the new viewport window.
	Viewport();

	/// \brief Destructor.
	virtual ~Viewport();

    /// \brief Enqueues the viewport for an update.
    ///
	/// Calling this method will redraw the viewport contents unless the viewport is hidden.
	/// This function does not cause an immediate repaint; instead it schedules a
	/// paint event, which is processed execution returns to the main event loop.
	///
	/// To update all viewports at once you should use ViewportManager::updateViewports().
	void updateViewport() { /*update();*/ }

	/// Returns the widget that contains the viewport's rendering window.
	QWidget* widget() const { return _widget; }

	/// Returns the object that stores the settings for this viewport.
	ViewportConfiguration* settings() const { return _settings; }

	/// Indicates whether the rendering of the viewport contents is currently in progress.
	bool isRendering() const { OVITO_ASSERT(false); return false; }

	/// \brief Displays the context menu for the viewport.
	/// \param pos The position in where the context menu should be displayed.
	void showViewportMenu(const QPoint& pos = QPoint(0,0));

	/// \brief Returns a color value for drawing something in the viewport. The user can configure the color for each element.
	/// \param which The enum constant that specifies what type of element to draw.
	/// \return The color that should be used for the given element type.
	static const Color& viewportColor(ViewportSettings::ViewportColor which) {
		return ViewportSettings::getSettings().viewportColor(which);
	}

private:

	/// The widget that contains the viewport's rendering window.
	QWidget* _widget;

	/// This object holds settings of this viewport.
	ReferenceField<ViewportConfiguration> _settings;

	/// The zone in the upper left corner of the viewport where
	/// the context menu can be activated by the user.
	QRect _contextMenuArea;

	/// Flag that indicates that the mouse cursor is currently hovering over the viewport's caption.
	bool _mouseOverCaption;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_settings);
};

};

#endif // __OVITO_VIEWPORT_H
