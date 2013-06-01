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
 * \file ViewportSettings.h
 * \brief Contains the definition of the Ovito::ViewportSettings class.
 */

#ifndef __OVITO_GLOBAL_VIEWPORT_SETTINGS_H
#define __OVITO_GLOBAL_VIEWPORT_SETTINGS_H

#include <core/Core.h>

namespace Ovito {

/**
 * \brief Stores general settings related to the viewports.
 */
class ViewportSettings
{
public:

	/// Standard colors for drawing several things in the viewport.
	enum ViewportColor {
		COLOR_VIEWPORT_BKG,				//< The viewport background color
		COLOR_GRID,						//< The color of the minor construction grid lines
		COLOR_GRID_INTENS,				//< The color of the major construction grid lines
		COLOR_GRID_AXIS,				//< The color of the construction grid axis lines
		COLOR_VIEWPORT_CAPTION,			//< The color used to render the viewport caption
		COLOR_ACTIVE_VIEWPORT_CAPTION,	//< The color used to render the viewport caption if the mouse cursor is hovering over it.
		COLOR_SELECTION,				//< The color used for selected objects in wireframe mode
		COLOR_ACTIVE_AXIS,				//< The color used to indicate the active axis in the axis tripod
		COLOR_INACTIVE_AXIS,			//< The color used to indicate the inactive axes in the axis tripod
		COLOR_VIEWPORT_BORDER,			//< The color used to draw the border of inactive viewports
		COLOR_ACTIVE_VIEWPORT_BORDER,	//< The color used to draw the border of the active viewport
		COLOR_SNAPPING_MARKER,			//< The color used to render snapping markers
		COLOR_ANIMATION_MODE,			//< The color used to indicate that the animation mode is active
		COLOR_RENDER_FRAME,				//< The color used to draw the rendering frame around the viewport
		COLOR_CAMERAS,					//< The color used to render camera dummies in the viewports
		COLOR_LIGHTS,					//< The color used to render light dummies in the viewports

		NUMBER_OF_COLORS
	};
	Q_ENUMS(ViewportColor)

	/// Possible settings for the viewport rotation axis, which defines
	/// the up direction for the orbit input mode.
	enum RotationAxisType {
		ROTATION_X_AXIS, //< Rotate around X axis
		ROTATION_Y_AXIS, //< Rotate around Y axis
		ROTATION_Z_AXIS, //< Rotate around Z axis (the default)
	};
	Q_ENUMS(RotationAxisType)

public:

	/// Default constructor. Initializes all settings to their default values.
	ViewportSettings();

	/// \brief Returns a color value for drawing something in the viewports.
	/// \param which The enum constant that specifies what to draw.
	/// \return The color that should be used for the given element.
	const Color& viewportColor(ViewportColor which) const;

	/// \brief Sets the color for drawing something in the viewports.
	/// \param which The enum constant that specifies what to draw.
	/// \param color The color that should be used for the given element.
	void setViewportColor(ViewportColor which, const Color& color);

	/// \brief Sets all viewport colors to their default values.
	void restoreDefaultViewportColors();

	/// Returns the rotation axis to be used with orbit mode.
	Vector3 rotationAxis() const;

	/// Returns the selected rotation axis type.
	RotationAxisType rotationAxisType() const { return _rotationAxisType; }

	/// Sets the rotation axis type.
	void setRotationAxisType(RotationAxisType t) { _rotationAxisType = t; }

	/// Loads the settings from the given settings store.
	void load(QSettings& store);

	/// Saves the settings to the given settings store.
	void save(QSettings& store) const;

	/// Returns a (read-only) reference to the current global settings object.
	static const ViewportSettings& getSettings();

	/// Replaces the current global settings with new values.
	static void setSettings(const ViewportSettings& settings);

private:

	/// The colors for viewport drawing.
	Color _viewportColors[NUMBER_OF_COLORS];

	/// The selected rotation axis type for orbit mode.
	RotationAxisType _rotationAxisType;

	/// The current settings record.
	static ViewportSettings _currentSettings;

	/// Indicates whether the settings have already been loaded from the application's settings store.
	static bool _settingsLoaded;
};

};

Q_DECLARE_METATYPE(Ovito::ViewportSettings::ViewportColor)
Q_DECLARE_METATYPE(Ovito::ViewportSettings::RotationAxisType)
Q_DECLARE_TYPEINFO(Ovito::ViewportSettings::ViewportColor, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ViewportSettings::RotationAxisType, Q_PRIMITIVE_TYPE);


#endif	// __OVITO_VIEWPORT_SETTINGS_H
