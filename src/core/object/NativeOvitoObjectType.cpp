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
#if 0
#include <core/reference/PropertyFieldDescriptor.h>
#endif
#include <core/gui/undo/UndoManager.h>
#include <core/plugins/Plugin.h>

namespace Ovito {

NativeOvitoObjectType* NativeOvitoObjectType::_firstInfo = NULL;

/******************************************************************************
* Creates an object of the appropriate kind.
* Throws an exception if the containing plugin failed to load.
******************************************************************************/
OORef<OvitoObject> NativeOvitoObjectType::createInstanceImpl()
{
	UndoSuspender noUndo;

	OvitoObject* obj = qobject_cast<OvitoObject*>(_qtClassInfo->newInstance());
	if(!obj)
		throw Exception(Plugin::tr("Cannot instantiate abstract class '%1'.").arg(name()));

	return obj;
}

/******************************************************************************
* Searches for a property field defined in this class.
******************************************************************************/
const PropertyFieldDescriptor* NativeOvitoObjectType::findNativePropertyField(const char* identifier) const
{
#if 0
	for(const PropertyFieldDescriptor* field = firstNativePropertyField(); field; field = field->next())
		if(qstrcmp(field->identifier(), identifier) == 0) return field;
#endif
	return NULL;
}


};
