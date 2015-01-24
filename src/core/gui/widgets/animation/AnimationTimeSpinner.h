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

#ifndef __OVITO_ANIMATION_TIME_SPINNER_H
#define __OVITO_ANIMATION_TIME_SPINNER_H

#include <core/Core.h>
#include <core/gui/widgets/general/SpinnerWidget.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A spinner control for the current animation time.
 */
class OVITO_CORE_EXPORT AnimationTimeSpinner : public SpinnerWidget
{
	Q_OBJECT
	
public:
	
	/// Constructs the spinner control.
	AnimationTimeSpinner(MainWindow* mainWindow, QWidget* parent = 0);

protected Q_SLOTS:

	/// Is called when a another dataset has become the active dataset.
	void onDataSetReplaced(DataSet* newDataSet);

	/// This is called when new animation settings have been loaded.
	void onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings);

	/// This is called by the AnimManager when the current animation time has changed.
	void onTimeChanged(TimePoint newTime);

	/// This is called by the AnimManager when the animation interval has changed.
	void onIntervalChanged(TimeInterval newAnimationInterval);
	
	/// Is called when the spinner value has been changed by the user.
	void onSpinnerValueChanged();

private:

	/// The current animation settings object.
	AnimationSettings* _animSettings;

	QMetaObject::Connection _animIntervalChangedConnection;
	QMetaObject::Connection _timeChangedConnection;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_ANIMATION_TIME_SPINNER_H
