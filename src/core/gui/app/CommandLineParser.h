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
 * \file CommandLineParser.h
 * \brief Contains the definition of the Ovito::CommandLineParser class.
 */

#ifndef __OVITO_COMMAND_LINE_PARSER_H
#define __OVITO_COMMAND_LINE_PARSER_H

#include <core/Core.h>

namespace Ovito {

/**
 * \brief This class defines a possible command-line option.
 */
class OVITO_CORE_EXPORT CommandLineOption
{
public:

	/// Constructs a command line option object with the given arguments.
	CommandLineOption(const QString& name, const QString& description = QString(), const QString& valueName = QString(),
			const QString& defaultValue = QString()) : _names(name), _description(description), _valueName(valueName) {
		if(!defaultValue.isEmpty())
			setDefaultValue(defaultValue);
	}

	/// Constructs a command line option object with the given arguments.
	CommandLineOption(const QStringList& names, const QString& description = QString(), const QString& valueName = QString(),
			const QString& defaultValue = QString()) : _names(names), _description(description), _valueName(valueName) {
		if(!defaultValue.isEmpty())
			setDefaultValue(defaultValue);
	}

	/// Returns the names set for this option.
	const QStringList& names() const { return _names; }

	/// Returns the description set for this option.
	const QString& description() const { return _description; }

	/// Sets the description used for this option to description.
	/// It is customary to add a "." at the end of the description.
	void setDescription(const QString& description) { _description = description; }

	/// Returns the default values set for this option.
	const QStringList& defaultValues() const { return _defaultValues; }

	/// Sets the list of default values used for this option.
	/// The default values are used if the user of the application does not specify the option on the command line.
	void setDefaultValues(const QStringList& defaultValues) { _defaultValues = defaultValues; }

	/// Sets the default value used for this option.
	/// The default value is used if the user of the application does not specify the option on the command line.
	void setDefaultValue(const QString& defaultValue) { _defaultValues = QStringList(defaultValue); }

	/// Sets the name of the expected value, for the documentation.
	/// Options without a value assigned have a boolean-like behavior: either the user specifies --option or they don't.
	void setValueName(const QString& valueName) { _valueName = valueName; }

	/// Returns the name of the expected value.
	/// If empty, the option doesn't take a value.
	const QString& valueName() const { return _valueName; }

private:

	/// The names set for this option.
	QStringList _names;

	/// The description text.
	QString _description;

	/// The list of default values set for this option.
	QStringList _defaultValues;

	/// The name of the expected value.
	/// If empty, the option doesn't take a value.
	QString _valueName;
};


/**
 * \brief This class provides a means for handling the command line options.
 */
class OVITO_CORE_EXPORT CommandLineParser
{
public:

	/// Adds the option to look for while parsing.
	/// Returns true if adding the option was successful; otherwise returns false.
	/// Adding the option fails if there is no name attached to the option, or the option has a
	/// name that clashes with an option name added before.
	bool addOption(const CommandLineOption& option);

	/// Parses the command line arguments.
	/// Returns false in case of a parse error (unknown option or missing value); returns true otherwise.
	bool parse(int argc, char** argv);

	/// Parses the command line arguments.
	/// Returns false in case of a parse error (unknown option or missing value); returns true otherwise.
	bool parse(const QStringList& arguments);

	/// Returns an error text for the user. This should only be called when parse() has returned false.
	QString errorText() const { return _errorText; }

	/// Returns a list of positional arguments.
	/// These are all of the arguments that were not recognized as part of an option.
	const QStringList& positionalArguments() const { return _positionalArguments; }

	/// Checks whether an option was passed to the application.
	/// Returns true if the option was set, false otherwise.
	bool isSet(const QString& optionName) const;

	/// Returns a list of option values found for the given option, or an empty list if not found.
	/// For options found by the parser, the list will contain an entry for each time the option was encountered by the parser.
	/// If the option wasn't specified on the command line, the default values are returned.
	/// An empty list is returned if the option does not take a value.
	QStringList values(const QString& optionName) const;

	/// Returns the option value found for the given option, or an empty string if not found.
	/// For options found by the parser, the last value found for that option is returned.
	/// If the option wasn't specified on the command line, the default value is returned.
	/// An empty string is returned if the option does not take a value.
	QString value(const QString& optionName) const;

private:

	/// The list of registered command-line options.
	QList<CommandLineOption> _options;

	/// The parsed parameter values.
	QMap<const CommandLineOption*, QString> _values;

	/// The list of parsed positional arguments.
	QStringList _positionalArguments;

	/// Parsing error message.
	QString _errorText;
};

};

#endif // __OVITO_COMMAND_LINE_PARSER_H
