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
#include <core/animation/TimeInterval.h>

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

	/// Conversion function.
	static QScriptValue toScriptValue(QScriptEngine* engine, const Vector3& v) {
		return engine->newVariant(QVariant::fromValue(v));
	}

	/// Conversion function.
	static void fromScriptValue(const QScriptValue& sv, Vector3& v) {
		if(sv.isVariant())
			v = sv.toVariant().value<Vector3>();
		else if(sv.isArray() && sv.property("length").toInt32() == 3) {
			v.x() = sv.property(0).toNumber();
			v.y() = sv.property(1).toNumber();
			v.z() = sv.property(2).toNumber();
		}
	}

	/// Returns the x component of a vector.
	FloatType x() { return qscriptvalue_cast<Vector3>(thisObject()).x(); }

	/// Returns the y component of a vector.
	FloatType y() { return qscriptvalue_cast<Vector3>(thisObject()).y(); }

	/// Returns the z component of a vector.
	FloatType z() { return qscriptvalue_cast<Vector3>(thisObject()).z(); }

	/// Sets the x component of a vector.
	void setX(FloatType x) {
		if(Vector3* vec = qscriptvalue_cast<Vector3*>(thisObject()))
			vec->x() = x;
		else
	        context()->throwError(QScriptContext::TypeError, tr("Vector.prototype.setX: this object is not a Vector."));
	}

	/// Sets the y component of a vector.
	void setY(FloatType y) {
		if(Vector3* vec = qscriptvalue_cast<Vector3*>(thisObject()))
			vec->y() = y;
		else
	        context()->throwError(QScriptContext::TypeError, tr("Vector.prototype.setY: this object is not a Vector."));
	}

	/// Sets the z component of a vector.
	void setZ(FloatType z) {
		if(Vector3* vec = qscriptvalue_cast<Vector3*>(thisObject()))
			vec->z() = z;
		else
	        context()->throwError(QScriptContext::TypeError, tr("Vector.prototype.setZ: this object is not a Vector."));
	}

	/// Converts a Vector3 value to a string.
	static QScriptValue toString(QScriptContext* context, QScriptEngine* engine) {
		QScriptValue obj = context->thisObject();
		if(!obj.isVariant() && context->argumentCount() > 0)
			obj = context->argument(0);
		if(!obj.isVariant())
			return context->throwError(QScriptContext::TypeError, tr("Vector.prototype.toString: this object is not a Vector."));
		return qscriptvalue_cast<Vector3>(obj).toString();
	}

	/// Converts a Vector3 value to a script array.
	static QScriptValue toArray(QScriptContext* context, QScriptEngine* engine) {
		if(!context->thisObject().isVariant())
			return context->throwError(QScriptContext::TypeError, tr("Vector.prototype.toArray: this object is not a Vector."));
		return qScriptValueFromSequence(engine, qscriptvalue_cast<Vector3>(context->thisObject()));
	}

	/// Adds two vectors.
	Q_INVOKABLE Vector3 plus(const Vector3& b) {
		if(Vector3* vec = qscriptvalue_cast<Vector3*>(thisObject()))
			return (*vec) + b;
		else {
	        context()->throwError(QScriptContext::TypeError, tr("Vector.prototype.plus: this object is not a Vector."));
	        return Vector3::Zero();
		}
	}

	/// Subtracts a vector.
	Q_INVOKABLE Vector3 minus(const Vector3& b) {
		if(Vector3* vec = qscriptvalue_cast<Vector3*>(thisObject()))
			return (*vec) - b;
		else {
	        context()->throwError(QScriptContext::TypeError, tr("Vector.prototype.minus: this object is not a Vector."));
	        return Vector3::Zero();
		}
	}

	/// Computes the dot product of two vectors.
	Q_INVOKABLE FloatType dot(const Vector3& b) {
		if(Vector3* vec = qscriptvalue_cast<Vector3*>(thisObject()))
			return vec->dot(b);
		else {
	        context()->throwError(QScriptContext::TypeError, tr("Vector.prototype.dot: this object is not a Vector."));
	        return 0;
		}
	}

	/// Computes the cross product of two vectors.
	Q_INVOKABLE Vector3 cross(const Vector3& b) {
		if(Vector3* vec = qscriptvalue_cast<Vector3*>(thisObject()))
			return vec->cross(b);
		else {
	        context()->throwError(QScriptContext::TypeError, tr("Vector.prototype.cross: this object is not a Vector."));
	        return Vector3::Zero();
		}
	}

	/// Compares two vectors.
	Q_INVOKABLE bool equals(const Vector3& b) {
		if(Vector3* vec = qscriptvalue_cast<Vector3*>(thisObject()))
			return (*vec) == b;
		else {
	        context()->throwError(QScriptContext::TypeError, tr("Vector.prototype.equals: this object is not a Vector."));
	        return false;
		}
	}

	Q_PROPERTY(FloatType x READ x WRITE setX);
	Q_PROPERTY(FloatType y READ y WRITE setY);
	Q_PROPERTY(FloatType z READ z WRITE setZ);
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

	/// Conversion function.
	static QScriptValue toScriptValue(QScriptEngine* engine, const Point3& p) {
		return engine->newVariant(QVariant::fromValue(p));
	}

	/// Conversion function.
	static void fromScriptValue(const QScriptValue& sv, Point3& p) {
		if(sv.isVariant())
			p = sv.toVariant().value<Point3>();
		else if(sv.isArray() && sv.property("length").toInt32() == 3) {
			p.x() = sv.property(0).toNumber();
			p.y() = sv.property(1).toNumber();
			p.z() = sv.property(2).toNumber();
		}
	}

	/// Returns the x coordinate of a point.
	FloatType x() { return qscriptvalue_cast<Point3>(thisObject()).x(); }

	/// Returns the y coordinate of a point.
	FloatType y() { return qscriptvalue_cast<Point3>(thisObject()).y(); }

	/// Returns the z coordinate of a point.
	FloatType z() { return qscriptvalue_cast<Point3>(thisObject()).z(); }

	/// Sets the x component of a point.
	void setX(FloatType x) {
		if(Point3* p = qscriptvalue_cast<Point3*>(thisObject()))
			p->x() = x;
		else
	        context()->throwError(QScriptContext::TypeError, tr("Point.prototype.setX: this object is not a Point."));
	}

	/// Sets the y component of a point.
	void setY(FloatType y) {
		if(Point3* p = qscriptvalue_cast<Point3*>(thisObject()))
			p->y() = y;
		else
	        context()->throwError(QScriptContext::TypeError, tr("Point.prototype.setY: this object is not a Point."));
	}

	/// Sets the z component of a point.
	void setZ(FloatType z) {
		if(Point3* p = qscriptvalue_cast<Point3*>(thisObject()))
			p->z() = z;
		else
	        context()->throwError(QScriptContext::TypeError, tr("Point.prototype.setZ: this object is not a Point."));
	}

	/// Converts a Point3 value to a string.
	static QScriptValue toString(QScriptContext* context, QScriptEngine* engine) {
		QScriptValue obj = context->thisObject();
		if(!obj.isVariant() && context->argumentCount() > 0)
			obj = context->argument(0);
		if(!obj.isVariant())
			return context->throwError(QScriptContext::TypeError, tr("Point.prototype.toString: this object is not a Point."));
		return qscriptvalue_cast<Point3>(obj).toString();
	}

	/// Converts a Point3 value to a script array.
	static QScriptValue toArray(QScriptContext* context, QScriptEngine* engine) {
		if(!context->thisObject().isVariant())
			return context->throwError(QScriptContext::TypeError, tr("Point.prototype.toArray: this object is not a Point."));
		return qScriptValueFromSequence(engine, qscriptvalue_cast<Point3>(context->thisObject()));
	}

	/// Compares two points.
	Q_INVOKABLE bool equals(const Point3& b) {
		if(Point3* p = qscriptvalue_cast<Point3*>(thisObject()))
			return (*p) == b;
		else {
	        context()->throwError(QScriptContext::TypeError, tr("Point.prototype.equals: this object is not a Point."));
	        return false;
		}
	}

	Q_PROPERTY(FloatType x READ x WRITE setX);
	Q_PROPERTY(FloatType y READ y WRITE setY);
	Q_PROPERTY(FloatType z READ z WRITE setZ);
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

	/// Conversion function.
	static QScriptValue toScriptValue(QScriptEngine* engine, const Color& c) {
		return engine->newVariant(QVariant::fromValue(c));
	}

	/// Conversion function.
	static void fromScriptValue(const QScriptValue& sv, Color& c) {
		if(sv.isVariant())
			c = sv.toVariant().value<Color>();
		else if(sv.isArray() && sv.property("length").toInt32() == 3) {
			c.r() = sv.property(0).toNumber();
			c.g() = sv.property(1).toNumber();
			c.b() = sv.property(2).toNumber();
		}
	}

	/// Returns the red component of a color.
	FloatType r() { return qscriptvalue_cast<Color>(thisObject()).r(); }

	/// Returns the green component of a color.
	FloatType g() { return qscriptvalue_cast<Color>(thisObject()).g(); }

	/// Returns the blue component of a color.
	FloatType b() { return qscriptvalue_cast<Color>(thisObject()).b(); }

	/// Sets the red component of a color.
	void setR(FloatType r) {
		if(Color* c = qscriptvalue_cast<Color*>(thisObject()))
			c->r() = r;
		else
	        context()->throwError(QScriptContext::TypeError, tr("Color.prototype.setR: this object is not a Color."));
	}

	/// Sets the green component of a color.
	void setG(FloatType g) {
		if(Color* c = qscriptvalue_cast<Color*>(thisObject()))
			c->g() = g;
		else
	        context()->throwError(QScriptContext::TypeError, tr("Color.prototype.setG: this object is not a Color."));
	}

	/// Sets the blue component of a color.
	void setB(FloatType b) {
		if(Color* c = qscriptvalue_cast<Color*>(thisObject()))
			c->b() = b;
		else
	        context()->throwError(QScriptContext::TypeError, tr("Color.prototype.setB: this object is not a Color."));
	}

	/// Converts a Color value to a string.
	static QScriptValue toString(QScriptContext* context, QScriptEngine* engine) {
		QScriptValue obj = context->thisObject();
		if(!obj.isVariant() && context->argumentCount() > 0)
			obj = context->argument(0);
		if(!obj.isVariant())
			return context->throwError(QScriptContext::TypeError, tr("Color.prototype.toString: this object is not a Color."));
		return qscriptvalue_cast<Color>(obj).toString();
	}

	/// Converts a Color value to a script array.
	static QScriptValue toArray(QScriptContext* context, QScriptEngine* engine) {
		if(!context->thisObject().isVariant())
			return context->throwError(QScriptContext::TypeError, tr("Color.prototype.toArray: this object is not a Color."));
		return qScriptValueFromSequence(engine, qscriptvalue_cast<Color>(context->thisObject()));
	}

	Q_PROPERTY(FloatType r READ r WRITE setR);
	Q_PROPERTY(FloatType g READ g WRITE setG);
	Q_PROPERTY(FloatType b READ b WRITE setB);
};

/**
 * \brief Binding for the TimeInterval data type.
 */
class TimeIntervalPrototype : public QObject, public QScriptable
{
	Q_OBJECT

public:

	/// Constructor function.
	static QScriptValue constructor(QScriptContext* context, QScriptEngine* engine);

	TimePoint start() { return qscriptvalue_cast<TimeInterval>(thisObject()).start(); }
	TimePoint end() { return qscriptvalue_cast<TimeInterval>(thisObject()).end(); }
	TimePoint duration() { return qscriptvalue_cast<TimeInterval>(thisObject()).duration(); }
	bool isEmpty() { return qscriptvalue_cast<TimeInterval>(thisObject()).isEmpty(); }

	Q_PROPERTY(TimePoint start READ start);
	Q_PROPERTY(TimePoint end READ end);
	Q_PROPERTY(TimePoint duration READ duration);
	Q_PROPERTY(bool isEmpty READ isEmpty);

	/// Converts a TimeInterval to a string.
	static QScriptValue toString(QScriptContext* context, QScriptEngine* engine) {
		QScriptValue obj = context->thisObject();
		if(!obj.isVariant() && context->argumentCount() > 0)
			obj = context->argument(0);
		if(!obj.isVariant())
			return context->throwError(QScriptContext::TypeError, tr("TimeInterval.prototype.toString: this object is not a TimeInterval."));
		TimeInterval interval = qscriptvalue_cast<TimeInterval>(obj);
		return QStringLiteral("[%1,%2]").arg(interval.start()).arg(interval.end());
	}
};

};	// End of namespace

#endif
