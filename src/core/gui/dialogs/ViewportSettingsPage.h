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

#ifndef __OVITO_VIEWPORT_SETTINGS_PAGE_H
#define __OVITO_VIEWPORT_SETTINGS_PAGE_H

#include <core/Core.h>
#include <core/gui/dialogs/ApplicationSettingsDialog.h>
#include <core/viewport/ViewportSettings.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * Page of the application settings dialog, which hosts viewport-related program options.
 */
class OVITO_CORE_EXPORT ViewportSettingsPage : public ApplicationSettingsDialogPage
{
public:

	/// Default constructor.
	Q_INVOKABLE ViewportSettingsPage() : ApplicationSettingsDialogPage() {}

	/// \brief Creates the widget.
	virtual void insertSettingsDialogPage(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget) override;

	/// \brief Lets the settings page to save all values entered by the user.
	/// \param settingsDialog The settings dialog box.
	virtual bool saveValues(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget) override;

private:

	/// The settings object being modified.
	ViewportSettings _settings;

	QButtonGroup* _upDirectionGroup;
	QCheckBox* _restrictVerticalRotationBox;
	QButtonGroup* _colorScheme;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_VIEWPORT_SETTINGS_PAGE_H
