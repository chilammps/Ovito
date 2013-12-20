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
 * \file TransformationController.h 
 * \brief Contains the definition of the abstract Ovito::TransformationController class
 *        and its default implementation the Ovito::PRSTransformationController class.
 */

#ifndef __OVITO_TRANSFORMATION_CONTOLLER_H
#define __OVITO_TRANSFORMATION_CONTOLLER_H

#include <core/Core.h>
#include "Controller.h"

namespace Ovito {

/**
 * \brief Base class for all transformation controller implementations.
 * 
 * A transformation controller is used to animate position, rotation and
 * scaling of an object.
 * 
 * Each SceneNode has a transformation controller that can be accessed via
 * the SceneNode::transformationController() method.
 */
class OVITO_CORE_EXPORT TransformationController : public TypedControllerBase<AffineTransformation, AffineTransformation>
{
protected:
	
	/// \brief The constructor.
	TransformationController(DataSet* dataset) : TypedControllerBase<AffineTransformation, AffineTransformation>(dataset) {}
	
public:

	/// \brief Queries the controller for its absolute value at a certain time.
	/// \param[in] time The animation time for which the controller's value should be returned.
	/// \param[out] result Contains the controller's value at the animation time \a time after the method returns.
	/// \param[in,out] validityInterval This interval is reduced such that it contains only those times
	///                                 during which the controller's value does not change.
	virtual void getValue(TimePoint time, AffineTransformation& result, TimeInterval& validityInterval) override {
		result.setIdentity();
		applyValue(time, result, validityInterval);
	}

	/// \brief Adds a translation to the transformation.
	/// \param time The animation at which the translation should be applied to the transformation.
	/// \param translation The translation vector to add to the transformation. This is specified in the coordinate system given by \a axisSystem. 
	/// \param axisSystem The coordinate system in which the translation should be performed.
	/// \undoable
	virtual void translate(TimePoint time, const Vector3& translation, const AffineTransformation& axisSystem) {
		setValue(time, AffineTransformation::translation(axisSystem * translation), false);
	}

	/// \brief Adds a rotation to the transformation.
	/// \param time The animation at which the rotation should be applied to the transformation.
	/// \param rot The rotation to add to the transformation. This is specified in the coordinate system given by \a axisSystem. 
	/// \param axisSystem The coordinate system in which the rotation should be performed.
	/// \undoable
	virtual void rotate(TimePoint time, const Rotation& rot, const AffineTransformation& axisSystem) {
		setValue(time, AffineTransformation::rotation(Rotation(axisSystem * rot.axis(), rot.angle())), false);
	}

	/// \brief Adds a scaling to the transformation.
	/// \param time The animation at which the sclaing should be applied to the transformation.
	/// \param scaling The scaling to add to the transformation. 
	/// \undoable
	virtual void scale(TimePoint time, const Scaling& scaling) {
		setValue(time, AffineTransformation::scaling(scaling), false);
	}

	/// \brief This asks the controller to adjust its value after a scene node has got a new
	///        parent node.
	/// \param time The animation at which to change the controller parent.
	/// \param oldParentTM The transformation of the old parent node.
	/// \param newParentTM The transformation of the new parent node.
	/// \param contextNode The node to which this controller is assigned to.
	///
	/// This method is called by the SceneNode that owns the transformation controller when it
	/// is placed into the scene or below some parent node in the scene node hierarchy.
	///
	/// \undoable
	virtual void changeParent(TimePoint time, const AffineTransformation& oldParentTM, const AffineTransformation& newParentTM, SceneNode* contextNode) = 0;

private:
	
	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief Standard implementation of a transformation controller.
 * 
 * This controller uses three sub-controllers to animate position, rotation and scaling.
 */
class OVITO_CORE_EXPORT PRSTransformationController : public TransformationController
{
public:

	/// Default constructor.
	Q_INVOKABLE PRSTransformationController(DataSet* dataset);

	/// \brief Let the controller apply its value at a certain time to some input variable.
	/// \param[in] time The animation time for which the controller's value should be applied.
	/// \param[in,out] result The controller's value at time \a time is applied to the input value and is returned
	///                       in the same reference variable. 
	/// \param[in,out] validityInterval This interval is reduced such that it contains only those times
	///                                 during which the controller's value does not change.
	/// 
	/// \sa getValue()
	virtual void applyValue(TimePoint time, AffineTransformation& result, TimeInterval& validityInterval) override;

	/// \brief Sets the controller's value at the specified time.
	/// \param time The animation for which the controller's value should be set.
	/// \param newValue The new value to be assigned to the controller.
	/// \param isAbsoluteValue Specifies whether the new value should completely replace the 
	///                        controller's old value or whether it should be added to the old value.
	///
	/// \undoable
	/// \sa getValue()
	virtual void setValue(TimePoint time, const AffineTransformation& newValue, bool isAbsoluteValue) override;

	/// \brief This asks the controller to adjust its value after a scene node has got a new
	///        parent node.
	/// \param time The animation at which to change the controller parent.
	/// \param oldParentTM The transformation of the old parent node.
	/// \param newParentTM The transformation of the new parent node.
	/// \param contextNode The node to which this controller is assigned to.
	///
	/// This method is called by the SceneNode that owns the transformation controller when it
	/// is placed into the scene or below some parent node in the scene node hierarchy.
	///
	/// \undoable
	virtual void changeParent(TimePoint time, const AffineTransformation& oldParentTM, const AffineTransformation& newParentTM, SceneNode* contextNode) override;

	/// \brief Calculates the largest time interval containing the given time during which the
	///        controller's value does not change.
	virtual TimeInterval validityInterval(TimePoint time) override;

	/// \brief Returns the position sub-controller.
	/// \return The sub-controller that controls the translational part of the transformation.
	/// \sa setPositionController()
	PositionController* positionController() const { return _position; }

	/// \brief Returns the rotation sub-controller.
	/// \return The sub-controller that controls the rotational part of the transformation.
	/// \sa setRotationController()
	RotationController* rotationController() const { return _rotation; }

	/// \brief Returns the scaling sub-controller.
	/// \return The sub-controller that controls the scaling part of the transformation.
	/// \sa setScacingController()
	ScalingController* scalingController() const { return _scaling; }

	/// \brief Assigns a new sub-controller for the translational component of the transformation.
	/// \param position The new controller.
	/// \sa positionController()
	/// \undoable
	void setPositionController(PositionController* position) { 
		OVITO_CHECK_OBJECT_POINTER(position);
		this->_position = position;
	}

	/// \brief Assigns a new sub-controller for the translational component of the transformation.
	/// \param position The new controller.
	/// \note This is the same method as above but takes a smart pointer instead of a raw pointer.
	/// \undoable
	void setPositionController(const OORef<PositionController>& position) { setPositionController(position.get()); }

	/// \brief Assigns a new sub-controller for the rotational component of the transformation.
	/// \param rotation The new controller.
	/// \sa rotationController()
	/// \undoable
	void setRotationController(RotationController* rotation) { 
		OVITO_CHECK_OBJECT_POINTER(rotation);
		this->_rotation = rotation;
	}

	/// \brief Assigns a new sub-controller for the rotational component of the transformation.
	/// \param rotation The new controller.
	/// \note This is the same method as above but takes a smart pointer instead of a raw pointer.
	/// \undoable
	void setRotationController(const OORef<RotationController>& rotation) { setRotationController(rotation.get()); }

	/// \brief Assigns a new sub-controller for the scaling component of the transformation.
	/// \param scaling The new controller.
	/// \sa scalingController()
	/// \undoable
	void setScalingController(ScalingController* scaling) { 
		OVITO_CHECK_OBJECT_POINTER(scaling);
		this->_scaling = scaling;
	}

	/// \brief Assigns a new sub-controller for the scaling component of the transformation.
	/// \param scaling The new controller.
	/// \note This is the same method as above but takes a smart pointer instead of a raw pointer.
	/// \undoable
	void setScalingController(const OORef<ScalingController>& scaling) { setScalingController(scaling.get()); }

	/// \brief Adds a translation to the transformation.
	/// \param time The animation at which the translation should be applied to the transformation.
	/// \param translation The translation vector to add to the transformation. This is specified in the coordinate system given by \a axisSystem. 
	/// \param axisSystem The coordinate system in which the translation should be performed.
	/// \undoable
	virtual void translate(TimePoint time, const Vector3& translation, const AffineTransformation& axisSystem) override {
		// Transform translation vector to reference coordinate system.
		positionController()->setValue(time, axisSystem * translation, false);
	}

	/// \brief Adds a rotation to the transformation.
	/// \param time The animation at which the rotation should be applied to the transformation.
	/// \param rot The rotation to add to the transformation. This is specified in the coordinate system given by \a axisSystem. 
	/// \param axisSystem The coordinate system in which the rotation should be performed.
	/// \undoable
	virtual void rotate(TimePoint time, const Rotation& rot, const AffineTransformation& axisSystem) override {
		rotationController()->setValue(time, Rotation(axisSystem * rot.axis(), rot.angle()), false);
	}

	/// \brief Adds a scaling to the transformation.
	/// \param time The animation at which the sclaing should be applied to the transformation.
	/// \param s The scaling to add to the transformation. 
	/// \undoable
	virtual void scale(TimePoint time, const Scaling& s) override {
		scalingController()->setValue(time, s, false);
	}

private:

	/// The sub-controller for translation.
	ReferenceField<PositionController> _position;

	/// The sub-controller for rotation.
	ReferenceField<RotationController> _rotation;

	/// The sub-controller for scaling.
	ReferenceField<ScalingController> _scaling;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_position);
	DECLARE_REFERENCE_FIELD(_rotation);
	DECLARE_REFERENCE_FIELD(_scaling);
};


};

#endif // __OVITO_TRANSFORMATION_CONTOLLER_H
