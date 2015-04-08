///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2015) Alexander Stukowski
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

#ifndef __OVITO_CREATE_TRAJECTORY_APPLET_H
#define __OVITO_CREATE_TRAJECTORY_APPLET_H

#include <plugins/particles/Particles.h>
#include <core/plugins/utility/UtilityApplet.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief This utility applet creates a trajectory object from the selected particles.
 */
class OVITO_PARTICLES_EXPORT CreateTrajectoryApplet : public UtilityApplet
{
public:

	/// Constructor.
	Q_INVOKABLE CreateTrajectoryApplet() : UtilityApplet(), _panel(nullptr) {}

	/// Shows the UI of the utility in the given RolloutContainer.
	virtual void openUtility(MainWindow* mainWindow, RolloutContainer* container, const RolloutInsertionParameters& rolloutParams = RolloutInsertionParameters()) override;

	/// Removes the UI of the utility from the RolloutContainer.
	virtual void closeUtility(RolloutContainer* container) override;

public Q_SLOTS:

	/// Is called when the user clicks the 'Create trajectory' button.
	void onCreateTrajectory();

private:

	QWidget* _panel;
	MainWindow* _mainWindow;
	QRadioButton* _allParticlesButton;
	QRadioButton* _selectedParticlesButton;
	QRadioButton* _animationIntervalButton;
	QRadioButton* _customIntervalButton;
	SpinnerWidget* _customRangeStartSpinner;
	SpinnerWidget* _customRangeEndSpinner;
	SpinnerWidget* _everyNthFrameSpinner;

	Q_CLASSINFO("DisplayName", "Create particle trajectory");

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_CREATE_TRAJECTORY_APPLET_H
