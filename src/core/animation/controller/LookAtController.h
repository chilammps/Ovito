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
 * \file LookAtController.h 
 * \brief Contains the definition of the Ovito::LookAtController class.
 */

#ifndef __OVITO_LOOKAT_CONTROLLER_H
#define __OVITO_LOOKAT_CONTROLLER_H

#include <core/Core.h>
#include "Controller.h"
#include <core/scene/SceneNode.h>

namespace Ovito {

/**
 * \brief Rotation controller that lets an object always "look" at another scene object.
 * 
 * This RotationController computes a rotation matrix for a SceneNode such
 * that it always faces into the direction of another SceneNode.
 */
class OVITO_CORE_EXPORT LookAtController : public RotationController
{
public:
	
	/// \brief Default constructor.
	Q_INVOKABLE LookAtController();

	/// Queries the controller for its absolute value at a certain time.
	virtual void getValue(TimePoint time, Rotation& result, TimeInterval& validityInterval) override;

	/// Sets the controller's value at the specified time.
	virtual void setValue(TimePoint time, const Rotation& newValue, bool isAbsoluteValue) override;

	/// Let the controller add its value at a certain time to the input value.
	virtual void applyValue(TimePoint time, AffineTransformation& result, TimeInterval& validityInterval) override;

	/// Computes the largest time interval containing the given time during which the
	/// controller's value is constant.
	virtual TimeInterval validityInterval(TimePoint time) override;

	/// This asks the controller to adjust its value after a scene node has got a new
	/// parent node.
	///		oldParentTM - The transformation of the old parent node
	///		newParentTM - The transformation of the new parent node
	///		contextNode - The node to which this controller is assigned to
	virtual void changeParent(TimePoint time, const AffineTransformation& oldParentTM, const AffineTransformation& newParentTM, SceneNode* contextNode) override {}

	/// \brief Returns the target node to look at.
	/// \return The target this rotation controller is looking at or \c NULL if it
	///         has not been set yet.
	SceneNode* targetNode() const { return _targetNode; }

	/// \brief Sets the target node to look at.
	/// \param target The new target this rotation controller should use for its
	///               calculation of the rotation matrix.
	/// \undoable
	void setTargetNode(SceneNode* target) { _targetNode = target; }

	/// \brief Sets the target node to look at.
	/// \param target The new target this rotation controller should use for its
	///               calculation of the rotation matrix.
	/// \note This is the same method as above but takes a smart pointer instead of a raw pointer.
	/// \undoable
	void setTargetNode(const OORef<SceneNode>& target) { setTargetNode(target.get()); }
	
	/// \brief Returns the sub-controller that controls the rolling parameter.
	/// \return The sub-controller for the rolling angle.
	FloatController* rollController() const { return _rollCtrl; }

	/// \brief Sets the sub-controller that controls the rolling parameter.
	/// \param ctrl The new roll angle controller. 
	/// \undoable
	void setRollController(FloatController* ctrl) { _rollCtrl = ctrl; }

	/// \brief Sets the sub-controller that controls the rolling parameter.
	/// \param ctrl The new roll angle controller. 
	/// \note This is the same method as above but takes a smart pointer instead of a raw pointer.
	/// \undoable
	void setRollController(const OORef<FloatController>& ctrl) { setRollController(ctrl.get()); }
	
private:

	/// The sub-controller for rolling.
	ReferenceField<FloatController> _rollCtrl;

	/// The target scene node to look at.
	ReferenceField<SceneNode> _targetNode;

	/// Stores the position of the source node.
	Vector3 _sourcePos;

	/// Stores the validity interval of the saved source position.
	TimeInterval _sourcePosValidity;

private:

	Q_OBJECT
	OVITO_OBJECT
	DECLARE_REFERENCE_FIELD(_rollCtrl)
	DECLARE_REFERENCE_FIELD(_targetNode)
};

};

#endif // __OVITO_LOOKAT_CONTROLLER_H
