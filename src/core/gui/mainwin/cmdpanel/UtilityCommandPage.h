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

#ifndef __OVITO_UTILITY_COMMAND_PAGE_H
#define __OVITO_UTILITY_COMMAND_PAGE_H

#include <core/Core.h>
#include <core/gui/widgets/general/RolloutContainer.h>
#include <core/plugins/utility/UtilityApplet.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * The utility page lets the user invoke utility plugins.
 */
class OVITO_CORE_EXPORT UtilityCommandPage : public QWidget
{
	Q_OBJECT

public:

	/// Initializes the utility page.
    UtilityCommandPage(MainWindow* mainWindow, QWidget* parent);

protected:

	/// This event handler is called when the page is hidden.
	void hideEvent(QHideEvent* event) override {
		QWidget::hideEvent(event);
		closeUtility();
	}

protected Q_SLOTS:

	/// Is called when the user invokes one of the utility plugins.
	void onUtilityButton(QAbstractButton* button);

	/// Closes the current utility.
	void closeUtility();

private:

	/// The container of the current dataset.
	DataSetContainer& _datasetContainer;

	/// This panel shows the utility plugin UI.
	RolloutContainer* rolloutContainer;

	/// The utility that is currently active or NULL.
	OORef<UtilityApplet> currentUtility;

	/// The button that has been activated by the user.
	QAbstractButton* currentButton;

	/// Contains one button per utility.
	QButtonGroup* utilitiesButtonGroup;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_UTILITY_COMMAND_PAGE_H
