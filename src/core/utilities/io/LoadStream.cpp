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
#include <core/utilities/io/LoadStream.h>
#include <core/utilities/Exception.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(IO)

using namespace std;

/******************************************************************************
* Opens the stream for reading.
******************************************************************************/
LoadStream::LoadStream(QDataStream& source) : _is(source), _isOpen(false)
{
	OVITO_ASSERT_MSG(!_is.device()->isSequential(), "LoadStream constructor", "LoadStream class requires a seekable input stream.");
    if(_is.device()->isSequential())
		throw Exception("LoadStream class requires a seekable input stream.");
    
	// Read file header. 

	_isOpen = true;

	// Check magic numbers.
	quint32 magic1, magic2;
	*this >> magic1 >> magic2;
	*this >> _fileFormat;
	*this >> _fpPrecision;
	
	_isOpen = false;

	if(magic1 != 0x0FACC5AB || magic2 != 0x0AFCCA5A)
		throw Exception(tr("Unknown file format. This is not a scene file written by %1.").arg(QCoreApplication::applicationName()));

	_is.setVersion(QDataStream::Qt_5_1);
	_is.setFloatingPointPrecision(_fpPrecision == 4 ? QDataStream::SinglePrecision : QDataStream::DoublePrecision);
	_isOpen = true;

	// Read application name.
	*this >> _applicationName;

	// Read application version.
	*this >> _applicationMajorVersion;
	*this >> _applicationMinorVersion;
	*this >> _applicationRevisionVersion;

	// Check file format version.
	if(_fileFormat > OVITO_FILE_FORMAT_VERSION)
		throw Exception(tr("Unsupported file format revision %1. This file has been written by %2 %3.%4.%5. Please upgrade to the newest program version to open this file.")
				.arg(_fileFormat).arg(_applicationName).arg(_applicationMajorVersion).arg(_applicationMinorVersion).arg(_applicationRevisionVersion));
}

/******************************************************************************
* Closes the stream.
******************************************************************************/
void LoadStream::close()
{
	if(isOpen()) {
		_isOpen = false;
		if(!_backpatchPointers.empty())
			throw Exception(tr("Deserialization error: Not all pointers in the input file have been resolved."));
	}
}

/******************************************************************************
* Loads an array of bytes from the input stream.
******************************************************************************/
void LoadStream::read(void* buffer, size_t numBytes)
{
	if(_is.device()->read((char*)buffer, numBytes) != numBytes) {
		if(_is.atEnd())
            throw Exception(tr("Unexpected end of file."));
		else
			throw Exception(tr("Failed to read data from input file. %1").arg(_is.device()->errorString()));
	}
	if(!_chunks.empty()) {
		qint64 chunkEnd = _chunks.back().second;
		OVITO_ASSERT_MSG(chunkEnd >= filePosition(), "LoadStream::read", "Tried to read past end of file chunk.");
		if(chunkEnd < filePosition())
			throw Exception(tr("Inconsistent file format."));
	}
}

/******************************************************************************
* Opens the next chunk in the stream.
******************************************************************************/
quint32 LoadStream::openChunk()
{    	
	quint32 chunkId;
	quint32 chunkSize; 
	*this >> chunkId >> chunkSize;
	_chunks.emplace_back(chunkId, (qint64)chunkSize + filePosition());
	return chunkId;
}

/******************************************************************************
* Opens the next chunk and throws an exception if the id doesn't match.
******************************************************************************/
void LoadStream::expectChunk(quint32 chunkId)
{
	quint32 cid = openChunk();
	if(cid != chunkId) {
        Exception ex(tr("Invalid file structure. This error might be caused by old files that are no longer supported by the current program version."));
        ex.appendDetailMessage(tr("Expected chunk ID %1 (0x%2) but found chunk ID %3 (0x%4).").arg(chunkId).arg(chunkId, 0, 16).arg(cid).arg(cid, 0, 16));
		throw ex;
	}
}

/******************************************************************************
* Opens the next chunk and throws an exception if the id is not in a given range.
******************************************************************************/
quint32 LoadStream::expectChunkRange(quint32 chunkBaseId, quint32 maxVersion)
{
	quint32 cid = openChunk();
	if(cid < chunkBaseId) {
        Exception ex(tr("Invalid file structure. This error might be caused by old files that are no longer supported by the current program version."));
        ex.appendDetailMessage(tr("Expected chunk ID range %1-%2 (0x%3-0x%4), but found chunk ID %5 (0x%6).").arg(chunkBaseId).arg(chunkBaseId, 0, 16).arg(chunkBaseId+maxVersion).arg(chunkBaseId+maxVersion, 0, 16).arg(cid).arg(cid, 0, 16));
		throw ex;
	}
	else if(cid > chunkBaseId + maxVersion) {
        Exception ex(tr("Unexpected chunk ID. This error might be caused by files that have been written by a newer program version."));
        ex.appendDetailMessage(tr("Expected chunk ID range %1-%2 (0x%3-0x%4), but found chunk ID %5 (0x%6).").arg(chunkBaseId).arg(chunkBaseId, 0, 16).arg(chunkBaseId+maxVersion).arg(chunkBaseId+maxVersion, 0, 16).arg(cid).arg(cid, 0, 16));
		throw ex;
	}
	else return cid - chunkBaseId;
}

/******************************************************************************
* Closes the current chunk.
******************************************************************************/
void LoadStream::closeChunk()
{
	OVITO_ASSERT(!_chunks.empty());
	qint64 chunkEnd = _chunks.back().second;
	OVITO_ASSERT_MSG(chunkEnd >= filePosition(), "LoadStream::closeChunk()", "Read past end of chunk.");
	qint64 currentPos = filePosition();
	if(currentPos > chunkEnd)
		throw Exception(tr("File parsing error: Read past end of chunk."));
	_chunks.pop_back();

    // Go to end of chunk
	if(currentPos != chunkEnd)
		setFilePosition(chunkEnd);

	// Check end code.
	quint32 code;
	*this >> code;
	if(code != 0x0FFFFFFF)
		throw Exception(tr("Inconsistent file structure."));
}

/******************************************************************************
* Reads a pointer to an object of type T from the input stream.
* This method will patch the address immediately if it is available, 
* otherwise it will happen later when it is known. 
******************************************************************************/
quint64 LoadStream::readPointer(void** patchPointer) 
{
	quint64 id;
	*this >> id;
	if(id == 0) { *patchPointer = NULL; }
	else {
		if(_pointerMap.size() > id && _resolvedPointers[id])
			*patchPointer = _pointerMap[id];
		else
			_backpatchPointers.insert(make_pair(id, patchPointer));
	}
	return id;
}

/******************************************************************************
* Resolves an ID with the real pointer.
* This method will backpatch all registered pointers with the given ID.
******************************************************************************/
void LoadStream::resolvePointer(quint64 id, void* pointer)
{	
	OVITO_ASSERT(id != 0);
	OVITO_ASSERT(id >= _resolvedPointers.size() || !_resolvedPointers[id]);
	if(id >= _pointerMap.size()) {
		_pointerMap.resize(id+1);
		_resolvedPointers.resize(id+1);
	}
	_pointerMap[id] = pointer;
	_resolvedPointers[id] = true;

	// Backpatch pointers.
	auto first = _backpatchPointers.find(id);
	if(first == _backpatchPointers.end()) return;
	auto last = first;
	for(; last != _backpatchPointers.end(); ++last) {
		if(last->first != id) break;
		*(last->second) = pointer;
	}

	_backpatchPointers.erase(first, last);
}

/******************************************************************************
* Checks the status of the underlying input stream and throws an exception
* if an error has occurred.
******************************************************************************/
void LoadStream::checkErrorCondition()
{
	if(dataStream().status() != QDataStream::Ok) {
		if(dataStream().status() == QDataStream::ReadPastEnd)
			throw Exception(tr("Unexpected end of file."));
		else if(dataStream().status() == QDataStream::ReadCorruptData)
			throw Exception(tr("File contains corrupted data."));
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

