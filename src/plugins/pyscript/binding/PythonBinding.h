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

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

/// \brief Adds the initXXX() function of a plugin to an internal list so that the scripting engine can discover and register all internal modules.
/// Use the OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE macro to create an instance of this structure on application startup.
struct OVITO_PYSCRIPT_EXPORT PythonPluginRegistration
{
	/// The identifier of the plugin to register.
	const char* _pluginName;
	/// The initXXX() function to be registered with the Python interpreter.
	void (*_initFunc)();
	/// Next structure in linked list.
	PythonPluginRegistration* _next;

	PythonPluginRegistration(const char* pluginName, void (*initFunc)()) : _pluginName(pluginName), _initFunc(initFunc) {
		_next = linkedlist;
		linkedlist = this;
	}

	/// The initXXX() functions for each of the registered plugins.
	static PythonPluginRegistration* linkedlist;
};

/// This macro must be used exactly once by every plugin that contains a Python scripting interface.
#define OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(pluginName) \
	static PyScript::PythonPluginRegistration __pyscript_unused_variable##pluginName(#pluginName, init##pluginName);

// A model of the Boost.Python concept ResultConverterGenerator which wraps
// a raw pointer to an OvitoObject derived class in a OORef<> smart pointer.
struct ovito_object_reference
{
	struct make_ooref_holder {
		template <class T> static PyObject* execute(T* p) {
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
	ovito_abstract_class() : class_<OvitoObjectClass, bases<BaseClass>, OORef<OvitoObjectClass>, boost::noncopyable>(OvitoObjectClass::OOType.className(), no_init) {}
};

/// Defines a Python class for an OvitoObject-derived C++ class.
template<class OvitoObjectClass, class BaseClass>
class ovito_class : public class_<OvitoObjectClass, bases<BaseClass>, OORef<OvitoObjectClass>, boost::noncopyable>
{
public:

	/// Constructor.
	ovito_class() : class_<OvitoObjectClass, bases<BaseClass>, OORef<OvitoObjectClass>, boost::noncopyable>(OvitoObjectClass::OOType.className(), no_init) {
		// Define a constructor for this Python class, which allows to create instances
		// of the OvitoObject class from a script without passing a DataSet pointer.
		this->def("__init__", make_constructor(&construct_instance));
		// Also define a constructor that takes a dictionary, which is used to initialize
		// properties of the newly created object.
		this->def("__init__", make_constructor(&construct_instance_with_params));
	}

private:

	/// This constructs a new instance of the OvitoObject class and passes the DataSet
	/// of the active script engine to its constructor.
	static OORef<OvitoObjectClass> construct_instance() {
		ScriptEngine* engine = ScriptEngine::activeEngine();
		if(!engine) throw Exception("Invalid interpreter state. There is no active script engine.");
		DataSet* dataset = engine->dataset();
		if(!dataset) throw Exception("Invalid interpreter state. There is no active dataset.");
		return new OvitoObjectClass(dataset);
	}

	/// This constructs a new instance of the OvitoObject class and initializes
	/// its properties using the values stored in a dictionary.
	static OORef<OvitoObjectClass> construct_instance_with_params(const dict& params) {
		// Construct the C++ object instance.
		OORef<OvitoObjectClass> obj = construct_instance();
		// Create a Python wrapper for the object so we can set its attributes.
		object pyobj(obj);
		// Iterate over the keys of the dictionary and set attributes of the
		// newly created object.
		PyObject *key, *value;
		Py_ssize_t pos = 0;
		while(PyDict_Next(params.ptr(), &pos, &key, &value)) {
			// Check if the attribute exists. Otherwise raise error.
			if(!PyObject_HasAttr(pyobj.ptr(), key)) {
				const char* keystr = extract<char const*>(object(handle<>(borrowed(key))));
				PyErr_Format(PyExc_AttributeError, "Error in constructor. Object type %s does not have an attribute with the name '%s'.",
						OvitoObjectClass::OOType.className(), keystr);
				throw_error_already_set();
			}
			// Set attribute value.
			pyobj.attr(object(handle<>(borrowed(key)))) = handle<>(borrowed(value));
		}
		return obj;
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
				index += container.size();
			if(index >= container.size() || index < 0) {
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

};	// End of namespace

#endif
