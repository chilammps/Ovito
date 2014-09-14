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
#include <base/io/gzdevice/qtiocompressor.h>

namespace Ovito {

/**
 * \brief A helper class that allows reading text-based files that may be compressed.
 *
 * The input file is uncompressed on the fly if it has a .gz name suffix.
 * Otherwise the text data is directly read from the underlying I/O device.
 *
 * The class provides functions to efficiently read one text line at a time.
 */
class OVITO_CORE_EXPORT CompressedTextParserStream : public QObject
{
public:

	/// Constructor that opens the given input device for reading.
	CompressedTextParserStream(QFileDevice& input, const QString& originalFilePath);

	/// Returns the name of the input file (if known).
	const QString& filename() const { return _filename; }

	/// Returns the underlying I/O device.
	QFileDevice& device() { return _device; }

	/// Returns whether data is being read from a compressed file.
	bool isCompressed() const { return _stream != &_device; }

	/// Reads in the next line.
	const char* readLine(int maxSize = 0);

	/// Checks whether the end of file is reached.
	bool eof() const {
		return _stream->atEnd();
	}

	/// Returns the last line read from the stream.
	const char* line() const { return _line.data(); }

	/// Returns true if the current line starts with the given string.
	bool lineStartsWith(const char* s) const {
		for(const char* l = line(); *s; ++s, ++l) {
			if(*l != *s) return false;
		}
		return true;
	}

	/// Returns the current line as a string.
	QString lineString() const { return QString::fromLocal8Bit(_line.data()); }

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

	/// Maps the input file to memory, starting at the current offset and to end of the file.
	std::pair<const char*, const char*> mmap() {
		return mmap(underlyingByteOffset(), underlyingSize() - underlyingByteOffset());
	}

	/// Maps a part of the input file to memory.
	std::pair<const char*, const char*> mmap(qint64 offset, qint64 size);

	/// Unmaps the file from memory.
	void munmap();

private:

	/// The name of the input file (if known).
	QString _filename;

	/// Buffer holding the current text line.
	std::vector<char> _line;

	/// The current line number.
	int _lineNumber;

	/// The current position in the uncompressed data stream.
	qint64 _byteOffset;

	/// The underlying input device.
	QFileDevice& _device;

	/// The uncompressing filter.
	QtIOCompressor _uncompressor;

	/// The input stream from which uncompressed data is read.
	QIODevice* _stream;

	/// The pointer to the memory-mapped data.
	uchar* _mmapPointer;

	Q_OBJECT
};

};	// End of namespace

#endif // __OVITO_COMPRESSED_TEXT_PARSER_STREAM_H
