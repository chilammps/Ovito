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
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

// Automatic Python to Vector3/Point3/Color conversion.
template<typename T>
struct python_to_vector_conversion
{
	python_to_vector_conversion() {
		converter::registry::push_back(&convertible, &construct, type_id<T>());
	}

	static void* convertible(PyObject* obj_ptr) {
		// Check if Python object can be converted to target type.
		if(PyTuple_Check(obj_ptr)) return obj_ptr;
		return nullptr;
	}

	static void construct(PyObject* obj_ptr, converter::rvalue_from_python_stage1_data* data) {
		T v;
		tuple t = extract<tuple>(obj_ptr);
		if(len(t) != v.size()) {
			PyErr_Format(PyExc_ValueError, "Conversion to %s works only for tuples of length %i.", boost::python::type_id<T>().name(), (int)v.size());
			throw_error_already_set();
		}
		for(size_t i = 0; i < v.size(); i++) {
			extract<FloatType> ex(t[i]);
			if(!ex.check()) {
				PyErr_Format(PyExc_TypeError, "Conversion to %s works only for tuples containing numbers.", boost::python::type_id<T>().name());
				throw_error_already_set();
			}
			v[i] = ex();
		}
		void* storage = ((converter::rvalue_from_python_storage<T>*)data)->storage.bytes;
		new (storage) T(v);
		data->convertible = storage;
	}
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Vector3_normalizeSafely_overloads, normalizeSafely, 0, 1);

void setupLinearAlgebraBinding()
{
	class_<Vector3>("Vector3", init<FloatType, FloatType, FloatType>())
		.def(init<FloatType>())
		.def("__init__", make_constructor((Vector3* (*)())([]() { return new Vector3(Vector3::Zero()); })))
		.add_property("x", (FloatType (Vector3::*)() const)&Vector3::x, (void (*)(Vector3&,FloatType))([](Vector3& v, FloatType x) { v.x() = x; }))
		.add_property("y", (FloatType (Vector3::*)() const)&Vector3::y, (void (*)(Vector3&,FloatType))([](Vector3& v, FloatType y) { v.y() = y; }))
		.add_property("z", (FloatType (Vector3::*)() const)&Vector3::z, (void (*)(Vector3&,FloatType))([](Vector3& v, FloatType z) { v.z() = z; }))
		.def(self + other<Vector3>())
		.def(self += other<Vector3>())
		.def(self - other<Vector3>())
		.def(self -= other<Vector3>())
		.def(self * FloatType())
		.def(FloatType() * self)
		.def(self *= FloatType())
		.def(self / FloatType())
		.def(self /= FloatType())
		.def(-self)
		.def(Point3::Origin() + self)
		.def(self == other<Vector3>())
		.def(self != other<Vector3>())
		.add_property("length", &Vector3::length)
		.add_property("squaredLength", &Vector3::squaredLength)
		.def("normalize", &Vector3::normalize)
		.def("normalized", &Vector3::normalized)
		.def("normalizeSafely", &Vector3::normalizeSafely, Vector3_normalizeSafely_overloads())
		.def("resize", &Vector3::resize)
		.def("resized", &Vector3::resized)
		.def("cross", &Vector3::cross)
		.def("dot", &Vector3::dot)
		.add_property("maxComponent", &Vector3::maxComponent)
		.add_property("minComponent", &Vector3::minComponent)
		.def(array_indexing_suite<Vector3>())
		.def("__str__", &Vector3::toString)
	;

	// Install automatic Python tuple to Vector3 conversion.
	python_to_vector_conversion<Vector3>();

	class_<Point3>("Point3", init<FloatType, FloatType, FloatType>())
		.def(init<FloatType>())
		.def("__init__", make_constructor((Point3* (*)())([]() { return new Point3(Point3::Origin()); })))
		.add_property("x", (FloatType (Point3::*)() const)&Point3::x, (void (*)(Point3&,FloatType))([](Point3& p, FloatType x) { p.x() = x; }))
		.add_property("y", (FloatType (Point3::*)() const)&Point3::y, (void (*)(Point3&,FloatType))([](Point3& p, FloatType y) { p.y() = y; }))
		.add_property("z", (FloatType (Point3::*)() const)&Point3::z, (void (*)(Point3&,FloatType))([](Point3& p, FloatType z) { p.z() = z; }))
		.def(self + other<Vector3>())
		.def(other<Vector3>() + self)
		.def(self += other<Vector3>())
		.def(self - other<Vector3>())
		.def(self -= other<Vector3>())
		.def(self - other<Point3>())
		.def(self * FloatType())
		.def(FloatType() * self)
		.def(self *= FloatType())
		.def(self / FloatType())
		.def(self /= FloatType())
		.def(self == other<Point3>())
		.def(self != other<Point3>())
		.add_property("maxComponent", &Point3::maxComponent)
		.add_property("minComponent", &Point3::minComponent)
		.def(array_indexing_suite<Point3>())
		.def("__str__", &Point3::toString)
	;

	// Install automatic Python tuple to Point3 conversion.
	python_to_vector_conversion<Point3>();

	class_<Color>("Color", init<FloatType, FloatType, FloatType>())
		.def(init<FloatType>())
		.def("__init__", make_constructor((Color* (*)())([]() { return new Color(0,0,0); })))
		.add_property("r", (FloatType (Color::*)() const)&Color::r, (void (*)(Color&,FloatType))([](Color& c, FloatType r) { c.r() = r; }))
		.add_property("g", (FloatType (Color::*)() const)&Color::g, (void (*)(Color&,FloatType))([](Color& c, FloatType g) { c.g() = g; }))
		.add_property("b", (FloatType (Color::*)() const)&Color::b, (void (*)(Color&,FloatType))([](Color& c, FloatType b) { c.b() = b; }))
		.def(self + other<Color>())
		.def(self += other<Color>())
		.def(self * other<Color>())
		.def(self * FloatType())
		.def(FloatType() * self)
		.def(self == other<Color>())
		.def(self != other<Color>())
		.def("clampMin", &Color::clampMin)
		.def("clampMax", &Color::clampMax)
		.def("clampMinMax", &Color::clampMinMax)
		.def("setWhite", &Color::setWhite)
		.def("setBlack", &Color::setBlack)
		.def(array_indexing_suite<Color>())
		.def("__str__", &Color::toString)
	;

	// Install automatic Python tuple to Color conversion.
	python_to_vector_conversion<Color>();

	class_<ColorA>("ColorA", init<FloatType, FloatType, FloatType, optional<FloatType>>())
		.def("__init__", make_constructor((ColorA* (*)())([]() { return new ColorA(0,0,0); })))
		.add_property("r", (FloatType (ColorA::*)() const)&ColorA::r, (void (*)(ColorA&,FloatType))([](ColorA& c, FloatType r) { c.r() = r; }))
		.add_property("g", (FloatType (ColorA::*)() const)&ColorA::g, (void (*)(ColorA&,FloatType))([](ColorA& c, FloatType g) { c.g() = g; }))
		.add_property("b", (FloatType (ColorA::*)() const)&ColorA::b, (void (*)(ColorA&,FloatType))([](ColorA& c, FloatType b) { c.b() = b; }))
		.add_property("a", (FloatType (ColorA::*)() const)&ColorA::a, (void (*)(ColorA&,FloatType))([](ColorA& c, FloatType a) { c.a() = a; }))
		.def(self + other<ColorA>())
		.def(self += other<ColorA>())
		.def(self * other<ColorA>())
		.def(self * FloatType())
		.def(FloatType() * self)
		.def(self == other<ColorA>())
		.def(self != other<ColorA>())
		.def("clampMin", &ColorA::clampMin)
		.def("clampMax", &ColorA::clampMax)
		.def("clampMinMax", &ColorA::clampMinMax)
		.def("setWhite", &ColorA::setWhite)
		.def("setBlack", &ColorA::setBlack)
		.def(array_indexing_suite<ColorA>())
		.def("__str__", &ColorA::toString)
	;

	// Install automatic Python tuple to ColorA conversion.
	python_to_vector_conversion<ColorA>();

}

};
