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

#include <core/gui/mainwin/cmdpanel/CommandPanel.h>
#include <core/gui/widgets/RolloutContainer.h>
#include "UtilityApplet.h"

namespace Ovito {

/******************************************************************************
* The utility page lets the user invoke utility plugins.
******************************************************************************/
class OVITO_CORE_EXPORT UtilityCommandPage : public CommandPanelPage
{
	Q_OBJECT

public:

	/// Initializes the utility page.
    UtilityCommandPage();

	/// Resets the utility panel to the initial state.
	virtual void reset() override;

	/// Is called when the user selects another page.
	virtual void onLeave() override;

	/// Closes the current utility.
	void closeUtility();

protected Q_SLOTS:

	/// Is called when the user invokes one of the utility plugins.
	void onUtilityButton(QAbstractButton* button);

private:

	/// This panel shows the utility plugin UI.
	RolloutContainer* rolloutContainer;

	/// Displays the available utility plugins.
    QWidget* utilityListPanel;

	/// The list of installed utility plugin classes.
	QVector<const OvitoObjectType*> classes;

	/// The utility that is currently active or NULL.
	OORef<UtilityApplet> currentUtility;

	/// The button that has been activated by the user.
	QAbstractButton* currentButton;

	/// Contains one button per utility.
	QButtonGroup* utilitiesButtonGroup;

	/// Finds all utility classes provided by the installed plugins.
	void scanInstalledPlugins();

	/// Updates the displayed button in the utility selection panel.
	void rebuildUtilityList();
};


};
