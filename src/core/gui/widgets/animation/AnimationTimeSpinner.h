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

/** 
 * \file AnimationTimeSpinner.h 
 * \brief Contains the definition of the Ovito::AnimationTimeSpinner class.
 */

#ifndef __OVITO_ANIMATION_TIME_SPINNER_H
#define __OVITO_ANIMATION_TIME_SPINNER_H

#include <core/Core.h>
#include <core/gui/widgets/general/SpinnerWidget.h>
#include <core/animation/AnimationSettings.h>
#include <core/utilities/units/UnitsManager.h>

namespace Ovito {

/**
 * \brief A spinner control for changing the current animation time.
 * 
 * It displays the current animation time given by AnimManager::time() and changes
 * it when the user enters a new time value.
 */
class OVITO_CORE_EXPORT AnimationTimeSpinner : public SpinnerWidget
{
	Q_OBJECT
	
public:
	
	/// Constructs the spinner control.
	AnimationTimeSpinner(QWidget* parent = 0) : SpinnerWidget(parent) {
		setUnit(UnitsManager::instance().timeUnit());
		connect(&AnimManager::instance(), SIGNAL(timeChanged(TimePoint)), SLOT(onTimeChanged(TimePoint)));
		connect(&AnimManager::instance(), SIGNAL(intervalChanged(TimeInterval)), SLOT(onIntervalChanged(TimeInterval)));
		connect(this, SIGNAL(spinnerValueChanged()), SLOT(onSpinnerValueChanged()));
	}

protected Q_SLOTS:

	/// This is called by the AnimManager when the current animation time has changed.
	void onTimeChanged(TimePoint newTime) {
		// Set the value of the spinner to the new animation time.
		setIntValue(newTime);
	}

	/// This is called by the AnimManager when the animation interval has changed.
	void onIntervalChanged(TimeInterval newAnimationInterval) {
		// Set the limits of the spinner to the new animation time interval.
		setMinValue(newAnimationInterval.start());
		setMaxValue(newAnimationInterval.end());
		setEnabled(newAnimationInterval.duration() != 0);
	}
	
	/// Is called when the spinner value has been changed by the user.
	void onSpinnerValueChanged() {
		// Set a new animation time.
		AnimManager::instance().setTime(intValue());
	}
};

};

#endif // __OVITO_ANIMATION_TIME_SPINNER_H
