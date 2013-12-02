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
#include <core/gui/undo/UndoManager.h>
#include <core/plugins/Plugin.h>

namespace Ovito {

/// Head of linked list.
NativeOvitoObjectType* NativeOvitoObjectType::_firstInfo = nullptr;

/******************************************************************************
* Creates an object of the appropriate kind.
* Throws an exception if the containing plugin failed to load.
******************************************************************************/
OvitoObject* NativeOvitoObjectType::createInstanceImpl() const
{
	UndoSuspender noUndo;

	OvitoObject* obj = qobject_cast<OvitoObject*>(_qtClassInfo->newInstance());
	if(!obj)
		throw Exception(Plugin::tr("Failed to instantiate class '%1'.").arg(name()));

	return obj;
}

};
