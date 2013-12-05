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

#ifndef __OVITO_ANIMATION_TIME_SLIDER_H
#define __OVITO_ANIMATION_TIME_SLIDER_H

#include <core/Core.h>

namespace Ovito {

/**
 * This slider component controls the current scene time.
 */
class AnimationTimeSlider : public QFrame
{
	Q_OBJECT

public:

	/// Constructor.
	AnimationTimeSlider(MainWindow* mainWindow, QWidget* parentWindow = nullptr);

protected:

	/// Handles paint events.
	virtual void paintEvent(QPaintEvent* event) override;
	
	/// Handles mouse down events.
	virtual void mousePressEvent(QMouseEvent* event) override;

	/// Handles mouse up events.
	virtual void mouseReleaseEvent(QMouseEvent* event) override;

	/// Handles mouse move events.
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	
	/// Returns the recommended size of the widget.
	virtual QSize sizeHint() const override;

	/// Returns the minimum size of the widget.
	virtual QSize minimumSizeHint() const override { return sizeHint(); }
	
protected Q_SLOTS:

	/// This is called when new animation settings have been loaded.
	void onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings);

	/// Is called whenever the Auto Key mode is activated or deactivated.
	void onAutoKeyModeChanged(bool active);

private:

	/// Computes the current position of the slider thumb.
	QRect thumbRectangle();

	/// Computes the width of the thumb.
	int thumbWidth() const;
	
	/// The dragging start position.
	int _dragPos;

	/// The default palette used to the draw the time slide background.
	QPalette _normalPalette;

	/// The color palette used to the draw the time slide background when Auto Key mode is active.
	QPalette _autoKeyModePalette;

	/// The main window.
	MainWindow* _mainWindow;

	/// The current animation settings object.
	OORef<AnimationSettings> _animSettings;

	QMetaObject::Connection _autoKeyModeChangedConnection;
	QMetaObject::Connection _animIntervalChangedConnection;
	QMetaObject::Connection _timeFormatChangedConnection;
	QMetaObject::Connection _timeChangedConnection;
};

};

#endif // __OVITO_ANIMATION_TIME_SLIDER_H
