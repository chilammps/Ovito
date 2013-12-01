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
#include <core/dataset/DataSetContainer.h>
#include <core/viewport/ViewportManager.h>
#include <core/gui/actions/ActionManager.h>

namespace Ovito {

/// The singleton instance of the class.
AnimManager* AnimManager::_instance = nullptr;


/******************************************************************************
* Resets the animation manager.
******************************************************************************/
void AnimManager::reset()
{
	setAutoKeyMode(false);
	
	if(_settings) {
		disconnect(_settings.get(), SIGNAL(timeChanged(TimePoint)), this, SIGNAL(timeChanged(TimePoint)));
		disconnect(_settings.get(), SIGNAL(intervalChanged(TimeInterval)), this, SIGNAL(intervalChanged(TimeInterval)));
		disconnect(_settings.get(), SIGNAL(speedChanged(int)), this, SIGNAL(speedChanged(int)));
	}

	_settings = DataSetManager::instance().currentSet()->animationSettings();

	if(_settings) {
		connect(_settings.get(), SIGNAL(timeChanged(TimePoint)), SIGNAL(timeChanged(TimePoint)));
		connect(_settings.get(), SIGNAL(intervalChanged(TimeInterval)), SIGNAL(intervalChanged(TimeInterval)));
		connect(_settings.get(), SIGNAL(speedChanged(int)), SIGNAL(speedChanged(int)));

		Q_EMIT speedChanged(_settings->ticksPerFrame());
		Q_EMIT intervalChanged(_settings->animationInterval());
		Q_EMIT timeChanged(_settings->time());
	}
}



};
