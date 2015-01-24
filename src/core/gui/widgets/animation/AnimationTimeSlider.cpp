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
#include "AnimationTimeSlider.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

using namespace std;

/******************************************************************************
* The constructor of the AnimationTimeSlider class.
******************************************************************************/
AnimationTimeSlider::AnimationTimeSlider(MainWindow* mainWindow, QWidget* parent) :
	QFrame(parent), _mainWindow(mainWindow), _dragPos(-1), _animSettings(nullptr)
{
	_normalPalette = palette();
	_autoKeyModePalette = _normalPalette;
	_autoKeyModePalette.setColor(QPalette::Window, QColor(240, 60, 60));
	_sliderPalette = _normalPalette;
	_sliderPalette.setColor(QPalette::Button, _sliderPalette.color(QPalette::Button).darker(110));

	setFrameShape(QFrame::NoFrame);
	setAutoFillBackground(true);
	setMouseTracking(true);

	connect(&mainWindow->datasetContainer(), &DataSetContainer::animationSettingsReplaced, this, &AnimationTimeSlider::onAnimationSettingsReplaced);
}

/******************************************************************************
* This is called when new animation settings have been loaded.
******************************************************************************/
void AnimationTimeSlider::onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings)
{
	disconnect(_autoKeyModeChangedConnection);
	disconnect(_animIntervalChangedConnection);
	disconnect(_timeFormatChangedConnection);
	disconnect(_timeChangedConnection);
	_animSettings = newAnimationSettings;
	if(newAnimationSettings) {
		_autoKeyModeChangedConnection = connect(newAnimationSettings, &AnimationSettings::autoKeyModeChanged, this, &AnimationTimeSlider::onAutoKeyModeChanged);
		_animIntervalChangedConnection = connect(newAnimationSettings, &AnimationSettings::intervalChanged, this, (void (AnimationTimeSlider::*)())&AnimationTimeSlider::update);
		_timeFormatChangedConnection = connect(newAnimationSettings, &AnimationSettings::timeFormatChanged, this, (void (AnimationTimeSlider::*)())&AnimationTimeSlider::update);
		_timeChangedConnection = connect(newAnimationSettings, &AnimationSettings::timeChanged, this, (void (AnimationTimeSlider::*)())&AnimationTimeSlider::repaint);
		onAutoKeyModeChanged(_animSettings->autoKeyMode());
	}
	else onAutoKeyModeChanged(false);
	update();
}

/******************************************************************************
* Handles paint events.
******************************************************************************/
void AnimationTimeSlider::paintEvent(QPaintEvent* event)
{
	QFrame::paintEvent(event);
	if(!_animSettings) return;

	// Show slider only if there is more than one animation frame.
	int numFrames = (int)(_animSettings->animationInterval().duration() / _animSettings->ticksPerFrame()) + 1;
	if(numFrames > 1) {
		QStylePainter painter(this);

		QRect clientRect = frameRect();
		clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
		int thumbWidth = this->thumbWidth();
		TimePoint startTime, timeStep, endTime;
		std::tie(startTime, timeStep, endTime) = tickRange(maxTickLabelWidth());

		painter.setPen(QPen(QColor(180,180,220)));
		for(TimePoint time = startTime; time <= endTime; time += timeStep) {
			QString labelText = QString::number(_animSettings->timeToFrame(time));
			painter.drawText(timeToPos(time) - thumbWidth/2, clientRect.y(), thumbWidth, clientRect.height(), Qt::AlignCenter, labelText);
		}

		QStyleOptionButton btnOption;
		btnOption.initFrom(this);
		btnOption.rect = thumbRectangle();
		btnOption.text = _animSettings->timeToString(_animSettings->time());
		if(_animSettings->animationInterval().start() == 0)
			btnOption.text += " / " + _animSettings->timeToString(_animSettings->animationInterval().end());
		btnOption.state = ((_dragPos >= 0) ? QStyle::State_Sunken : QStyle::State_Raised) | QStyle::State_Enabled;
		btnOption.palette = _sliderPalette;
		painter.drawPrimitive(QStyle::PE_PanelButtonCommand, btnOption);
		btnOption.palette = _normalPalette;
		painter.drawControl(QStyle::CE_PushButtonLabel, btnOption);
	}
}

/******************************************************************************
* Computes the maximum width of a frame tick label.
******************************************************************************/
int AnimationTimeSlider::maxTickLabelWidth()
{
	QString label = QString::number(_animSettings->timeToFrame(_animSettings->animationInterval().end()));
	return fontMetrics().boundingRect(label).width() + 20;
}

/******************************************************************************
* Computes the time ticks to draw.
******************************************************************************/
std::tuple<TimePoint,TimePoint,TimePoint> AnimationTimeSlider::tickRange(int tickWidth)
{
	QRect clientRect = frameRect();
	clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
	int thumbWidth = this->thumbWidth();
	int clientWidth = clientRect.width() - thumbWidth;
	int firstFrame = _animSettings->timeToFrame(_animSettings->animationInterval().start());
	int lastFrame = _animSettings->timeToFrame(_animSettings->animationInterval().end());
	int numFrames = lastFrame - firstFrame + 1;
	int nticks = std::min(clientWidth / tickWidth, numFrames);
	int ticksevery = numFrames / std::max(nticks, 1);
	if(ticksevery <= 1) ticksevery = ticksevery;
	else if(ticksevery <= 5) ticksevery = 5;
	else if(ticksevery <= 10) ticksevery = 10;
	else if(ticksevery <= 20) ticksevery = 20;
	else if(ticksevery <= 50) ticksevery = 50;
	else if(ticksevery <= 100) ticksevery = 100;
	else if(ticksevery <= 500) ticksevery = 500;
	if(ticksevery > 0) {
		return std::make_tuple(
				_animSettings->frameToTime(firstFrame),
				_animSettings->ticksPerFrame() * ticksevery,
				_animSettings->frameToTime(lastFrame));
	}
	else {
		return std::tuple<TimePoint,TimePoint,TimePoint>(0, 1, 0);
	}
}

/******************************************************************************
* Computes the x position within the widget corresponding to the given animation time.
******************************************************************************/
int AnimationTimeSlider::timeToPos(TimePoint time)
{
	FloatType percentage = (FloatType)(time - _animSettings->animationInterval().start()) / (FloatType)(_animSettings->animationInterval().duration() + 1);
	QRect clientRect = frameRect();
	int tw = thumbWidth();
	int space = clientRect.width() - 2*frameWidth() - tw;
	return clientRect.x() + frameWidth() + (int)(percentage * space) + tw / 2;
}

/******************************************************************************
* Converts a distance in pixels to a time difference.
******************************************************************************/
TimePoint AnimationTimeSlider::distanceToTimeDifference(int distance)
{
	QRect clientRect = frameRect();
	int tw = thumbWidth();
	int space = clientRect.width() - 2*frameWidth() - tw;
	return (int)((qint64)(_animSettings->animationInterval().duration() + 1) * distance / space);
}

/******************************************************************************
* Returns the recommended size for the widget.
******************************************************************************/
QSize AnimationTimeSlider::sizeHint() const
{
	return QSize(QFrame::sizeHint().width(), fontMetrics().height() + frameWidth() * 2 + 6);
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
	TimeInterval interval = _animSettings->animationInterval();
	TimePoint newTime = (TimePoint)((qint64)newPos * (qint64)(interval.duration() + 1) / (qint64)(rectWidth - thumbSize)) + interval.start();

	// Clamp new time to animation interval.
	newTime = std::max(newTime, interval.start());
	newTime = std::min(newTime, interval.end());

	// Snap to frames
	int newFrame = _animSettings->timeToFrame(newTime + _animSettings->ticksPerFrame()/2);
	if(_dragPos >= 0) {

		TimePoint newTime = _animSettings->frameToTime(newFrame);
		if(newTime == _animSettings->time()) return;

		// Set new time
		_animSettings->setTime(newTime);

		// Force immediate viewport update.
		_mainWindow->processViewportUpdates();
	}
	else if(interval.duration() > 0) {
		if(thumbRectangle().contains(event->pos()) == false) {
			FloatType percentage = (FloatType)(_animSettings->frameToTime(newFrame) - _animSettings->animationInterval().start())
					/ (FloatType)(_animSettings->animationInterval().duration() + 1);
			QRect clientRect = frameRect();
			clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
			int clientWidth = clientRect.width() - thumbWidth();
			QPoint pos(clientRect.x() + (int)(percentage * clientWidth) + thumbWidth() / 2, clientRect.height() / 2);
			QString frameName = _animSettings->namedFrames()[newFrame];
			QString tooltipText;
			if(!frameName.isEmpty())
				tooltipText = QString("%1 - %2").arg(newFrame).arg(frameName);
			else
				tooltipText = QString("%1").arg(newFrame);
			QToolTip::showText(mapToGlobal(pos), tooltipText, this);
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
	if(!_animSettings)
		return QRect(0,0,0,0);

	TimeInterval interval = _animSettings->animationInterval();
	TimePoint value = std::min(std::max(_animSettings->time(), interval.start()), interval.end());
	FloatType percentage = (FloatType)(value - interval.start()) / (FloatType)(interval.duration() + 1);

	QRect clientRect = frameRect();
	clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
	int thumbSize = thumbWidth();
	int thumbPos = (int)((clientRect.width() - thumbSize) * percentage);
	return QRect(thumbPos + clientRect.x(), clientRect.y(), thumbSize, clientRect.height());
}

/******************************************************************************
* Is called whenever the Auto Key mode is activated or deactivated.
******************************************************************************/
void AnimationTimeSlider::onAutoKeyModeChanged(bool active)
{
	setPalette(active ? _autoKeyModePalette : _normalPalette);
	update();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
