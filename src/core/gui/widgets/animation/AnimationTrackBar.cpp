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

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

using namespace std;

/******************************************************************************
* The constructor of the AnimationTrackBar class.
******************************************************************************/
AnimationTrackBar::AnimationTrackBar(MainWindow* mainWindow, AnimationTimeSlider* timeSlider, QWidget* parent) :
	QFrame(parent), _timeSlider(timeSlider), _animSettings(nullptr),
	_keyPen(Qt::black), _selectedKeyPen(QColor(255,255,255)), _selectionCursor(Qt::CrossCursor),
	_isDragging(false), _dragStartPos(-1)
{
	_keyBrushes[0] = QBrush(QColor(150,150,200));	// Color for float controller keys
	_keyBrushes[1] = QBrush(QColor(150,150,200));	// Color for integer controller keys
	_keyBrushes[2] = QBrush(QColor(150,200,150));	// Color for vector controller keys
	_keyBrushes[3] = QBrush(QColor(200,150,150));	// Color for position controller keys
	_keyBrushes[4] = QBrush(QColor(200,200,150));	// Color for rotation controller keys
	_keyBrushes[5] = QBrush(QColor(150,200,200));	// Color for scaling controller keys
	_keyBrushes[6] = QBrush(QColor(150,150,150));	// Color for transformation controller keys

	setFrameShape(QFrame::NoFrame);
	setAutoFillBackground(true);
	setMouseTracking(true);

	connect(&mainWindow->datasetContainer(), &DataSetContainer::animationSettingsReplaced, this, &AnimationTrackBar::onAnimationSettingsReplaced);
	connect(&mainWindow->datasetContainer(), &DataSetContainer::selectionChangeComplete, this, &AnimationTrackBar::onRebuildControllerList);
	connect(&_objects, &VectorRefTargetListener<RefTarget>::notificationEvent, this, &AnimationTrackBar::onObjectNotificationEvent);
	connect(&_controllers, &VectorRefTargetListener<KeyframeController>::notificationEvent, this, &AnimationTrackBar::onControllerNotificationEvent);
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
			painter.drawLine(pos, clientRect.top(), pos, clientRect.center().y());
	}

	// Draw the animation keys.
	for(KeyframeController* ctrl : _controllers.targets()) {
		// Draw keys only if there are more two or more of them.
		if(ctrl->keys().size() >= 2) {
			for(AnimationKey* key : ctrl->keys()) {
				paintKey(painter, key, ctrl);
			}
		}
	}

	// Draw the current time marker.
	int currentTimePos = _timeSlider->timeToPos(_animSettings->time());
	painter.setBrush(Qt::blue);
	painter.setPen(Qt::black);
	QPoint marker[3] = {{ currentTimePos - 3, clientRect.top() },
						{ currentTimePos + 3, clientRect.top() },
						{ currentTimePos    , clientRect.top() + 3 }
	};
	painter.drawConvexPolygon(marker, 3);
}

/******************************************************************************
* Computes the display rectangle of an animation key.
******************************************************************************/
QRect AnimationTrackBar::keyRect(AnimationKey* key, bool forDisplay) const
{
	// Don't draw keys that are not within the active animation interval.
	if(key->time() < _animSettings->animationInterval().start() ||
		key->time() > _animSettings->animationInterval().end()) return QRect();

	QRect clientRect = frameRect();
	clientRect.adjust(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());

	int width = 6;
	int pos = _timeSlider->timeToPos(key->time());
	int offset = 0;

	bool done = false;
	for(KeyframeController* ctrl : _controllers.targets()) {
		// Draw keys only if there are more two or more of them.
		if(ctrl->keys().size() >= 2) {
			for(AnimationKey* key2 : ctrl->keys()) {
				if(key2 == key) done = true;
				else if(key->time() == key2->time()) offset++;
			}
		}
		if(done && forDisplay) break;
	}
	if(forDisplay)
		return QRect(pos - width / 2 + offset*2, clientRect.top() + 4 - offset*2, width, clientRect.height() - 5);
	else
		return QRect(pos - width / 2, clientRect.top() + 4 - offset*2, width + offset*2, clientRect.height() - 5 + offset*2);
}

/******************************************************************************
* Paints the symbol for a single animation key.
******************************************************************************/
void AnimationTrackBar::paintKey(QPainter& painter, AnimationKey* key, KeyframeController* ctrl) const
{
	QRect rect = keyRect(key, true);
	if(!rect.isValid())
		return;

	painter.setBrush(_keyBrushes[ctrl->controllerType() % _keyBrushes.size()]);
	painter.setPen(_selectedKeys.targets().contains(key) ? _selectedKeyPen : _keyPen);
	painter.drawRect(rect);
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
	_selectedKeys.clear();
	_parameterNames.clear();

	if(_animSettings) {
		// Traverse object graphs of selected scene nodes to find all animation controllers.
		SelectionSet* selection = _animSettings->dataset()->selection();
		for(SceneNode* node : selection->nodes()) {
			if(ObjectNode* objNode = dynamic_object_cast<ObjectNode>(node))
				findControllers(objNode);
		}
	}

	update();
}

/******************************************************************************
* Recursive function that finds all controllers in the object graph.
******************************************************************************/
void AnimationTrackBar::findControllers(RefTarget* target)
{
	OVITO_CHECK_OBJECT_POINTER(target);

	bool hasSubAnimatables = false;

	// Iterate over all reference fields of the current target.
	for(const OvitoObjectType* clazz = &target->getOOType(); clazz != nullptr; clazz = clazz->superClass()) {
		for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field != nullptr; field = field->next()) {
			if(field->isReferenceField() && field->flags().testFlag(PROPERTY_FIELD_NO_SUB_ANIM) == false) {
				hasSubAnimatables = true;
				if(field->isVector() == false) {
					if(RefTarget* subTarget = target->getReferenceField(*field)) {
						findControllers(subTarget);
						addController(subTarget, target, field);
					}
				}
				else {
					for(RefTarget* subTarget : target->getVectorReferenceField(*field).targets()) {
						if(subTarget) {
							findControllers(subTarget);
							addController(subTarget, target, field);
						}
					}
				}
			}
		}
	}

	if(hasSubAnimatables)
		_objects.push_back(target);
}

/******************************************************************************
* Checks if the given ref target is a controller, and, if yes, add it to our
* list of controllers.
******************************************************************************/
void AnimationTrackBar::addController(RefTarget* target, RefTarget* owner, const PropertyFieldDescriptor* field)
{
	if(KeyframeController* ctrl = dynamic_object_cast<KeyframeController>(target)) {
		int ctrlIndex = _controllers.targets().indexOf(ctrl);
		if(ctrlIndex == -1) {
			_controllers.push_back(ctrl);
			_parameterNames.push_back(owner->objectTitle() + QStringLiteral(" - ") + field->displayName());
		}
		else {
			_parameterNames[ctrlIndex] += QStringLiteral(",") + owner->objectTitle() + QStringLiteral(" - ") + field->displayName();
		}
	}
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

/******************************************************************************
* Is called whenever one of the controllers being monitored sends a notification signal.
******************************************************************************/
void AnimationTrackBar::onControllerNotificationEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::TargetChanged ||
			event->type() == ReferenceEvent::ReferenceChanged
			|| event->type() == ReferenceEvent::ReferenceAdded || event->type() == ReferenceEvent::ReferenceRemoved) {
		// Repaint track bar whenever a key has been created, deleted, or moved.
		update();
	}
	else if(event->type() == ReferenceEvent::TargetDeleted) {
		_parameterNames.removeAt(_controllers.targets().indexOf(static_cast<KeyframeController*>(source)));
	}
}

/******************************************************************************
* Finds all keys under the mouse cursor.
******************************************************************************/
QVector<AnimationKey*> AnimationTrackBar::hitTestKeys(QPoint pos) const
{
	QVector<AnimationKey*> result;
	for(KeyframeController* ctrl : _controllers.targets()) {
		if(ctrl->keys().size() >= 2) {
			for(int index = ctrl->keys().size() - 1; index >= 0; index--) {
				AnimationKey* key = ctrl->keys()[index];
				if((result.empty() && keyRect(key, false).contains(pos)) || (!result.empty() && result.front()->time() == key->time())) {
					result.push_back(key);
				}
			}
		}
	}
	return result;
}

/******************************************************************************
* Returns the list index of the controller that owns the given key.
******************************************************************************/
int AnimationTrackBar::controllerIndexFromKey(AnimationKey* key) const
{
	for(int index = 0; index < _controllers.targets().size(); index++) {
		if(_controllers.targets()[index]->keys().contains(key))
			return index;
	}
	OVITO_ASSERT(false);
	return -1;
}

/******************************************************************************
* Handles mouse press events.
******************************************************************************/
void AnimationTrackBar::mousePressEvent(QMouseEvent* event)
{
	_dragStartPos = -1;
	if(event->button() == Qt::LeftButton) {
		QVector<AnimationKey*> clickedKeys = hitTestKeys(event->pos());
		if(!event->modifiers().testFlag(Qt::ControlModifier)) {
			if(clickedKeys.empty() ||
					std::find_first_of(clickedKeys.begin(), clickedKeys.end(), _selectedKeys.targets().begin(), _selectedKeys.targets().end()) == clickedKeys.end())
				_selectedKeys.setTargets(clickedKeys);
		}
		else {
			for(AnimationKey* key : clickedKeys) {
				if(!_selectedKeys.targets().contains(key))
					_selectedKeys.push_back(key);
				else
					_selectedKeys.remove(key);
			}
		}
		if(!clickedKeys.empty()) {
			_dragStartPos = event->pos().x();
			qDebug() << "pos(time=100):" << _timeSlider->timeToPos(_animSettings->frameToTime(100)) << "_dragStartPos:" << _dragStartPos;
		}
		_isDragging = false;
		update();
	}
	else if(event->button() == Qt::RightButton) {
		if(_isDragging) {
			_isDragging = false;
			_animSettings->dataset()->undoStack().endCompoundOperation(false);
		}
		else {
			_isDragging = false;
			QVector<AnimationKey*> clickedKeys = hitTestKeys(event->pos());
			if(clickedKeys.empty() ||
					std::find_first_of(clickedKeys.begin(), clickedKeys.end(), _selectedKeys.targets().begin(), _selectedKeys.targets().end()) == clickedKeys.end()) {
				_selectedKeys.setTargets(clickedKeys);
				update();
			}
			showKeyContextMenu(event->pos(), clickedKeys);
		}
	}
}

/******************************************************************************
* Handles mouse move events.
******************************************************************************/
void AnimationTrackBar::mouseMoveEvent(QMouseEvent* event)
{
	if(event->buttons() == Qt::NoButton) {
		QVector<AnimationKey*> keys = hitTestKeys(event->pos());
		if(keys.empty()) {
			unsetCursor();
			QToolTip::hideText();
		}
		else {
			setCursor(_selectionCursor);
			QString tooltipText = tr("<p style='white-space:pre'>Time %1:").arg(_animSettings->timeToString(keys.front()->time()));
			for(AnimationKey* key : keys) {
				tooltipText += QStringLiteral("<br>  %1: %2")
						.arg(_parameterNames[controllerIndexFromKey(key)])
						.arg(keyValueString(key));
			}
			tooltipText += QStringLiteral("</p>");
			QToolTip::showText(mapToGlobal(event->pos()), tooltipText, this);
		}
	}
	else if(_dragStartPos >= 0) {
		if(!_isDragging && std::abs(_dragStartPos - event->pos().x()) > 4) {
			_animSettings->dataset()->undoStack().beginCompoundOperation(tr("Move animation keys"));
			_isDragging = true;
		}
		if(_isDragging) {
			int delta = event->pos().x() - _dragStartPos;
			TimePoint timeDelta = _timeSlider->distanceToTimeDifference(delta);
			timeDelta = _animSettings->snapTime(timeDelta);
			TimeInterval interval = _animSettings->animationInterval();
			_animSettings->dataset()->undoStack().resetCurrentCompoundOperation();
			// Clamp to animation interval.
			for(AnimationKey* key : _selectedKeys.targets()) {
				TimePoint newTime = key->time() + timeDelta;
				if(newTime < interval.start()) timeDelta += interval.start() - newTime;
				if(newTime > interval.end()) timeDelta -= newTime - interval.end();
			}
			// Move keys.
			for(KeyframeController* ctrl : _controllers.targets()) {
				ctrl->moveKeys(_selectedKeys.targets(), timeDelta);
			}
		}
	}
}

/******************************************************************************
* Handles mouse release events.
******************************************************************************/
void AnimationTrackBar::mouseReleaseEvent(QMouseEvent* event)
{
	if(_isDragging) {
		_isDragging = false;
		if(event->button() == Qt::LeftButton)
			_animSettings->dataset()->undoStack().endCompoundOperation(true);
	}
}

/******************************************************************************
* Returns a text representation of a key's value.
******************************************************************************/
QString AnimationTrackBar::keyValueString(AnimationKey* key) const
{
	QVariant value = key->property("value");
	if(value.userType() == qMetaTypeId<FloatType>())
		return QString::number(value.value<FloatType>());
	else if(value.userType() == qMetaTypeId<int>())
		return QString::number(value.value<int>());
	else if(value.userType() == qMetaTypeId<Vector3>()) {
		Vector3 vec = value.value<Vector3>();
		return QString("(%1, %2, %3)").arg(vec.x()).arg(vec.y()).arg(vec.z());
	}
	else if(value.userType() == qMetaTypeId<Rotation>()) {
		Rotation rot = value.value<Rotation>();
		return QString("axis (%1, %2, %3), angle: %4°").arg(rot.axis().x()).arg(rot.axis().y()).arg(rot.axis().z()).arg(rot.angle() * FloatType(180) / FLOATTYPE_PI);
	}
	else if(value.userType() == qMetaTypeId<Scaling>()) {
		Scaling s = value.value<Scaling>();
		return QString("(%1, %2, %3)]").arg(s.S.x()).arg(s.S.y()).arg(s.S.z());
	}
	else
		return value.toString();
}

/******************************************************************************
* Displays the context menu.
******************************************************************************/
void AnimationTrackBar::showKeyContextMenu(const QPoint& pos, const QVector<AnimationKey*>& clickedKeys)
{
	QMenu contextMenu(this);

	// Action: Unselect key.
	QMenu* unselectKeyMenu = contextMenu.addMenu(tr("Unselect key"));
	unselectKeyMenu->setEnabled(!_selectedKeys.targets().empty());
	for(AnimationKey* key : _selectedKeys.targets()) {
		QString label = QStringLiteral("%1: %2")
				.arg(_parameterNames[controllerIndexFromKey(key)])
				.arg(keyValueString(key));
		QAction* unselectAction = unselectKeyMenu->addAction(label);
		connect(unselectAction, &QAction::triggered, [this, key]() {
			_selectedKeys.remove(key);
			update();
		});
	}

	// Action: Delete selected keys
	contextMenu.addSeparator();
	contextMenu.addAction(tr("Deleted selected keys"), this, SLOT(onDeleteSelectedKeys()))->setEnabled(_selectedKeys.targets().empty() == false);

	// Action: Jump to key
	contextMenu.addSeparator();
	QAction* jumpToTimeAction = contextMenu.addAction(tr("Jump to key"));
	if(clickedKeys.empty() == false) {
		TimePoint time = clickedKeys.front()->time();
		connect(jumpToTimeAction, &QAction::triggered, [this, time]() {
			_animSettings->setTime(time);
		});
	}
	else jumpToTimeAction->setEnabled(false);

	contextMenu.exec(mapToGlobal(pos));
}

/******************************************************************************
* Deletes the selected animation keys.
******************************************************************************/
void AnimationTrackBar::onDeleteSelectedKeys()
{
	UndoableTransaction::handleExceptions(_animSettings->dataset()->undoStack(), tr("Delete animation keys"), [this]() {
		for(KeyframeController* ctrl : _controllers.targets()) {
			ctrl->deleteKeys(_selectedKeys.targets());
		}
	});
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
