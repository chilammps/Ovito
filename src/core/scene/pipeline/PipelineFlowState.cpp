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

namespace Ovito {

/******************************************************************************
* Adds an additional scene object to this state.
******************************************************************************/
void PipelineFlowState::addObject(SceneObject* obj)
{
	OVITO_CHECK_OBJECT_POINTER(obj);
	OVITO_ASSERT_MSG(std::find(_objects.begin(), _objects.end(), obj) == _objects.end(), "PipelineFlowState::addObject", "Cannot add the same scene object more than once.");
	_revisionNumbers.push_back(obj->revisionNumber());
	_objects.push_back(obj);
}

/******************************************************************************
* Replaces a scene object with a new one.
******************************************************************************/
void PipelineFlowState::replaceObject(SceneObject* oldObj, const OORef<SceneObject>& newObj)
{
	OVITO_CHECK_OBJECT_POINTER(oldObj);
	for(int index = 0; index < _objects.size(); index++) {
		if(_objects[index] == oldObj) {
			if(newObj) {
				_revisionNumbers[index] = newObj->revisionNumber();
				_objects[index] = newObj;
			}
			else {
				_revisionNumbers.remove(index);
				_objects.remove(index);
			}
			return;
		}
	}
	OVITO_ASSERT_MSG(false, "PipelineFlowState::replaceObject", "Scene object not found.");
}

/******************************************************************************
* Updates the stored revision number for a scene object.
******************************************************************************/
void PipelineFlowState::updateRevisionNumber(SceneObject* obj)
{
	OVITO_ASSERT(_objects.size() == _revisionNumbers.size());
	for(int index = 0; index < _objects.size(); index++) {
		if(_objects[index] == obj) {
			_revisionNumbers[index] = obj->revisionNumber();
			return;
		}
	}
	OVITO_ASSERT_MSG(false, "PipelineFlowState::updateRevisionNumber", "Scene object not found.");
}

/******************************************************************************
* Updates the stored revision numbers for all scene objects.
******************************************************************************/
void PipelineFlowState::updateRevisionNumbers()
{
	OVITO_ASSERT(_objects.size() == _revisionNumbers.size());
	for(int index = 0; index < _objects.size(); index++) {
		_revisionNumbers[index] = _objects[index]->revisionNumber();
	}
}

};
