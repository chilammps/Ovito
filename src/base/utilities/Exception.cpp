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

#include <base/Base.h>
#include <base/utilities/Exception.h>

namespace Ovito {

/// The exception handler for displaying error messages.
Exception::ExceptionHandler Exception::exceptionHandler = NULL;

Exception::Exception()
{
	_messages.push_back("An exception has occurred.");
}

Exception::Exception(const QString& message)
{
	_messages.push_back(message);
}

Exception& Exception::appendDetailMessage(const QString& message)
{
	_messages.push_back(message);
	return *this;
}

Exception& Exception::prependGeneralMessage(const QString& message)
{
	_messages.push_front(message);
	return *this;
}

/// Logs the error message to the console/debugger.
void Exception::logError() const
{
	for(int i = 0; i < _messages.size(); i++)
		qCritical("%s", qPrintable(_messages[i]));
}

/// Displays the error message using the current message handler.
void Exception::showError() const
{
	if(exceptionHandler == NULL) {
		logError();
		return;
	}

	exceptionHandler(*this);
}

};	// End of namespace Base
