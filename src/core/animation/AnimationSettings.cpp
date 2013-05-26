///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2008) Alexander Stukowski
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
#include <core/scene/animation/AnimationSettings.h>
#include <core/undo/UndoManager.h>

namespace Core {

IMPLEMENT_SERIALIZABLE_PLUGIN_CLASS(AnimationSettings, RefTarget)

/******************************************************************************
* Default constructor. Assigns default values.
******************************************************************************/
AnimationSettings::AnimationSettings(bool isLoading) : RefTarget(isLoading), 
	_ticksPerFrame(TICKS_PER_SECOND/25), _playbackSpeed(1)	
{
    _animationInterval = TimeInterval(frameToTime(0, _ticksPerFrame), frameToTime(0, _ticksPerFrame));
    _time = _animationInterval.start();
}

/******************************************************************************
* Sets the current animation time.
******************************************************************************/
void AnimationSettings::setTime(TimeTicks time)
{
	if(time == _time) return;
		
	// Make the settings change undoable.
	class ChangeTimeOperation : public UndoableOperation {
	public:
		ChangeTimeOperation(AnimationSettings* _as) : as(_as), time(_as->time()) {}
		virtual void undo() { TimeTicks old = as->time(); as->setTime(time); time = old; }
		virtual void redo() { undo(); }
		virtual QString displayName() const { return "Change Animation Time"; }
	private:
		AnimationSettings::SmartPtr as; 
		TimeTicks time;
	};	  
	if(UNDO_MANAGER.isRecording())
		UNDO_MANAGER.addOperation(new ChangeTimeOperation(this));
	
	_time = time;
	timeChanged(_time);
	
	notifyDependents(REFTARGET_CHANGED);
}

/******************************************************************************
* Sets the animation interval.
******************************************************************************/
void AnimationSettings::setAnimationInterval(const TimeInterval& interval)
{
	if(interval == animationInterval()) return;
		
	// Make the settings change undoable.
	class ChangeIntervalOperation : public UndoableOperation {
	public:
		ChangeIntervalOperation(AnimationSettings* _as) : as(_as), interval(_as->animationInterval()) {}
		virtual void undo() { TimeInterval old = as->animationInterval(); as->setAnimationInterval(interval); interval = old; }
		virtual void redo() { undo(); }
		virtual QString displayName() const { return "Change Animation Interval"; }
	private:
		AnimationSettings::SmartPtr as; 
		TimeInterval interval;
	};	  
	if(UNDO_MANAGER.isRecording())
		UNDO_MANAGER.addOperation(new ChangeIntervalOperation(this));
	
	_animationInterval = interval;
	intervalChanged(animationInterval());
	
	notifyDependents(REFTARGET_CHANGED);
}

/******************************************************************************
* Sets the number of time ticks per frame.
******************************************************************************/
void AnimationSettings::setTicksPerFrame(int tpf)
{
	if(tpf == ticksPerFrame()) return;
		
	// Make the settings change undoable.
	class ChangeTPFOperation : public UndoableOperation {
	public:
		ChangeTPFOperation(AnimationSettings* _as) : as(_as), tpf(_as->ticksPerFrame()) {}
		virtual void undo() { int oldtpf = as->ticksPerFrame(); as->setTicksPerFrame(tpf); tpf = oldtpf; }
		virtual void redo() { undo(); }
		virtual QString displayName() const { return "Change Ticks Per Frame"; }
	private:
		AnimationSettings::SmartPtr as;
		int tpf;
	};	
	if(UNDO_MANAGER.isRecording())
		UNDO_MANAGER.addOperation(new ChangeTPFOperation(this));
	
	_ticksPerFrame = tpf;
	speedChanged(ticksPerFrame());
	
	notifyDependents(REFTARGET_CHANGED);
}

/******************************************************************************
* Sets tee playback speed factor that is used for animation playback in the viewport.
******************************************************************************/
void AnimationSettings::setPlaybackSpeed(int factor)
{
	if(factor == playbackSpeed()) return;
		
	// Make the settings change undoable.
	class ChangePlaybackSpeedOperation : public UndoableOperation {
	public:
		ChangePlaybackSpeedOperation(AnimationSettings* _as) : as(_as), factor(_as->playbackSpeed()) {}
		virtual void undo() { int old = as->playbackSpeed(); as->setPlaybackSpeed(factor); factor = old; }
		virtual void redo() { undo(); }
		virtual QString displayName() const { return "Change Playback Speed"; }
	private:
		AnimationSettings::SmartPtr as; 
		int factor;
	};	  
	if(UNDO_MANAGER.isRecording())
		UNDO_MANAGER.addOperation(new ChangePlaybackSpeedOperation(this));
	
	_playbackSpeed = factor;
	
	notifyDependents(REFTARGET_CHANGED);
}


/******************************************************************************
* Saves the class' contents to the given stream. 
******************************************************************************/
void AnimationSettings::saveToStream(ObjectSaveStream& stream)
{
	RefTarget::saveToStream(stream);
	stream.beginChunk(0x0123333);
	stream << _time;
	stream << _animationInterval;
	stream << _ticksPerFrame;
	stream << _playbackSpeed;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream. 
******************************************************************************/
void AnimationSettings::loadFromStream(ObjectLoadStream& stream)
{
	RefTarget::loadFromStream(stream);
	stream.expectChunk(0x0123333);
	stream >> _time;
	stream >> _animationInterval;
	stream >> _ticksPerFrame;
	stream >> _playbackSpeed;
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object. 
******************************************************************************/
RefTarget::SmartPtr AnimationSettings::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	AnimationSettings::SmartPtr clone = static_object_cast<AnimationSettings>(RefTarget::clone(deepCopy, cloneHelper));

	clone->_time = this->_time;
	clone->_animationInterval = this->_animationInterval;
	clone->_ticksPerFrame = this->_ticksPerFrame;
	clone->_playbackSpeed = this->_playbackSpeed;	
	
	return clone;
}

};
