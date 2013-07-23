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
#include "UtilityApplet.h"
#include "UtilityCommandPage.h"

namespace Ovito {

/******************************************************************************
* Initializes the utility panel.
******************************************************************************/
UtilityCommandPage::UtilityCommandPage() : CommandPanelPage(),
	utilitiesButtonGroup(NULL)
{
	scanInstalledPlugins();

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(2,2,2,2);

	// Create the rollout container.
	rolloutContainer = new RolloutContainer(this);
	rolloutContainer->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	layout->addWidget(rolloutContainer, 1);

	// Create rollout that displays the list of installed utility plugins.
	utilityListPanel = new QWidget();
	QGridLayout* gridLayout = new QGridLayout();
#ifndef Q_OS_MACX
	gridLayout->setContentsMargins(4,4,4,4);
#endif
	utilityListPanel->setLayout(gridLayout);
	rolloutContainer->addRollout(utilityListPanel, tr("Utilities"));

#ifndef Q_OS_MACX
	utilityListPanel->setStyleSheet("QPushButton:checked { "
							   		"background-color: moccasin; "
							   		"}");
#endif

	setLayout(layout);
	rebuildUtilityList();
}


/******************************************************************************
* Resets the panel to the initial state.
******************************************************************************/
void UtilityCommandPage::reset()
{
	CommandPanelPage::reset();
}

/******************************************************************************
* Is called when the user selects another page.
******************************************************************************/
void UtilityCommandPage::onLeave()
{
	CommandPanelPage::onLeave();
	closeUtility();
}

/******************************************************************************
* Finds all utility classes provided by the installed plugins.
******************************************************************************/
void UtilityCommandPage::scanInstalledPlugins()
{
	Q_FOREACH(const OvitoObjectType* clazz, PluginManager::instance().listClasses(UtilityApplet::OOType)) {
		classes.push_back(clazz);
	}
}

/******************************************************************************
*Updates the displayed button in the utility selection panel.
******************************************************************************/
void UtilityCommandPage::rebuildUtilityList()
{
	Q_FOREACH(QObject* w, utilityListPanel->children())
		if(w->isWidgetType()) delete w;

	if(utilitiesButtonGroup) {
		delete utilitiesButtonGroup;
		utilitiesButtonGroup = NULL;
	}

	utilitiesButtonGroup = new QButtonGroup(utilityListPanel);
	utilitiesButtonGroup->setExclusive(false);

	Q_FOREACH(const OvitoObjectType* descriptor, classes) {

   		QString displayName = descriptor->name();

		// Create a button that activates the utility.
		QPushButton* btn = new QPushButton(displayName, utilityListPanel);
		btn->setCheckable(true);
		utilitiesButtonGroup->addButton(btn);
		utilityListPanel->layout()->addWidget(btn);

		// Associate button with the utility plugin class.
		btn->setProperty("ClassDescriptor", qVariantFromValue((void*)descriptor));
	}

	// Listen for events.
	connect(utilitiesButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onUtilityButton(QAbstractButton*)));

	// Resize the rollout panel.
	utilityListPanel->layout()->update();
}

/******************************************************************************
* Is called when the user invokes one of the utility plugins.
******************************************************************************/
void UtilityCommandPage::onUtilityButton(QAbstractButton* button)
{
	const OvitoObjectType* descriptor = static_cast<const OvitoObjectType*>(button->property("ClassDescriptor").value<void*>());
	OVITO_CHECK_POINTER(descriptor);

	if(button->isChecked() && currentUtility && currentUtility->getOOType() == *descriptor) {
		closeUtility();
		currentButton->setChecked(false);
		return;
	}

	// Close previous utility.
	closeUtility();

	try {
		// Create an instance of the utility plugin.
		currentUtility = static_object_cast<UtilityApplet>(descriptor->createInstance());
		currentButton = button;
		currentButton->setChecked(true);

		currentUtility->openUtility(rolloutContainer, RolloutInsertionParameters().animate());
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Closes the current utility.
******************************************************************************/
void UtilityCommandPage::closeUtility()
{
	if(!currentUtility) return;
	OVITO_CHECK_OBJECT_POINTER(currentUtility.get());
	OVITO_CHECK_POINTER(currentButton);
	OVITO_ASSERT(currentButton->property("ClassDescriptor").value<void*>() == &currentUtility->getOOType());

	// Close the utility.
	currentUtility->closeUtility(rolloutContainer);

	// Deactivate the button.
	currentButton->setChecked(false);

	currentButton = NULL;
	OVITO_CHECK_OBJECT_POINTER(currentUtility.get());
	currentUtility = NULL;
}

};
