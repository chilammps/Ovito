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

#include <plugins/pyscript/PyScript.h>
#include <core/gui/app/Application.h>
#include <core/dataset/DataSetContainer.h>
#include "ScriptAutostarter.h"
#include "ScriptEngine.h"

namespace PyScript {

IMPLEMENT_OVITO_OBJECT(PyScript, ScriptAutostarter, AutoStartObject);

/******************************************************************************
* Registers plugin-specific command line options.
******************************************************************************/
void ScriptAutostarter::registerCommandLineOptions(QCommandLineParser& cmdLineParser)
{
	// Register the --script command line option.
	cmdLineParser.addOption(QCommandLineOption("script", tr("Runs a Python script file."), tr("FILE")));

	// Register the --scriptarg command line option.
	cmdLineParser.addOption(QCommandLineOption("scriptarg", tr("Passes a command line option to the Python script."), tr("ARG")));

	// Register the --exec command line option.
	cmdLineParser.addOption(QCommandLineOption("exec", tr("Executes a single Python statement."), tr("CMD")));
}

/******************************************************************************
* Is called after the application has been completely initialized.
******************************************************************************/
void ScriptAutostarter::applicationStarted()
{
	// Execute the script commands and files passed on the command line.
	QStringList scriptCommands = Application::instance().cmdLineParser().values("exec");
	QStringList scriptFiles = Application::instance().cmdLineParser().values("script");

	if(!scriptCommands.empty() || !scriptFiles.empty()) {

		// Get the current dataset.
		DataSet* dataset = Application::instance().datasetContainer()->currentSet();

		// Suppress undo recording. Actions performed by startup scripts cannot be undone.
		UndoSuspender noUndo(dataset);

		// Set up script engine.
		ScriptEngine engine(dataset);

		// Pass command line parameters to the script.
		QStringList scriptArguments = Application::instance().cmdLineParser().values("scriptarg");

		// Execute script commands.
		for(int index = scriptCommands.size() - 1; index >= 0; index--) {
			const QString& command = scriptCommands[index];
			try {
				engine.execute(command, scriptArguments);
			}
			catch(Exception& ex) {
				ex.prependGeneralMessage(tr("Error during Python script execution."));
				throw;
			}
		}

		// Execute script files.
		for(int index = scriptFiles.size() - 1; index >= 0; index--) {
			engine.executeFile(scriptFiles[index], scriptArguments);
		}
	}
}

};
