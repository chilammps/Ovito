///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_GENERAL_SETTINGS_PAGE_H
#define __OVITO_GENERAL_SETTINGS_PAGE_H

#include <core/Core.h>
#include <core/gui/dialogs/ApplicationSettingsDialog.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * Page of the application settings dialog, which hosts general program options.
 */
class OVITO_CORE_EXPORT GeneralSettingsPage : public ApplicationSettingsDialogPage
{
public:

	/// Default constructor.
	Q_INVOKABLE GeneralSettingsPage() : ApplicationSettingsDialogPage() {}

	/// \brief Creates the widget.
	virtual void insertSettingsDialogPage(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget) override;

	/// \brief Lets the settings page to save all values entered by the user.
	/// \param settingsDialog The settings dialog box.
	virtual bool saveValues(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget) override;

private:

	QCheckBox* _useQtFileDialog;
	QCheckBox* _overrideGLContextSharing;
	QComboBox* _contextSharingMode;
	QCheckBox* _overrideUseOfPointSprites;
	QComboBox* _pointSpriteMode;
	QCheckBox* _overrideUseOfGeometryShaders;
	QComboBox* _geometryShaderMode;
	QCheckBox* _enableUpdateChecks;
	QCheckBox* _enableUsageStatistics;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_GENERAL_SETTINGS_PAGE_H
