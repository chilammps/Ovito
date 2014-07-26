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

#ifndef __OVITO_PYSCRIPT_ENGINE_H
#define __OVITO_PYSCRIPT_ENGINE_H

#include <plugins/pyscript/PyScript.h>
#include <core/dataset/DataSet.h>
#include "ScriptBinding.h"

namespace PyScript {

using namespace Ovito;

/**
 * \brief A scripting engine that provides bindings to OVITO's C++ classes.
 */
class OVITO_PYSCRIPT_EXPORT ScriptEngine : public QObject
{
public:

	/// \brief Initializes the scripting engine and sets up the environment.
	/// \param dataset The engine will execute scripts in the context of this dataset.
	/// \param parent The owner of this QObject.
	ScriptEngine(DataSet* dataset, QObject* parent = nullptr);

	/// \brief Returns the dataset that provides the context for the script.
	DataSet* dataset() const { return _dataset; }

	/// \brief Executes one or more Python statements.
	void execute(const QString& commands);

	/// \brief Executes a Python program.
	void executeFile(const QString& file);

private:

	/// Initializes the Python interpreter and sets up the global namespace.
	static void initializeInterpreter();

	/// The dataset that provides the context for the script execution.
	OORef<DataSet> _dataset;

	/// The namespace (scope) the script will be executed in by this script engine.
	boost::python::object _mainNamespace;

	/// The prototype namespace that is used to initialize new script engines with.
	static boost::python::object _prototypeMainNamespace;

	Q_OBJECT
};

};	// End of namespace

#endif
