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

#include <core/Core.h>
#include "NativeOvitoObjectType.h"
#include "OvitoObject.h"
#include <core/reference/PropertyFieldDescriptor.h>
#include <core/dataset/DataSet.h>
#include <core/plugins/Plugin.h>

namespace Ovito { namespace ObjectSystem { namespace Internal {

// Head of linked list.
NativeOvitoObjectType* NativeOvitoObjectType::_firstInfo = nullptr;

/******************************************************************************
* Constructs the plugin class descriptor object.
******************************************************************************/
NativeOvitoObjectType::NativeOvitoObjectType(const QString& name, const char* pluginId, const NativeOvitoObjectType* superClass, const QMetaObject* qtClassInfo, bool isSerializable)
	: OvitoObjectType(name, superClass, (qtClassInfo->constructorCount() == 0), isSerializable), _pluginId(pluginId), _qtClassInfo(qtClassInfo), _pureClassName(nullptr)
{
	// Insert into linked list of all object types.
	_next = _firstInfo;
	_firstInfo = this;

	// Interpret Qt class info fields.
	for(int i = qtClassInfo->classInfoOffset(); i < qtClassInfo->classInfoCount(); i++) {
		if(qstrcmp(qtClassInfo->classInfo(i).name(), "DisplayName") == 0) {
			// Fetch display name assigned to the Qt object class.
			setDisplayName(QString::fromLocal8Bit(qtClassInfo->classInfo(i).value()));
		}
		else if(qstrcmp(qtClassInfo->classInfo(i).name(), "ClassNameAlias") == 0) {
			// Load name alias assigned to the Qt object class.
			setNameAlias(QString::fromLocal8Bit(qtClassInfo->classInfo(i).value()));
		}
	}
}

/******************************************************************************
* Returns a pointer to the class name string (without namespace qualifier).
******************************************************************************/
const char* NativeOvitoObjectType::className() const
{
	if(_pureClassName) return _pureClassName;

	// Remove namespace qualifier from Qt's class name.
	NativeOvitoObjectType* this_ = const_cast<NativeOvitoObjectType*>(this);
	this_->_pureClassName = _qtClassInfo->className();
	for(const char* p = this_->_pureClassName; *p != '\0'; p++) {
		if(p[0] == ':' && p[1] == ':') {
			p++;
			this_->_pureClassName = p+1;
		}
	}

	return this_->_pureClassName;
}

/******************************************************************************
* Creates an instance of this object class.
******************************************************************************/
OvitoObject* NativeOvitoObjectType::createInstanceImpl(DataSet* dataset) const
{
#ifdef OVITO_DEBUG
	// Check if class hierarchy is consistent.
	OVITO_ASSERT(superClass() != nullptr);
	const QMetaObject* qtSuperClass = qtMetaObject()->superClass();
	while(qtSuperClass && qtSuperClass != superClass()->qtMetaObject())
		qtSuperClass = qtSuperClass->superClass();
	OVITO_ASSERT_MSG(qtSuperClass != nullptr, "NativeOvitoObjectType::createInstanceImpl", qPrintable(QString("Class %1 is not derived from base class %2 as specified by the object type descriptor.").arg(name()).arg(superClass()->name())));
#endif

	OvitoObject* obj;

	if(isDerivedFrom(RefTarget::OOType) && *this != DataSet::OOType) {
		UndoSuspender noUndo(dataset->undoStack());
		obj = qobject_cast<OvitoObject*>(qtMetaObject()->newInstance(Q_ARG(DataSet*, dataset)));
	}
	else {
		obj = qobject_cast<OvitoObject*>(qtMetaObject()->newInstance());
	}

	if(!obj)
		throw Exception(Plugin::tr("Failed to instantiate class '%1'.").arg(name()));

	return obj;
}

}}}	// End of namespace
