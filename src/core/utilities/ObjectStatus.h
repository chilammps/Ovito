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
 * \file ObjectStatus.h
 * \brief Contains the definition of the Ovito::ObjectStatus class.
 */

#ifndef __OVITO_OBJECT_STATUS_H
#define __OVITO_OBJECT_STATUS_H

#include <core/Core.h>

namespace Ovito {

/**
 * \brief Encapsulates the status information associated with a scene object or
 *        a pipeline evaluation.
 */
class OVITO_CORE_EXPORT ObjectStatus
{
public:

	enum StatusType {
		Success,
		Warning,
		Error,
		Pending
	};

	/// Default constructor that creates a status object with status StatusType::Success and an empty text.
	ObjectStatus() : _type(Success) {}

	/// Constructs a status object with the given status and optional text string describing the status.
	ObjectStatus(StatusType t, const QString& text = QString()) :
		_type(t), _text(text) {}

	/// Returns the type of status stores in this object.
	StatusType type() const { return _type; }

	/// Returns a text string describing the status.
	const QString& text() const { return _text; }

	/// Tests two status objects for equality.
	bool operator==(const ObjectStatus& other) const {
		return (_type == other._type) &&
				(_text == other._text);
	}

	/// Tests two status objects for inequality.
	bool operator!=(const ObjectStatus& other) const { return !(*this == other); }

private:

	/// The status.
	StatusType _type;

	/// A human-readable string describing the status.
	QString _text;

	friend SaveStream& operator<<(SaveStream& stream, const ObjectStatus& s);
	friend LoadStream& operator>>(LoadStream& stream, ObjectStatus& s);
};

/// \brief Writes a status object to a file stream.
/// \param stream The output stream.
/// \param s The status to write to the output stream \a stream.
/// \return The output stream \a stream.
inline SaveStream& operator<<(SaveStream& stream, const ObjectStatus& s)
{
	stream.beginChunk(0x02);
	stream.writeEnum(s._type);
	stream << s._text;
	stream.endChunk();
	return stream;
}

/// \brief Reads a status object from a binary input stream.
/// \param stream The input stream.
/// \param s Reference to a variable where the parsed data will be stored.
/// \return The input stream \a stream.
inline LoadStream& operator>>(LoadStream& stream, ObjectStatus& s)
{
	quint32 version = stream.expectChunkRange(0x0, 0x02);
	stream.readEnum(s._type);
	stream >> s._text;
	if(version <= 0x01)
		stream >> s._text;
	stream.closeChunk();
	return stream;
}

/// \brief Writes a status object to the log stream.
inline QDebug operator<<(QDebug debug, const ObjectStatus& s)
{
	switch(s.type()) {
	case ObjectStatus::Success: debug << "Success"; break;
	case ObjectStatus::Pending: debug << "Pending"; break;
	case ObjectStatus::Warning: debug << "Warning"; break;
	case ObjectStatus::Error: debug << "Error"; break;
	}
	if(s.text().isEmpty() == false)
		debug << s.text();
	return debug;
}

};

#endif // __OVITO_OBJECT_STATUS_H
