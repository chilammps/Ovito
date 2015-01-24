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
#include <core/utilities/units/UnitsManager.h>
#include <core/gui/mainwin/MainWindow.h>
#include "AnimationTimeSpinner.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

using namespace std;

/******************************************************************************
* Constructs the spinner control.
******************************************************************************/
AnimationTimeSpinner::AnimationTimeSpinner(MainWindow* mainWindow, QWidget* parent) : SpinnerWidget(parent),
		_animSettings(nullptr)
{
	connect(this, &SpinnerWidget::spinnerValueChanged, this, &AnimationTimeSpinner::onSpinnerValueChanged);
	connect(&mainWindow->datasetContainer(), &DataSetContainer::dataSetChanged, this, &AnimationTimeSpinner::onDataSetReplaced);
	connect(&mainWindow->datasetContainer(), &DataSetContainer::animationSettingsReplaced, this, &AnimationTimeSpinner::onAnimationSettingsReplaced);

	onDataSetReplaced(mainWindow->datasetContainer().currentSet());
	onAnimationSettingsReplaced(mainWindow->datasetContainer().currentSet() ? mainWindow->datasetContainer().currentSet()->animationSettings() : nullptr);
}

/******************************************************************************
* Is called when a another dataset has become the active dataset.
******************************************************************************/
void AnimationTimeSpinner::onDataSetReplaced(DataSet* newDataSet)
{
	if(newDataSet)
		setUnit(newDataSet->unitsManager().timeUnit());
	else
		setUnit(nullptr);
}

/******************************************************************************
* This is called when new animation settings have been loaded.
******************************************************************************/
void AnimationTimeSpinner::onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings)
{
	disconnect(_animIntervalChangedConnection);
	disconnect(_timeChangedConnection);
	_animSettings = newAnimationSettings;
	if(newAnimationSettings) {
		_animIntervalChangedConnection = connect(newAnimationSettings, &AnimationSettings::intervalChanged, this, &AnimationTimeSpinner::onIntervalChanged);
		_timeChangedConnection = connect(newAnimationSettings, &AnimationSettings::timeChanged, this, &AnimationTimeSpinner::onTimeChanged);
		onIntervalChanged(newAnimationSettings->animationInterval());
		onTimeChanged(newAnimationSettings->time());
	}
	else {
		onIntervalChanged(TimeInterval(0));
		onTimeChanged(0);
	}
}

/******************************************************************************
* This is called whenever the current animation time has changed.
******************************************************************************/
void AnimationTimeSpinner::onTimeChanged(TimePoint newTime)
{
	// Set the value of the spinner to the new animation time.
	setIntValue(newTime);
}

/******************************************************************************
* This is called whenever the active animation interval has changed.
******************************************************************************/
void AnimationTimeSpinner::onIntervalChanged(TimeInterval newAnimationInterval)
{
	// Set the limits of the spinner to the new animation time interval.
	setMinValue(newAnimationInterval.start());
	setMaxValue(newAnimationInterval.end());
	setEnabled(newAnimationInterval.duration() != 0);
}

/******************************************************************************
* Is called when the spinner value has been changed by the user.
******************************************************************************/
void AnimationTimeSpinner::onSpinnerValueChanged()
{
	// Set a new animation time.
	if(_animSettings)
		_animSettings->setTime(intValue());
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
