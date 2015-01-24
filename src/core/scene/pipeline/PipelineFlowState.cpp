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
#include <core/scene/pipeline/PipelineFlowState.h>
#include <core/scene/objects/DataObject.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

/******************************************************************************
* Tries to convert one of the to data objects stored in this flow state to
* the given object type.
******************************************************************************/
OORef<DataObject> PipelineFlowState::convertObject(const OvitoObjectType& objectClass, TimePoint time) const
{
	for(const auto& o : _objects) {
		if(OORef<DataObject> obj = o->convertTo(objectClass, time))
			return obj;
	}
	return {};
}

/******************************************************************************
* Returns true if the given object is part of this pipeline flow state.
* The method ignores the revision number of the object.
******************************************************************************/
bool PipelineFlowState::contains(DataObject* obj) const
{
	for(DataObject* o : _objects)
		if(o == obj) return true;
	return false;
}

/******************************************************************************
* Adds an additional data object to this state.
******************************************************************************/
void PipelineFlowState::addObject(DataObject* obj)
{
	OVITO_CHECK_OBJECT_POINTER(obj);
	OVITO_ASSERT_MSG(!contains(obj), "PipelineFlowState::addObject", "Cannot add the same data object more than once.");
	_objects.push_back(obj);
}

/******************************************************************************
* Replaces a data object with a new one.
******************************************************************************/
void PipelineFlowState::replaceObject(DataObject* oldObj, DataObject* newObj)
{
	OVITO_CHECK_OBJECT_POINTER(oldObj);
	for(int index = 0; index < _objects.size(); index++) {
		if(_objects[index].get() == oldObj) {
			if(newObj)
				_objects[index] = newObj;
			else
				_objects.remove(index);
			return;
		}
	}
	OVITO_ASSERT_MSG(false, "PipelineFlowState::replaceObject", "Old object not found.");
}

/******************************************************************************
* Updates the stored revision numbers for all data objects.
******************************************************************************/
void PipelineFlowState::updateRevisionNumbers()
{
	for(auto& o : _objects)
		o.updateRevisionNumber();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
