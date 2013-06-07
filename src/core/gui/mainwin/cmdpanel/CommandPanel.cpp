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
#include "CommandPanel.h"
//#include "render/RenderCommandPage.h"

namespace Ovito {

/******************************************************************************
* The constructor of the command panel class.
******************************************************************************/
CommandPanel::CommandPanel(QWidget* parent) : QWidget(parent), lastPage(0)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0,0,0,0);

	// Create tab widget
	tabWidget = new QTabWidget(this);
	layout->addWidget(tabWidget, 1);

	// Create the pages.
	tabWidget->setDocumentMode(true);
	//tabWidget->addTab(_modifyPage = new ModifyCommandPage(), QIcon(":/core/command_panel/tab_modify.png"), QString());
	//tabWidget->addTab(_renderPage = new RenderCommandPage(), QIcon(":/core/command_panel/tab_render.png"), QString());
	//tabWidget->addTab(_utilityPage = new UtilityCommandPage(), QIcon(":/core/command_panel/tab_utilities.png"), QString());
	//tabWidget->setTabToolTip(0, tr("Modify"));
	//tabWidget->setTabToolTip(1, tr("Render"));
	//tabWidget->setTabToolTip(2, tr("Utilities"));
	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabSwitched()));

	setLayout(layout);

	connect(&DataSetManager::instance(), SIGNAL(selectionChangeComplete(SelectionSet*)), this, SLOT(onSelectionChangeComplete(SelectionSet*)));

	// Update the command panel when a new scene file has been loaded.
	connect(&DataSetManager::instance(), SIGNAL(dataSetReset(DataSet*)), this, SLOT(reset()));

	reset();
	setCurrentPage(MODIFY_PAGE);
}

/******************************************************************************
* Resets the command panel to the initial state.
******************************************************************************/
void CommandPanel::reset()
{
	// Reset each page.
	for(int i=0; i<tabWidget->count(); i++)
		((CommandPanelPage*)tabWidget->widget(i))->reset();
}

/******************************************************************************
* Activate one of the command pages.
******************************************************************************/
void CommandPanel::setCurrentPage(Page page)
{
#if 0
	OVITO_ASSERT(page < tabWidget->count());
	tabWidget->setCurrentIndex((int)page);
#endif
}

/******************************************************************************
* Is called when the user has switched to another tab in the command panel.
******************************************************************************/
void CommandPanel::onTabSwitched()
{
	if(lastPage >= 0) {
		CommandPanelPage* page = qobject_cast<CommandPanelPage*>(tabWidget->widget(lastPage));
		OVITO_CHECK_POINTER(page);
		page->onLeave();
	}
	lastPage = tabWidget->currentIndex();
	if(lastPage >= 0) {
		CommandPanelPage* page = qobject_cast<CommandPanelPage*>(tabWidget->widget(lastPage));
		OVITO_CHECK_POINTER(page);
		page->onEnter();
	}
}

/******************************************************************************
* This is called after all changes to the selection set have been completed.
******************************************************************************/
void CommandPanel::onSelectionChangeComplete(SelectionSet* newSelection)
{
	CommandPanelPage* page = qobject_cast<CommandPanelPage*>(tabWidget->currentWidget());
	// Pass message on to current page.
	if(page != NULL)
		page->onSelectionChangeComplete(newSelection);
}

#if 0

/******************************************************************************
* Returns the object that is currently being edited in the command panel.
* Return NULL if no object is selected.
******************************************************************************/
RefTarget* CommandPanel::editObject() const
{
	ModifyCommandPage* modifyPage = qobject_cast<ModifyCommandPage*>(tabWidget->currentWidget());
	if(modifyPage)
		return modifyPage->editObject();
	return NULL;
}

#endif

};
