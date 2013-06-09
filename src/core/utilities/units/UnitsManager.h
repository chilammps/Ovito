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
 * \file UnitsManager.h
 * \brief Contains the definition of the Ovito::UnitsManager class.
 */

#ifndef __OVITO_UNITS_MANAGER_H
#define __OVITO_UNITS_MANAGER_H

#include <core/Core.h>
#include <core/animation/AnimManager.h>

namespace Ovito {

/**
 * \brief Base class for parameter unit conversion services. 
 * 
 * A ParameterUnit is used to convert a controller value from the controller's native units to
 * another unit presented to the user and vice versa. One example for
 * a ParameterUnit is the AngleUnit class which converts between radians and degrees.
 */
class ParameterUnit : public OvitoObject
{
	Q_OBJECT
	OVITO_OBJECT

protected:
	
	/// \brief Default constructor.
	ParameterUnit() {}
    
public:

	/// \brief Converts a value from native units to the units presented to the user.
	/// \param nativeValue The value in internal units to be converted.
	/// \return The value as converted to the user units.
	/// \sa userToNative()
	virtual FloatType nativeToUser(FloatType nativeValue) = 0;
	
	/// \brief Converts a value from user units to the native units used internally.
	/// \param userValue The value to be converted back to internal units.
	/// \return The converted value.
	/// \sa nativeToUser()
	virtual FloatType userToNative(FloatType userValue) = 0;
	
	/// \brief Converts the given string to a value.
	/// \param valueString This is a string representation of a value as it might have
	///                    been produced by formatValue() or entered by the user.
	/// \return The parsed value in user units.
	/// \throw Exception when the value could not be parsed.
	/// \sa formatValue()
	virtual FloatType parseString(const QString& valueString) = 0;
	
	/// \brief Converts a numeric value to a string.
	/// \param value The value to be converted. This is in user units.
	/// \return The string representation of the value. This can be converted back using parseString().
	/// \sa parseString()
	virtual QString formatValue(FloatType value) = 0;
	
	/// \brief Returns the step size used by spinner widgets for this paremeter unit type.
	/// \param currentValue The current value of the spinner in native units. This can be used to make the step size value dependent.
	/// \param upDirection Specifies whether the spinner is dragged in the positive or the negative direction.
	/// \return The numeric step size used by SpinnerWidget for this parameter type. This is in native units.
	/// 
	/// The default implementation just returns 1.
	virtual FloatType stepSize(FloatType currentValue, bool upDirection) { return 1; }

Q_SIGNALS:

	/// \brief This signal is emitted by the parameter unit when the display
	///        format or conversion factor has changed and the value to string
	///        conversion has to be re-done.
	void formatChanged();
};

/**
 * \brief Default parameter unit that is used by float controllers that
 *        have no custom ParameterUnit class assigned. 
 * 
 * It does no unit conversion at all. Values are formatted as floating-point strings.
 */
class FloatParameterUnit : public ParameterUnit
{
	Q_OBJECT
	OVITO_OBJECT

public:
	
	/// \brief Default constructor.
	Q_INVOKABLE FloatParameterUnit() : ParameterUnit() {}

	/// \brief Converts a value from native units to the units presented to the user.
	/// \param nativeValue The value in internal units to be converted.
	/// \return The value as converted to the user units.
	///
	/// This implementation just returns the unmodified input value.
	/// \sa userToNative()
	virtual FloatType nativeToUser(FloatType nativeValue) Q_DECL_OVERRIDE { return nativeValue; }
	
	/// \brief Converts a value from user units to the native units used internally.
	/// \param userValue The value to be converted back to internal units.
	/// \return The converted value.
	///
	/// This default implementation just returns the unmodified input value.
	/// \sa nativeToUser()
	virtual FloatType userToNative(FloatType userValue) Q_DECL_OVERRIDE { return userValue; }

	/// \brief Converts the given string to a value.
	/// \param valueString This is a string representation of a value as it might have
	///                    been produced by formatValue() or entered by the user.
	/// \return The parsed value in user units.
	/// \throw Exception when the value could not be parsed.
	/// \sa formatValue()
	virtual FloatType parseString(const QString& valueString) Q_DECL_OVERRIDE {
		double value;
		bool ok;
		value = valueString.toDouble(&ok);		
		if(!ok)
			throw Exception(tr("Invalid floating point value: %1").arg(valueString));
		return (FloatType)value;
	}
		
	/// \brief Converts a numeric value to a string.
	/// \param value The value to be converted. This is in user units.
	/// \return The string representation of the value. This can be converted back using parseString().
	/// \sa parseString()
	virtual QString formatValue(FloatType value) Q_DECL_OVERRIDE {
		return QString::number(value);
	}

	/// \brief Returns the step size used by spinner widgets for this parameter unit type.
	/// \param currentValue The current value of the spinner in native units. This can be used to make the step size value dependent.
	/// \param upDirection Specifies whether the spinner is dragged in the positive or the negative direction.
	/// \return The numeric step size used by SpinnerWidget for this parameter type. This is in native units.
	virtual FloatType stepSize(FloatType currentValue, bool upDirection) Q_DECL_OVERRIDE {
		int exponent;
		currentValue = nativeToUser(currentValue);
		if(currentValue != 0) {
			if(upDirection ^ (currentValue < 0.0))
				exponent = (int)floor(log10(abs(currentValue))-1.0);
			else
				exponent = (int)floor(log10(abs(currentValue))-1.0001);
			if(exponent < -5) exponent = -5;
			else if(exponent > 5) exponent = 5;
		}
		else exponent = 0;
		return userToNative(pow(10.0, exponent));
	}
};

/**
 * \brief Default parameter unit that is used by integer controllers fields that
 *        have no custom ParameterUnit class assigned. 
 * 
 * It does no unit conversion at all. Values are formatted as integer value strings.
 */
class IntegerParameterUnit : public ParameterUnit
{
	Q_OBJECT
	OVITO_OBJECT

public:
	
	/// \brief Default constructor.
	Q_INVOKABLE IntegerParameterUnit() : ParameterUnit() {}

	/// \brief Converts a value from native units to the units presented to the user.
	/// \param nativeValue The value in internal units to be converted.
	/// \return The value as converted to the user units.
	///
	/// This implementation just returns the unmodified input value.
	/// \sa userToNative()
	virtual FloatType nativeToUser(FloatType nativeValue) Q_DECL_OVERRIDE { return nativeValue; }
	
	/// \brief Converts a value from user units to the native units used internally.
	/// \param userValue The value to be converted back to internal units.
	/// \return The converted value.
	///
	/// This default implementation just returns the unmodified input value.
	/// \sa nativeToUser()
	virtual FloatType userToNative(FloatType userValue) Q_DECL_OVERRIDE { return userValue; }

	/// \brief Converts the given string to a value.
	/// \param valueString This is a string representation of a value as it might have
	///                    been produced by formatValue() or entered by the user.
	/// \return The parsed value in user units.
	/// \throw Exception when the value could not be parsed.
	/// \sa formatValue()
	virtual FloatType parseString(const QString& valueString) Q_DECL_OVERRIDE {
		int value;
		bool ok;
		value = valueString.toInt(&ok);		
		if(!ok)
			throw Exception(tr("Invalid integer value: %1").arg(valueString));
		return (FloatType)value;
	}
		
	/// \brief Converts a numeric value to a string.
	/// \param value The value to be converted. This is in user units.
	/// \return The string representation of the value. This can be converted back using parseString().
	/// \sa parseString()
	virtual QString formatValue(FloatType value) Q_DECL_OVERRIDE {
		return QString::number((int)value);
	}
};

/*
 * \brief This ParameterUnit is used by parameter values that specify a distance or a position in space.
 */
class WorldParameterUnit : public FloatParameterUnit
{
	Q_OBJECT
	OVITO_OBJECT

public:

	/// \brief Default constructor.
	Q_INVOKABLE WorldParameterUnit() : FloatParameterUnit() {}
};

/**
 * \brief This ParameterUnit implementation converts between radians and degrees.
 */
class AngleParameterUnit : public FloatParameterUnit
{
	Q_OBJECT
	OVITO_OBJECT

public:
	
	/// \brief Default constructor.
	Q_INVOKABLE AngleParameterUnit() : FloatParameterUnit() {}

	/// \brief Converts a value from native units to the units presented to the user.
	/// \param nativeValue The value in internal units to be converted.
	/// \return The value as converted to the user units.
	///
	/// This implementation converts from radians to degrees.
	/// \sa userToNative()
	virtual FloatType nativeToUser(FloatType nativeValue) Q_DECL_OVERRIDE { return nativeValue * (180.0 / FLOATTYPE_PI); }
	
	/// \brief Converts a value from user units to the native units used internally.
	/// \param userValue The value to be converted back to internal units.
	/// \return The converted value.
	///
	/// This default implementation converts from degrees to radians.
	/// \sa nativeToUser()
	virtual FloatType userToNative(FloatType userValue) Q_DECL_OVERRIDE { return userValue * (FLOATTYPE_PI / 180.0); }
};

/**
 * \brief This ParameterUnit implementation is used for percentage values.
 */
class PercentParameterUnit : public FloatParameterUnit
{
	Q_OBJECT
	OVITO_OBJECT

public:
	
	/// \brief Default constructor.
	Q_INVOKABLE PercentParameterUnit() : FloatParameterUnit() {}

	/// \brief Converts a value from native units to the units presented to the user.
	/// \param nativeValue The value in internal units to be converted.
	/// \return The value as converted to the user units.
	///
	/// This implementation converts from the [0,1] range to the [0,100] percent range.
	/// \sa userToNative()
	virtual FloatType nativeToUser(FloatType nativeValue) Q_DECL_OVERRIDE { return nativeValue * 100.0; }
	
	/// \brief Converts a value from user units to the native units used internally.
	/// \param userValue The value to be converted back to internal units.
	/// \return The converted value.
	///
	/// This default implementation converts from the [0,100] percent range to the [0,1] range.
	/// \sa nativeToUser()
	virtual FloatType userToNative(FloatType userValue) Q_DECL_OVERRIDE { return userValue / 100.0; }
};

/**
 * \brief This ParameterUnit is used by parameter values that specify a time value.
 */
class TimeParameterUnit : public IntegerParameterUnit
{
	Q_OBJECT
	OVITO_OBJECT

public:
	
	/// \brief Default constructor.
	Q_INVOKABLE TimeParameterUnit() : IntegerParameterUnit() {
		// If the animation speed changes or the time format has been changed then
		// this parameter unit will send a signal to the UI controls.
		connect(&AnimManager::instance(), SIGNAL(speedChanged(int)), SIGNAL(formatChanged()));
		connect(&AnimManager::instance(), SIGNAL(timeFormatChanged()), SIGNAL(formatChanged()));
	}

	/// \brief Converts the given string to a time value.
	/// \param valueString This is a string representation of a value as it might have
	///                    been produced by formatValue() or entered by the user.
	/// \return The parsed value in TimeTicks.
	/// \throw Exception when the value could not be parsed.
	/// \sa formatValue()
	virtual FloatType parseString(const QString& valueString) override {
		return AnimManager::instance().stringToTime(valueString);
	}
		
	/// \brief Converts a time value to a string.
	/// \param value The time value to be converted. This is in TimeTicks units.
	/// \return The string representation of the value. This can be converted back using parseString().
	/// \sa parseString()
	virtual QString formatValue(FloatType value) override {
		return AnimManager::instance().timeToString((TimePoint)value);
	}

	/// \brief Returns the step size used by spinner widgets for this parameter unit type.
	/// \param currentValue The current value of the spinner. This can be used to make the step size value dependent.
	/// \param upDirection Specifies whether the spinner is dragged in the positive or the negative direction.
	/// \return The numeric step size used by SpinnerWidget for this parameter type. This is in TimeTicks units.
	virtual FloatType stepSize(FloatType currentValue, bool upDirection) override {
		return AnimManager::instance().ticksPerFrame();
	}
};

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Manages the parameter units.
 */
class UnitsManager : public QObject
{
	Q_OBJECT

public:

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the UnitsManager singleton class.
	inline static UnitsManager& instance() {
		OVITO_ASSERT_MSG(_instance != nullptr, "UnitsManager::instance", "Singleton object is not initialized yet.");
		return *_instance;
	}

	/// \brief Returns the instance of a parameter unit service.
	/// \param parameterUnitClass Specifies the unit type. This must a ParameterUnit derived class.
	/// \return The global instance of the requested class. The UnitsManager will always return
	///         the same instance of a ParameterUnit class.
	ParameterUnit* getUnit(const OvitoObjectType& parameterUnitClass);

	/// \brief Returns the special identity parameter unit that does no unit conversion at all and that
	///        formats values as floating-point.
	FloatParameterUnit* floatIdentityUnit() { return _floatIdentityUnit; }

	/// \brief Returns the special identity parameter unit that does no unit conversion at all and that
	///        formats values as integer.
	IntegerParameterUnit* integerIdentityUnit() { return _integerIdentityUnit; }

	/// \brief Returns the instance of a parameter unit service for time values.
	TimeParameterUnit* timeUnit() { return _timeUnit; }

	/// \brief Returns the instance of a parameter unit service for percentage values.
	PercentParameterUnit* percentUnit() { return _percentUnit; }

	/// \brief Returns the instance of a parameter unit service for angles.
	AngleParameterUnit* angleUnit() { return _angleUnit; }

	/// \brief Returns the instance of a parameter unit service for world space distances.
	WorldParameterUnit* worldUnit() { return _worldUnit; }

private:

	/// The list of unit types.
	std::map<const OvitoObjectType*, OORef<ParameterUnit>> _units;

	/// The special float identity unit.
	FloatParameterUnit* _floatIdentityUnit;

	/// The special integer identity unit.
	IntegerParameterUnit* _integerIdentityUnit;

	/// A parameter unit service for time values.
	TimeParameterUnit* _timeUnit;

	/// A parameter unit service for percentage values.
	PercentParameterUnit* _percentUnit;

	/// A parameter unit service for angles.
	AngleParameterUnit* _angleUnit;

	/// A parameter unit service for world space distances.
	WorldParameterUnit* _worldUnit;

private:
    
	/// This is a singleton class. No public instances allowed.
	UnitsManager();

	/// Create the singleton instance of this class.
	static void initialize() { _instance = new UnitsManager(); }

	/// Deletes the singleton instance of this class.
	static void shutdown() { delete _instance; _instance = nullptr; }

	/// The singleton instance of this class.
	static UnitsManager* _instance;

	friend class Application;
};


};

#endif // __OVITO_UNITS_MANAGER_H
