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
#include <core/gui/mainwin/MainWindow.h>
#include <core/dataset/DataSetContainer.h>
#include "UtilityCommandPage.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Initializes the utility panel.
******************************************************************************/
UtilityCommandPage::UtilityCommandPage(MainWindow* mainWindow, QWidget* parent) : QWidget(parent),
		_datasetContainer(mainWindow->datasetContainer()), utilitiesButtonGroup(nullptr)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(2,2,2,2);

	// Create the rollout container.
	rolloutContainer = new RolloutContainer(this);
	rolloutContainer->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	layout->addWidget(rolloutContainer, 1);

	// Create a rollout that displays the list of installed utility plugins.
	QWidget* utilityListPanel = new QWidget();
	QGridLayout* gridLayout = new QGridLayout(utilityListPanel);
	gridLayout->setContentsMargins(4,4,4,4);
	rolloutContainer->addRollout(utilityListPanel, tr("Utilities"));

#ifndef Q_OS_MACX
	utilityListPanel->setStyleSheet("QPushButton:checked { "
							   		"background-color: moccasin; "
							   		"}");
#endif

	// Close open utility when loading a new data set.
	connect(&_datasetContainer, &DataSetContainer::dataSetChanged, this, &UtilityCommandPage::closeUtility);

	utilitiesButtonGroup = new QButtonGroup(utilityListPanel);
	utilitiesButtonGroup->setExclusive(false);

	for(const OvitoObjectType* descriptor : PluginManager::instance().listClasses(UtilityApplet::OOType)) {
   		QString displayName = descriptor->displayName();

		// Create a button that activates the utility.
		QPushButton* btn = new QPushButton(displayName, utilityListPanel);
		btn->setCheckable(true);
		utilitiesButtonGroup->addButton(btn);
		utilityListPanel->layout()->addWidget(btn);

		// Associate button with the utility plugin class.
		btn->setProperty("ClassDescriptor", qVariantFromValue((void*)descriptor));
	}

	// Listen for events.
	connect(utilitiesButtonGroup, (void (QButtonGroup::*)(QAbstractButton*))&QButtonGroup::buttonClicked, this, &UtilityCommandPage::onUtilityButton);
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
		currentUtility = static_object_cast<UtilityApplet>(descriptor->createInstance(nullptr));
		currentButton = button;
		currentButton->setChecked(true);

		currentUtility->openUtility(_datasetContainer.mainWindow(), rolloutContainer, RolloutInsertionParameters().animate());
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
	OVITO_CHECK_OBJECT_POINTER(currentUtility);
	OVITO_CHECK_POINTER(currentButton);
	OVITO_ASSERT(currentButton->property("ClassDescriptor").value<void*>() == &currentUtility->getOOType());

	// Close the utility.
	currentUtility->closeUtility(rolloutContainer);

	// Deactivate the button.
	currentButton->setChecked(false);

	currentButton = nullptr;
	OVITO_CHECK_OBJECT_POINTER(currentUtility);
	currentUtility = nullptr;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
