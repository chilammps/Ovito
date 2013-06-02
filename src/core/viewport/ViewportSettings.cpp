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
#include <core/viewport/ViewportSettings.h>

namespace Ovito {

/// The current settings record.
ViewportSettings ViewportSettings::_currentSettings;

/// Indicates whether the settings have already been loaded from the application's settings store.
bool ViewportSettings::_settingsLoaded = false;

/******************************************************************************
* Returns a reference to the current global settings object.
******************************************************************************/
const ViewportSettings& ViewportSettings::getSettings()
{
	if(!_settingsLoaded) {
		QSettings settingsStore;
		settingsStore.beginGroup("core/viewport/");
		_currentSettings.load(settingsStore);
		settingsStore.endGroup();
		_settingsLoaded = true;
	}
	return _currentSettings;
}

/******************************************************************************
* Replaces the current global settings with new values.
******************************************************************************/
void ViewportSettings::setSettings(const ViewportSettings& settings)
{
	_currentSettings = settings;

	QSettings settingsStore;
	settingsStore.beginGroup("core/viewport/");
	_currentSettings.save(settingsStore);
	settingsStore.endGroup();
}

/******************************************************************************
* Default constructor.
******************************************************************************/
ViewportSettings::ViewportSettings()
{
	_upDirection = Z_AXIS;
	restoreDefaultViewportColors();
}

/******************************************************************************
* Sets all viewport colors to their default values.
******************************************************************************/
void ViewportSettings::restoreDefaultViewportColors()
{
	_viewportColors[COLOR_VIEWPORT_BKG] = Color(0.0f, 0.0f, 0.0f);
	_viewportColors[COLOR_GRID] = Color(0.5f, 0.5f, 0.5f);
	_viewportColors[COLOR_GRID_INTENS] = Color(0.6f, 0.6f, 0.6f);
	_viewportColors[COLOR_GRID_AXIS] = Color(0.7f, 0.7f, 0.7f);
	_viewportColors[COLOR_VIEWPORT_CAPTION] = Color(1.0f, 1.0f, 1.0f);
	_viewportColors[COLOR_ACTIVE_VIEWPORT_CAPTION] = Color(0.7f, 0.7f, 1.0f);
	_viewportColors[COLOR_SELECTION] = Color(1.0f, 1.0f, 1.0f);
	_viewportColors[COLOR_ACTIVE_AXIS] = Color(1.0f, 0.0f, 0.0f);
	_viewportColors[COLOR_INACTIVE_AXIS] = Color(0.0f, 0.0f, 0.0f);
	_viewportColors[COLOR_VIEWPORT_BORDER] = Color(0.5f, 0.5f, 0.5f);
	_viewportColors[COLOR_ACTIVE_VIEWPORT_BORDER] = Color(1.0f, 1.0f, 0.0f);
	_viewportColors[COLOR_SNAPPING_MARKER] = Color(0.8f, 1.0f, 1.0f);
	_viewportColors[COLOR_ANIMATION_MODE] = Color(1.0f, 0.0f, 0.0f);
	_viewportColors[COLOR_RENDER_FRAME] = Color(0.0f, 1.0f, 0.0f);
	_viewportColors[COLOR_CAMERAS] = Color(0.5f, 0.5f, 1.0f);
	_viewportColors[COLOR_LIGHTS] = Color(1.0f, 1.0f, 0.0f);
}

/******************************************************************************
* Returns color values for drawing in the viewports.
******************************************************************************/
const Color& ViewportSettings::viewportColor(ViewportColor which) const
{
	OVITO_ASSERT(which >= 0 && which < sizeof(_viewportColors)/sizeof(_viewportColors[0]));
	return _viewportColors[which];
}

/******************************************************************************
* Sets color values for drawing in the viewports.
******************************************************************************/
void ViewportSettings::setViewportColor(ViewportColor which, const Color& clr)
{
	OVITO_ASSERT(which >= 0 && which < sizeof(_viewportColors)/sizeof(_viewportColors[0]));
	_viewportColors[which] = clr;
}

/******************************************************************************
* Returns the rotation axis to be used with orbit mode.
******************************************************************************/
Vector3 ViewportSettings::upVector() const
{
	switch(_upDirection) {
	case X_AXIS: return Vector3(1,0,0);
	case Y_AXIS: return Vector3(0,1,0);
	case Z_AXIS: return Vector3(0,0,1);
	default: return Vector3(0,0,1);
	}
}

/******************************************************************************
* Returns a matrix that transforms the default coordinate system
* (with Z being the "up" direction) to the orientation given by the
* current "up" vector.
******************************************************************************/
Matrix3 ViewportSettings::coordinateSystemOrientation() const
{
	switch(_upDirection) {
	case X_AXIS: return Matrix3(Vector3(0,1,0), Vector3(0,0,1), Vector3(1,0,0));
	case Y_AXIS: return Matrix3(Vector3(-1,0,0), Vector3(0,0,1), Vector3(0,1,0));
	case Z_AXIS:
	default:
		return Matrix3::Identity();
	}
}

/******************************************************************************
* Loads the settings from the given settings store.
******************************************************************************/
void ViewportSettings::load(QSettings& store)
{
	_upDirection = (UpDirection)store.value("UpDirection", qVariantFromValue((int)_upDirection)).toInt();
	int arraySize = store.beginReadArray("colors");
	for(int i = 0; i < NUMBER_OF_COLORS && i < arraySize; i++) {
		store.setArrayIndex(i);
		_viewportColors[i].r() = store.value("R").value<FloatType>();
		_viewportColors[i].g() = store.value("G").value<FloatType>();
		_viewportColors[i].b() = store.value("B").value<FloatType>();
	}
	store.endArray();
}

/******************************************************************************
* Saves the settings to the given settings store.
******************************************************************************/
void ViewportSettings::save(QSettings& store) const
{
	store.setValue("UpDirection", (int)_upDirection);
	store.beginWriteArray("colors");
	for(int i = 0; i < NUMBER_OF_COLORS; i++) {
		store.setArrayIndex(i);
		store.setValue("R", _viewportColors[i].r());
		store.setValue("G", _viewportColors[i].g());
		store.setValue("B", _viewportColors[i].b());
	}
	store.endArray();
}

};
