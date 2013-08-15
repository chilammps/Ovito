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
#include <core/viewport/ViewportManager.h>
#include <core/dataset/DataSetManager.h>
#include "AnimationTimeSlider.h"

namespace Ovito {

using namespace std;

/******************************************************************************
* The constructor of the AnimationTimeSlider class.
******************************************************************************/
AnimationTimeSlider::AnimationTimeSlider(QWidget* parent) :
	QFrame(parent), _dragPos(-1)
{
	setFrameShape(QFrame::Panel);
	setFrameShadow(QFrame::Sunken);
	setAutoFillBackground(true);
	setMouseTracking(true);

	// Repaint slider if the current animation settings have changed.
	connect(&AnimManager::instance(), SIGNAL(timeChanged(TimePoint)), SLOT(repaint()));
	connect(&AnimManager::instance(), SIGNAL(timeFormatChanged()), SLOT(update()));
	connect(&AnimManager::instance(), SIGNAL(intervalChanged(TimeInterval)), SLOT(update()));
	connect(&AnimManager::instance(), SIGNAL(animationModeChanged(bool)), SLOT(update()));
}

/******************************************************************************
* Handles paint events.
******************************************************************************/
void AnimationTimeSlider::paintEvent(QPaintEvent* event)
{
	QFrame::paintEvent(event);

	// Show slider only if there is more than one animation frame.
	int numFrames = (int)(AnimManager::instance().animationInterval().duration() / AnimManager::instance().ticksPerFrame()) + 1;
	if(numFrames > 1) {
		QStylePainter painter(this);

		QRect clientRect = frameRect();
		clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
		int thumbWidth = this->thumbWidth();
		int clientWidth = clientRect.width() - thumbWidth;
		int firstFrame = AnimManager::instance().timeToFrame(AnimManager::instance().animationInterval().start());
		int lastFrame = AnimManager::instance().timeToFrame(AnimManager::instance().animationInterval().end());
		int labelWidth = painter.fontMetrics().boundingRect(QString::number(lastFrame)).width();
		int nticks = std::min(clientWidth / (labelWidth + 20), numFrames);
		int ticksevery = numFrames / std::max(nticks, 1);
		if(ticksevery <= 1) ticksevery = ticksevery;
		else if(ticksevery <= 5) ticksevery = 5;
		else if(ticksevery <= 10) ticksevery = 10;
		else if(ticksevery <= 20) ticksevery = 20;
		else if(ticksevery <= 50) ticksevery = 50;
		else if(ticksevery <= 100) ticksevery = 100;
		else if(ticksevery <= 500) ticksevery = 500;
		int labelypos = clientRect.y() + (clientRect.height() + painter.fontMetrics().height())/2 - painter.fontMetrics().descent();
		if(ticksevery > 0) {
			painter.setPen(QPen(QColor(200,200,255)));
			for(int frame = firstFrame; frame <= lastFrame; frame += ticksevery) {
				TimePoint time = AnimManager::instance().frameToTime(frame);
				FloatType percentage = (FloatType)(time - AnimManager::instance().animationInterval().start()) / (FloatType)(AnimManager::instance().animationInterval().duration() + 1);
				int pos = clientRect.x() + (int)(percentage * clientWidth) + thumbWidth / 2;
				QString labelText = QString::number(frame);
				QRect labelRect = painter.fontMetrics().boundingRect(labelText);
				painter.drawText(pos - labelRect.width()/2, labelypos, labelText);
			}
		}

		QStyleOptionButton btnOption;
		btnOption.initFrom(this);
		btnOption.rect = thumbRectangle();
		btnOption.text = AnimManager::instance().timeToString(AnimManager::instance().time());
		if(AnimManager::instance().animationInterval().start() == 0)
			btnOption.text += " / " + AnimManager::instance().timeToString(AnimManager::instance().animationInterval().end());
		btnOption.state = ((_dragPos >= 0) ? QStyle::State_Sunken : QStyle::State_Raised) | QStyle::State_Enabled;
		painter.drawPrimitive(QStyle::PE_PanelButtonCommand, btnOption);
		painter.drawControl(QStyle::CE_PushButtonLabel, btnOption);
	}
}

/******************************************************************************
* Returns the recommended size for the widget.
******************************************************************************/
QSize AnimationTimeSlider::sizeHint() const
{
#ifndef Q_OS_MACX
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
	int newPos;
	int thumbSize = thumbWidth();

	if(_dragPos < 0)
		newPos = event->x() - thumbSize / 2;
	else
		newPos = event->x() - _dragPos;

	int rectWidth = frameRect().width() - 2*frameWidth();
	TimeInterval interval = AnimManager::instance().animationInterval();
	TimePoint newTime = (TimePoint)((qint64)newPos * (qint64)(interval.duration() + 1) / (qint64)(rectWidth - thumbSize));

	// Clamp new time to animation interval.
	newTime = std::max(newTime, interval.start());
	newTime = std::min(newTime, interval.end());

	// Snap to frames
	int newFrame = AnimManager::instance().timeToFrame(newTime + AnimManager::instance().ticksPerFrame()/2);
	if(_dragPos >= 0) {

		TimePoint newTime = AnimManager::instance().frameToTime(newFrame);
		if(newTime == AnimManager::instance().time()) return;

		// Set new time
		AnimManager::instance().setTime(newTime);

		// Force immediate viewport update.
		ViewportManager::instance().processViewportUpdates();
	}
	else if(interval.duration() > 0) {
		if(thumbRectangle().contains(event->pos()) == false) {
			QString frameName = DataSetManager::instance().currentSet()->animationSettings()->namedFrames()[newFrame];
			FloatType percentage = (FloatType)(AnimManager::instance().frameToTime(newFrame) - AnimManager::instance().animationInterval().start()) / (FloatType)(AnimManager::instance().animationInterval().duration() + 1);
			QRect clientRect = frameRect();
			clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
			int clientWidth = clientRect.width() - thumbWidth();
			QPoint pos(clientRect.x() + (int)(percentage * clientWidth) + thumbWidth() / 2, clientRect.height() / 2);
			QToolTip::showText(mapToGlobal(pos), QString("%1 - %2").arg(newFrame).arg(frameName), this);
		}
		else QToolTip::hideText();
	}
}

/******************************************************************************
* Computes the width of the thumb.
******************************************************************************/
int AnimationTimeSlider::thumbWidth() const
{
	int clientWidth = frameRect().width() - 2*frameWidth();
	return std::min(clientWidth / 2, 90);
}

/******************************************************************************
* Computes the coordinates of the slider thumb.
******************************************************************************/
QRect AnimationTimeSlider::thumbRectangle()
{
	if(DataSetManager::instance().currentSet() == nullptr)
		return QRect(0,0,0,0);

	TimeInterval interval = AnimManager::instance().animationInterval();
	TimePoint value = std::min(std::max(AnimManager::instance().time(), interval.start()), interval.end());
	FloatType percentage = (FloatType)(value - interval.start()) / (FloatType)(interval.duration() + 1);

	QRect clientRect = frameRect();
	clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
	int thumbSize = thumbWidth();
	int thumbPos = (int)((clientRect.width() - thumbSize) * percentage);
	return QRect(thumbPos + clientRect.x(), clientRect.y(), thumbSize, clientRect.height());
}

};
