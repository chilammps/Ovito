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
#include <core/gui/app/Application.h>

/**
 * This is the main entry point for the Windows version of the "ovitos.exe" script launcher program.
 *
 * It is similar to the main "ovito.exe" program, but performs some preprocessing of the command
 * line parameters to give them the format expected by OVITO.
 */
int main(int argc, char** argv)
{
	// Preprocess command line parameters in a similar manner as the "ovitos" Bash shell script on Linux does.
	std::vector<const char*> newargv;
	newargv.push_back(*argv++);
	argc--;
	
	const char* loadFile = nullptr;
	bool graphicalMode = false;
	while(argc > 0) {
		if(strcmp(*argv, "-o") == 0) {
			if(argc >= 2)
				loadFile = argv[1];
			argv += 2;
			argc -= 2;
		}
		else if(strcmp(*argv, "-h") == 0 || strcmp(*argv, "--help") == 0) {
			std::cout << "OVITO Script Interpreter" << std::endl << std::endl;
			std::cout << "Usage: ovitos.exe [-o FILE] [-g] [-v] [script.py] [args...]" << std::endl;
			return 0;
		}
		else if(strcmp(*argv, "-v") == 0 || strcmp(*argv, "--version") == 0) {
			argc--;
			newargv.push_back(*argv++);
		}
		else if(strcmp(*argv, "-g") == 0 || strcmp(*argv, "--gui") == 0) {
			argc--;
			argv++;
			graphicalMode = true;
		}
		else break;
	}
	if(!graphicalMode)
		newargv.insert(newargv.begin() + 1, "--nogui");
	
	if(argc >= 1) {
		// Parse script name and any subsequent arguments.
		newargv.push_back("--script");
		newargv.push_back(*argv++);
		argc--;
		while(argc > 0) {
			// Escape script arguments with --scriptarg option.
			newargv.push_back("--scriptarg");
			newargv.push_back(*argv++);
			argc--;
		}
	}
	else {
		// If no script file has been specified, activate interactive interpreter mode.
		newargv.push_back("--exec");
		newargv.push_back("import code; code.interact(banner=\"This is OVITO\'s interactive Python interpreter. Use quit() or Ctrl-Z to exit.\");");
	}
	
	// The OVITO file to be loaded must come last in the parameter list passed to OVITO.
	if(loadFile) newargv.push_back(loadFile);

	// Initialize the application.
	int newargc = newargv.size();
	if(!Ovito::Application().instance().initialize(newargc, const_cast<char**>(newargv.data())))
		return 1;

	// Enter event loop.
	int result = Ovito::Application().instance().runApplication();

	// Shut down application.
	Ovito::Application().instance().shutdown();

	return result;
}
