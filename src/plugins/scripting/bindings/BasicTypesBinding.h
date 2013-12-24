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

#ifndef __OVITO_SCRIPTING_BASIC_TYPES_BINDING_H
#define __OVITO_SCRIPTING_BASIC_TYPES_BINDING_H

#include <plugins/scripting/Scripting.h>
#include <plugins/scripting/engine/ScriptBinding.h>

namespace Scripting {

using namespace Ovito;

/**
 * \brief Installs script bindings for the basic data types of OVITO.
 */
class BasicTypesBinding : public ScriptBinding
{
public:

	/// \brief Default constructor.
	Q_INVOKABLE BasicTypesBinding() {}

	/// \brief Sets up the global object of the script engine.
	virtual void setupBinding(ScriptEngine& engine) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief Binding for the Vector3 data type.
 */
class Vector3Prototype : public QObject, public QScriptable
{
	Q_OBJECT

public:

	/// Constructor function.
	static QScriptValue constructor(QScriptContext* context, QScriptEngine* engine);

	/// Returns the x component of a vector.
	FloatType x() { return qscriptvalue_cast<Vector3>(thisObject()).x(); }

	/// Returns the y component of a vector.
	FloatType y() { return qscriptvalue_cast<Vector3>(thisObject()).y(); }

	/// Returns the z component of a vector.
	FloatType z() { return qscriptvalue_cast<Vector3>(thisObject()).z(); }

	Q_PROPERTY(FloatType x READ x);
	Q_PROPERTY(FloatType y READ y);
	Q_PROPERTY(FloatType z READ z);

	/// Converts a Vector3 value to a string.
	Q_INVOKABLE QString toString() { return qscriptvalue_cast<Vector3>(thisObject()).toString(); }

};

/**
 * \brief Binding for the Point3 data type.
 */
class Point3Prototype : public QObject, public QScriptable
{
	Q_OBJECT

public:

	/// Constructor function.
	static QScriptValue constructor(QScriptContext* context, QScriptEngine* engine);

	/// Returns the x coordinate of a point.
	FloatType x() { return qscriptvalue_cast<Point3>(thisObject()).x(); }

	/// Returns the y coordinate of a point.
	FloatType y() { return qscriptvalue_cast<Point3>(thisObject()).y(); }

	/// Returns the z coordinate of a point.
	FloatType z() { return qscriptvalue_cast<Point3>(thisObject()).z(); }

	Q_PROPERTY(FloatType x READ x);
	Q_PROPERTY(FloatType y READ y);
	Q_PROPERTY(FloatType z READ z);

	/// Converts a Point3 value to a string.
	Q_INVOKABLE QString toString() { return qscriptvalue_cast<Point3>(thisObject()).toString(); }

};

/**
 * \brief Binding for the Color data type.
 */
class ColorPrototype : public QObject, public QScriptable
{
	Q_OBJECT

public:

	/// Constructor function.
	static QScriptValue constructor(QScriptContext* context, QScriptEngine* engine);

	/// Returns the red component of a color.
	FloatType r() { return qscriptvalue_cast<Color>(thisObject()).r(); }

	/// Returns the green component of a color.
	FloatType g() { return qscriptvalue_cast<Color>(thisObject()).g(); }

	/// Returns the blue component of a color.
	FloatType b() { return qscriptvalue_cast<Color>(thisObject()).b(); }

	Q_PROPERTY(FloatType r READ r);
	Q_PROPERTY(FloatType g READ g);
	Q_PROPERTY(FloatType b READ b);

	/// Converts a Color value to a string.
	Q_INVOKABLE QString toString() { return qscriptvalue_cast<Color>(thisObject()).toString(); }

};

};	// End of namespace

#endif
