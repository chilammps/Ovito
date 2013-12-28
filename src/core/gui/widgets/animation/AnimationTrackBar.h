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
#include <core/reference/RefTargetListener.h>
#include <core/animation/controller/Controller.h>
#include "AnimationTimeSlider.h"

namespace Ovito {

/**
 * The track bar displays the animation keys of the selected scene node.
 */
class AnimationTrackBar : public QFrame
{
	Q_OBJECT

public:

	/// Constructor.
	AnimationTrackBar(MainWindow* mainWindow, AnimationTimeSlider* timeSlider, QWidget* parentWindow = nullptr);

protected:

	/// Handles paint events.
	virtual void paintEvent(QPaintEvent* event) override;
	
	/// Returns the recommended size of the widget.
	virtual QSize sizeHint() const override;

	/// Returns the minimum size of the widget.
	virtual QSize minimumSizeHint() const override { return sizeHint(); }
	
	/// Recursive function that finds all controllers in the object graph.
	void findControllers(RefTarget* target);

protected Q_SLOTS:

	/// This is called when new animation settings have been loaded.
	void onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings);

	/// This is called when the current scene node selection has changed.
	void onRebuildControllerList();

	/// is called whenever one of the objects being monitored sends a notification signal.
	void onObjectNotificationEvent(RefTarget* source, ReferenceEvent* event);

private:

	/// Pointer to the animation time slider widget.
	AnimationTimeSlider* _timeSlider;

	/// The current animation settings object.
	AnimationSettings* _animSettings;

	/// This list of animation controllers that are shown in the track bar.
	VectorRefTargetListener<Controller> _controllers;

	/// List of all reference targets in the selected object reference tree.
	VectorRefTargetListener<RefTarget> _objects;

	QMetaObject::Connection _animIntervalChangedConnection;
	QMetaObject::Connection _timeFormatChangedConnection;
	QMetaObject::Connection _timeChangedConnection;
};

};

#endif // __OVITO_ANIMATION_TRACK_BAR_H
