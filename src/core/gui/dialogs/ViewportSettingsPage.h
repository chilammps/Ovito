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
#include <core/gui/widgets/ColorPickerWidget.h>
#include <core/viewport/ViewportSettings.h>

namespace Ovito {

/**
 * \brief A dialog page that is plugged into the application's settings dialog to let the user
 *        configure the general viewport settings.
 */
class OVITO_CORE_EXPORT ViewportSettingsPage : public ApplicationSettingsPage
{
public:

	/// Default constructor.
	Q_INVOKABLE ViewportSettingsPage() : ApplicationSettingsPage() {}

	/// \brief Creates the widget.
	virtual void insertSettingsDialogPage(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget) override;

	/// \brief Lets the settings page to save all values entered by the user.
	/// \param settingsDialog The settings dialog box.
	virtual bool saveValues(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget) override;

private Q_SLOTS:

	/// Is called when the user selects another entry in the color list.
	void onColorListItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

	/// Is called when the user has changed a viewport element color.
	void onColorChanged();

	/// Is called when the user clicks the "Restore defaults" button in the color section.
	void onRestoreDefaultColors();

private:

	/// The settings object being modified.
	ViewportSettings _settings;

	QButtonGroup* _upDirectionGroup;
	QListWidget* _colorList;
	ColorPickerWidget* _colorPicker;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_VIEWPORT_SETTINGS_PAGE_H
