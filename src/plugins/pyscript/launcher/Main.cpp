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
 * This is the main entry point for "ovitos" script launcher program.
 *
 * It performs preprocessing of the command line arguments to convert them to the format expected by OVITO.
 */
int main(int argc, char** argv)
{
	std::vector<const char*> newargv;
	newargv.push_back(*argv++);
	argc--;
	
	const char* loadFile = nullptr;
	bool graphicalMode = false;
	bool execMode = false;
	while(argc > 0) {
		if(strcmp(*argv, "-o") == 0) {
			if(argc >= 2)
				loadFile = argv[1];
			argv += 2;
			argc -= 2;
		}
		else if(strcmp(*argv, "-m") == 0) {
			if(argc >= 2) {
				newargv.push_back("--exec");
				static std::string moduleCommand("import runpy; runpy.run_module('");
				moduleCommand += argv[1];
				moduleCommand += "', run_name='__main__');"; 
				newargv.push_back(moduleCommand.c_str());
			}
			argv += 2;
			argc -= 2;
			execMode = true;
			break;
		}
		else if(strcmp(*argv, "-c") == 0) {
			if(argc >= 2) {
				newargv.push_back("--exec");
				newargv.push_back(argv[1]);
			}
			argv += 2;
			argc -= 2;
			execMode = true;
			break;
		}
		else if(strcmp(*argv, "-h") == 0 || strcmp(*argv, "--help") == 0) {
			std::cout << "OVITO Script Interpreter" << std::endl << std::endl;
			std::cout << "Usage: ovitos [-o FILE] [-g] [-v] [-c command | -m module-name | script-file] [arguments]" << std::endl;
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
	
	if(!execMode) {
		if(argc >= 1) {
			// Parse script name and any subsequent arguments.
			newargv.push_back("--script");
			newargv.push_back(*argv++);
			argc--;
		}
		else {
			// If no script file has been specified, activate interactive interpreter mode.
			newargv.push_back("--exec");
#if WIN32
			newargv.push_back("import code; code.interact(banner=\"This is OVITO\'s interactive Python interpreter. Use quit() or Ctrl-Z to exit.\");");
#else
			newargv.push_back("import code; code.interact(banner=\"This is OVITO\'s interactive Python interpreter. Use quit() or Ctrl-D to exit.\");");
#endif
		}
	}

	// Escape script arguments with --scriptarg option.
	while(argc > 0) {
		newargv.push_back("--scriptarg");
		newargv.push_back(*argv++);
		argc--;
	}
	
	// The OVITO file to be loaded must come last in the parameter list passed to OVITO.
	if(loadFile) newargv.push_back(loadFile);

	// Initialize the application.
	int newargc = (int)newargv.size();
	if(!Ovito::Application().instance().initialize(newargc, const_cast<char**>(newargv.data())))
		return 1;

	// Enter event loop.
	int result = Ovito::Application().instance().runApplication();

	// Shut down application.
	Ovito::Application().instance().shutdown();

	return result;
}
