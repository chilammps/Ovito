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
#include "RenderCommandPage.h"
#include "ModifyCommandPage.h"
#include "OverlayCommandPage.h"
#include "UtilityCommandPage.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* The constructor of the command panel class.
******************************************************************************/
CommandPanel::CommandPanel(MainWindow* mainWindow, QWidget* parent) : QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);

	// Create tab widget
	_tabWidget = new QTabWidget(this);
	layout->addWidget(_tabWidget, 1);

	// Create the tabs.
	_tabWidget->setDocumentMode(true);
	_tabWidget->addTab(_modifyPage = new ModifyCommandPage(mainWindow, _tabWidget), QIcon(":/core/mainwin/command_panel/tab_modify.png"), QString());
	_tabWidget->addTab(_renderPage = new RenderCommandPage(mainWindow, _tabWidget), QIcon(":/core/mainwin/command_panel/tab_render.png"), QString());
	_tabWidget->addTab(_overlayPage = new OverlayCommandPage(mainWindow, _tabWidget), QIcon(":/core/mainwin/command_panel/tab_overlays.png"), QString());
	_tabWidget->addTab(_utilityPage = new UtilityCommandPage(mainWindow, _tabWidget), QIcon(":/core/mainwin/command_panel/tab_utilities.png"), QString());
	_tabWidget->setTabToolTip(0, tr("Modify"));
	_tabWidget->setTabToolTip(1, tr("Render"));
	_tabWidget->setTabToolTip(2, tr("Overlays"));
	_tabWidget->setTabToolTip(3, tr("Utilities"));
	setCurrentPage(MODIFY_PAGE);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
