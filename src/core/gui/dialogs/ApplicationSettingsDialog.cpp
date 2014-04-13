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
#include <core/plugins/PluginManager.h>
#include "ApplicationSettingsDialog.h"

namespace Ovito {
	
IMPLEMENT_OVITO_OBJECT(Core, ApplicationSettingsPage, OvitoObject)

/******************************************************************************
* The constructor of the settings dialog class.
******************************************************************************/
ApplicationSettingsDialog::ApplicationSettingsDialog(QWidget* parent) : QDialog(parent)
{
	setWindowTitle(tr("Application Settings")); 
	
	QVBoxLayout* layout1 = new QVBoxLayout(this);
	
	// Create dialog contents.
	_tabWidget = new QTabWidget(this);
	layout1->addWidget(_tabWidget);

	// Create an iterator that retrieves all ApplicationSettingsPage derived classes.
	Q_FOREACH(OvitoObjectType* clazz, PluginManager::instance().listClasses(ApplicationSettingsPage::OOType)) {
		try {
			OORef<ApplicationSettingsPage> page = static_object_cast<ApplicationSettingsPage>(clazz->createInstance(nullptr));
			_pages.push_back(page);
			page->insertSettingsDialogPage(this, _tabWidget);
		}
		catch(const Exception& ex) {
			ex.showError();
		}	
	}

	// Ok and Cancel buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &ApplicationSettingsDialog::onOk);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &ApplicationSettingsDialog::reject);
	layout1->addWidget(buttonBox);
}

/******************************************************************************
* This is called when the user has pressed the OK button of the settings dialog.
* Validates and saves all settings made by the user and closes the dialog box.
******************************************************************************/
void ApplicationSettingsDialog::onOk()
{
	try {
		// Let all pages save their settings.
		for(const OORef<ApplicationSettingsPage>& page : _pages) {
			if(!page->saveValues(this, _tabWidget)) {
				return;
			}
		}
		
		// Close dialog box.
		accept();
	}
	catch(const Exception& ex) {
		ex.showError();
		return;
	}
}

};
