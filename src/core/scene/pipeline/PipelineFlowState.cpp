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
	OVITO_ASSERT_MSG(_objects.find(obj) == _objects.end(), "PipelineFlowState::addObject", "Cannot add the same scene object more than once.");
	_objects.insert(std::make_pair(obj, obj->revisionNumber()));
}

/******************************************************************************
* Updates the stored revision number for a scene object.
******************************************************************************/
void PipelineFlowState::setRevisionNumber(SceneObject* obj, unsigned int revNumber)
{
	for(auto& entry : _objects) {
		if(entry.first == obj) {
			entry.second = revNumber;
			return;
		}
	}
	OVITO_ASSERT_MSG(false, "PipelineFlowState::setRevisionNumber", "Scene object not found.");
}

};
