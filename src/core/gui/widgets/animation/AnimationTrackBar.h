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

#ifndef __OVITO_ANIMATION_TRACK_BAR_H
#define __OVITO_ANIMATION_TRACK_BAR_H

#include <core/Core.h>

namespace Ovito {

/**
 * The track bar displays the animation keys of the selected scene node.
 */
class AnimationTrackBar : public QFrame
{
	Q_OBJECT

public:

	/// Constructor.
	AnimationTrackBar(MainWindow* mainWindow, QWidget* parentWindow = nullptr);

protected:

	/// Handles paint events.
	virtual void paintEvent(QPaintEvent* event) override;
	
	/// Returns the recommended size of the widget.
	virtual QSize sizeHint() const override;

	/// Returns the minimum size of the widget.
	virtual QSize minimumSizeHint() const override { return sizeHint(); }
	
protected Q_SLOTS:

	/// This is called when new animation settings have been loaded.
	void onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings);

private:

	/// The current animation settings object.
	AnimationSettings* _animSettings;

	QMetaObject::Connection _animIntervalChangedConnection;
	QMetaObject::Connection _timeFormatChangedConnection;
	QMetaObject::Connection _timeChangedConnection;
};

};

#endif // __OVITO_ANIMATION_TRACK_BAR_H
