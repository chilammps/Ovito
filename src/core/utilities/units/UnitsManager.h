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

#ifndef __OVITO_UNITS_MANAGER_H
#define __OVITO_UNITS_MANAGER_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Units)

/**
 * \brief Base class for parameter unit conversion services. 
 * 
 * A ParameterUnit is used to convert a controller value from the controller's native units to
 * another unit presented to the user and vice versa. One example for
 * a ParameterUnit is the AngleUnit class which converts between radians and degrees.
 */
class OVITO_CORE_EXPORT ParameterUnit : public QObject
{
	Q_OBJECT

protected:
	
	/// \brief Constructor.
	ParameterUnit(QObject* parent, DataSet* dataset) : QObject(parent) {}
    
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
	
	/// \brief Returns positive the step size used by spinner widgets for this parameter unit type.
	/// \param currentValue The current value of the spinner in native units. This can be used to make the step size value dependent.
	/// \param upDirection Specifies whether the spinner is dragged in the positive or the negative direction.
	/// \return The numeric step size used by SpinnerWidget for this parameter type. This is in native units.
	/// 
	/// The default implementation just returns 1.
	virtual FloatType stepSize(FloatType currentValue, bool upDirection) { return 1; }

	/// \brief Given an arbitrary value, which is potentially invalid, rounds it to the closest valid value.
	///
	/// The default implementation does no rounding and simply returns the unmodified value.
	virtual FloatType roundValue(FloatType value) { return value; }

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
class OVITO_CORE_EXPORT FloatParameterUnit : public ParameterUnit
{
	Q_OBJECT

public:
	
	/// \brief Default constructor.
	Q_INVOKABLE FloatParameterUnit(QObject* parent, DataSet* dataset) : ParameterUnit(parent, dataset) {}

	/// \brief Converts a value from native units to the units presented to the user.
	/// \param nativeValue The value in internal units to be converted.
	/// \return The value as converted to the user units.
	///
	/// This implementation just returns the unmodified input value.
	/// \sa userToNative()
	virtual FloatType nativeToUser(FloatType nativeValue) override { return nativeValue; }
	
	/// \brief Converts a value from user units to the native units used internally.
	/// \param userValue The value to be converted back to internal units.
	/// \return The converted value.
	///
	/// This default implementation just returns the unmodified input value.
	/// \sa nativeToUser()
	virtual FloatType userToNative(FloatType userValue) override { return userValue; }

	/// \brief Converts the given string to a value.
	/// \param valueString This is a string representation of a value as it might have
	///                    been produced by formatValue() or entered by the user.
	/// \return The parsed value in user units.
	/// \throw Exception when the value could not be parsed.
	/// \sa formatValue()
	virtual FloatType parseString(const QString& valueString) override {
		double value;
		bool ok;
		value = valueString.toDouble(&ok);		
		if(!ok)
			throw Exception(tr("Invalid floating-point value: %1").arg(valueString));
		return (FloatType)value;
	}
		
	/// \brief Converts a numeric value to a string.
	/// \param value The value to be converted. This is in user units.
	/// \return The string representation of the value. This can be converted back using parseString().
	/// \sa parseString()
	virtual QString formatValue(FloatType value) override {
		return QString::number(value);
	}

	/// \brief Returns the positive step size used by spinner widgets for this parameter unit type.
	/// \param currentValue The current value of the spinner in native units. This can be used to make the step size value dependent.
	/// \param upDirection Specifies whether the spinner is dragged in the positive or the negative direction.
	/// \return The numeric step size used by SpinnerWidget for this parameter type. This is in native units.
	virtual FloatType stepSize(FloatType currentValue, bool upDirection) override {
		int exponent;
		currentValue = nativeToUser(currentValue);
		if(currentValue != 0) {
			exponent = (int)std::floor(std::log10(std::abs(currentValue))-1.0);
			if(exponent < -5) exponent = -5;
			else if(exponent > 5) exponent = 5;
		}
		else exponent = 0;
		return userToNative(std::pow(FloatType(10), exponent));
	}
};

/**
 * \brief Default parameter unit that is used by integer controllers fields that
 *        have no custom ParameterUnit class assigned. 
 * 
 * It does no unit conversion at all. Values are formatted as integer value strings.
 */
class OVITO_CORE_EXPORT IntegerParameterUnit : public ParameterUnit
{
	Q_OBJECT

public:
	
	/// \brief Default constructor.
	Q_INVOKABLE IntegerParameterUnit(QObject* parent, DataSet* dataset) : ParameterUnit(parent, dataset) {}

	/// \brief Converts a value from native units to the units presented to the user.
	/// \param nativeValue The value in internal units to be converted.
	/// \return The value as converted to the user units.
	///
	/// This implementation just returns the unmodified input value.
	/// \sa userToNative()
	virtual FloatType nativeToUser(FloatType nativeValue) override { return nativeValue; }
	
	/// \brief Converts a value from user units to the native units used internally.
	/// \param userValue The value to be converted back to internal units.
	/// \return The converted value.
	///
	/// This default implementation just returns the unmodified input value.
	/// \sa nativeToUser()
	virtual FloatType userToNative(FloatType userValue) override { return userValue; }

	/// \brief Converts the given string to a value.
	/// \param valueString This is a string representation of a value as it might have
	///                    been produced by formatValue() or entered by the user.
	/// \return The parsed value in user units.
	/// \throw Exception when the value could not be parsed.
	/// \sa formatValue()
	virtual FloatType parseString(const QString& valueString) override {
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
	virtual QString formatValue(FloatType value) override {
		return QString::number((int)value);
	}

	/// \brief Given an arbitrary value, which is potentially invalid, rounds it to the closest valid value.
	virtual FloatType roundValue(FloatType value) override {
		return floor(value + (FloatType)0.5);
	}

};

/*
 * \brief This ParameterUnit is used by parameter values that specify a distance or a position in space.
 */
class OVITO_CORE_EXPORT WorldParameterUnit : public FloatParameterUnit
{
	Q_OBJECT

public:

	/// \brief Default constructor.
	Q_INVOKABLE WorldParameterUnit(QObject* parent, DataSet* dataset) : FloatParameterUnit(parent, dataset) {}
};

/**
 * \brief This ParameterUnit implementation converts between radians and degrees.
 */
class OVITO_CORE_EXPORT AngleParameterUnit : public FloatParameterUnit
{
	Q_OBJECT

public:
	
	/// \brief Default constructor.
	Q_INVOKABLE AngleParameterUnit(QObject* parent, DataSet* dataset) : FloatParameterUnit(parent, dataset) {}

	/// \brief Converts a value from native units to the units presented to the user.
	/// \param nativeValue The value in internal units to be converted.
	/// \return The value as converted to the user units.
	///
	/// This implementation converts from radians to degrees.
	/// \sa userToNative()
	virtual FloatType nativeToUser(FloatType nativeValue) override { return nativeValue * (FloatType(180) / FLOATTYPE_PI); }
	
	/// \brief Converts a value from user units to the native units used internally.
	/// \param userValue The value to be converted back to internal units.
	/// \return The converted value.
	///
	/// This default implementation converts from degrees to radians.
	/// \sa nativeToUser()
	virtual FloatType userToNative(FloatType userValue) override { return userValue * (FLOATTYPE_PI / FloatType(180)); }
};

/**
 * \brief This ParameterUnit implementation is used for percentage values.
 */
class OVITO_CORE_EXPORT PercentParameterUnit : public FloatParameterUnit
{
	Q_OBJECT

public:
	
	/// \brief Default constructor.
	Q_INVOKABLE PercentParameterUnit(QObject* parent, DataSet* dataset) : FloatParameterUnit(parent, dataset) {}

	/// \brief Converts a value from native units to the units presented to the user.
	/// \param nativeValue The value in internal units to be converted.
	/// \return The value as converted to the user units.
	///
	/// This implementation converts from the [0,1] range to the [0,100] percent range.
	/// \sa userToNative()
	virtual FloatType nativeToUser(FloatType nativeValue) override { return nativeValue * FloatType(100); }
	
	/// \brief Converts a value from user units to the native units used internally.
	/// \param userValue The value to be converted back to internal units.
	/// \return The converted value.
	///
	/// This default implementation converts from the [0,100] percent range to the [0,1] range.
	/// \sa nativeToUser()
	virtual FloatType userToNative(FloatType userValue) override { return userValue / FloatType(100); }
};

/**
 * \brief This ParameterUnit is used by parameter values that specify a time value.
 */
class OVITO_CORE_EXPORT TimeParameterUnit : public IntegerParameterUnit
{
	Q_OBJECT

public:
	
	/// \brief Constructor.
	Q_INVOKABLE TimeParameterUnit(QObject* parent, DataSet* dataset);

	/// \brief Converts the given string to a time value.
	/// \param valueString This is a string representation of a value as it might have
	///                    been produced by formatValue() or entered by the user.
	/// \return The parsed value in TimeTicks.
	/// \throw Exception when the value could not be parsed.
	/// \sa formatValue()
	virtual FloatType parseString(const QString& valueString) override;
		
	/// \brief Converts a time value to a string.
	/// \param value The time value to be converted. This is in TimeTicks units.
	/// \return The string representation of the value. This can be converted back using parseString().
	/// \sa parseString()
	virtual QString formatValue(FloatType value) override;

	/// \brief Returns the (positive) step size used by spinner widgets for this parameter unit type.
	/// \param currentValue The current value of the spinner. This can be used to make the step size value dependent.
	/// \param upDirection Specifies whether the spinner is dragged in the positive or the negative direction.
	/// \return The numeric step size used by SpinnerWidget for this parameter type. This is in TimeTicks units.
	virtual FloatType stepSize(FloatType currentValue, bool upDirection) override;

	/// \brief Given an arbitrary value, which is potentially invalid, rounds it to the closest valid value.
	virtual FloatType roundValue(FloatType value) override;

private Q_SLOTS:

	/// \brief This is called whenever the current animation settings of the dataset have been replaced by new ones.
	void onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings);

private:

	QMetaObject::Connection _speedChangedConnection;
	QMetaObject::Connection _timeFormatChangedConnection;

	// The current animation settings.
	AnimationSettings* _animSettings;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Manages the parameter units.
 */
class OVITO_CORE_EXPORT UnitsManager : public QObject
{
	Q_OBJECT

public:

	/// Constructor.
	UnitsManager(DataSet* dataset);

	/// \brief Returns the instance of a parameter unit service.
	/// \param parameterUnitClass Specifies the unit type. This must a ParameterUnit derived class.
	/// \return The global instance of the requested class. The UnitsManager will always return
	///         the same instance of a ParameterUnit class.
	ParameterUnit* getUnit(const QMetaObject* parameterUnitClass);

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

	/// The dataset this units manager belongs to.
	DataSet* _dataset;

	/// The list of unit types.
	std::map<const QMetaObject*, ParameterUnit*> _units;

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
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_UNITS_MANAGER_H
