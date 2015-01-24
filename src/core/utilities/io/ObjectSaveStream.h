///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_OBJECT_SAVESTREAM_H
#define __OVITO_OBJECT_SAVESTREAM_H

#include <core/Core.h>
#include "SaveStream.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(IO)

/**
 * \brief An output stream that can serialize an OvitoObject graph a file.
 *
 * This class is used to write OVITO scene files, which are on-disk representations of
 * OvitoObject graphs. The object graph can be read back from the file using ObjectLoadStream.
 *
 * \note All objects written to a stream must belong to the same DataSet.
 *
 * \sa ObjectLoadStream
 */
class OVITO_CORE_EXPORT ObjectSaveStream : public SaveStream
{
	Q_OBJECT	

public:

	/// \brief Constructs the object.
	/// \param destination The Qt data stream to which the objects will be written. This must be a
	///                    stream that supports random access.
	/// \throw Exception if the underlying data stream only supports sequential access.
	ObjectSaveStream(QDataStream& destination) : SaveStream(destination), _dataset(nullptr) {}

	// Calls close() to close the ObjectSaveStream.
	virtual ~ObjectSaveStream();

	/// \brief Closes this ObjectSaveStream, but not the underlying QDataStream passed to the constructor.
	/// \throw Exception if an I/O error has occurred.
	virtual void close() override;

	/// \brief Serializes an object and writes its data to the output stream.
	/// \throw Exception if an I/O error has occurred.
	/// \sa ObjectLoadStream::loadObject()
	void saveObject(OvitoObject* object);

private:

	/// Contains all objects stored so far and their IDs.
	std::map<OvitoObject*, quint32> _objectMap;

	/// Contains all objects ordered by ID.
	QVector<OvitoObject*> _objects;

	/// The current dataset being saved.
	DataSet* _dataset;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_OBJECT_SAVESTREAM_H
