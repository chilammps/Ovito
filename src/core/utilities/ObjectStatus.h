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
 * \brief Encapsulates status information about an operation, e.g. evaluating a Modifier in the geometry pipeline.
 */
class ObjectStatus
{
public:

	enum StatusType {
		Success,
		Warning,
		Error
	};

	/// Default constructor that creates a status object with status StatusType::Success and an empty text.
	ObjectStatus() : _type(Success) {}

	/// Constructs a status object with the given status and optional text string describing the status.
	ObjectStatus(StatusType t, const QString& shortText = QString(), const QString& longText = QString()) :
		_type(t), _shortText(shortText), _longText(longText) {}

	/// Returns the type of status stores in this object.
	StatusType type() const { return _type; }

	/// Returns a short text string describing the status.
	const QString& shortText() const { return _shortText; }

	/// Returns a more verbose text string describing the status.
	const QString& longText() const { return _longText; }

	/// Tests two status objects for equality.
	bool operator==(const ObjectStatus& other) const {
		return (_type == other._type) &&
				(_shortText == other._shortText) &&
				(_longText == other._longText);
	}

	/// Tests two status objects for inequality.
	bool operator!=(const ObjectStatus& other) const { return !(*this == other); }

private:

	/// The status.
	StatusType _type;

	/// A human-readable string describing the status.
	QString _shortText;

	/// A more verbose, human-readable string describing the status.
	QString _longText;

	friend SaveStream& operator<<(SaveStream& stream, const ObjectStatus& s);
	friend LoadStream& operator>>(LoadStream& stream, ObjectStatus& s);
};

/// \brief Writes a status object to a file stream.
/// \param stream The output stream.
/// \param s The status to write to the output stream \a stream.
/// \return The output stream \a stream.
inline SaveStream& operator<<(SaveStream& stream, const ObjectStatus& s)
{
	stream.beginChunk(0x01);
	stream.writeEnum(s._type);
	stream << s._shortText;
	stream << s._longText;
	stream.endChunk();
	return stream;
}

/// \brief Reads a status object from a binary input stream.
/// \param stream The input stream.
/// \param s Reference to a variable where the parsed data will be stored.
/// \return The input stream \a stream.
inline LoadStream& operator>>(LoadStream& stream, ObjectStatus& s)
{
	stream.expectChunk(0x01);
	stream.readEnum(s._type);
	stream >> s._shortText;
	stream >> s._longText;
	stream.closeChunk();
	return stream;
}

};

#endif // __OVITO_OBJECT_STATUS_H
