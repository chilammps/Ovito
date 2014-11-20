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
#include <core/scene/objects/SceneObject.h>

namespace Ovito { namespace ObjectSystem { namespace Scene {

/******************************************************************************
* Tries to convert one of the to scene objects stored in this flow state to
* the given object type.
******************************************************************************/
OORef<SceneObject> PipelineFlowState::convertObject(const OvitoObjectType& objectClass, TimePoint time) const
{
	for(const auto& o : _objects) {
		if(OORef<SceneObject> obj = o->convertTo(objectClass, time))
			return obj;
	}
	return {};
}

/******************************************************************************
* Returns true if the given object is part of this pipeline flow state.
* The method ignores the revision number of the object.
******************************************************************************/
bool PipelineFlowState::contains(SceneObject* obj) const
{
	for(SceneObject* o : _objects)
		if(o == obj) return true;
	return false;
}

/******************************************************************************
* Adds an additional scene object to this state.
******************************************************************************/
void PipelineFlowState::addObject(SceneObject* obj)
{
	OVITO_CHECK_OBJECT_POINTER(obj);
	OVITO_ASSERT_MSG(!contains(obj), "PipelineFlowState::addObject", "Cannot add the same scene object more than once.");
	_objects.push_back(obj);
}

/******************************************************************************
* Replaces a scene object with a new one.
******************************************************************************/
void PipelineFlowState::replaceObject(SceneObject* oldObj, SceneObject* newObj)
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
* Updates the stored revision numbers for all scene objects.
******************************************************************************/
void PipelineFlowState::updateRevisionNumbers()
{
	for(auto& o : _objects)
		o.updateRevisionNumber();
}

}}}	// End of namespace
