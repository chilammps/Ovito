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
#include <core/animation/AnimManager.h>
#include "AnimationTimeSlider.h"

namespace Ovito {

/******************************************************************************
* The constructor of the AnimationTimeSlider class.
******************************************************************************/
AnimationTimeSlider::AnimationTimeSlider(QWidget* parent) :
	QFrame(parent), _dragPos(-1)
{
	setFrameShape(QFrame::Panel);
	setFrameShadow(QFrame::Sunken);
	setAutoFillBackground(true);

	// Repaint slider if the current animation settings have changed.
	connect(&AnimManager::instance(), SIGNAL(timeChanged(TimeTicks)), SLOT(repaint()));
	connect(&AnimManager::instance(), SIGNAL(timeFormatChanged()), SLOT(update()));
	connect(&AnimManager::instance(), SIGNAL(intervalChanged(TimeInterval)), SLOT(update()));
	connect(&AnimManager::instance(), SIGNAL(animationModeChanged(bool)), SLOT(onAnimationModeChanged(bool)));
}

/******************************************************************************
* Handles paint events.
******************************************************************************/
void AnimationTimeSlider::paintEvent(QPaintEvent* event)
{
	QFrame::paintEvent(event);

#if 0
	// Show slider only if there is more than one animation frame.
	if(ANIM_MANAGER.animationInterval().duration() != 0 || APPLICATION_MANAGER.experimentalMode()) {
		QStylePainter painter(this);
		QStyleOptionButton btnOption;
		btnOption.initFrom(this);
		btnOption.rect = thumbRectangle();
		btnOption.text = ANIM_MANAGER.timeToString(ANIM_MANAGER.time());
		if(ANIM_MANAGER.animationInterval().start() == 0)
			btnOption.text += " / " + ANIM_MANAGER.timeToString(ANIM_MANAGER.animationInterval().end());
		btnOption.state = ((dragPos >= 0) ? QStyle::State_Sunken : QStyle::State_Raised) | QStyle::State_Enabled;
		painter.drawPrimitive(QStyle::PE_PanelButtonCommand, btnOption);
		painter.drawControl(QStyle::CE_PushButtonLabel, btnOption);
	}
#endif
}

/******************************************************************************
* Returns the recommended size for the widget.
******************************************************************************/
QSize AnimationTimeSlider::sizeHint() const
{
#ifndef Q_WS_MAC
	QStyleOptionButton btnOption;
	btnOption.initFrom(this);
	QSize sz = fontMetrics().size(Qt::TextSingleLine, "XXXXXXXXXX");
	return style()->sizeFromContents(QStyle::CT_PushButton, &btnOption, sz, this).expandedTo(QApplication::globalStrut());
#else
	QPushButton* btn = new QPushButton(QString("XXXXXXXXXX"));
	QSize sz = btn->sizeHint();
	delete btn;
	return sz;
#endif
}

/******************************************************************************
* Handles mouse down events.
******************************************************************************/
void AnimationTimeSlider::mousePressEvent(QMouseEvent* event)
{
	QRect thumbRect = thumbRectangle();
	if(thumbRect.contains(event->pos())) {
		_dragPos = event->x() - thumbRect.x();
	}
	else {
		_dragPos = thumbRect.width() / 2;
		mouseMoveEvent(event);
	}
	update();
}

/******************************************************************************
* Handles mouse up events.
******************************************************************************/
void AnimationTimeSlider::mouseReleaseEvent(QMouseEvent* event)
{
	_dragPos = -1;
	update();
}

/******************************************************************************
* Handles mouse move events.
******************************************************************************/
void AnimationTimeSlider::mouseMoveEvent(QMouseEvent* event)
{
	if(_dragPos < 0) return;
	QRect clientRect = rect();
	clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());

	int newPos = event->x() - _dragPos;

#if 0
	int thumbSize = min(clientRect.width() / 3, 70);
	TimeInterval interval = ANIM_MANAGER.animationInterval();
	TimeTicks newTime = (TimeTicks)((qint64)newPos * (qint64)(interval.duration() + 1) / (qint64)(clientRect.width() - thumbSize));

	// limit new time
	newTime = max(newTime, interval.start());
	newTime = min(newTime, interval.end());

	// Snap to frames
	newTime = ANIM_MANAGER.frameToTime(ANIM_MANAGER.timeToFrame(newTime));
	if(newTime == ANIM_MANAGER.time()) return;

	// Set new time
	ANIM_MANAGER.setTime(newTime);

	// Update viewports.
	VIEWPORT_MANAGER.processViewportUpdates();
#endif
}

/******************************************************************************
* Computes the coordinates of the slider thumb.
******************************************************************************/
QRect AnimationTimeSlider::thumbRectangle()
{
	return QRect(0,0,0,0);

#if 0
	if(DATASET_MANAGER.currentSet() == NULL)
		return QRect(0,0,0,0);

	QRect clientRect = rect();
	clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());

	// Position slider
	TimeInterval interval = ANIM_MANAGER.animationInterval();
	TimeTicks value = ANIM_MANAGER.time();
	if(value < interval.start()) value = interval.start();
	if(value > interval.end()) value = interval.end();

	int thumbSize = min(clientRect.width() / 3, 90);
	FloatType percentage = (FloatType)(value - interval.start()) / (FloatType)(interval.duration() + 1);
	int thumbPos = (int)((clientRect.width() - thumbSize) * percentage);
	return QRect(thumbPos + clientRect.x(), clientRect.y(), thumbSize, clientRect.height());
#endif
}

};
