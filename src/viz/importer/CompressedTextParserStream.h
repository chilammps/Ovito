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

#ifndef __OVITO_COMPRESSED_TEXT_PARSER_STREAM_H
#define __OVITO_COMPRESSED_TEXT_PARSER_STREAM_H

#include <core/Core.h>
#include <core/utilities/io/gzdevice/qtiocompressor.h>

namespace Viz {

/**
 * \brief A helper class that uncompresses gzipped text files on the fly.
 *
 * When opening the input file, it is uncompressed if it has a .gz suffix.
 * Otherwise the data is directly read from the underlying I/O device.
 */
class CompressedTextParserStream : public QObject
{
public:

	/// Constructor that opens the input stream.
	CompressedTextParserStream(QIODevice& input, const QString& originalFilePath) :
		_device(input), _lineNumber(0), /*_byteOffset(0),*/ _uncompressor(&input, 6, 0x100000)
	{
		// Try to find out what the filename is.
		if(originalFilePath.isEmpty() == false)
			_filename = QFileInfo(originalFilePath).fileName();
		else {
			QFileDevice* fileDevice = qobject_cast<QFileDevice*>(&input);
			if(fileDevice)
				_filename = fileDevice->fileName();
		}

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
			if(!input.open(QIODevice::ReadOnly | QIODevice::Text))
				throw Exception(tr("Failed to open input file: %1").arg(input.errorString()));
			_stream = &input;
		}
	}

	/// Returns the name of the input file (if known).
	const QString& filename() const { return _filename; }

	/// Reads in the next line.
	const QByteArray& readLine(qint64 maxSize = 0) {
		_lineNumber++;

		if(_stream->atEnd())
			throw Exception(tr("File parsing error. Unexpected end of file after line %1.").arg(_lineNumber));

		_line = _stream->readLine(maxSize);
		return _line;
	}

	/// Checks whether the end of file is reached.
	bool eof() {
		return _stream->atEnd();
	}

	/// Returns the last line read from the data stream.
	const QByteArray& line() const { return _line; }

	/// Returns the current line as a string.
	QString lineString() const { return QString::fromLocal8Bit(_line); }

	/// Returns the number of the current line.
	int lineNumber() const { return _lineNumber; }

	/// Returns the current position in the file.
	qint64 byteOffset() const {
		return _stream->pos();
	}

	/// Returns the current position in the compressed file if it is gzipped.
	qint64 underlyingByteOffset() const {
		return _device.pos();
	}

	/// Returns the size of the compressed file if it is gzipped.
	qint64 underlyingSize() const {
		return _device.size();
	}

	/// Jumps to the given position in the file.
	void seek(qint64 pos) {
		if(!_stream->seek(pos))
			throw Exception(tr("Failed to seek to byte offset %1 in file %2: %3").arg(pos).arg(_filename).arg(_stream->errorString()));
	}

private:

	/// The name of the input file (if known).
	QString _filename;

	/// The current line.
	QByteArray _line;

	/// The current line number.
	int _lineNumber;

	/// The current position in the uncompressed data stream.
	//qint64 _byteOffset;

	/// The underlying input device.
	QIODevice& _device;

	/// The uncompressing filter.
	QtIOCompressor _uncompressor;

	/// The input stream from which uncompressed data is read.
	QIODevice* _stream;

	/// The number of bytes for a line break;
	//int _lineBreakBytes;

	Q_OBJECT
};

};	// End of namespace

#endif // __OVITO_COMPRESSED_TEXT_PARSER_STREAM_H
