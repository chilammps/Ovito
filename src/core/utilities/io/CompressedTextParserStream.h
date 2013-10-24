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

namespace Ovito {

/**
 * \brief A helper class that uncompresses gzipped text files on the fly.
 *
 * When opening the input file, it is uncompressed if it has a .gz suffix.
 * Otherwise the data is directly read from the underlying I/O device.
 */
class OVITO_CORE_EXPORT CompressedTextParserStream : public QObject
{
public:

	/// Constructor that opens the input stream.
	CompressedTextParserStream(QIODevice& input, const QString& originalFilePath);

	/// Returns the name of the input file (if known).
	const QString& filename() const { return _filename; }

	/// Returns the underlying I/O device.
	QIODevice& device() { return _device; }

	/// Reads in the next line.
	const char* readLine(int maxSize = 0);

	/// Checks whether the end of file is reached.
	bool eof() const {
		return _stream->atEnd();
	}

	/// Returns the last line read from the data stream.
	const char* line() const { return _line.get(); }

	/// Returns true if the current line starts with the given string.
	bool lineStartsWith(const char* s) const {
		const char* l = line();
		while(*s) {
			if(*l != *s)
				return false;
			++s; ++l;
		}
		return true;
	}

	/// Returns the current line as a string.
	QString lineString() const { return QString::fromLocal8Bit(_line.get()); }

	/// Returns the number of the current line.
	int lineNumber() const { return _lineNumber; }

	/// Returns the current position in the file.
	qint64 byteOffset() const {
		return _byteOffset;
	}

	/// Jumps to the given position in the file.
	void seek(qint64 pos) {
		if(!_stream->seek(pos))
			throw Exception(tr("Failed to seek to byte offset %1 in file %2: %3").arg(pos).arg(_filename).arg(_stream->errorString()));
		_byteOffset = pos;
	}

	/// Returns the current position in the compressed file if it is gzipped.
	qint64 underlyingByteOffset() const {
		return _device.pos();
	}

	/// Returns the size of the compressed file if it is gzipped.
	qint64 underlyingSize() const {
		return _device.size();
	}

private:

	/// The name of the input file (if known).
	QString _filename;

	/// Buffer that holds the current line.
	std::unique_ptr<char[]> _line;

	/// The capacity of the line buffer.
	size_t _lineCapacity;

	/// The current line number.
	int _lineNumber;

	/// The current position in the uncompressed data stream.
	qint64 _byteOffset;

	/// The underlying input device.
	QIODevice& _device;

	/// The uncompressing filter.
	QtIOCompressor _uncompressor;

	/// The input stream from which uncompressed data is read.
	QIODevice* _stream;

	Q_OBJECT
};

};	// End of namespace

#endif // __OVITO_COMPRESSED_TEXT_PARSER_STREAM_H
