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
#include <core/scene/objects/PipelineFlowState.h>
#include <core/scene/objects/SceneObject.h>

namespace Ovito {

/******************************************************************************
* Default Constructor.
******************************************************************************/
PipelineFlowState::PipelineFlowState() : _stateValidity(TimeNever)
{
}

/******************************************************************************
* Constructor.
******************************************************************************/
PipelineFlowState::PipelineFlowState(SceneObject* sceneObject, const TimeInterval& validityInterval) 
	: RefMaker(), _stateValidity(validityInterval)
{
	INIT_PROPERTY_FIELD(PipelineFlowState, _sceneObject);
	setResult(sceneObject);
}

/******************************************************************************
* Copy constructor.
******************************************************************************/
PipelineFlowState::PipelineFlowState(const PipelineFlowState& right)
	: RefMaker()
{
	INIT_PROPERTY_FIELD(PipelineFlowState, _sceneObject);
	*this = right;
}

/******************************************************************************
* Sets the result object of the pipeline evaluation.
******************************************************************************/
void PipelineFlowState::setResult(SceneObject* newResult)
{
	_sceneObject = newResult;
}

/******************************************************************************
* Makes a (shallow) copy of the flow state.
******************************************************************************/
PipelineFlowState& PipelineFlowState::operator=(const PipelineFlowState& right)
{
	setResult(right.result());
	setStateValidity(right.stateValidity());
	return *this;
}

};
