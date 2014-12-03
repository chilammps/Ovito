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

#ifndef __OVITO_PYSCRIPT_BINDING_H
#define __OVITO_PYSCRIPT_BINDING_H

#include <plugins/pyscript/PyScript.h>
#include <plugins/pyscript/engine/ScriptEngine.h>

#include <boost/python/raw_function.hpp>
#include <boost/python/stl_iterator.hpp>

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

/// \brief Adds the initXXX() function of a plugin to an internal list so that the scripting engine can discover and register all internal modules.
/// Use the OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE macro to create an instance of this structure on application startup.
struct OVITO_PYSCRIPT_EXPORT PythonPluginRegistration
{
#if PY_MAJOR_VERSION >= 3
	typedef PyObject* (*InitFuncPointer)();
#else
	typedef void (*InitFuncPointer)();
#endif

	/// The identifier of the plugin to register.
	const char* _moduleName;
	/// The initXXX() function to be registered with the Python interpreter.
	InitFuncPointer _initFunc;
	/// Next structure in linked list.
	PythonPluginRegistration* _next;

	PythonPluginRegistration(const char* moduleName, InitFuncPointer initFunc) : _moduleName(moduleName), _initFunc(initFunc) {
		_next = linkedlist;
		linkedlist = this;
	}

	/// The initXXX() functions for each of the registered plugins.
	static PythonPluginRegistration* linkedlist;
};

/// This macro must be used exactly once by every plugin that contains a Python scripting interface.
#if PY_MAJOR_VERSION >= 3
	#define OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(pluginName) \
		static PyScript::PythonPluginRegistration __pyscript_unused_variable##pluginName(#pluginName, PyInit_##pluginName);
#else
	#define OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(pluginName) \
		static PyScript::PythonPluginRegistration __pyscript_unused_variable##pluginName(#pluginName, init##pluginName);
#endif

// A model of the Boost.Python ResultConverterGenerator concept which wraps
// a raw pointer to an OvitoObject derived class in a OORef<> smart pointer.
struct ovito_object_reference
{
	struct make_ooref_holder {
		template <class T>
		static PyObject* execute(T* p) {
			typedef objects::pointer_holder<OORef<T>, T> holder_t;
			OORef<T> ptr(const_cast<T*>(p));
			return objects::make_ptr_instance<T, holder_t>::execute(ptr);
		}
	};
	template <class T> struct apply {
        typedef to_python_indirect<T, make_ooref_holder> type;
    };
};

/// Defines a Python class for an abstract OvitoObject-derived C++ class.
template<class OvitoObjectClass, class BaseClass>
class ovito_abstract_class : public class_<OvitoObjectClass, bases<BaseClass>, OORef<OvitoObjectClass>, boost::noncopyable>
{
public:
	/// Constructor.
	ovito_abstract_class(const char* docstring = nullptr, const char* pythonClassName = nullptr)
		: class_<OvitoObjectClass, bases<BaseClass>, OORef<OvitoObjectClass>, boost::noncopyable>(pythonClassName ? pythonClassName : OvitoObjectClass::OOType.className(), docstring, no_init) {}
};

/// Defines a Python class for an OvitoObject-derived C++ class.
template<class OvitoObjectClass, class BaseClass>
class ovito_class : public class_<OvitoObjectClass, bases<BaseClass>, OORef<OvitoObjectClass>, boost::noncopyable>
{
public:

	/// Constructor.
	ovito_class(const char* docstring = nullptr, const char* pythonClassName = nullptr) : class_<OvitoObjectClass, bases<BaseClass>, OORef<OvitoObjectClass>, boost::noncopyable>(pythonClassName ? pythonClassName : OvitoObjectClass::OOType.className(), docstring, no_init) {
		// Define a constructor that takes a variable number of keyword arguments, which are used to initialize
		// properties of the newly created object.
		this->def("__init__", raw_constructor(&construct_instance_with_params));
	}

private:

	template <class F>
	struct raw_constructor_dispatcher {
		raw_constructor_dispatcher(F f) : f(make_constructor(f)) {}
		PyObject* operator()(PyObject* args, PyObject* keywords) {
			boost::python::detail::borrowed_reference_t* ra = boost::python::detail::borrowed_reference(args);
			object a(ra);
			return incref(object(f(
					  object(a[0])
					, object(a.slice(1, len(a)))
					, keywords ? dict(boost::python::detail::borrowed_reference(keywords)) : dict()
			)).ptr());
		}
	private:
		object f;
	};

	template <class F>
	object raw_constructor(F f, std::size_t min_args = 0) {
		return boost::python::detail::make_raw_function(
			boost::python::objects::py_function(
				raw_constructor_dispatcher<F>(f)
				, boost::mpl::vector2<void, object>()
				, min_args + 1
				, (std::numeric_limits<unsigned>::max)()
			)
		);
	}

	/// This constructs a new instance of the OvitoObject class and initializes
	/// its properties using the values stored in a dictionary.
	static OORef<OvitoObjectClass> construct_instance_with_params(const tuple& args, const dict& kwargs) {
		if(len(args) != 0) {
			if(len(args) > 1 || !extract<dict>(args[0]).check())
				throw Exception("Constructor function accepts only keyword arguments.");
		}
		// Construct the C++ object instance.
		ScriptEngine* engine = ScriptEngine::activeEngine();
		if(!engine) throw Exception("Invalid interpreter state. There is no active script engine.");
		DataSet* dataset = engine->dataset();
		if(!dataset) throw Exception("Invalid interpreter state. There is no active dataset.");
		OORef<OvitoObjectClass> obj(new OvitoObjectClass(dataset));
		// Create a Python wrapper for the object so we can set its attributes.
		object pyobj(obj);
		// Set attributes based on keyword arguments.
		applyParameters(pyobj, kwargs);
		// The caller may alternatively provide a dictionary with attributes.
		if(len(args) == 1)
			applyParameters(pyobj, extract<dict>(args[0]));
		return obj;
	}

	// Sets attributes of the given object as specified in the dictionary.
	static void applyParameters(object& obj, const dict& params) {
		PyObject *key, *value;
		Py_ssize_t pos = 0;
		// Iterate over the keys of the dictionary and set attributes of the
		// newly created object.
		while(PyDict_Next(params.ptr(), &pos, &key, &value)) {
			// Check if the attribute exists. Otherwise raise error.
			if(!PyObject_HasAttr(obj.ptr(), key)) {
				const char* keystr = extract<char const*>(object(handle<>(borrowed(key))));
				PyErr_Format(PyExc_AttributeError, "Error in constructor. Object type %s does not have an attribute named '%s'.",
						OvitoObjectClass::OOType.className(), keystr);
				throw_error_already_set();
			}
			// Set attribute value.
			obj.attr(object(handle<>(borrowed(key)))) = handle<>(borrowed(value));
		}
	}
};

/// Base indexing suite for vector-like containers.
template<class Container, class DerivedClass, typename value_type = typename Container::value_type>
class base_indexing_suite : public def_visitor<DerivedClass>
{
public:
    typedef value_type data_type;
    typedef value_type key_type;
    typedef typename Container::size_type index_type;
    typedef typename Container::size_type size_type;
    typedef typename Container::difference_type difference_type;

    template<class Class>
    void visit(Class& cl) const {
        cl
            .def("__len__", &get_size)
            .def("__delitem__", &delete_item)
            .def("__contains__", &contains)
        ;
    }

    static size_type get_size(const Container& container) { return container.size(); }
	static void delete_item(Container& container, PyObject* i) {
		PyErr_SetString(PyExc_NotImplementedError, "This sequence type does not allow deleting elements.");
		throw_error_already_set();
	}
	static bool contains(const Container& container, const key_type& key) { return std::find(container.begin(), container.end(), key) != container.end(); }

	static index_type convert_index(Container& container, PyObject* i_) {
		extract<long> i(i_);
		if(i.check()) {
			long index = i();
			if(index < 0)
				index += (long)container.size();
            if(index >= (long)container.size() || index < 0) {
				PyErr_SetString(PyExc_IndexError, "Index out of range");
				throw_error_already_set();
			}
			return static_cast<index_type>(index);
		}

		PyErr_SetString(PyExc_TypeError, "Invalid index type");
		throw_error_already_set();
		return index_type();
	}
};

/// Indexing suite for QVector<T*> containers, where T is an OvitoObject derived class.
/// This indexing suite exposes only read-only methods which do not modify the QVector container.
template<class T, typename Container = QVector<T*>>
class QVector_OO_readonly_indexing_suite : public base_indexing_suite<Container, QVector_OO_readonly_indexing_suite<T,Container>, T*>
{
public:
	typedef base_indexing_suite<Container, QVector_OO_readonly_indexing_suite<T,Container>, T*> base_class;
    typedef typename std::conditional<std::is_pointer<typename Container::value_type>::value,
    			return_value_policy<ovito_object_reference>,
    			return_value_policy<copy_non_const_reference>
    			>::type iterator_return_policy;
    typedef boost::python::iterator<Container, iterator_return_policy> def_iterator;

    template<class Class>
    void visit(Class& cl) const {
    	base_class::visit(cl);
        cl
        	.def("__setitem__", &set_item)
        	.def("__getitem__", make_function(&get_item, return_value_policy<ovito_object_reference>()))
        	.def("__iter__", def_iterator())
        ;
    }

    static T* get_item(back_reference<Container&> container, PyObject* i) {
        if(PySlice_Check(i)) {
        	PyErr_SetString(PyExc_NotImplementedError, "This sequence type does not support slicing.");
        	throw_error_already_set();
        }
        return container.get()[base_class::convert_index(container.get(), i)];
    }

    static void set_item(Container& container, PyObject* i, PyObject* v) {
		PyErr_SetString(PyExc_NotImplementedError, "This sequence type is read-only.");
		throw_error_already_set();
	}
};

/// Indexing suite for std::array<T> containers, where T is a value type.
template<class ArrayType>
class array_indexing_suite : public base_indexing_suite<ArrayType, array_indexing_suite<ArrayType>>
{
public:
	typedef base_indexing_suite<ArrayType, array_indexing_suite<ArrayType>> base_class;

	template<class Class>
    void visit(Class& cl) const {
		base_class::visit(cl);
        cl
            .def("__setitem__", &set_item)
            .def("__getitem__", &get_item)
            .def("__iter__", boost::python::iterator<ArrayType>())
        ;
    }

    static typename ArrayType::value_type get_item(back_reference<ArrayType&> container, PyObject* i) {
        if(PySlice_Check(i)) {
        	PyErr_SetString(PyExc_NotImplementedError, "This sequence type does not support slicing.");
        	throw_error_already_set();
        }
        return container.get()[base_class::convert_index(container.get(), i)];
    }

	static void set_item(ArrayType& container, PyObject* i, PyObject* v) {
		if(PySlice_Check(i)) {
			PyErr_SetString(PyExc_NotImplementedError, "This sequence type does not support slicing.");
			throw_error_already_set();
		}
		extract<typename ArrayType::value_type> ex(v);
		if(!ex.check()) {
			PyErr_SetString(PyExc_TypeError, "Invalid type in array assignment.");
			throw_error_already_set();
		}
        container[base_class::convert_index(container, i)] = ex();
	}
};

// Automatic Python sequence to C++ container conversion.
template<typename Container>
struct python_to_container_conversion
{
	python_to_container_conversion() {
		converter::registry::push_back(&convertible, &construct, type_id<Container>());
	}

	static void* convertible(PyObject* obj_ptr) {
		// Check if Python object can be converted to target type.
		if(PyObject_HasAttrString(obj_ptr, "__iter__")) return obj_ptr;
		return nullptr;
	}

	static void construct(PyObject* obj_ptr, converter::rvalue_from_python_stage1_data* data) {
		stl_input_iterator<typename Container::value_type> begin(object(handle<>(borrowed(obj_ptr))));
		stl_input_iterator<typename Container::value_type> end;
		void* storage = ((converter::rvalue_from_python_storage<Container>*)data)->storage.bytes;
		new (storage) Container();
		data->convertible = storage;
		Container& c = *reinterpret_cast<Container*>(storage);
		for(; begin != end; ++begin)
			c.push_back(*begin);
	}
};

// Automatic Python sequence to C++ set container conversion.
template<typename Container>
struct python_to_set_conversion
{
	python_to_set_conversion() {
		converter::registry::push_back(&convertible, &construct, type_id<Container>());
	}

	static void* convertible(PyObject* obj_ptr) {
		// Check if Python object can be converted to target type.
		if(PyObject_HasAttrString(obj_ptr, "__iter__")) return obj_ptr;
		return nullptr;
	}

	static void construct(PyObject* obj_ptr, converter::rvalue_from_python_stage1_data* data) {
		stl_input_iterator<typename Container::value_type> begin(object(handle<>(borrowed(obj_ptr))));
		stl_input_iterator<typename Container::value_type> end;
		void* storage = ((converter::rvalue_from_python_storage<Container>*)data)->storage.bytes;
		new (storage) Container();
		data->convertible = storage;
		Container& c = *reinterpret_cast<Container*>(storage);
		for(; begin != end; ++begin)
			c.insert(*begin);
	}
};

};	// End of namespace

#endif
