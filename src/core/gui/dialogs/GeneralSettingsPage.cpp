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

#include <core/Core.h>
#include "GeneralSettingsPage.h"

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, GeneralSettingsPage, ApplicationSettingsPage);

/******************************************************************************
* Creates the widget that contains the plugin specific setting controls.
******************************************************************************/
void GeneralSettingsPage::insertSettingsDialogPage(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget)
{
	QWidget* page = new QWidget();
	tabWidget->addTab(page, tr("General"));
	QVBoxLayout* layout1 = new QVBoxLayout(page);

	QSettings settings;

	QGroupBox* uiGroupBox = new QGroupBox(tr("User interface"), page);
	layout1->addWidget(uiGroupBox);
	QGridLayout* layout2 = new QGridLayout(uiGroupBox);

	_useQtFileDialog = new QCheckBox(tr("Use alternative file selection dialog"), uiGroupBox);
	_useQtFileDialog->setToolTip(tr(
			"<p>Use an alternative file selection dialog instead of the native dialog box provided by the operating system.</p>"));
	layout2->addWidget(_useQtFileDialog, 0, 0);
	_useQtFileDialog->setChecked(settings.value("file/use_qt_dialog", false).toBool());

	QGroupBox* updateGroupBox = new QGroupBox(tr("Program updates"), page);
	layout1->addWidget(updateGroupBox);
	layout2 = new QGridLayout(updateGroupBox);

	_enableUpdateChecks = new QCheckBox(tr("Auto-refresh news page from web server"), updateGroupBox);
	_enableUpdateChecks->setToolTip(tr(
			"<p>The news page is fetched from <i>www.ovito.org</i> and displayed on each program startup. "
			"It contains information about new program updates when they become available.</p>"));
	layout2->addWidget(_enableUpdateChecks, 0, 0);
	_enableUsageStatistics = new QCheckBox(tr("Send unique installation ID to web server"), updateGroupBox);
	_enableUsageStatistics->setToolTip(tr(
			"<p>Every installation of OVITO has a unique identifier, which is generated on first program start. "
			"This option enables the transmission of the anonymous identifier to the web server to help the developers collect "
			"program usage statistics.</p>"));
	layout2->addWidget(_enableUsageStatistics, 1, 0);

	_enableUpdateChecks->setChecked(settings.value("updates/check_for_updates", true).toBool());
	_enableUsageStatistics->setChecked(settings.value("updates/transmit_id", true).toBool());

	connect(_enableUpdateChecks, &QCheckBox::toggled, _enableUsageStatistics, &QCheckBox::setEnabled);
	_enableUsageStatistics->setEnabled(_enableUpdateChecks->isChecked());

	layout1->addStretch();
}

/******************************************************************************
* Lets the page save all changed settings.
******************************************************************************/
bool GeneralSettingsPage::saveValues(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget)
{
	QSettings settings;
	settings.setValue("file/use_qt_dialog", _useQtFileDialog->isChecked());
	settings.setValue("updates/check_for_updates", _enableUpdateChecks->isChecked());
	settings.setValue("updates/transmit_id", _enableUsageStatistics->isChecked());
	return true;
}

};
