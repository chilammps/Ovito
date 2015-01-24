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
#include <core/gui/dialogs/AnimationSettingsDialog.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/animation/AnimationSettings.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui)

/******************************************************************************
* Handles the ACTION_GOTO_START_OF_ANIMATION command.
******************************************************************************/
void ActionManager::on_AnimationGotoStart_triggered()
{
	_dataset->animationSettings()->jumpToAnimationStart();
}

/******************************************************************************
* Handles the ACTION_GOTO_END_OF_ANIMATION command.
******************************************************************************/
void ActionManager::on_AnimationGotoEnd_triggered()
{
	_dataset->animationSettings()->jumpToAnimationEnd();
}

/******************************************************************************
* Handles the ACTION_GOTO_PREVIOUS_FRAME command.
******************************************************************************/
void ActionManager::on_AnimationGotoPreviousFrame_triggered()
{
	_dataset->animationSettings()->jumpToPreviousFrame();
}

/******************************************************************************
* Handles the ACTION_GOTO_NEXT_FRAME command.
******************************************************************************/
void ActionManager::on_AnimationGotoNextFrame_triggered()
{
	_dataset->animationSettings()->jumpToNextFrame();
}

/******************************************************************************
* Handles the ACTION_START_ANIMATION_PLAYBACK command.
******************************************************************************/
void ActionManager::on_AnimationStartPlayback_triggered()
{
	if(!getAction(ACTION_TOGGLE_ANIMATION_PLAYBACK)->isChecked())
		getAction(ACTION_TOGGLE_ANIMATION_PLAYBACK)->trigger();
}

/******************************************************************************
* Handles the ACTION_STOP_ANIMATION_PLAYBACK command.
******************************************************************************/
void ActionManager::on_AnimationStopPlayback_triggered()
{
	if(getAction(ACTION_TOGGLE_ANIMATION_PLAYBACK)->isChecked())
		getAction(ACTION_TOGGLE_ANIMATION_PLAYBACK)->trigger();
}

/******************************************************************************
* Handles the ACTION_ANIMATION_SETTINGS command.
******************************************************************************/
void ActionManager::on_AnimationSettings_triggered()
{
	AnimationSettingsDialog(_dataset->animationSettings(), mainWindow()).exec();
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
