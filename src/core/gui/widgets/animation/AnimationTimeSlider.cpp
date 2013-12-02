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

namespace Ovito {

using namespace std;

/******************************************************************************
* The constructor of the AnimationTimeSlider class.
******************************************************************************/
AnimationTimeSlider::AnimationTimeSlider(MainWindow* mainWindow, QWidget* parent) :
	QFrame(parent), _mainWindow(mainWindow), _dragPos(-1)
{
	_normalPalette = palette();
	_autoKeyModePalette = _normalPalette;
	_autoKeyModePalette.setColor(QPalette::Window, QColor(240, 60, 60));

	setFrameShape(QFrame::Panel);
	setFrameShadow(QFrame::Sunken);
	setAutoFillBackground(true);
	setMouseTracking(true);

	connect(&mainWindow->datasetContainer(), &DataSetContainer::dataSetChanged, this, &AnimationTimeSlider::onDataSetChanged);
}

/******************************************************************************
* This is called when a new dataset has been loaded.
******************************************************************************/
void AnimationTimeSlider::onDataSetChanged(DataSet* newDataSet)
{
	OVITO_CHECK_OBJECT_POINTER(newDataSet);
	disconnect(_animationSettingsChangedConnection);
	_animationSettingsChangedConnection = connect(newDataSet, &DataSet::animationSettingsChanged, this, &AnimationTimeSlider::onAnimationSettingsChanged);
	onAnimationSettingsChanged(newDataSet->animationSettings());
}

/******************************************************************************
* This is called when new animation settings have been loaded.
******************************************************************************/
void AnimationTimeSlider::onAnimationSettingsChanged(AnimationSettings* newAnimationSettings)
{
	OVITO_CHECK_OBJECT_POINTER(newAnimationSettings);
	disconnect(_autoKeyModeChangedConnection);
	disconnect(_animIntervalChangedConnection);
	disconnect(_timeFormatChangedConnection);
	disconnect(_timeChangedConnection);

	_animSettings = newAnimationSettings;

	_autoKeyModeChangedConnection = connect(newAnimationSettings, &AnimationSettings::autoKeyModeChanged, this, &AnimationTimeSlider::onAutoKeyModeChanged);
	_animIntervalChangedConnection = connect(newAnimationSettings, &AnimationSettings::intervalChanged, this, (void (AnimationTimeSlider::*)())&AnimationTimeSlider::update);
	_timeFormatChangedConnection = connect(newAnimationSettings, &AnimationSettings::timeFormatChanged, this, (void (AnimationTimeSlider::*)())&AnimationTimeSlider::update);
	_timeChangedConnection = connect(newAnimationSettings, &AnimationSettings::timeChanged, this, (void (AnimationTimeSlider::*)())&AnimationTimeSlider::repaint);

	onAutoKeyModeChanged(_animSettings->autoKeyMode());
	update();
}

/******************************************************************************
* Handles paint events.
******************************************************************************/
void AnimationTimeSlider::paintEvent(QPaintEvent* event)
{
	QFrame::paintEvent(event);

	// Show slider only if there is more than one animation frame.
	int numFrames = (int)(_animSettings->animationInterval().duration() / _animSettings->ticksPerFrame()) + 1;
	if(numFrames > 1) {
		QStylePainter painter(this);

		QRect clientRect = frameRect();
		clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
		int thumbWidth = this->thumbWidth();
		int clientWidth = clientRect.width() - thumbWidth;
		int firstFrame = _animSettings->timeToFrame(_animSettings->animationInterval().start());
		int lastFrame = _animSettings->timeToFrame(_animSettings->animationInterval().end());
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
			painter.setPen(QPen(QColor(180,180,220)));
			for(int frame = firstFrame; frame <= lastFrame; frame += ticksevery) {
				TimePoint time = _animSettings->frameToTime(frame);
				FloatType percentage = (FloatType)(time - _animSettings->animationInterval().start()) / (FloatType)(_animSettings->animationInterval().duration() + 1);
				int pos = clientRect.x() + (int)(percentage * clientWidth) + thumbWidth / 2;
				QString labelText = QString::number(frame);
				QRect labelRect = painter.fontMetrics().boundingRect(labelText);
				painter.drawText(pos - labelRect.width()/2, labelypos, labelText);
			}
		}

		QStyleOptionButton btnOption;
		btnOption.initFrom(this);
		btnOption.rect = thumbRectangle();
		btnOption.text = _animSettings->timeToString(_animSettings->time());
		if(_animSettings->animationInterval().start() == 0)
			btnOption.text += " / " + _animSettings->timeToString(_animSettings->animationInterval().end());
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
	return QPushButton(QString("XXXXXXXXXX")).sizeHint();
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
			QString frameName = _animSettings->namedFrames()[newFrame];
			FloatType percentage = (FloatType)(_animSettings->frameToTime(newFrame) - _animSettings->animationInterval().start())
					/ (FloatType)(_animSettings->animationInterval().duration() + 1);
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


};
