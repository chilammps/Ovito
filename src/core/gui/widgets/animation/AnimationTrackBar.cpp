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
#include <core/animation/AnimationSettings.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/dataset/DataSetContainer.h>
#include <core/gui/mainwin/MainWindow.h>
#include "AnimationTrackBar.h"

namespace Ovito {

using namespace std;

/******************************************************************************
* The constructor of the AnimationTrackBar class.
******************************************************************************/
AnimationTrackBar::AnimationTrackBar(MainWindow* mainWindow, QWidget* parent) :
	QFrame(parent)
{
	setFrameShape(QFrame::Panel);
	setFrameShadow(QFrame::Sunken);
	setAutoFillBackground(true);
	setMouseTracking(true);

	connect(&mainWindow->datasetContainer(), &DataSetContainer::animationSettingsReplaced, this, &AnimationTrackBar::onAnimationSettingsReplaced);
}

/******************************************************************************
* This is called when new animation settings have been loaded.
******************************************************************************/
void AnimationTrackBar::onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings)
{
	disconnect(_animIntervalChangedConnection);
	disconnect(_timeFormatChangedConnection);
	disconnect(_timeChangedConnection);
	_animSettings = newAnimationSettings;
	if(newAnimationSettings) {
		_animIntervalChangedConnection = connect(newAnimationSettings, &AnimationSettings::intervalChanged, this, (void (AnimationTrackBar::*)())&AnimationTrackBar::update);
		_timeFormatChangedConnection = connect(newAnimationSettings, &AnimationSettings::timeFormatChanged, this, (void (AnimationTrackBar::*)())&AnimationTrackBar::update);
		_timeChangedConnection = connect(newAnimationSettings, &AnimationSettings::timeChanged, this, (void (AnimationTrackBar::*)())&AnimationTrackBar::repaint);
	}
	update();
}

/******************************************************************************
* Handles paint events.
******************************************************************************/
void AnimationTrackBar::paintEvent(QPaintEvent* event)
{
	QFrame::paintEvent(event);

	// Paint track bar only if there is more than one animation frame.
	if(!_animSettings) return;
	int numFrames = (int)(_animSettings->animationInterval().duration() / _animSettings->ticksPerFrame()) + 1;
	if(numFrames <= 1) return;

	QStylePainter painter(this);

	QRect clientRect = frameRect();
	clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
}

/******************************************************************************
* Returns the recommended size for the widget.
******************************************************************************/
QSize AnimationTrackBar::sizeHint() const
{
	return QSize(QFrame::sizeHint().width(), fontMetrics().height() * 2 + frameWidth() * 2);
}

};
