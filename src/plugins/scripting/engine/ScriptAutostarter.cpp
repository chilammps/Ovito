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

#include <plugins/scripting/Scripting.h>
#include <core/gui/app/Application.h>
#include <core/gui/app/CommandLineParser.h>
#include <core/dataset/DataSetContainer.h>
#include "ScriptAutostarter.h"
#include "ScriptEngine.h"

namespace Scripting {

IMPLEMENT_OVITO_OBJECT(Scripting, ScriptAutostarter, AutoStartObject);

/******************************************************************************
* Registers plugin-specific command line options.
******************************************************************************/
void ScriptAutostarter::registerCommandLineOptions(CommandLineParser& cmdLineParser)
{
	// Register the --script command line option.
	cmdLineParser.addOption(CommandLineOption("script", tr("Runs a script."), tr("FILE")));
}

/******************************************************************************
* Is called after the application has been completely initialized.
******************************************************************************/
void ScriptAutostarter::applicationStarted()
{
	// Execute the script files passed on the command line.
	QStringList scriptFiles = Application::instance().cmdLineParser().values("script");
	for(const QString& scriptFile : scriptFiles) {

		// Load the script program from file.
		QFile programFile(scriptFile);
		if(!programFile.open(QIODevice::ReadOnly | QIODevice::Text))
			throw Exception(tr("Failed to load script file '%1': %2").arg(scriptFile).arg(programFile.errorString()));
		QString program = QTextStream(&programFile).readAll();

		// Get the current dataset.
		DataSet* dataset = Application::instance().datasetContainer()->currentSet();

		// Suppress undo recording. Actions performed by startup scripts cannot be undone.
		UndoSuspender noUndo(dataset);

		// Set up script engine.
		ScriptEngine engine(dataset);

		// Execute script program.
		engine.evaluate(program, scriptFile);

		// Handle script errors by throwing a C++ exception, which will be handled by the main program.
		if(engine.hasUncaughtException())
			throw Exception(tr("Script error in line %1: %2")
					.arg(engine.uncaughtExceptionLineNumber())
					.arg(engine.uncaughtException().toString()));
	}
}

};
