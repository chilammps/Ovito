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
 * \file
 * \brief Contains the definition of the Ovito::Util::Exception class.
 */

#ifndef __OVITO_EXCEPTION_H
#define __OVITO_EXCEPTION_H

#include <base/Base.h>

namespace Ovito { namespace Util {

#ifdef QT_NO_EXCEPTIONS
	#error "OVITO requires Qt exception support. It seems that Qt has been built without exceptions (the macro QT_NO_EXCEPTIONS is defined). Please turn on exception support and rebuild the Qt library."
#endif

/**
 * \brief The base exception class used by this application.
 */
class OVITO_BASE_EXPORT Exception : public QException
{
public:

	/// \brief Default constructor that creates an exception object with a default error message.
	Exception();

	/// \brief Constructor that initializes the exception object with a message string describing
	///        the error that has occurred.
	/// \param message The human-readable (and translated) message that describes the error.
	explicit Exception(const QString& message);

	/// \brief Copy constructor.
	/// \param ex The exception object whose message strings are copied into the new exception object.
	Exception(const Exception& ex) : _messages(ex.messages()) {}

	/// \brief Multi-message constructor that initializes the exception object with multiple message string.
	/// \param errorMessages The list of message strings that describe the error. The list should be ordered with
	///                      the most general error description coming first followed by the more detailed information.
	explicit Exception(const QStringList& errorMessages) : _messages(errorMessages) {}

	/// \brief Class destructor.
	virtual ~Exception() throw() {}

	/// \brief Appends a message to this exception object that describes the error in more detail.
	/// \param message The human-readable and translated description string.
	/// \return A reference to this Exception object.
	Exception& appendDetailMessage(const QString& message);

	/// \brief Prepends a message to this exception object's list of messages that describes the error in a
	///        more general way than the existing message strings in the exception object.
	/// \param message The human-readable and translated description string.
	/// \return A reference to this Exception object.
	Exception& prependGeneralMessage(const QString& message);

	/// \brief Sets the error messages of this exception object.
	/// \param messages The new list of messages that completely replaces the old messages.
	void setMessages(const QStringList& messages) { this->_messages = messages; }

	/// \brief Returns the most general message string in the exception object that describes the error.
	/// \return The first message from the object's list of messages.
	const QString& message() const { return _messages.front(); }

	/// \brief Returns the message strings in the exception object that describe the error.
	/// \return The list of message string stored.
	const QStringList& messages() const { return _messages; }

	/// \brief Logs the error message to the active log target.
	/// \sa showError();
	void logError() const;

	/// \brief Displays the error message using the current error message handler.
	/// \sa setExceptionHandler()
	/// \sa logError()
	void showError() const;

	////////////////////////////////////////////////////////////////////////////////
	/// These functions are for QtConcurrent::Exception

	/// \brief Raises this exception object.
	virtual void raise() const { throw *this; }

	/// \brief Creates a copy of this exception object.
	virtual Exception *clone() const { return new Exception(*this); }

public:

	/// Prototype for a handler function for exceptions.
	typedef void (*ExceptionHandler)(const Exception& exception);

	/// \brief Installs a handler for exceptions.
	/// \param handler The new handler routine. This handler is responsible for showing the error message to the user.
	///
	/// \sa showError();
	static void setExceptionHandler(ExceptionHandler handler) { exceptionHandler = handler; }

private:

	/// The message strings describing the exception.
	/// This list is ordered with the most general error description coming first followed by the more detailed information.
	QStringList _messages;

	/// The exception handler for displaying error messages.
	static ExceptionHandler exceptionHandler;
};

}}	// End of namespace

#endif // __OVITO_EXCEPTION_H
