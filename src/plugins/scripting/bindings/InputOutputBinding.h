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

#ifndef __OVITO_SCRIPTING_INPUT_OUTPUT_BINDING_H
#define __OVITO_SCRIPTING_INPUT_OUTPUT_BINDING_H

#include <plugins/scripting/Scripting.h>
#include <plugins/scripting/engine/ScriptBinding.h>

namespace Scripting {

using namespace Ovito;

/**
 * \brief Exposes OVITO's file input/output functions to scripts.
 */
class InputOutputBinding : public ScriptBinding
{
public:

	/// \brief Default constructor.
	Q_INVOKABLE InputOutputBinding() {}

	/// \brief Sets up the global object of the script engine.
	virtual void setupBinding(ScriptEngine& engine) override;

public:

	/// Implementation of the 'load' script command.
	static QScriptValue load(QScriptContext* context, ScriptEngine* engine);

	/// Implementation of the 'save' script command.
	static QScriptValue save(QScriptContext* context, ScriptEngine* engine);

	/// Implementation of the 'cd' script command.
	static QScriptValue cd(QScriptContext* context, ScriptEngine* engine);

	/// Implementation of the 'pwd' script command.
	static QScriptValue pwd(QScriptContext* context, ScriptEngine* engine);

	/// Implementation of the 'wait' script command.
	static QScriptValue wait(QScriptContext* context, ScriptEngine* engine);

	/// Implementation of the 'assert' script function.
	static QScriptValue assertFunction(QScriptContext* context, ScriptEngine* engine);

	/// Converts a QUrl to a script value.
	static QScriptValue fromQUrl(QScriptEngine *engine, const QUrl& url);

	/// Converts a script value to a QUrl.
	static void toQUrl(const QScriptValue& sv, QUrl& url);

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif
