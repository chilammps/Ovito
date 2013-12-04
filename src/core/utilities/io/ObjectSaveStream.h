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
 * \file ObjectSaveStream.h 
 * \brief Contains definition of the Ovito::ObjectSaveStream class.
 */

#ifndef __OVITO_OBJECT_SAVESTREAM_H
#define __OVITO_OBJECT_SAVESTREAM_H

#include <core/Core.h>
#include <base/io/SaveStream.h>

namespace Ovito {

class OvitoObject;		// defined in OvitoObject.h

/**
 * \brief An output stream that can serialize a graph of OvitoObject instances to a file.
 */
class OVITO_CORE_EXPORT ObjectSaveStream : public SaveStream
{
	Q_OBJECT	

public:

	/// \brief Opens the stream for writing.
	/// \param destination The data stream to which the binary data is written. This must be 
	///                    stream that supports random access.
	/// \throw Exception when the given data stream does only support sequential access. 
	ObjectSaveStream(QDataStream& destination) : SaveStream(destination), _dataSet(nullptr) {}

	/// \brief The destructor closes the stream.
	virtual ~ObjectSaveStream();

	/// \brief Closes the stream.
	/// \note The underlying data stream is not closed by this method.
	virtual void close() override;

	/// \brief Saves an object with runtime type information to the stream.
	void saveObject(OvitoObject* object);

protected:

	/// Contains all objects stored so far and their IDs.
	std::map<OvitoObject*, quint32> _objectMap;

	/// Contains all objects ordered by ID.
	QVector<OvitoObject*> _objects;

	/// The current dataset being saved.
	DataSet* _dataSet;
};

};

#endif // __OVITO_OBJECT_SAVESTREAM_H
