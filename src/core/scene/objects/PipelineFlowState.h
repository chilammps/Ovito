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

/** 
 * \file PipelineFlowState.h 
 * \brief Contains the definition of the Ovito::PipelineFlowState class.
 */
 
#ifndef __OVITO_PIPELINE_FLOW_STATE_H
#define __OVITO_PIPELINE_FLOW_STATE_H

#include <core/Core.h>
#include <core/animation/TimeInterval.h>
#include <core/reference/RefMaker.h>

namespace Ovito {

class SceneObject;		// defined in SceneObject.h

/**
 * \brief This object flows down the geometry pipeline of an ObjectNode.
 */
class PipelineFlowState
{
public:

	/// \brief Default constructor that creates an empty state object.
	PipelineFlowState();

	/// \brief Constructor that creates an state object and initializes it with a SceneObject.
	/// \param sceneObject The object represents the current state of a geometry pipeline evaluation.
	/// \param validityInterval Specifies the time interval during which the returned object is valid.
	///                         For times outside this interval the geometry pipeline has to be re-evaluated.
	PipelineFlowState(SceneObject* sceneObject, const TimeInterval& validityInterval);

	/// \brief Copy constructor that creates a shallow copy of a state object.
	/// \param right The state object to be copied.
	/// \note This copy constructor does not copy the scene object contained in \a right but only the reference to it.  
	PipelineFlowState(const PipelineFlowState& right);
	
	/// \brief Returns the result object from the pipeline evaluation.
	/// \return The scene object that flows down the geometry pipeline.
	///         This can be \c NULL in some cases.
	/// \sa setResult()
	SceneObject* result() const { return _sceneObject; }

	/// \brief Sets the result object of the pipeline evaluation.
	/// \param newResult The new pipeline evaluation result.
	///
	/// This method replaces the scene object with a new one.
	/// \sa result() 
	void setResult(SceneObject* newResult);

	/// \brief Sets the result object of the pipeline evaluation.
	/// 
	/// Same function as above but accepts a smart pointer instead of a raw pointer.
	void setResult(const intrusive_ptr<SceneObject>& newResult) { setResult(newResult.get()); }

	/// \brief Gets the validity interval for this pipeline state.
	/// \return The time interval during which the returned object is valid.
	///         For times outside this interval the geometry pipeline has to be re-evaluated.
	const TimeInterval& stateValidity() const { return _stateValidity; }
	
	/// \brief Specifies the validity interval for this pipeline state.
	/// \param newInterval The time interval during which the object set by setResult() is valid.
	/// \sa intersectStateValidity()
	void setStateValidity(const TimeInterval& newInterval) { _stateValidity = newInterval; }

	/// \brief Reduces the validity interval of this pipeline state to include only the given time interval.
	/// \param intersectionInterval The current validity interval is reduced to include only this time interval.
	/// \sa setStateValidity()
	void intersectStateValidity(const TimeInterval& intersectionInterval) { _stateValidity.intersect(intersectionInterval); }

	/// \brief Makes a (shallow) copy of the flow state.
	/// \param right The state object to be copied.
	/// \note This copy operator does not copy the scene object contained in \a right but only the reference to it.
	PipelineFlowState& operator=(const PipelineFlowState& right);

private:

	/// Contains the object that flows up the geometry pipeline
	/// and is modified by modifiers.
	OORef<SceneObject> _sceneObject;

	/// Contains the validity interval for this pipeline flow state.
	TimeInterval _stateValidity;
};

};

#endif // __OVITO_PIPELINE_FLOW_STATE_H
