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

#ifndef __OVITO_COMPRESSED_TEXT_WRITER_H
#define __OVITO_COMPRESSED_TEXT_WRITER_H

#include <core/Core.h>
#include <core/utilities/io/gzdevice/qtiocompressor.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(IO)

/**
 * \brief A helper class for writing text-based files that are compressed (gzip format).
 *
 * If the destination filename has a .gz suffix, this output stream class compresses the
 * text data on the fly if.
 *
 * \sa CompressedTextReader
 */
class OVITO_CORE_EXPORT CompressedTextWriter : public QObject
{
public:

	/// Opens the given output file device for writing. The output file's name determines if the data gets compressed.
	/// \param output The underlying Qt output device from to data should be written.
	/// \throw Exception if an I/O error has occurred.
	CompressedTextWriter(QFileDevice& output);

	/// Returns the name of the output file.
	const QString& filename() const { return _filename; }

	/// Returns the underlying I/O device.
	QFileDevice& device() { return _device; }

	/// Returns whether data written to this stream is being compressed.
	bool isCompressed() const { return _stream != &_device; }

	/// Writes an integer number to the text-based output file.
	CompressedTextWriter& operator<<(qint32 i);

	/// Writes an unsigned integer number to the text-based output file.
	CompressedTextWriter& operator<<(quint32 i);

	/// Writes a 64-bit integer number to the text-based output file.
	CompressedTextWriter& operator<<(qint64 i);

	/// Writes a 64-bit unsigned integer number to the text-based output file.
	CompressedTextWriter& operator<<(quint64 i);

#if !defined(Q_OS_WIN) && (QT_POINTER_SIZE != 4)
	/// Writes an unsigned integer number to the text-based output file.
	CompressedTextWriter& operator<<(size_t i);
#endif

	/// Writes a floating-point number to the text-based output file.
	CompressedTextWriter& operator<<(FloatType f);

	/// Writes a text string to the text-based output file.
	CompressedTextWriter& operator<<(const char* s) {
		if(_stream->write(s) == -1)
			reportWriteError();
		return *this;
	}

	/// Writes a single character to the text-based output file.
	CompressedTextWriter& operator<<(char c) {
		if(!_stream->putChar(c))
			reportWriteError();
		return *this;
	}

	/// Writes a Qt string string to the text-based output file.
	CompressedTextWriter& operator<<(const QString& s) { return *this << s.toLocal8Bit().constData(); }

private:

	/// Throws an exception to report an I/O error.
	void reportWriteError();

	/// The name of the output file (if known).
	QString _filename;

	/// The underlying output device.
	QFileDevice& _device;

	/// The compression filter.
	QtIOCompressor _compressor;

	/// The output stream.
	QIODevice* _stream;

	Q_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_COMPRESSED_TEXT_WRITER_H
