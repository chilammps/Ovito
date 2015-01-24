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
#include <core/viewport/Viewport.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(View)

/// The current settings record.
Q_GLOBAL_STATIC(ViewportSettings, _currentViewportSettings);

/******************************************************************************
* Assignment.
******************************************************************************/
void ViewportSettings::assign(const ViewportSettings& other)
{
	_viewportColors = other._viewportColors;
	_upDirection = other._upDirection;
	_restrictVerticalRotation = other._restrictVerticalRotation;
	_viewportFont = other._viewportFont;

	Q_EMIT settingsChanged(this);
}

/******************************************************************************
* Returns a reference to the current global settings object.
******************************************************************************/
ViewportSettings& ViewportSettings::getSettings()
{
	/// Indicates whether the settings have already been loaded from the application's settings store.
	static bool settingsLoaded = false;

	if(!settingsLoaded) {
		QSettings settingsStore;
		settingsStore.beginGroup("core/viewport/");
		_currentViewportSettings->load(settingsStore);
		settingsStore.endGroup();
		settingsLoaded = true;
	}
	return *_currentViewportSettings;
}

/******************************************************************************
* Replaces the current global settings with new values.
******************************************************************************/
void ViewportSettings::setSettings(const ViewportSettings& settings)
{
	_currentViewportSettings->assign(settings);

	QSettings settingsStore;
	settingsStore.beginGroup("core/viewport/");
	_currentViewportSettings->save(settingsStore);
	settingsStore.endGroup();
}

/******************************************************************************
* Default constructor.
******************************************************************************/
ViewportSettings::ViewportSettings() :
	_viewportFont("Helvetica")
{
	_upDirection = Z_AXIS;
	_restrictVerticalRotation = true;
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
	_viewportColors[COLOR_SELECTION] = Color(1.0f, 1.0f, 1.0f);
	_viewportColors[COLOR_UNSELECTED] = Color(0.6f, 0.6f, 1.0f);
	_viewportColors[COLOR_ACTIVE_VIEWPORT_BORDER] = Color(1.0f, 1.0f, 0.0f);
	_viewportColors[COLOR_ANIMATION_MODE] = Color(1.0f, 0.0f, 0.0f);
	_viewportColors[COLOR_CAMERAS] = Color(0.5f, 0.5f, 1.0f);
}

/******************************************************************************
* Returns color values for drawing in the viewports.
******************************************************************************/
const Color& ViewportSettings::viewportColor(ViewportColor which) const
{
	OVITO_ASSERT(which >= 0 && which < _viewportColors.size());
	return _viewportColors[which];
}

/******************************************************************************
* Sets color values for drawing in the viewports.
******************************************************************************/
void ViewportSettings::setViewportColor(ViewportColor which, const Color& clr)
{
	OVITO_ASSERT(which >= 0 && which < _viewportColors.size());
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
	_restrictVerticalRotation = store.value("RestrictVerticalRotation", qVariantFromValue(_restrictVerticalRotation)).toBool();
	store.beginGroup("Colors");
	QMetaEnum colorEnum;
	for(int i = 0; i < ViewportSettings::staticMetaObject.enumeratorCount(); i++) {
		if(qstrcmp(ViewportSettings::staticMetaObject.enumerator(i).name(), "ViewportColor") == 0) {
			colorEnum = ViewportSettings::staticMetaObject.enumerator(i);
			break;
		}
	}
	OVITO_ASSERT(colorEnum.isValid());
	for(const QString& key : store.childKeys()) {
		QColor c = store.value(key).value<QColor>();
		bool ok;
		int index = colorEnum.keyToValue(key.toLatin1().constData(), &ok);
		if(ok && index >= 0 && index < NUMBER_OF_COLORS) {
			_viewportColors[index] = Color(c);
		}
	}
	store.endGroup();
}

/******************************************************************************
* Saves the settings to the given settings store.
******************************************************************************/
void ViewportSettings::save(QSettings& store) const
{
	store.setValue("UpDirection", (int)_upDirection);
	store.setValue("RestrictVerticalRotation", _restrictVerticalRotation);
	store.remove("Colors");
	store.beginGroup("Colors");
	QMetaEnum colorEnum;
	for(int i = 0; i < ViewportSettings::staticMetaObject.enumeratorCount(); i++) {
		if(qstrcmp(ViewportSettings::staticMetaObject.enumerator(i).name(), "ViewportColor") == 0) {
			colorEnum = ViewportSettings::staticMetaObject.enumerator(i);
			break;
		}
	}
	OVITO_ASSERT(colorEnum.isValid());
    for(size_t i = 0; i < _viewportColors.size(); i++) {
		store.setValue(colorEnum.key(i), QVariant::fromValue((QColor)_viewportColors[i]));
	}
	store.endGroup();
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
