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
#include <core/scene/pipeline/ModifierApplication.h>
#include <core/scene/objects/SceneObject.h>
#include <core/scene/SceneNode.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/display/DisplayObject.h>
#include <core/viewport/Viewport.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

BOOST_PYTHON_MODULE(PyScriptContainers)
{
	docstring_options docoptions(true, false);

	class_<QVector<Viewport*>, boost::noncopyable>("QVectorViewport", no_init)
		.def(QVector_OO_readonly_indexing_suite<Viewport>())
	;
	python_to_container_conversion<QVector<Viewport*>>();

	class_<QVector<DisplayObject*>, boost::noncopyable>("QVectorDisplayObject", no_init)
		.def(QVector_OO_readonly_indexing_suite<DisplayObject>())
	;
	python_to_container_conversion<QVector<DisplayObject*>>();

	class_<QVector<SceneNode*>, boost::noncopyable>("QVectorSceneNode", no_init)
		.def(QVector_OO_readonly_indexing_suite<SceneNode>())
	;
	python_to_container_conversion<QVector<SceneNode*>>();

	class_<QVector<SceneObject*>, boost::noncopyable>("QVectorSceneObject", no_init)
		.def(QVector_OO_readonly_indexing_suite<SceneObject>())
	;
	python_to_container_conversion<QVector<SceneObject*>>();

	class_<QVector<OORef<SceneObject>>, boost::noncopyable>("QVectorOORefSceneObject", no_init)
		.def(QVector_OO_readonly_indexing_suite<SceneObject, QVector<OORef<SceneObject>>>())
	;

	class_<QVector<VersionedOORef<SceneObject>>, boost::noncopyable>("QVectorVersionedOORefSceneObject", no_init)
		.def(QVector_OO_readonly_indexing_suite<SceneObject, QVector<VersionedOORef<SceneObject>>>())
	;

	class_<QVector<ModifierApplication*>, boost::noncopyable>("QVectorModifierApplication", no_init)
		.def(QVector_OO_readonly_indexing_suite<ModifierApplication>())
	;
	class_<QVector<int>>("QVectorInt")
		.def(array_indexing_suite<QVector<int>>())
	;
	class_<QVector<double>>("QVectorDouble")
		.def(array_indexing_suite<QVector<double>>())
	;
	class_<QList<int>>("QListInt")
		.def(array_indexing_suite<QList<int>>())
	;

	class_<QStringList>("QStringList")
		.def(array_indexing_suite<QStringList>())
	;
	python_to_container_conversion<QStringList>();

	struct QSetInt_to_python {
		static PyObject* convert(const QSet<int>& s) {
			object pyset(handle<>(PySet_New(NULL)));
			for(int v : s) {
				PySet_Add(pyset.ptr(), object(v).ptr());
			}
			return incref(pyset.ptr());
		}
	};
	to_python_converter<QSet<int>, QSetInt_to_python>();
	python_to_set_conversion<QSet<int>>();
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(PyScriptContainers);

};
