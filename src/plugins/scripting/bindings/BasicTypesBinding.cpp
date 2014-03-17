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
	qRegisterMetaType<TimePoint>("TimePoint");
	qRegisterMetaType<Vector3>("Vector3");
	qRegisterMetaType<Point3>("Point3");
	qRegisterMetaType<Color>("Color");
	qRegisterMetaType<AffineTransformation>("AffineTransformation");
	qRegisterMetaType<Matrix3>("Matrix3");
	qRegisterMetaType<TimeInterval>("TimeInterval");

	// Set prototype for Vector3 script values and register constructor functions.
	QScriptValue vector3Prototype = engine.newQObject(new Vector3Prototype());
	vector3Prototype.setProperty("toString", engine.newFunction(&Vector3Prototype::toString, 0));
	vector3Prototype.setProperty("toArray", engine.newFunction(&Vector3Prototype::toArray, 0));
	engine.globalObject().setProperty("Vector", engine.newFunction(Vector3Prototype::constructor, vector3Prototype));
	qScriptRegisterMetaType<Vector3>(&engine, &Vector3Prototype::toScriptValue, &Vector3Prototype::fromScriptValue, vector3Prototype);

	// Set prototype for Point3 script values and register constructor functions.
	QScriptValue point3Prototype = engine.newQObject(new Point3Prototype());
	point3Prototype.setProperty("toString", engine.newFunction(&Point3Prototype::toString, 0));
	point3Prototype.setProperty("toArray", engine.newFunction(&Point3Prototype::toArray, 0));
	engine.globalObject().setProperty("Point", engine.newFunction(Point3Prototype::constructor, point3Prototype));
	qScriptRegisterMetaType<Point3>(&engine, &Point3Prototype::toScriptValue, &Point3Prototype::fromScriptValue, point3Prototype);

	// Set prototype for Color script values and register constructor functions.
	QScriptValue colorPrototype = engine.newQObject(new ColorPrototype());
	colorPrototype.setProperty("toString", engine.newFunction(&ColorPrototype::toString, 0));
	colorPrototype.setProperty("toArray", engine.newFunction(&ColorPrototype::toArray, 0));
	engine.globalObject().setProperty("Color", engine.newFunction(ColorPrototype::constructor, colorPrototype));
	qScriptRegisterMetaType<Color>(&engine, &ColorPrototype::toScriptValue, &ColorPrototype::fromScriptValue, colorPrototype);

	// Set prototype for TimeInterval script values and register constructor functions.
	QScriptValue timeIntervalPrototype = engine.newQObject(new TimeIntervalPrototype());
	timeIntervalPrototype.setProperty("toString", engine.newFunction(&TimeIntervalPrototype::toString, 0));
	engine.setDefaultPrototype(qRegisterMetaType<TimeInterval>("TimeInterval"), timeIntervalPrototype);
	engine.globalObject().setProperty("TimeInterval", engine.newFunction(TimeIntervalPrototype::constructor, timeIntervalPrototype));

	// Set prototype for AffineTransformation script values and register constructor functions.
	QScriptValue affineTransformationPrototype = engine.newQObject(new AffineTransformationPrototype());
	affineTransformationPrototype.setProperty("toString", engine.newFunction(&AffineTransformationPrototype::toString, 0));
	engine.globalObject().setProperty("AffineTransformation", engine.newFunction(AffineTransformationPrototype::constructor, affineTransformationPrototype));
	qScriptRegisterMetaType<AffineTransformation>(&engine, &AffineTransformationPrototype::toScriptValue, &AffineTransformationPrototype::fromScriptValue, affineTransformationPrototype);
}

/******************************************************************************
* Constructor function for Vector3 values.
******************************************************************************/
QScriptValue Vector3Prototype::constructor(QScriptContext* context, QScriptEngine* engine)
{
	Vector3 v;
	if(context->argumentCount() == 3) {
		for(quint32 i = 0; i < 3; i++) {
			if(!context->argument(i).isNumber())
				return context->throwError(QScriptContext::TypeError, tr("Vector constructor: Argument %1 is not a number.").arg(i+1));
			v[i] = context->argument(i).toNumber();
		}
	}
	else if(context->argumentCount() == 1 && context->argument(0).isArray() && context->argument(0).property("length").toInt32() == 3) {
		for(quint32 i = 0; i < 3; i++) {
			if(!context->argument(0).property(i).isNumber())
				return context->throwError(QScriptContext::TypeError, tr("Vector constructor: List element %1 is not a number.").arg(i+1));
			v[i] = context->argument(0).property(i).toNumber();
		}
	}
	else {
		return context->throwError(tr("Vector constructor takes 3 arguments or an array with 3 elements."));
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
		for(quint32 i = 0; i < 3; i++) {
			if(!context->argument(i).isNumber())
				return context->throwError(QScriptContext::TypeError, tr("Point constructor: Argument %1 is not a number.").arg(i+1));
			p[i] = context->argument(i).toNumber();
		}
	}
	else if(context->argumentCount() == 1 && context->argument(0).isArray() && context->argument(0).property("length").toInt32() == 3) {
		for(quint32 i = 0; i < 3; i++) {
			if(!context->argument(0).property(i).isNumber())
				return context->throwError(QScriptContext::TypeError, tr("Point constructor: List element %1 is not a number.").arg(i+1));
			p[i] = context->argument(0).property(i).toNumber();
		}
	}
	else {
		return context->throwError(tr("Point constructor takes 3 arguments or an array with 3 elements."));
	}
	return engine->toScriptValue(p);
}

/******************************************************************************
* Constructor function for Color values.
******************************************************************************/
QScriptValue ColorPrototype::constructor(QScriptContext* context, QScriptEngine* engine)
{
	Color c;
	if(context->argumentCount() == 3) {
		for(quint32 i = 0; i < 3; i++) {
			if(!context->argument(i).isNumber())
				return context->throwError(QScriptContext::TypeError, tr("Color constructor: Argument %1 is not a number.").arg(i+1));
			c[i] = context->argument(i).toNumber();
		}
	}
	else if(context->argumentCount() == 1 && context->argument(0).isArray() && context->argument(0).property("length").toInt32() == 3) {
		for(quint32 i = 0; i < 3; i++) {
			if(!context->argument(0).property(i).isNumber())
				return context->throwError(QScriptContext::TypeError, tr("Color constructor: List element %1 is not a number.").arg(i+1));
			c[i] = context->argument(0).property(i).toNumber();
		}
	}
	else {
		return context->throwError(tr("Color constructor takes 3 arguments or an array with 3 elements."));
	}
	return engine->toScriptValue(c);
}

/******************************************************************************
* Constructor function for TimeInterval values.
******************************************************************************/
QScriptValue TimeIntervalPrototype::constructor(QScriptContext* context, QScriptEngine* engine)
{
	TimeInterval iv;
	if(context->argumentCount() == 1) {
		if(!context->argument(0).isNumber())
			return context->throwError(QScriptContext::TypeError, tr("TimeInterval constructor: Argument error: not a number."));
		iv.setInstant(context->argument(0).toInt32());
	}
	else if(context->argumentCount() == 2) {
		if(!context->argument(0).isNumber() || !context->argument(1).isNumber())
			return context->throwError(QScriptContext::TypeError, tr("TimeInterval constructor: Argument error: not a number."));
		iv = TimeInterval(context->argument(0).toInt32(), context->argument(1).toInt32());
	}
	else {
		return context->throwError(tr("TimeInterval constructor takes 1 or 2 arguments."));
	}
	return engine->toScriptValue(iv);
}

/******************************************************************************
* Constructor function for AffineTransformation values.
******************************************************************************/
QScriptValue AffineTransformationPrototype::constructor(QScriptContext* context, QScriptEngine* engine)
{
	AffineTransformation tm = AffineTransformation::Identity();
	return engine->toScriptValue(tm);
}

};
