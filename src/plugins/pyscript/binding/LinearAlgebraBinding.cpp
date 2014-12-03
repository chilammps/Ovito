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
			extract<typename T::value_type> ex(t[i]);
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

// Automatic Python to Matrix3/AffineTransformation conversion.
template<typename T>
struct python_to_matrix_conversion
{
	python_to_matrix_conversion() {
		converter::registry::push_back(&convertible, &construct, type_id<T>());
	}

	static void* convertible(PyObject* obj_ptr) {
		// Check if Python object can be converted to target type.
		if(PySequence_Check(obj_ptr)) return obj_ptr;
		return nullptr;
	}

	static void construct(PyObject* obj_ptr, converter::rvalue_from_python_stage1_data* data) {
		Py_ssize_t numRows = PySequence_Length(obj_ptr);
		if(numRows < 0) {
			PyErr_SetString(PyExc_TypeError, "This Python object cannot be converted to a matrix.");
			throw_error_already_set();
		}
		else if(numRows != T::row_count()) {
			PyErr_Format(PyExc_ValueError, "Conversion to %ix%i matrix failed. Wrong Python sequence length. Nested list of outer length %i expected.", (int)T::row_count(), (int)T::col_count(), (int)T::row_count());
			throw_error_already_set();
		}
		void* storage = ((converter::rvalue_from_python_storage<T>*)data)->storage.bytes;
		new (storage) T();
		data->convertible = storage;
		T& m = *reinterpret_cast<T*>(storage);
        for(Py_ssize_t i = 0; i < numRows; i++) {
			object row(handle<>(PySequence_ITEM(obj_ptr, i)));
			Py_ssize_t numCols = len(row);
			if(numCols != T::col_count()) {
				PyErr_Format(PyExc_ValueError, "Conversion to %ix%i matrix failed. Wrong Python sequence length. Nested list of inner length %i expected.", (int)T::row_count(), (int)T::col_count(), (int)T::col_count());
				throw_error_already_set();
			}
            for(Py_ssize_t j = 0; j < numCols; j++) {
				m(i,j) = extract<typename T::element_type>(row[j]);
			}
		}
	}
};

template<typename T>
dict Matrix__array_interface__(T& m)
{
	dict ai;
	ai["shape"] = boost::python::make_tuple(m.row_count(), m.col_count());
	ai["strides"] = boost::python::make_tuple(sizeof(typename T::element_type), sizeof(typename T::column_type));
#ifndef Q_CC_MSVC
    OVITO_STATIC_ASSERT(sizeof(T) == T::col_count() * sizeof(typename T::column_type));
#else
    OVITO_ASSERT(sizeof(T) == T::col_count() * sizeof(typename T::column_type));
#endif
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
	ai["typestr"] = str("<f") + str(sizeof(typename T::element_type));
#else
	ai["typestr"] = str(">f") + str(sizeof(typename T::element_type));
#endif
	ai["data"] = boost::python::make_tuple((std::intptr_t)m.data(), false);
	ai["version"] = 3;
	return ai;
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Vector_normalizeSafely_overloads, normalizeSafely, 0, 1);

BOOST_PYTHON_MODULE(PyScriptLinearAlgebra)
{
	docstring_options docoptions(true, false);

	class_<Vector3>("Vector3", init<FloatType, FloatType, FloatType>())
		.def(init<FloatType>())
        .def("__init__", make_constructor(static_cast<Vector3* (*)()>([]() { return new Vector3(Vector3::Zero()); })))
        .add_property("x", static_cast<FloatType (Vector3::*)() const>(&Vector3::x), static_cast<void (*)(Vector3&,FloatType)>([](Vector3& v, FloatType x) { v.x() = x; }))
        .add_property("y", static_cast<FloatType (Vector3::*)() const>(&Vector3::y), static_cast<void (*)(Vector3&,FloatType)>([](Vector3& v, FloatType y) { v.y() = y; }))
        .add_property("z", static_cast<FloatType (Vector3::*)() const>(&Vector3::z), static_cast<void (*)(Vector3&,FloatType)>([](Vector3& v, FloatType z) { v.z() = z; }))
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
		.def("normalizeSafely", &Vector3::normalizeSafely, Vector_normalizeSafely_overloads())
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

	class_<Vector2>("Vector2", init<FloatType, FloatType>())
		.def(init<FloatType>())
        .def("__init__", make_constructor(static_cast<Vector2* (*)()>([]() { return new Vector2(Vector2::Zero()); })))
        .add_property("x", static_cast<FloatType (Vector2::*)() const>(&Vector2::x), static_cast<void (*)(Vector2&,FloatType)>([](Vector2& v, FloatType x) { v.x() = x; }))
        .add_property("y", static_cast<FloatType (Vector2::*)() const>(&Vector2::y), static_cast<void (*)(Vector2&,FloatType)>([](Vector2& v, FloatType y) { v.y() = y; }))
		.def(self + other<Vector2>())
		.def(self += other<Vector2>())
		.def(self - other<Vector2>())
		.def(self -= other<Vector2>())
		.def(self * FloatType())
		.def(FloatType() * self)
		.def(self *= FloatType())
		.def(self / FloatType())
		.def(self /= FloatType())
		.def(-self)
		.def(self == other<Vector2>())
		.def(self != other<Vector2>())
		.add_property("length", &Vector2::length)
		.add_property("squaredLength", &Vector2::squaredLength)
		.def("normalize", &Vector2::normalize)
		.def("normalized", &Vector2::normalized)
		.def("normalizeSafely", &Vector2::normalizeSafely, Vector_normalizeSafely_overloads())
		.def("resize", &Vector2::resize)
		.def("resized", &Vector2::resized)
		.def("dot", &Vector2::dot)
		.add_property("maxComponent", &Vector2::maxComponent)
		.add_property("minComponent", &Vector2::minComponent)
		.def(array_indexing_suite<Vector2>())
		.def("__str__", &Vector2::toString)
	;

	// Install automatic Python tuple to Vector2 conversion.
	python_to_vector_conversion<Vector2>();

	class_<Vector4>("Vector4", init<FloatType, FloatType, FloatType, FloatType>())
		.def(init<FloatType>())
        .def("__init__", make_constructor(static_cast<Vector4* (*)()>([]() { return new Vector4(Vector4::Zero()); })))
        .add_property("x", static_cast<FloatType (Vector4::*)() const>(&Vector4::x), static_cast<void (*)(Vector4&,FloatType)>([](Vector4& v, FloatType x) { v.x() = x; }))
        .add_property("y", static_cast<FloatType (Vector4::*)() const>(&Vector4::y), static_cast<void (*)(Vector4&,FloatType)>([](Vector4& v, FloatType y) { v.y() = y; }))
        .add_property("z", static_cast<FloatType (Vector4::*)() const>(&Vector4::z), static_cast<void (*)(Vector4&,FloatType)>([](Vector4& v, FloatType z) { v.z() = z; }))
        .add_property("w", static_cast<FloatType (Vector4::*)() const>(&Vector4::w), static_cast<void (*)(Vector4&,FloatType)>([](Vector4& v, FloatType w) { v.w() = w; }))
		.def(self + other<Vector4>())
		.def(self += other<Vector4>())
		.def(self - other<Vector4>())
		.def(self -= other<Vector4>())
		.def(self * FloatType())
		.def(FloatType() * self)
		.def(self *= FloatType())
		.def(self / FloatType())
		.def(self /= FloatType())
		.def(-self)
		.def(self == other<Vector4>())
		.def(self != other<Vector4>())
		.add_property("length", &Vector4::length)
		.add_property("squaredLength", &Vector4::squaredLength)
		.def("normalize", &Vector4::normalize)
		.def("normalized", &Vector4::normalized)
		.def("normalizeSafely", &Vector4::normalizeSafely, Vector_normalizeSafely_overloads())
		.def("dot", &Vector4::dot)
		.add_property("maxComponent", &Vector4::maxComponent)
		.add_property("minComponent", &Vector4::minComponent)
		.def(array_indexing_suite<Vector4>())
		.def("__str__", &Vector4::toString)
	;

	// Install automatic Python tuple to Vector4 conversion.
	python_to_vector_conversion<Vector4>();

	class_<Point3>("Point3", init<FloatType, FloatType, FloatType>())
		.def(init<FloatType>())
        .def("__init__", make_constructor(static_cast<Point3* (*)()>([]() { return new Point3(Point3::Origin()); })))
        .add_property("x", static_cast<FloatType (Point3::*)() const>(&Point3::x), static_cast<void (*)(Point3&,FloatType)>([](Point3& p, FloatType x) { p.x() = x; }))
        .add_property("y", static_cast<FloatType (Point3::*)() const>(&Point3::y), static_cast<void (*)(Point3&,FloatType)>([](Point3& p, FloatType y) { p.y() = y; }))
        .add_property("z", static_cast<FloatType (Point3::*)() const>(&Point3::z), static_cast<void (*)(Point3&,FloatType)>([](Point3& p, FloatType z) { p.z() = z; }))
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

	class_<Point2>("Point2", init<FloatType, FloatType>())
		.def(init<FloatType>())
        .def("__init__", make_constructor(static_cast<Point2* (*)()>([]() { return new Point2(Point2::Origin()); })))
        .add_property("x", static_cast<FloatType (Point2::*)() const>(&Point2::x), static_cast<void (*)(Point2&,FloatType)>([](Point2& p, FloatType x) { p.x() = x; }))
        .add_property("y", static_cast<FloatType (Point2::*)() const>(&Point2::y), static_cast<void (*)(Point2&,FloatType)>([](Point2& p, FloatType y) { p.y() = y; }))
		.def(self + other<Vector2>())
		.def(other<Vector2>() + self)
		.def(self += other<Vector2>())
		.def(self - other<Vector2>())
		.def(self -= other<Vector2>())
		.def(self - other<Point2>())
		.def(self * FloatType())
		.def(FloatType() * self)
		.def(self *= FloatType())
		.def(self / FloatType())
		.def(self /= FloatType())
		.def(self == other<Point2>())
		.def(self != other<Point2>())
		.add_property("maxComponent", &Point2::maxComponent)
		.add_property("minComponent", &Point2::minComponent)
		.def(array_indexing_suite<Point2>())
		.def("__str__", &Point2::toString)
	;

	// Install automatic Python tuple to Point2 conversion.
	python_to_vector_conversion<Point2>();

	class_<Quaternion>("Quaternion", init<FloatType, FloatType, FloatType, FloatType>())
		.def(init<const AffineTransformation&>())
        .def("__init__", make_constructor(static_cast<Quaternion* (*)()>([]() { return new Quaternion(Quaternion::Identity()); })))
        .add_property("x", static_cast<FloatType (Quaternion::*)() const>(&Quaternion::x), static_cast<void (*)(Quaternion&,FloatType)>([](Quaternion& q, FloatType x) { q.x() = x; }))
        .add_property("y", static_cast<FloatType (Quaternion::*)() const>(&Quaternion::y), static_cast<void (*)(Quaternion&,FloatType)>([](Quaternion& q, FloatType y) { q.y() = y; }))
        .add_property("z", static_cast<FloatType (Quaternion::*)() const>(&Quaternion::z), static_cast<void (*)(Quaternion&,FloatType)>([](Quaternion& q, FloatType z) { q.z() = z; }))
        .add_property("w", static_cast<FloatType (Quaternion::*)() const>(&Quaternion::w), static_cast<void (*)(Quaternion&,FloatType)>([](Quaternion& q, FloatType w) { q.w() = w; }))
		.def(self * other<Quaternion>())
		.def(self * other<Vector3>())
		.def(self *= FloatType())
		.def(self /= FloatType())
		.def(-self)
		.def(self == other<Quaternion>())
		.def(self != other<Quaternion>())
		.def("setIdentity", &Quaternion::setIdentity, return_self<>())
		.def("inverse", &Quaternion::inverse)
		.def("normalize", &Quaternion::normalize)
		.def("normalized", &Quaternion::normalized)
		.def("dot", &Quaternion::dot)
		.def(array_indexing_suite<Quaternion>())
		.def("__str__", &Quaternion::toString)
	;

	// Install automatic Python tuple to Quaternion conversion.
	python_to_vector_conversion<Quaternion>();

	class_<Rotation>("Rotation", init<const Vector3&, FloatType, optional<bool>>())
		.def(init<const AffineTransformation&>())
		.def(init<const Quaternion&>())
		.def(init<const Vector3&, const Vector3&>())
        .def("__init__", make_constructor(static_cast<Rotation* (*)()>([]() { return new Rotation(Rotation::Identity()); })))
		.add_property("axis", make_function(&Rotation::axis, return_internal_reference<>()), &Rotation::setAxis)
		.add_property("angle", &Rotation::angle, &Rotation::setAngle)
		.add_property("revolutions", &Rotation::revolutions, &Rotation::setRevolutions)
		.def("inverse", &Rotation::inverse)
		.def("setIdentity", &Rotation::setIdentity, return_self<>())
		.def(self * other<Rotation>())
		.def(self += other<Rotation>())
		.def(self -= other<Rotation>())
		.def(self == other<Rotation>())
		.def(self != other<Rotation>())
		.def("__str__", &Rotation::toString)
	;

	class_<Scaling>("Scaling", init<const Vector3&, const Quaternion&>())
        .def("__init__", make_constructor(static_cast<Scaling* (*)()>([]() { return new Scaling(Scaling::Identity()); })))
		.def("inverse", &Scaling::inverse)
		.def("setIdentity", &Scaling::setIdentity, return_self<>())
		.def(self * other<Scaling>())
		.def(self += other<Scaling>())
		.def(self -= other<Scaling>())
		.def(self == other<Scaling>())
		.def(self != other<Scaling>())
		.def("__str__", &Scaling::toString)
	;

	class_<Color>("Color", init<FloatType, FloatType, FloatType>())
		.def(init<FloatType>())
        .def("__init__", make_constructor(static_cast<Color* (*)()>([]() { return new Color(0,0,0); })))
        .add_property("r", static_cast<FloatType (Color::*)() const>(&Color::r), static_cast<void (*)(Color&,FloatType)>([](Color& c, FloatType r) { c.r() = r; }))
        .add_property("g", static_cast<FloatType (Color::*)() const>(&Color::g), static_cast<void (*)(Color&,FloatType)>([](Color& c, FloatType g) { c.g() = g; }))
        .add_property("b", static_cast<FloatType (Color::*)() const>(&Color::b), static_cast<void (*)(Color&,FloatType)>([](Color& c, FloatType b) { c.b() = b; }))
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
        .def("__init__", make_constructor(static_cast<ColorA* (*)()>([]() { return new ColorA(0,0,0); })))
        .add_property("r", static_cast<FloatType (ColorA::*)() const>(&ColorA::r), static_cast<void (*)(ColorA&,FloatType)>([](ColorA& c, FloatType r) { c.r() = r; }))
        .add_property("g", static_cast<FloatType (ColorA::*)() const>(&ColorA::g), static_cast<void (*)(ColorA&,FloatType)>([](ColorA& c, FloatType g) { c.g() = g; }))
        .add_property("b", static_cast<FloatType (ColorA::*)() const>(&ColorA::b), static_cast<void (*)(ColorA&,FloatType)>([](ColorA& c, FloatType b) { c.b() = b; }))
        .add_property("a", static_cast<FloatType (ColorA::*)() const>(&ColorA::a), static_cast<void (*)(ColorA&,FloatType)>([](ColorA& c, FloatType a) { c.a() = a; }))
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

	class_<Matrix3>("Matrix3", init<FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType>())
		.def(init<const Vector3&,const Vector3&,const Vector3&>())
        .def("__init__", make_constructor(static_cast<Matrix3* (*)()>([]() { return new Matrix3(Matrix3::Identity()); })))
		.add_property("determinant", &Matrix3::determinant)
		.add_property("row_count", &Matrix3::row_count)
		.add_property("col_count", &Matrix3::col_count)
		.def("inverse", (Matrix3 (Matrix3::*)() const)&Matrix3::inverse)
		.def("transposed", &Matrix3::transposed)
		.def("orthonormalize", &Matrix3::orthonormalize)
		.def("column", (Vector3& (Matrix3::*)(Matrix3::size_type))&Matrix3::column, return_internal_reference<>())
        .def("get", static_cast<FloatType (*)(const Matrix3&, int, int)>([](const Matrix3& m, int row, int col) { return m(row,col); }))
        .def("set", static_cast<void (*)(Matrix3&, int, int, FloatType)>([](Matrix3& m, int row, int col, FloatType v) { m(row,col) = v; }))
		.def(self * other<AffineTransformation>())
		.def(self * other<Matrix3>())
		.def(self * other<Point3>())
		.def(self * other<Vector3>())
		.def(self * other<FloatType>())
		.def("setZero", &Matrix3::setZero, return_self<>())
		.def("setIdentity", &Matrix3::setIdentity, return_self<>())
		.def("rotationX", &Matrix3::rotationX)
		.staticmethod("rotationX")
		.def("rotationY", &Matrix3::rotationY)
		.staticmethod("rotationY")
		.def("rotationZ", &Matrix3::rotationZ)
		.staticmethod("rotationZ")
		.def("rotation", (Matrix3 (*)(const Rotation&))&Matrix3::rotation)
		.def("rotation", (Matrix3 (*)(const Quaternion&))&Matrix3::rotation)
		.staticmethod("rotation")
		.def("scaling", &Matrix3::scaling)
		.staticmethod("scaling")
		.add_property("__array_interface__", &Matrix__array_interface__<Matrix3>)
	;

	// Install automatic Python list to C++ matrix conversion.
	python_to_matrix_conversion<Matrix3>();

	class_<AffineTransformation>("AffineTransformation", init<FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType>())
		.def(init<FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType>())
		.def(init<const Vector3&,const Vector3&,const Vector3&,const Vector3&>())
        .def("__init__", make_constructor(static_cast<AffineTransformation* (*)()>([]() { return new AffineTransformation(AffineTransformation::Identity()); })))
		.add_property("determinant", &AffineTransformation::determinant)
		.add_property("row_count", &AffineTransformation::row_count)
		.add_property("col_count", &AffineTransformation::col_count)
		.def("orthonormalize", &AffineTransformation::orthonormalize)
		.def("inverse", (AffineTransformation (AffineTransformation::*)() const)&AffineTransformation::inverse)
		.def("column", (Vector3& (AffineTransformation::*)(AffineTransformation::size_type))&AffineTransformation::column, return_internal_reference<>())
        .def("get", static_cast<FloatType (*)(const AffineTransformation&, int, int)>([](const AffineTransformation& m, int row, int col) { return m(row,col); }))
        .def("set", static_cast<void (*)(AffineTransformation&, int, int, FloatType)>([](AffineTransformation& m, int row, int col, FloatType v) { m(row,col) = v; }))
		.def(self * other<AffineTransformation>())
		.def(self * other<Matrix3>())
		.def(self * other<Point3>())
		.def(self * other<Vector3>())
		.def(self * other<FloatType>())
		.def(other<FloatType>() * self)
		.def(self == other<AffineTransformation>())
		.def(self != other<AffineTransformation>())
		.def("setZero", &AffineTransformation::setZero, return_self<>())
		.def("setIdentity", &AffineTransformation::setIdentity, return_self<>())
		.def("translation", (AffineTransformation (*)(const Vector3&))&AffineTransformation::translation)
		.staticmethod("translation")
		.def("shear", &AffineTransformation::shear)
		.staticmethod("shear")
		.def("lookAt", &AffineTransformation::lookAt)
		.staticmethod("lookAt")
		.def("lookAlong", &AffineTransformation::lookAlong)
		.staticmethod("lookAlong")
		.def("rotationX", &AffineTransformation::rotationX)
		.staticmethod("rotationX")
		.def("rotationY", &AffineTransformation::rotationY)
		.staticmethod("rotationY")
		.def("rotationZ", &AffineTransformation::rotationZ)
		.staticmethod("rotationZ")
		.def("rotation", (AffineTransformation (*)(const Rotation&))&AffineTransformation::rotation)
		.def("rotation", (AffineTransformation (*)(const Quaternion&))&AffineTransformation::rotation)
		.staticmethod("rotation")
		.def("scaling", (AffineTransformation (*)(const Scaling&))&AffineTransformation::scaling)
		.staticmethod("scaling")
		.add_property("__array_interface__", &Matrix__array_interface__<AffineTransformation>)
	;

	// Install automatic Python list to C++ matrix conversion.
	python_to_matrix_conversion<AffineTransformation>();

	class_<Matrix4>("Matrix4", init<FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType,FloatType>())
		.def(init<const AffineTransformation&>())
        .def("__init__", make_constructor(static_cast<Matrix4* (*)()>([]() { return new Matrix4(Matrix4::Identity()); })))
		.add_property("determinant", &Matrix4::determinant)
		.add_property("row_count", &Matrix4::row_count)
		.add_property("col_count", &Matrix4::col_count)
		.def("inverse", (Matrix4 (Matrix4::*)() const)&Matrix4::inverse)
        .def("get", static_cast<FloatType (*)(const Matrix4&, int, int)>([](const Matrix4& m, int row, int col) { return m(row,col); }))
        .def("set", static_cast<void (*)(Matrix4&, int, int, FloatType)>([](Matrix4& m, int row, int col, FloatType v) { m(row,col) = v; }))
		.def(self * other<AffineTransformation>())
		.def(self * other<Matrix4>())
		.def(self * other<Point3>())
		.def(self * other<Vector3>())
		.def(self * other<FloatType>())
		.def("setZero", &Matrix4::setZero, return_self<>())
		.def("setIdentity", &Matrix4::setIdentity, return_self<>())
		.def("translation", &Matrix4::translation)
		.staticmethod("translation")
		.def("perspective", &Matrix4::perspective)
		.staticmethod("perspective")
		.def("ortho", &Matrix4::ortho)
		.staticmethod("ortho")
		.add_property("__array_interface__", &Matrix__array_interface__<Matrix4>)
	;

	// Install automatic Python list to C++ matrix conversion.
	python_to_matrix_conversion<Matrix4>();
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(PyScriptLinearAlgebra);

};
