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
#include "CompressedTextParserStream.h"

namespace Ovito {

/******************************************************************************
* Opens the stream for reading.
******************************************************************************/
CompressedTextParserStream::CompressedTextParserStream(QFileDevice& input, const QString& originalFilePath) :
	_device(input), _lineNumber(0), _byteOffset(0), _uncompressor(&input, 6, 0x100000),
	_lineCapacity(0)
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
const char* CompressedTextParserStream::readLine(int maxSize) 
{
	_lineNumber++;

	if(_stream->atEnd())
		throw Exception(tr("File parsing error. Unexpected end of file after line %1.").arg(_lineNumber));

	qint64 readBytes = 0;
	if(!maxSize) {
		if(_lineCapacity <= 1) {
			_line.reset(new char[2]);
			_lineCapacity = 2;
		}
		readBytes = _stream->readLine(_line.get(), _lineCapacity);

		if(readBytes == _lineCapacity - 1 && _line[readBytes - 1] != '\n') {
			qint64 readResult;
			do {
				size_t newCapacity = _lineCapacity + 16384;
				std::unique_ptr<char[]> newBuffer(new char[newCapacity]);
				memcpy(newBuffer.get(), _line.get(), _lineCapacity);
				_line.reset(newBuffer.release());
				_lineCapacity = newCapacity;
				readResult = _stream->readLine(_line.get() + readBytes, _lineCapacity - readBytes);
				if(readResult > 0 || readBytes == 0)
					readBytes += readResult;
			}
			while(readResult == Q_INT64_C(16384) && _line[readBytes - 1] != '\n');
		}
	}
	else {
		if(maxSize > _lineCapacity) {
			_lineCapacity = maxSize + 1;
			_line.reset(new char[_lineCapacity]);
		}
		OVITO_ASSERT(_line.get() != nullptr);
		readBytes = _stream->readLine(_line.get(), _lineCapacity);
	}

	OVITO_ASSERT(_line.get() != nullptr);
	if(readBytes <= 0)
		_line[0] = '\0';
	else {
		_line[readBytes] = '\0';
		_byteOffset += readBytes;
	}

	return _line.get();
}
	
};
