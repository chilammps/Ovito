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
#include "CloneHelper.h"
#include "RefTarget.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

/******************************************************************************
* This creates a copy of the RefTarget. 
* If there has already a copy of this RefTarget  
* created by this clone helper earlier, then it will be returned.
*    obj - The object to be cloned.
*    deepCopy - Controls whether sub-objects referenced by this RefTarget are copied too.
* Returns the copy of the source object.
******************************************************************************/
OORef<RefTarget> CloneHelper::cloneObjectImpl(RefTarget* obj, bool deepCopy)
{
	if(obj == NULL) return NULL;
	OVITO_CHECK_OBJECT_POINTER(obj);
	
	OORef<RefTarget> copy(_cloneTable[obj]);
	if(copy) return copy;
	
	copy = obj->clone(deepCopy, *this);
	if(!copy)
		throw Exception(tr("Object of class %1 cannot be cloned. It does not implement the clone() method.").arg(obj->getOOType().name()));
		
	OVITO_ASSERT_MSG(copy->getOOType().isDerivedFrom(obj->getOOType()), "CloneHelper::cloneObject", QString("The clone method of class %1 did not return a compatible class instance.").arg(obj->getOOType().name()).toLocal8Bit().constData());
	
	_cloneTable[obj] = copy;
	return copy;	
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
