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
#include <core/gui/actions/ActionManager.h>
#include <core/animation/AnimManager.h>
#include <core/viewport/input/ViewportInputHandler.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/viewport/input/NavigationModes.h>
#include <core/dataset/DataSetManager.h>

namespace Ovito {

/******************************************************************************
* This viewport mode plays the animation while it is active.
******************************************************************************/
class AnimationPlaybackViewportMode : public ViewportInputHandler
{
	Q_OBJECT

public:

	/// Constructor.
	AnimationPlaybackViewportMode() : _isTimerScheduled(false) {}

	/// Returns the activation behavior of this input handler.
	virtual InputHandlerType handlerType() override { return ViewportInputHandler::TEMPORARY; }

	/// Handle mouse down events.
	virtual void mousePressEvent(Viewport* vp, QMouseEvent* event) override {
		if(event->button() == Qt::LeftButton) {
			activateTemporaryNavigationMode(OrbitMode::instance());
			temporaryNavigationMode()->mousePressEvent(vp, event);
		}
		else ViewportInputHandler::mousePressEvent(vp, event);
	}

protected:

	/// This is called by the system after the input handler has become active.
	virtual void activated() override {
		scheduleNextFrame();
	}

protected Q_SLOTS:

	/// Start a timer that will fire an event when the next animation frame should be shown.
	void scheduleNextFrame() {
		if(ViewportInputManager::instance().currentHandler() == this && !_isTimerScheduled) {
			int timerSpeed = 1000;
			if(AnimManager::instance().playbackSpeed() > 1) timerSpeed /= AnimManager::instance().playbackSpeed();
			else if(AnimManager::instance().playbackSpeed() < -1) timerSpeed *= -AnimManager::instance().playbackSpeed();
			_isTimerScheduled = true;
			QTimer::singleShot(timerSpeed / AnimManager::instance().framesPerSecond(), this, SLOT(onTimer()));
		}
	}

	/// Is periodically called by the timer.
	void onTimer() {
		_isTimerScheduled = false;

		// Check if the animation playback mode has been deactivated in the meantime.
		if(ViewportInputManager::instance().currentHandler() != this)
			return;

		// Add one frame to current time
		int newFrame = AnimManager::instance().timeToFrame(AnimManager::instance().time()) + 1;
		TimePoint newTime = AnimManager::instance().frameToTime(newFrame);

		// Loop back to first frame if end has been reached.
		if(newTime > AnimManager::instance().animationInterval().end())
			newTime = AnimManager::instance().animationInterval().start();

		// Set new time
		AnimManager::instance().setTime(newTime);

		// Wait until the scene is ready. Then jump to the next frame.
		DataSetManager::instance().runWhenSceneIsReady(std::bind(&AnimationPlaybackViewportMode::scheduleNextFrame, this));
	}

private:

	bool _isTimerScheduled;
};

// This is required by Automoc.
#include "AnimationActions.moc"

OORef<ViewportInputHandler> ActionManager::createAnimationPlaybackViewportMode()
{
	static OORef<ViewportInputHandler> instance(new AnimationPlaybackViewportMode());
	return instance;
}

/******************************************************************************
* Handles the ACTION_GOTO_START_OF_ANIMATION command.
******************************************************************************/
void ActionManager::on_AnimationGotoStart_triggered()
{
	AnimManager::instance().setTime(AnimManager::instance().animationInterval().start());
}

/******************************************************************************
* Handles the ACTION_GOTO_END_OF_ANIMATION command.
******************************************************************************/
void ActionManager::on_AnimationGotoEnd_triggered()
{
	AnimManager::instance().setTime(AnimManager::instance().animationInterval().end());
}

/******************************************************************************
* Handles the ACTION_GOTO_PREVIOUS_FRAME command.
******************************************************************************/
void ActionManager::on_AnimationGotoPreviousFrame_triggered()
{
	// Subtract one frame from current time.
	TimePoint newTime = AnimManager::instance().frameToTime(AnimManager::instance().timeToFrame(AnimManager::instance().time()) - 1);
	// Clamp new time
	newTime = std::max(newTime, AnimManager::instance().animationInterval().start());
	// Set new time.
	AnimManager::instance().setTime(newTime);
}

/******************************************************************************
* Handles the ACTION_GOTO_NEXT_FRAME command.
******************************************************************************/
void ActionManager::on_AnimationGotoNextFrame_triggered()
{
	// Subtract one frame from current time.
	TimePoint newTime = AnimManager::instance().frameToTime(AnimManager::instance().timeToFrame(AnimManager::instance().time()) + 1);
	// Clamp new time
	newTime = std::min(newTime, AnimManager::instance().animationInterval().end());
	// Set new time.
	AnimManager::instance().setTime(newTime);
}

/******************************************************************************
* Handles the ACTION_START_ANIMATION_PLAYBACK command.
******************************************************************************/
void ActionManager::on_AnimationStartPlayback_triggered()
{
	ViewportInputManager::instance().pushInputHandler(createAnimationPlaybackViewportMode().get());
}

/******************************************************************************
* Handles the ACTION_STOP_ANIMATION_PLAYBACK command.
******************************************************************************/
void ActionManager::on_AnimationStopPlayback_triggered()
{
	ViewportInputManager::instance().removeInputHandler(createAnimationPlaybackViewportMode().get());
}

/******************************************************************************
* Handles the ACTION_ANIMATION_SETTINGS command.
******************************************************************************/
void ActionManager::on_AnimationSettings_triggered()
{
}

};
