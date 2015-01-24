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

#include <core/Core.h>
#include "CompressedTextReader.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(IO)

/******************************************************************************
* Opens the stream for reading.
******************************************************************************/
CompressedTextReader::CompressedTextReader(QFileDevice& input, const QString& originalFilePath) :
	_device(input), _lineNumber(0), _byteOffset(0), _uncompressor(&input, 6, 0x100000), _mmapPointer(nullptr)
{
	// Try to find out what the filename is.
	if(originalFilePath.isEmpty() == false)
		_filename = QFileInfo(originalFilePath).fileName();
	else
		_filename = input.fileName();

	// Check if file is compressed (i.e. filename ends with .gz).
	if(_filename.endsWith(".gz", Qt::CaseInsensitive)) {
		// Open compressed file for reading.
		_uncompressor.setStreamFormat(QtIOCompressor::GzipFormat);
		if(!_uncompressor.open(QIODevice::ReadOnly))
			throw Exception(tr("Failed to open input file: %1").arg(_uncompressor.errorString()));
		_stream = &_uncompressor;
	}
	else {
		// Open uncompressed file for reading.
		if(!input.open(QIODevice::ReadOnly))
			throw Exception(tr("Failed to open input file: %1").arg(input.errorString()));
		_stream = &input;
	}
}

/******************************************************************************
* Reads in the next line.
******************************************************************************/
const char* CompressedTextReader::readLine(int maxSize) 
{
	_lineNumber++;

	if(_stream->atEnd())
		throw Exception(tr("File parsing error. Unexpected end of file after line %1.").arg(_lineNumber));

	qint64 readBytes = 0;
	if(!maxSize) {
		if(_line.size() <= 1) {
			_line.resize(1024);
		}
		readBytes = _stream->readLine(_line.data(), _line.size());

		if(readBytes == _line.size() - 1 && _line[readBytes - 1] != '\n') {
			qint64 readResult;
			do {
				_line.resize(_line.size() + 16384);
				readResult = _stream->readLine(_line.data() + readBytes, _line.size() - readBytes);
				if(readResult > 0 || readBytes == 0)
					readBytes += readResult;
			}
			while(readResult == Q_INT64_C(16384) && _line[readBytes - 1] != '\n');
		}
	}
	else {
        if(maxSize > (int)_line.size()) {
			_line.resize(maxSize + 1);
		}
		readBytes = _stream->readLine(_line.data(), _line.size());
	}

	if(readBytes <= 0)
		_line[0] = '\0';
	else {
		_line[readBytes] = '\0';
		_byteOffset += readBytes;
	}

	return _line.data();
}

/******************************************************************************
* Maps a part of the input file to memory.
******************************************************************************/
std::pair<const char*, const char*> CompressedTextReader::mmap(qint64 offset, qint64 size)
{
	OVITO_ASSERT(_mmapPointer == nullptr);
	if(isCompressed() == false)
		_mmapPointer = device().map(underlyingByteOffset(), size);
	return std::make_pair(
			reinterpret_cast<const char*>(_mmapPointer),
			reinterpret_cast<const char*>(_mmapPointer) + size);
}
	
/******************************************************************************
* Unmaps the file from memory.
******************************************************************************/
void CompressedTextReader::munmap()
{
	OVITO_ASSERT(_mmapPointer != nullptr);
	device().unmap(_mmapPointer);
	_mmapPointer = nullptr;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
