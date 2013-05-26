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
#include <core/gui/actions/ActionManager.h>
#include <core/gui/mainwin/MainWindow.h>

namespace Ovito {

/******************************************************************************
* Handles ACTION_QUIT command.
******************************************************************************/
void ActionManager::on_Quit_triggered()
{
	if(!Application::instance().guiMode()) return;
	MainWindow::instance().close();
}

/******************************************************************************
* Handles ACTION_HELP_ABOUT command.
******************************************************************************/
void ActionManager::on_HelpAbout_triggered()
{
	if(!Application::instance().guiMode()) return;

	QString text = QString("Ovito (Open Visualization Tool)\n"
			"%1\n\n"
			"Visualization and analysis software for atomistic simulation data.\n\n"
			"Copyright 2013, Alexander Stukowski\n"
			"This program comes with ABSOLUTELY NO WARRANTY.\n"
			"This is free software, and you are welcome to redistribute\n"
			"it under certain conditions. See the source for copying conditions.\n\n"
			"http://www.ovito.org/").arg(QCoreApplication::applicationVersion());

	QMessageBox::about(&MainWindow::instance(), QCoreApplication::applicationName(), text);
}

/******************************************************************************
* Handles ACTION_HELP_SHOW_ONLINE_HELP command.
******************************************************************************/
void ActionManager::on_HelpShowOnlineHelp_triggered()
{
	if(!Application::instance().guiMode()) return;

	// Use the web browser to display online help.
	QString fullURL = QString("http://www.ovito.org/manual_v%1.%2.%3/")
					.arg(OVITO_VERSION_MAJOR)
					.arg(OVITO_VERSION_MINOR)
					.arg(OVITO_VERSION_REVISION);

	if(!QDesktopServices::openUrl(QUrl(fullURL))) {
		Exception(tr("Could not launch web browser to display online manual. The requested URL is %1").arg(fullURL)).showError();
	}
}

};
