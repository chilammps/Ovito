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
#include <core/gui/app/CommandLineParser.h>

namespace Ovito {

/******************************************************************************
* Adds the option to look for while parsing.
******************************************************************************/
bool CommandLineParser::addOption(const CommandLineOption& option)
{
	if(option.names().empty())
		return false;

	// Check for duplicate option names.
	for(const CommandLineOption& o : _options) {
		for(const QString& existingName : o.names()) {
			if(option.names().contains(existingName))
				return false;
		}
	}

	// Register option.
	_options.push_back(option);

	return true;
}

/******************************************************************************
* Parses the command line arguments.
******************************************************************************/
bool CommandLineParser::parse(int argc, char** argv, bool ignoreUnknownOptions)
{
	QStringList arguments;
	arguments.reserve(argc);
	for(int i = 0; i < argc; i++)
		arguments << QString::fromLocal8Bit(argv[i]);
	return parse(arguments, ignoreUnknownOptions);
}

/******************************************************************************
* Parses the command line arguments.
******************************************************************************/
bool CommandLineParser::parse(const QStringList& arguments, bool ignoreUnknownOptions)
{
	_values.clear();
	_positionalArguments.clear();

	if(arguments.empty()) {
		_errorText = QStringLiteral("The first command line argument should always specify the program name.");
		return false;
	}

	bool allPositional = false;
	for(int index = 1; index < arguments.size(); index++) {
		const QString& arg = arguments[index];
		if(allPositional) {
			_positionalArguments.push_back(arg);
		}
		else if(arg == QStringLiteral("--")) {
			// Treat all following arguments as positional.
			allPositional = true;
		}
		else if(arg.startsWith(QChar('-'))) {
			QString optionName;
			if(arg.startsWith(QStringLiteral("--")))
				optionName = arg.mid(2);
			else
				optionName = arg.mid(1);
			bool foundOption = false;
			for(const CommandLineOption& option : _options) {
				if(option.names().contains(optionName)) {
					foundOption = true;
					if(option.valueName().isEmpty()) {
						_values.insert(&option, QString());
					}
					else {
						if(index == arguments.size() - 1) {
							_errorText = QString("Expected argument after command line option %1.").arg(arg);
							return false;
						}
						_values.insertMulti(&option, arguments[index+1]);
						index++;
					}
					break;
				}
			}
			if(!foundOption && !ignoreUnknownOptions) {
				_errorText = QString("Unknown command line option: %1").arg(arg);
				return false;
			}
		}
		else {
			_positionalArguments.push_back(arg);
		}
	}

	_errorText = QString();
	return true;
}

/******************************************************************************
* Checks whether an option was passed to the application.
******************************************************************************/
bool CommandLineParser::isSet(const QString& optionName) const
{
	for(const CommandLineOption& option : _options) {
		if(option.names().contains(optionName)) {
			return _values.contains(&option);
		}
	}
	OVITO_ASSERT_MSG(false, "CommandLineParser::isSet()", "Option with this name has not been registered.");
	return false;
}

/******************************************************************************
* Returns a list of option values found for the given option, or an empty list if not found.
******************************************************************************/
QStringList CommandLineParser::values(const QString& optionName) const
{
	for(const CommandLineOption& option : _options) {
		if(option.names().contains(optionName)) {
			if(option.valueName().isEmpty()) return QStringList();
			QStringList results = _values.values(&option);
			if(results.empty()) return option.defaultValues();
			return results;
		}
	}
	OVITO_ASSERT_MSG(false, "CommandLineParser::values()", "Option with this name has not been registered.");
	return QStringList();
}

/******************************************************************************
* Returns the option value found for the given option, or an empty list if not found.
******************************************************************************/
QString CommandLineParser::value(const QString& optionName) const
{
	for(const CommandLineOption& option : _options) {
		if(option.names().contains(optionName)) {
			if(option.valueName().isEmpty()) return QString();
			return _values.value(&option, option.defaultValues().empty() ? QString() : option.defaultValues().front());
		}
	}
	OVITO_ASSERT_MSG(false, "CommandLineParser::value()", "Option with this name has not been registered.");
	return QString();
}

};
