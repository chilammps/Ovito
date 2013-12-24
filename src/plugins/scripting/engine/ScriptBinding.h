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

#ifndef __OVITO_SCRIPTING_SCRIPT_BINDING_H
#define __OVITO_SCRIPTING_SCRIPT_BINDING_H

#include <plugins/scripting/Scripting.h>

namespace Scripting {

using namespace Ovito;

class ScriptEngine;		// defined in ScriptEngine.h

/**
 * \brief Abstract base class for script bindings of OVITO plugins.
 *
 * Plugins that would like to make their functions and classes available to scripts should
 * derive a class from ScriptBinding. The ScriptEngine will automatically create an instance
 * of every ScriptBinding class to allow them to set up and modify the script environment.
 *
 * Each ScriptBinding object is destroyed together with the ScriptEngine that created it.
 */
class OVITO_SCRIPTING_EXPORT ScriptBinding : public OvitoObject
{
public:

	/// \brief Default constructor.
	ScriptBinding() {}

	/// \brief Is called to set up the global object of the script engine.
	///
	/// This method is called by the ScriptEngine to initialize the script environment.
	/// Implementations of this method should call functions of the ScriptEngine to
	/// set up the binding and make C++ functions and types available.
	virtual void setupBinding(ScriptEngine& engine) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif
