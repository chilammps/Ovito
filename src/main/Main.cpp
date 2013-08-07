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
#include <core/gui/app/Application.h>

#ifdef OVITO_MONOLITHIC_BUILD
	#ifdef Q_OS_LINUX
		Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
	#endif
#endif

int main(int argc, char** argv)
{

#ifdef OVITO_MONOLITHIC_BUILD
	// If we build a monolithic executable with static libraries then
	// the core's resources are not automatically initialized. Therefore
	// it needs to be explicitely done here.
	Q_INIT_RESOURCE(core);
#endif

	// Initialize the application.
	Ovito::Application app(argc, argv);
	if(!app.initialize())
		return 1;

	// Enter event loop.
	int result = app.runApplication();

	// Shutdown application.
	app.shutdown();

	return result;
}
