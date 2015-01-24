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

#ifndef __OVITO_COMPRESSED_TEXT_READER_H
#define __OVITO_COMPRESSED_TEXT_READER_H

#include <core/Core.h>
#include <core/utilities/io/gzdevice/qtiocompressor.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(IO)

/**
 * \brief A helper class for reading text-based files, which may be compressed (gzip format).
 *
 * This stream class uncompresses data from the underlying I/O device on the fly if
 * the input filename has a .gz suffix. Otherwise it simply reads the original, uncompressed file contents.
 *
 * Call readLine() to read the next line of text from the file. The last line
 * read can be accessed via the line() method. The class keeps track of the
 * current line number, which is returned by lineNumber().
 *
 * \sa CompressedTextWriter
 */
class OVITO_CORE_EXPORT CompressedTextReader : public QObject
{
public:

	/// Opens the given I/O device for reading.
	/// \param input The underlying Qt input device from which data should be read.
	/// \param originalFilePath The file path of the file being read. This is used to determine if the file is compressed (ends with .gz suffix).
	/// \throw Exception if an I/O error has occurred.
	CompressedTextReader(QFileDevice& input, const QString& originalFilePath);

	/// Returns the name of the input file (without the path), which was passed to the constructor.
	const QString& filename() const { return _filename; }

	/// Returns the underlying I/O device.
	QFileDevice& device() { return _device; }

	/// Indicates whether the input file is compressed.
	bool isCompressed() const { return _stream != &_device; }

	/// Reads the next line of text from the input file.
	/// \throw Exception if an I/O error has occurred of if there is no more line to read.
	const char* readLine(int maxSize = 0);

	/// Checks whether the end of file has been reached. Do not call readLine() when this returns \c true.
	bool eof() const {
		return _stream->atEnd();
	}

	/// Returns the last line read via readLine().
	const char* line() const { return _line.data(); }

	/// Tests \c true if the last line read via readLine() begins with the given substring.
	bool lineStartsWith(const char* s) const {
		for(const char* l = line(); *s; ++s, ++l) {
			if(*l != *s) return false;
		}
		return true;
	}

	/// Returns the last line read via readLine() as a Qt string.
	QString lineString() const { return QString::fromLocal8Bit(_line.data()); }

	/// Returns the current line number.
	int lineNumber() const { return _lineNumber; }

	/// Returns the current read position in the (uncompressed) input stream.
	/// \sa underlyingByteOffset(), seek()
	qint64 byteOffset() const {
		return _byteOffset;
	}

	/// Jumps to the given byte position in the (uncompressed) input stream.
	/// \throw Exception if an I/O error has occurred.
	/// \sa byteOffset()
	void seek(qint64 pos) {
		if(!_stream->seek(pos))
			throw Exception(tr("Failed to seek to byte offset %1 in file %2: %3").arg(pos).arg(_filename).arg(_stream->errorString()));
		_byteOffset = pos;
	}

	/// Returns the current read position in the input file (which may be a compressed data stream).
	/// \sa byteOffset()
	qint64 underlyingByteOffset() const {
		return _device.pos();
	}

	/// Returns the size of the input file in bytes (the compressed size if file is gzipped).
	qint64 underlyingSize() const {
		return _device.size();
	}

	/// Maps the input file to memory, starting at the current offset and to end of the file.
	/// \throw Exception if an I/O error has occurred.
	std::pair<const char*, const char*> mmap() {
		return mmap(underlyingByteOffset(), underlyingSize() - underlyingByteOffset());
	}

	/// Maps a part of the input file to memory.
	/// \throw Exception if an I/O error has occurred.
	std::pair<const char*, const char*> mmap(qint64 offset, qint64 size);

	/// Unmaps the file from memory.
	/// \throw Exception if an I/O error has occurred.
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

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_COMPRESSED_TEXT_READER_H
