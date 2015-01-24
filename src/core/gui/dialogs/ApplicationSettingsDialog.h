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

#ifndef __OVITO_APPLICATION_SETTINGS_DIALOG_H
#define __OVITO_APPLICATION_SETTINGS_DIALOG_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Dialogs)
	
class ApplicationSettingsDialog;		// defined below.

/**
 * \brief Abstract base class for tab providers for the application's settings dialog.
 */
class OVITO_CORE_EXPORT ApplicationSettingsDialogPage : public OvitoObject
{
protected:

	/// Base class constructor.
	ApplicationSettingsDialogPage() {}

public:

	/// \brief Creates the tab that is inserted into the settings dialog.
	/// \param settingsDialog The settings dialog box. 
	/// \param tabWidget The QTabWidget into which the method should insert the settings page.
	virtual void insertSettingsDialogPage(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget) = 0;

	/// \brief Lets the settings page to save all values entered by the user.
	/// \param settingsDialog The settings dialog box. 
	/// \return true if the settings are valid; false if settings need to be corrected by the user and the dialog should not be closed.
	virtual bool saveValues(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget) { return true; }

private:

	Q_OBJECT
	OVITO_OBJECT
};
	
/**
 * \brief The dialog window that lets the user change the global application settings.
 * 
 * Plugins can add additional pages to this dialog by deriving new classes from
 * the ApplicationSettingsDialogPage class.
 */
class OVITO_CORE_EXPORT ApplicationSettingsDialog : public QDialog
{
	Q_OBJECT
	
public:

	/// \brief Constructs the dialog window.
	/// \param parent The parent window of the settings dialog.
	ApplicationSettingsDialog(QWidget* parent);
	
protected Q_SLOTS:

	/// This is called when the user has pressed the OK button of the settings dialog.
	/// Validates and saves all settings made by the user and closes the dialog box.
	void onOk();

private:

	QVector<OORef<ApplicationSettingsDialogPage>> _pages;
	QTabWidget* _tabWidget;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_APPLICATION_SETTINGS_DIALOG_H
