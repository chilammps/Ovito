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
 * \file PRSTransformationController.h
 * \brief Contains the definition of the Ovito::PRSTransformationController class.
 */

#ifndef __OVITO_PRS_TRANSFORMATION_CONTROLLER_H
#define __OVITO_PRS_TRANSFORMATION_CONTROLLER_H

#include <core/Core.h>
#include "Controller.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

/**
 * \brief Standard implementation of a transformation controller.
 * 
 * This controller uses three sub-controllers to animate position, rotation, and scaling.
 */
class OVITO_CORE_EXPORT PRSTransformationController : public Controller
{
public:

	/// Default constructor.
	Q_INVOKABLE PRSTransformationController(DataSet* dataset);

	/// \brief Returns the value type of the controller.
	virtual ControllerType controllerType() const override { return ControllerTypeTransformation; }

	/// \brief Lets the transformation controller apply its value to an existing transformation matrix.
	/// \param[in] time The animation time.
	/// \param[in,out] result The controller will apply its transformation to this matrix.
	/// \param[in,out] validityInterval This interval is reduced to the period during which the controller's value doesn't change.
	virtual void applyTransformation(TimePoint time, AffineTransformation& result, TimeInterval& validityInterval) override;

	/// \brief Gets a position controller's value at a certain animation time.
	/// \param[in] time The animation time at which the controller's value should be computed.
	/// \param[out] result This output variable takes the controller's values.
	/// \param[in,out] validityInterval This interval is reduced to the period during which the controller's value doesn't change.
	virtual void getPositionValue(TimePoint time, Vector3& result, TimeInterval& validityInterval) override { positionController()->getPositionValue(time, result, validityInterval); }

	/// \brief Gets a rotation controller's value at a certain animation time.
	/// \param[in] time The animation time at which the controller's value should be computed.
	/// \param[out] result This output variable takes the controller's values.
	/// \param[in,out] validityInterval This interval is reduced to the period during which the controller's value doesn't change.
	virtual void getRotationValue(TimePoint time, Rotation& result, TimeInterval& validityInterval) override { rotationController()->getRotationValue(time, result, validityInterval); }

	/// \brief Gets a scaling controller's value at a certain animation time.
	/// \param[in] time The animation time at which the controller's value should be computed.
	/// \param[out] result This output variable takes the controller's values.
	/// \param[in,out] validityInterval This interval is reduced to the period during which the controller's value doesn't change.
	virtual void getScalingValue(TimePoint time, Scaling& result, TimeInterval& validityInterval) override { scalingController()->getScalingValue(time, result, validityInterval); }

	/// \brief Sets a transformation controller's value at the given animation time.
	/// \param time The animation time at which to set the controller's value.
	/// \param newValue The new value to be assigned to the controller.
	/// \param isAbsolute Specifies whether the transformation is absolute or should be applied to the existing transformation.
	virtual void setTransformationValue(TimePoint time, const AffineTransformation& newValue, bool isAbsolute) override;

	/// \brief Sets a position controller's value at the given animation time.
	/// \param time The animation time at which to set the controller's value.
	/// \param newValue The new value to be assigned to the controller.
	/// \param isAbsolute Specifies whether the value is absolute or should be applied to the existing transformation.
	virtual void setPositionValue(TimePoint time, const Vector3& newValue, bool isAbsolute) override { positionController()->setPositionValue(time, newValue, isAbsolute); }

	/// \brief Sets a rotation controller's value at the given animation time.
	/// \param time The animation time at which to set the controller's value.
	/// \param newValue The new value to be assigned to the controller.
	/// \param isAbsolute Specifies whether the value is absolute or should be applied to the existing transformation.
	virtual void setRotationValue(TimePoint time, const Rotation& newValue, bool isAbsolute) override { rotationController()->setRotationValue(time, newValue, isAbsolute); }

	/// \brief Sets a scaling controller's value at the given animation time.
	/// \param time The animation time at which to set the controller's value.
	/// \param newValue The new value to be assigned to the controller.
	/// \param isAbsolute Specifies whether the value is absolute or should be applied to the existing transformation.
	virtual void setScalingValue(TimePoint time, const Scaling& newValue, bool isAbsolute) override { scalingController()->setScalingValue(time, newValue, isAbsolute); }

	/// \brief Adjusts the controller's value after a scene node has gotten a new parent node.
	/// \param time The animation at which to change the controller parent.
	/// \param oldParentTM The transformation of the old parent node.
	/// \param newParentTM The transformation of the new parent node.
	/// \param contextNode The node to which this controller is assigned to.
	virtual void changeParent(TimePoint time, const AffineTransformation& oldParentTM, const AffineTransformation& newParentTM, SceneNode* contextNode) override;

	/// \brief Calculates the largest time interval containing the given time during which the
	///        controller's value does not change.
	virtual TimeInterval validityInterval(TimePoint time) override;

	/// \brief Returns the position sub-controller.
	/// \return The sub-controller that controls the translational part of the transformation.
	Controller* positionController() const { return _position; }

	/// \brief Returns the rotation sub-controller.
	/// \return The sub-controller that controls the rotational part of the transformation.
	Controller* rotationController() const { return _rotation; }

	/// \brief Returns the scaling sub-controller.
	/// \return The sub-controller that controls the scaling part of the transformation.
	Controller* scalingController() const { return _scaling; }

	/// \brief Assigns a new sub-controller for the translational component of the transformation.
	/// \param position The new controller.
	void setPositionController(Controller* position) {
		OVITO_CHECK_OBJECT_POINTER(position);
		this->_position = position;
	}

	/// \brief Assigns a new sub-controller for the rotational component of the transformation.
	/// \param rotation The new controller.
	void setRotationController(Controller* rotation) {
		OVITO_CHECK_OBJECT_POINTER(rotation);
		this->_rotation = rotation;
	}

	/// \brief Assigns a new sub-controller for the scaling component of the transformation.
	/// \param scaling The new controller.
	void setScalingController(Controller* scaling) {
		OVITO_CHECK_OBJECT_POINTER(scaling);
		this->_scaling = scaling;
	}

	/// \brief Adds a translation to the transformation.
	/// \param time The animation at which the translation should be applied to the transformation.
	/// \param translation The translation vector to add to the transformation. This is specified in the coordinate system given by \a axisSystem. 
	/// \param axisSystem The coordinate system in which the translation should be performed.
	/// \undoable
	virtual void translate(TimePoint time, const Vector3& translation, const AffineTransformation& axisSystem) override {
		// Transform translation vector to reference coordinate system.
		positionController()->setPositionValue(time, axisSystem * translation, false);
	}

	/// \brief Adds a rotation to the transformation.
	/// \param time The animation at which the rotation should be applied to the transformation.
	/// \param rot The rotation to add to the transformation. This is specified in the coordinate system given by \a axisSystem. 
	/// \param axisSystem The coordinate system in which the rotation should be performed.
	/// \undoable
	virtual void rotate(TimePoint time, const Rotation& rot, const AffineTransformation& axisSystem) override {
		rotationController()->setRotationValue(time, Rotation(axisSystem * rot.axis(), rot.angle()), false);
	}

	/// \brief Adds a scaling to the transformation.
	/// \param time The animation at which the scaling should be applied to the transformation.
	/// \param s The scaling to add to the transformation. 
	/// \undoable
	virtual void scale(TimePoint time, const Scaling& s) override {
		scalingController()->setScalingValue(time, s, false);
	}

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Transformation"); }

private:

	/// The sub-controller for translation.
	ReferenceField<Controller> _position;

	/// The sub-controller for rotation.
	ReferenceField<Controller> _rotation;

	/// The sub-controller for scaling.
	ReferenceField<Controller> _scaling;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_position);
	DECLARE_REFERENCE_FIELD(_rotation);
	DECLARE_REFERENCE_FIELD(_scaling);
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_PRS_TRANSFORMATION_CONTROLLER_H
