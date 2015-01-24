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
#include "Exception.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util)

Exception::ExceptionHandler Exception::exceptionHandler = nullptr;

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

void Exception::logError() const
{
	for(int i = 0; i < _messages.size(); i++)
		qCritical("%s", qPrintable(_messages[i]));
}

void Exception::showError() const
{
	if(exceptionHandler == NULL) {
		logError();
		return;
	}

	exceptionHandler(*this);
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
