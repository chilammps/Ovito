///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2008) Alexander Stukowski
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
 * \file EvaluationStatus.h
 * \brief Contains the definition of the Core::EvaluationStatus class.
 */

#ifndef __OVITO_EVALUATION_STATUS_H
#define __OVITO_EVALUATION_STATUS_H

#include <core/Core.h>

namespace Core {

/**
 * \brief Contains status information about the evaluation of a Modifier or something similar.
 *
 * \author Alexander Stukowski
 */
class EvaluationStatus
{
public:

	enum EvaluationStatusType {
		EVALUATION_SUCCESS,
		EVALUATION_WARNING,
		EVALUATION_ERROR
	};

	EvaluationStatus(EvaluationStatusType t = EVALUATION_SUCCESS, const QString& shortMessage = QString()) :
		_type(t), _shortMessage(shortMessage), _longMessage(shortMessage) {}

	EvaluationStatus(EvaluationStatusType t, const QString& shortMessage, const QString& longMessage) :
		_type(t), _shortMessage(shortMessage), _longMessage(longMessage) {}

	EvaluationStatus(const EvaluationStatus& other) : _type(other._type), _shortMessage(other._shortMessage), _longMessage(other._longMessage) {}

	EvaluationStatusType type() const { return _type; }

	const QString& shortMessage() const { return _shortMessage; }

	const QString& longMessage() const { return _longMessage; }

	bool operator==(const EvaluationStatus& other) const {
		return _type == other._type && _shortMessage == other._shortMessage && _longMessage == other._longMessage;
	}

	EvaluationStatus& operator=(const EvaluationStatus& other) {
		_type = other._type;
		_shortMessage = other._shortMessage;
		_longMessage = other._longMessage;
		return *this;
	}

private:
	EvaluationStatusType _type;
	QString _shortMessage;
	QString _longMessage;
	friend SaveStream& operator<<(SaveStream& stream, const EvaluationStatus& s);
	friend LoadStream& operator>>(LoadStream& stream, EvaluationStatus& s);
};

/// \brief Writes a status object to a file stream.
/// \param stream The output stream.
/// \param s The status to write to the output stream \a stream.
/// \return The output stream \a stream.
inline SaveStream& operator<<(SaveStream& stream, const EvaluationStatus& s)
{
	stream.beginChunk(0x0000001);
	stream.writeEnum(s._type);
	stream << s._shortMessage;
	stream << s._longMessage;
	stream.endChunk();
	return stream;
}

/// \brief Reads a status object from a binary input stream.
/// \param stream The input stream.
/// \param s Reference to a variable where the parsed data will be stored.
/// \return The input stream \a stream.
inline LoadStream& operator>>(LoadStream& stream, EvaluationStatus& s)
{
	stream.expectChunk(0x0000001);
	stream.readEnum(s._type);
	stream >> s._shortMessage;
	stream >> s._longMessage;
	stream.closeChunk();
	return stream;
}

};

#endif // __OVITO_EVALUATION_STATUS_H
