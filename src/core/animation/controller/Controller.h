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
 * \file Controller.h 
 * \brief Contains the definition of the Ovito::Controller class, its derived classes,
 *        and the Ovito::ControllerManager class.
 */

#ifndef __OVITO_CONTROLLER_H
#define __OVITO_CONTROLLER_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/animation/TimeInterval.h>
#include <core/animation/AnimManager.h>

namespace Ovito {

class SceneNode;		// defined in SceneNode.h

/**
 * \brief Base class for all animation controllers.
 * 
 * Controllers are used to describe animatable parameters of an object. A Controller
 * controls how the object parameter changes with time.
 * 
 * Instances of Controller-derived classes can be created using the ControllerManager.
 */
class OVITO_CORE_EXPORT Controller : public RefTarget
{
protected:
	
	/// \brief Default constructor.
	Controller() {}

public:
	
	/// \brief Calculates the largest time interval containing the given time during which the
	///        controller's value does not change.
	/// \param[in] time The animation time at which the controller's validity interval is requested.
	/// \return The interval during which the controller's value does not change.
	virtual TimeInterval validityInterval(TimePoint time) = 0;
	
	/// \brief Rescales the key times of all keys from the old animation interval to the new interval.
	/// \param oldAnimationInterval The old animation interval that will be mapped to the new animation interval.
	/// \param newAnimationInterval The new animation interval.
	/// 
	/// For keyed controllers this will rescale the key times of all keys from the 
	/// old animation interval to the new interval using a linear mapping.
	///
	/// Please note that keys that lie outside the old animation interval will also be scaled 
	/// according to a linear extrapolation.
	///
	/// The default implementation does nothing. 
	///
	/// \undoable
	virtual void rescaleTime(const TimeInterval& oldAnimationInterval, const TimeInterval& newAnimationInterval) {}

private:
	
	Q_OBJECT
	OVITO_OBJECT
};


/**
 * \brief This template class is used to Controller types.
 * 
 * This template class is used to define Controller classes for
 * various data types. It defines getter and setter methods for the controller's value.
 * 
 * The template parameter \c ValueType specifies the data type of the controller's value.
 * 
 * The template parameter \c ApplicationType specifies the data type to which the 
 * controller's value can be applied. This is only meaningful for position, rotation and scale
 * controllers, which can apply their values to an AffineTransformation.
 */
template<typename ValueType, typename ApplicationType>
class TypedController : public Controller 
{
protected:
	
    /// \brief Default constructor.
	TypedController() {}

public:
	
	/// \brief Queries the controller for its absolute value at a certain time.
	/// \param[in] time The animation time for which the controller's value should be returned.
	/// \param[out] result Contains the controller's value at the animation time \a time after the method returns.
	/// \param[in,out] validityInterval This interval is reduced such that it contains only those times
	///                                 during which the controller's value does not change.
	///
	/// If the validity interval is not not important than getValueAtTime() can also be used.
	///
	/// \sa applyValue()
	/// \sa getValueAtTime(), getCurrentValue()
	/// \sa setValue()
	virtual void getValue(TimePoint time, ValueType& result, TimeInterval& validityInterval) = 0;

	/// \brief Let the controller apply its value at a certain time to some input variable.
	/// \param[in] time The animation time for which the controller's value should be applied.
	/// \param[in,out] result The controller's value at time \a time is applied to the input value and is returned
	///                       in the same reference variable. How the value is applied is data type dependent.
	/// \param[in,out] validityInterval This interval is reduced such that it contains only those times
	///                                 during which the controller's value does not change.
	/// 
	/// \sa getValue()
	virtual void applyValue(TimePoint time, ApplicationType& result, TimeInterval& validityInterval) = 0;
	
	/// \brief Queries the controller for its absolute value at the given animation time.
	/// \param time The animation time for which the controller's value should be returned.
	/// \return The controller's value at the animation time \a time.
	/// \sa getValue(), getCurrentValue()
	ValueType getValueAtTime(TimePoint time) {
		TimeInterval iv; ValueType v;
		getValue(time, v, iv);
		return v;
	}

	/// \brief Queries the controller for its absolute value at the current animation time.
	/// \return The controller's value at the current animation time given by AnimManager::time().
	/// \sa getValueAtTime()
	/// \sa setCurrentValue()
	ValueType currentValue() {
		return getValueAtTime(AnimManager::instance().time());
	}

	/// \brief Sets the controller's value at the specified time.
	/// \param time The animation for which the controller's value should be set.
	/// \param newValue The new value to be assigned to the controller.
	/// \param isAbsoluteValue Specifies whether the new value should completely replace the 
	///                        controller's old value or whether it should be added to the old value.
	///
	/// \undoable
	/// \sa getValue()
	virtual void setValue(TimePoint time, const ValueType& newValue, bool isAbsoluteValue = true) = 0;

	/// \brief Sets the controller's value at the current animation time.
	/// \param newValue The new absolute value assigned to the controller at the current
	///                 animation time given by AnimManager::time().
	///
	/// \undoable
	/// \sa setValue()
	/// \sa getCurrentValue()
	void setCurrentValue(const ValueType& newValue) { 
		setValue(AnimManager::instance().time(), newValue, true);
	}

	/// \brief Calculates the largest time interval containing the given time during which the
	///        controller's value does not change.
	/// \param[in] time The animation time at which the controller's validity interval is requested.
	/// \return The interval during which the controller's value does not change.
	///
	/// This implementation returns the interval computed by getValue().
	virtual TimeInterval validityInterval(TimePoint time) override {
		ValueType v;
		TimeInterval iv(TimeInterval::forever());
		getValue(time, v, iv);
		return iv;
	}
	
	/// \brief Creates a new key at the given time with the specified value.
	/// \param time The animation where a key should be created.
	/// \param value The absolute value of the new animation key.
	///
	/// Any existing key at that time is replaced with the new key.
	///
	/// The default implementation does nothing. This method should be overridden
	/// by controller classes that use animation keys.
	///
	/// \undoable
	virtual void createKey(TimePoint time, const ValueType& value) {}
};

/**
 * \brief Base class for all float value controller implementations.
 * 
 * This controller class is used for object parameters with the FloatType data type.
 */
class OVITO_CORE_EXPORT FloatController : public TypedController<FloatType, FloatType>
{ 
protected:

	/// The default constructor.
	FloatController() {}
	
public:
	
	/// Let the controller add its value at a certain time to the input value.
	virtual void applyValue(TimePoint time, FloatType& result, TimeInterval& validityInterval) override {
		FloatType v;
		getValue(time, v, validityInterval);
		result += v;
	}
	
private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief Base class for all integer value controller implementations.
 * 
 * This controller class is used for object parameters with the \c int data type.
 */
class OVITO_CORE_EXPORT IntegerController : public TypedController<int, int>
{
protected: 

	/// The default constructor.
	IntegerController() {}
	
public:

	/// Let the controller add its value at a certain time to the input value.
	virtual void applyValue(TimePoint time, int& result, TimeInterval& validityInterval) override {
		int v;
		getValue(time, v, validityInterval);
		result += v;
	}
	
private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief Base class for all boolean value controller implementations.
 * 
 * This controller class is used for object parameters with the \c bool data type.
 */
class OVITO_CORE_EXPORT BooleanController : public TypedController<bool, bool>
{
protected:

	/// The default constructor.
	BooleanController() {}
	
public:	

	/// Let the controller add its value at a certain time to the input value.
	virtual void applyValue(TimePoint time, bool& result, TimeInterval& validityInterval) override {
		bool v;
		getValue(time, v, validityInterval);
		result ^= v;
	}
	
private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief Base class for all vector value controller implementations.
 * 
 * This controller class is used for object parameters with the Vector3 data type.
 */
class OVITO_CORE_EXPORT VectorController : public TypedController<Vector3, Vector3>
{
protected:

	/// The default constructor.
	VectorController() {}
	
public:
	
	/// Let the controller add its value at a certain time to the input value.
	virtual void applyValue(TimePoint time, Vector3& result, TimeInterval& validityInterval) override {
		Vector3 v;
		getValue(time, v, validityInterval);
		result += v;
	}

	/// Queries the controller for its absolute value at a certain time and converts the Vector3 to a color value.
	void getValue(TimePoint time, Color& result, TimeInterval& validityInterval) {
		Vector3 c;
		getValue(time, c, validityInterval);
		result = Color(c);
	}

	/// Queries the controller for its absolute value at a certain time.
	virtual void getValue(TimePoint time, Vector3& result, TimeInterval& validityInterval) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};


/**
 * \brief Base class for all position controller implementations.
 * 
 * A position controller is used to animate the position of an object.
 */
class OVITO_CORE_EXPORT PositionController : public TypedController<Vector3, AffineTransformation>
{
protected:

	/// The default constructor.
	PositionController() {}
	
public:

	/// Let the controller add its value at a certain time to the input value.
	virtual void applyValue(TimePoint time, AffineTransformation& result, TimeInterval& validityInterval) override {
		Vector3 t;
		getValue(time, t, validityInterval);
		result = result * AffineTransformation::translation(t);
	}

	/// \brief This asks the controller to adjust its value after a scene node has got a new
	///        parent node.
	/// \param time The animation at which to change the controller parent.
	/// \param oldParentTM The transformation of the old parent node.
	/// \param newParentTM The transformation of the new parent node.
	/// \param contextNode The node to which this controller is assigned to.
	///
	/// \undoable
	virtual void changeParent(TimePoint time, const AffineTransformation& oldParentTM, const AffineTransformation& newParentTM, SceneNode* contextNode) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief Base class for all rotation controller implementations.
 * 
 * A position controller is used to animate the orientation of an object.
 */
class OVITO_CORE_EXPORT RotationController : public TypedController<Rotation, AffineTransformation>
{
protected:

	/// The default constructor.
	RotationController() {}
	
public:

	/// Let the controller add its value at a certain time to the input value.
	virtual void applyValue(TimePoint time, AffineTransformation& result, TimeInterval& validityInterval) override {
		Rotation r;
		getValue(time, r, validityInterval);
		result = result * Matrix3::rotation(r);
	}

	/// \brief This asks the controller to adjust its value after a scene node has got a new
	///        parent node.
	/// \param time The animation at which to change the controller parent.
	/// \param oldParentTM The transformation of the old parent node.
	/// \param newParentTM The transformation of the new parent node.
	/// \param contextNode The node to which this controller is assigned to.
	///
	/// \undoable
	virtual void changeParent(TimePoint time, const AffineTransformation& oldParentTM, const AffineTransformation& newParentTM, SceneNode* contextNode) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief Base class for all scaling controller implementations.
 * 
 * A position controller is used to animate the scaling of an object.
 */
class OVITO_CORE_EXPORT ScalingController : public TypedController<Scaling, AffineTransformation>
{ 
protected:

	/// The default constructor.
	ScalingController() {}
	
public:

	/// Let the controller add its value at a certain time to the input value.
	virtual void applyValue(TimePoint time, AffineTransformation& result, TimeInterval& validityInterval) override {
		Scaling sv;
		getValue(time, sv, validityInterval);
		result = result * Matrix3::scaling(sv);
	}

	/// \brief This asks the controller to adjust its value after a scene node has got a new
	///        parent node.
	/// \param time The animation at which to change the controller parent.
	/// \param oldParentTM The transformation of the old parent node.
	/// \param newParentTM The transformation of the new parent node.
	/// \param contextNode The node to which this controller is assigned to.
	///
	/// \undoable
	virtual void changeParent(TimePoint time, const AffineTransformation& oldParentTM, const AffineTransformation& newParentTM, SceneNode* contextNode) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

///////////////////////////////// Controller instantiation //////////////////////////////

/**
 * \brief Provides access to default controller implementations.
 */
class OVITO_CORE_EXPORT ControllerManager
{
public:

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the ControllerManager singleton class.
	inline static ControllerManager& instance() {
		OVITO_ASSERT_MSG(_instance != nullptr, "ControllerManager::instance", "Singleton object is not initialized yet.");
		return *_instance;
	}

	/// \brief Creates a new instance of the default implementation for the given base Controller type.
	/// \param controllerBaseClass The type of controller to create. This must be one of the base Controller derived
	///                            classes like IntegerController or FloatController.
	/// \return The newly created instance of the controller class set as default for the requested base controller class.
	///         If no default is set for the given base class than \c NULL is returned.
	OORef<Controller> createDefaultController(const OvitoObjectType& controllerBaseClass);

	/// \brief Creates a new instance of the default implementation for the given base controller type.
	/// \return The newly created instance of the controller class set as default for the requested base controller class.
	///         If no default is set for the given base class than \c NULL is returned.
	/// 
	/// This is the template version of the above method. The function parameter is replaced with a template parameter.
	template<typename T>
	OORef<T> createDefaultController() { return static_object_cast<T>(createDefaultController(T::OOType)); }

private:

	/// Stores the default implementations used for the different controller types.
	std::map<const OvitoObjectType*, const OvitoObjectType*> _defaultMap;

	/// Private constructor.
	/// This is a singleton class; no public instances are allowed.
	ControllerManager();

	/// Create the singleton instance of this class.
	static void initialize() { _instance = new ControllerManager(); }

	/// Deletes the singleton instance of this class.
	static void shutdown() { delete _instance; _instance = nullptr; }

	/// The singleton instance of this class.
	static ControllerManager* _instance;

	friend class Application;
};

};

#endif // __OVITO_CONTROLLER_H
