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
 * \file StandardConstControllers.h 
 * \brief Contains the definition of the Ovito::StandardConstController template class and
 *        its derived classes. 
 */

#ifndef __OVITO_STD_CONST_CONTROLLERS_H
#define __OVITO_STD_CONST_CONTROLLERS_H

#include <core/Core.h>
#include "Controller.h"
#include <core/gui/undo/UndoManager.h>
#include <core/animation/AnimManager.h>

namespace Ovito {

/**
 * Template class for constant controllers.
 */
template<class BaseControllerClass, typename ValueType, typename NullValue, class AddFunction = std::plus<ValueType>>
class StandardConstController : public BaseControllerClass
{
	typedef StandardConstController<BaseControllerClass, ValueType, NullValue, AddFunction> ThisClassType;
	
protected:

	// An undo stack record that restores the old controller value.
	class ChangeValueOperation : public UndoableOperation {
	public:
		ChangeValueOperation(StandardConstController* ctrl) : _controller(ctrl), _storedValue(ctrl->_value) {}
		virtual void undo() override {
			// Restore old controller value.
			std::swap(_controller->_value, _storedValue);
			_controller->notifyDependents(ReferenceEvent::TargetChanged);
		}
		virtual void redo() override { undo(); }
	private:
		OORef<StandardConstController> _controller;
		ValueType _storedValue;
	};

public:

    // Default constructor.
	StandardConstController() : _value(NullValue()) {}

	/// Queries the controller for its absolute value.
	virtual void getValue(TimePoint time, ValueType& result, TimeInterval& validityInterval) override {
		result = _value;
	}

	/// Sets the controller's value at the specified time.
	virtual void setValue(TimePoint time, const ValueType& newValue, bool isAbsoluteValue) override {
		ValueType newValue2;
		if(!isAbsoluteValue) {
			AddFunction func;
			newValue2 = func(newValue, _value);
		}
		else newValue2 = newValue;
		if(newValue2 == _value) return;	// No value change.

		if(UndoManager::instance().isRecording())
			UndoManager::instance().push(new ChangeValueOperation(this));
		_value = newValue2;
		this->notifyDependents(ReferenceEvent::TargetChanged);
	}

protected:

	/// Saves the class' contents to the given stream. 
	virtual void saveToStream(ObjectSaveStream& stream) override {
		BaseControllerClass::saveToStream(stream);
		stream.beginChunk(0x01);
		stream << _value;
		stream.endChunk(); 
	}

	/// Loads the class' contents from the given stream. 
	virtual void loadFromStream(ObjectLoadStream& stream) override {
		BaseControllerClass::loadFromStream(stream);
		stream.expectChunk(0x01);
		stream >> _value;
		stream.closeChunk(); 
	}

	/// Creates a copy of this object. 
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override {
		// Let the base class create an instance of this class.
		OORef<ThisClassType> clone = static_object_cast<ThisClassType>(BaseControllerClass::clone(deepCopy, cloneHelper));
		clone->_value = this->_value;
		return clone;
	}

	/// The value of the constant controller.
    ValueType _value;
};

// Define some standard constant controllers.
class ConstFloatController : public StandardConstController<FloatController, FloatType, FloatType> {
public:
	Q_INVOKABLE ConstFloatController() {}
private:
	Q_OBJECT
	OVITO_OBJECT
};

class ConstIntegerController : public StandardConstController<IntegerController, int, int> {
public:
	Q_INVOKABLE ConstIntegerController() {}
private:
	Q_OBJECT
	OVITO_OBJECT
};

class ConstVectorController : public StandardConstController<VectorController, Vector3, Vector3::Zero> {
public:
	Q_INVOKABLE ConstVectorController() {}
private:
	Q_OBJECT
	OVITO_OBJECT
};

struct _BooleanValueAddFunction : public std::binary_function<bool, bool, bool> {
	bool operator()(const bool& b1, const bool& b2) const { return b2; }
};
class ConstBooleanController : public StandardConstController<BooleanController, bool, bool, _BooleanValueAddFunction> {
public:
	Q_INVOKABLE ConstBooleanController() {}
private:
	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_STD_CONST_CONTROLLERS_H
