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

/// Indexing suite for QVector<T*> containers, where T is an OvitoObject derived class.
/// This indexing suite exposes only read-only methods which do not modify the QVector container.
template<class T, typename Container = QVector<T*>>
class QVector_OO_readonly_indexing_suite : public def_visitor<QVector_OO_readonly_indexing_suite<T>>
{
public:
    typedef T* data_type;
    typedef T* key_type;
    typedef typename Container::size_type index_type;
    typedef typename Container::size_type size_type;
    typedef typename Container::difference_type difference_type;
    typedef return_value_policy<ovito_object_reference> iterator_return_policy;

    typedef boost::python::iterator<Container, iterator_return_policy> def_iterator;

    template<class Class>
    void visit(Class& cl) const {
        cl
            .def("__len__", &get_size)
            .def("__setitem__", &set_item)
            .def("__delitem__", &delete_item)
            .def("__getitem__", make_function(&get_item, return_value_policy<ovito_object_reference>()))
            .def("__contains__", &contains)
            .def("__iter__", def_iterator())
        ;
    }

    static size_type get_size(Container& container) {
    	return container.size();
    }

    static T* get_item(back_reference<Container&> container, PyObject* i) {
        if(PySlice_Check(i)) {
        	PyErr_SetString(PyExc_NotImplementedError, "This sequence type does not support slicing.");
        	throw_error_already_set();
        }
        return container.get()[convert_index(container.get(), i)];
    }

	static void set_item(Container& container, PyObject* i, PyObject* v) {
		PyErr_SetString(PyExc_NotImplementedError, "This sequence type is read-only.");
		throw_error_already_set();
	}

	static void delete_item(Container& container, PyObject* i) {
		PyErr_SetString(PyExc_NotImplementedError, "This sequence type is read-only.");
		throw_error_already_set();
	}

	static bool contains(Container& container, key_type const& key) {
		return container.contains(key);
	}

	static index_type convert_index(Container& container, PyObject* i_) {
		extract<index_type> i(i_);
		if(i.check()) {
			index_type index = i();
			if(index < 0)
				index += container.size();
			if(index >= container.size() || index < 0) {
				PyErr_SetString(PyExc_IndexError, "Index out of range");
				throw_error_already_set();
			}
			return index;
		}

		PyErr_SetString(PyExc_TypeError, "Invalid index type");
		throw_error_already_set();
		return index_type();
	}
};

};	// End of namespace

#endif
