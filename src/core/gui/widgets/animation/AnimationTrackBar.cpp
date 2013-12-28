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
#include <core/scene/SelectionSet.h>
#include <core/gui/mainwin/MainWindow.h>
#include "AnimationTrackBar.h"

namespace Ovito {

using namespace std;

/******************************************************************************
* The constructor of the AnimationTrackBar class.
******************************************************************************/
AnimationTrackBar::AnimationTrackBar(MainWindow* mainWindow, AnimationTimeSlider* timeSlider, QWidget* parent) :
	QFrame(parent), _timeSlider(timeSlider), _animSettings(nullptr)
{
	setFrameShape(QFrame::NoFrame);
	setAutoFillBackground(true);
	setMouseTracking(true);

	connect(&mainWindow->datasetContainer(), &DataSetContainer::animationSettingsReplaced, this, &AnimationTrackBar::onAnimationSettingsReplaced);
	connect(&mainWindow->datasetContainer(), &DataSetContainer::selectionChangeComplete, this, &AnimationTrackBar::onRebuildControllerList);
	connect(&_objects, &VectorRefTargetListener<RefTarget>::notificationEvent, this, &AnimationTrackBar::onObjectNotificationEvent);
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

	QPainter painter(this);

	QRect clientRect = frameRect();
	clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
	TimePoint startTime, timeStep, endTime;
	std::tie(startTime, timeStep, endTime) = _timeSlider->tickRange(10);

	TimePoint startTimeMajor, timeStepMajor;
	std::tie(startTimeMajor, timeStepMajor, std::ignore) = _timeSlider->tickRange(_timeSlider->maxTickLabelWidth());

	painter.setPen(QPen(QColor(180,180,220)));
	for(TimePoint time = startTime; time <= endTime; time += timeStep) {
		int pos = _timeSlider->timeToPos(time);
		if((time - startTimeMajor)%timeStepMajor == 0)
			painter.drawLine(pos, clientRect.top(), pos, clientRect.bottom());
		else
			painter.drawLine(pos, clientRect.center().y(), pos, clientRect.bottom());
	}
}

/******************************************************************************
* Returns the recommended size for the widget.
******************************************************************************/
QSize AnimationTrackBar::sizeHint() const
{
	return QSize(QFrame::sizeHint().width(), fontMetrics().height() * 1 + frameWidth() * 2 + 6);
}

/******************************************************************************
* This is called when the current scene node selection has changed.
******************************************************************************/
void AnimationTrackBar::onRebuildControllerList()
{
	// Rebuild the list of controllers shown in the track bar.
	_controllers.clear();
	_objects.clear();

	if(_animSettings) {
		// Traverse object graphs of selected scene nodes to find all animation controllers.
		SelectionSet* selection = _animSettings->dataset()->selection();
		for(SceneNode* node : selection->nodes()) {
			if(ObjectNode* objNode = dynamic_object_cast<ObjectNode>(node))
				findControllers(objNode);
		}
	}
}

/******************************************************************************
* Recursive function that finds all controllers in the object graph.
******************************************************************************/
void AnimationTrackBar::findControllers(RefTarget* target)
{
	OVITO_CHECK_OBJECT_POINTER(target);

	// Check if it is an animation controller. If yes, add it to the list.
	if(Controller* ctrl = dynamic_object_cast<Controller>(target)) {
		if(_controllers.targets().contains(ctrl) == false) {
			_controllers.push_back(ctrl);
		}
	}

	bool hasSubAnimatables = false;

	// Iterate over all reference fields of the current target.
	for(const OvitoObjectType* clazz = &target->getOOType(); clazz != nullptr; clazz = clazz->superClass()) {
		for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field != nullptr; field = field->next()) {
			if(field->isReferenceField() && field->flags().testFlag(PROPERTY_FIELD_NO_SUB_ANIM) == false) {
				hasSubAnimatables = true;
				if(field->isVector() == false) {
					if(RefTarget* subTarget = target->getReferenceField(*field))
						findControllers(subTarget);
				}
				else {
					for(RefTarget* subTarget : target->getVectorReferenceField(*field).targets()) {
						if(subTarget)
							findControllers(subTarget);
					}
				}
			}
		}
	}

	if(hasSubAnimatables)
		_objects.push_back(target);
}

/******************************************************************************
* Is called whenever one of the objects being monitored sends a notification signal.
******************************************************************************/
void AnimationTrackBar::onObjectNotificationEvent(RefTarget* source, ReferenceEvent* event)
{
	// Rebuild the complete controller list whenever the reference object changes.
	if(event->type() == ReferenceEvent::ReferenceChanged
			|| event->type() == ReferenceEvent::ReferenceAdded || event->type() == ReferenceEvent::ReferenceRemoved) {
		if(!_objects.targets().empty()) {
			_objects.clear();
			_controllers.clear();
			QMetaObject::invokeMethod(this, "onRebuildControllerList", Qt::QueuedConnection);
		}
	}
}

};
