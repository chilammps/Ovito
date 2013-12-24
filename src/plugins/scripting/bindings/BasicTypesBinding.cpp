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

#include <plugins/scripting/Scripting.h>
#include <plugins/scripting/engine/ScriptEngine.h>
#include "BasicTypesBinding.h"

namespace Scripting {

IMPLEMENT_OVITO_OBJECT(Scripting, BasicTypesBinding, ScriptBinding);

/******************************************************************************
* Sets up the global object of the script engine.
******************************************************************************/
void BasicTypesBinding::setupBinding(ScriptEngine& engine)
{
	qRegisterMetaType<FloatType>("FloatType");

	// Set prototype for Vector3 script values and register constructor functions.
	QScriptValue vector3Prototype = engine.newQObject(new Vector3Prototype());
	engine.setDefaultPrototype(qRegisterMetaType<Vector3>("Vector3"), vector3Prototype);
	engine.globalObject().setProperty("Vector", engine.newFunction(Vector3Prototype::constructor, vector3Prototype));

	// Set prototype for Point3 script values and register constructor functions.
	QScriptValue point3Prototype = engine.newQObject(new Point3Prototype());
	engine.setDefaultPrototype(qRegisterMetaType<Point3>("Point3"), point3Prototype);
	engine.globalObject().setProperty("Point", engine.newFunction(Point3Prototype::constructor, point3Prototype));

	// Set prototype for Color script values and register constructor functions.
	QScriptValue colorPrototype = engine.newQObject(new ColorPrototype());
	engine.setDefaultPrototype(qRegisterMetaType<Color>("Color"), colorPrototype);
	engine.globalObject().setProperty("Color", engine.newFunction(ColorPrototype::constructor, colorPrototype));
}

/******************************************************************************
* Constructor function for Vector3 values.
******************************************************************************/
QScriptValue Vector3Prototype::constructor(QScriptContext* context, QScriptEngine* engine)
{
	Vector3 v;
	if(context->argumentCount() == 3) {
		v.x() = context->argument(0).toNumber();
		v.y() = context->argument(1).toNumber();
		v.z() = context->argument(2).toNumber();
	}
	else {
		return context->throwError("Vector constructor takes 3 arguments.");
	}
	return engine->toScriptValue(v);
}

/******************************************************************************
* Constructor function for Point3 values.
******************************************************************************/
QScriptValue Point3Prototype::constructor(QScriptContext* context, QScriptEngine* engine)
{
	Point3 p;
	if(context->argumentCount() == 3) {
		p.x() = context->argument(0).toNumber();
		p.y() = context->argument(1).toNumber();
		p.z() = context->argument(2).toNumber();
	}
	else {
		return context->throwError("Point constructor takes 3 arguments.");
	}
	return engine->toScriptValue(p);
}

/******************************************************************************
* Constructor function for Color values.
******************************************************************************/
QScriptValue ColorPrototype::constructor(QScriptContext* context, QScriptEngine* engine)
{
	Color c;
	if(context->argumentCount() == 1) {
		c.r() = c.g() = c.b() = context->argument(0).toNumber();
	}
	else if(context->argumentCount() == 3) {
		c.r() = context->argument(0).toNumber();
		c.g() = context->argument(1).toNumber();
		c.b() = context->argument(2).toNumber();
	}
	else {
		return context->throwError("Color constructor takes 1 or 3 arguments.");
	}
	return engine->toScriptValue(c);
}

};
