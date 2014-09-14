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
#include "CompressedTextWriterStream.h"

#include <boost/spirit/include/karma.hpp>

namespace Ovito {

/******************************************************************************
* Opens the output file for writing.
******************************************************************************/
CompressedTextWriterStream::CompressedTextWriterStream(QFileDevice& output) :
	_device(output), _compressor(&output)
{
	_filename = output.fileName();

	// Check if file should be compressed (i.e. filename ends with .gz).
	if(_filename.endsWith(".gz", Qt::CaseInsensitive)) {
		// Open file for writing.
		_compressor.setStreamFormat(QtIOCompressor::GzipFormat);
		if(!_compressor.open(QIODevice::WriteOnly))
			throw Exception(tr("Failed to open output file '%1' for writing: %2").arg(_compressor.errorString()));
		_stream = &_compressor;
	}
	else {
		// Open file for writing.
		if(!output.open(QIODevice::WriteOnly | QIODevice::Text))
			throw Exception(tr("Failed to open output file '%1' for writing: %2").arg(_filename).arg(output.errorString()));
		_stream = &output;
	}
}

/******************************************************************************
* Writes an integer number to the text-based output file.
******************************************************************************/
CompressedTextWriterStream& CompressedTextWriterStream::operator<<(qint32 i)
{
	using namespace boost::spirit;

	char buffer[16];
	char *s = buffer;
	karma::generate(s, karma::int_generator<qint32>(), i);
	OVITO_ASSERT(s - buffer < sizeof(buffer));
	if(_stream->write(buffer, s - buffer) == -1)
		reportWriteError();

	return *this;
}

/******************************************************************************
* Writes an integer number to the text-based output file.
******************************************************************************/
CompressedTextWriterStream& CompressedTextWriterStream::operator<<(quint32 i)
{
	using namespace boost::spirit;

	char buffer[16];
	char *s = buffer;
	karma::generate(s, karma::uint_generator<quint32>(), i);
	OVITO_ASSERT(s - buffer < sizeof(buffer));
	if(_stream->write(buffer, s - buffer) == -1)
		reportWriteError();

	return *this;
}

/******************************************************************************
* Writes an integer number to the text-based output file.
******************************************************************************/
CompressedTextWriterStream& CompressedTextWriterStream::operator<<(qint64 i)
{
	using namespace boost::spirit;

	char buffer[32];
	char *s = buffer;
	karma::generate(s, karma::int_generator<qint64>(), i);
	OVITO_ASSERT(s - buffer < sizeof(buffer));
	if(_stream->write(buffer, s - buffer) == -1)
		reportWriteError();

	return *this;
}

/******************************************************************************
* Writes an integer number to the text-based output file.
******************************************************************************/
CompressedTextWriterStream& CompressedTextWriterStream::operator<<(quint64 i)
{
	using namespace boost::spirit;

	char buffer[32];
	char *s = buffer;
	karma::generate(s, karma::uint_generator<quint64>(), i);
	OVITO_ASSERT(s - buffer < sizeof(buffer));
	if(_stream->write(buffer, s - buffer) == -1)
		reportWriteError();

	return *this;
}

#if !defined(Q_CC_GNU)
/******************************************************************************
* Writes an integer number to the text-based output file.
******************************************************************************/
CompressedTextWriterStream& CompressedTextWriterStream::operator<<(size_t i)
{
	using namespace boost::spirit;

	char buffer[32];
	char *s = buffer;
	karma::generate(s, karma::uint_generator<size_t>(), i);
	OVITO_ASSERT(s - buffer < sizeof(buffer));
	if(_stream->write(buffer, s - buffer) == -1)
		reportWriteError();

	return *this;
}
#endif

/******************************************************************************
* Writes an integer number to the text-based output file.
******************************************************************************/
CompressedTextWriterStream& CompressedTextWriterStream::operator<<(FloatType f)
{
	using namespace boost::spirit;

	// Define Boost Karma generator to control floating-point to string conversion.
	struct floattype_format_policy : karma::real_policies<FloatType> {
	    static unsigned precision(FloatType n) { return 10; }
	};
	static const karma::real_generator<FloatType, floattype_format_policy> floattype_generator;

	char buffer[32];
	char *s = buffer;
	karma::generate(s, floattype_generator, f);
	OVITO_ASSERT(s - buffer < sizeof(buffer));
	if(_stream->write(buffer, s - buffer) == -1)
		reportWriteError();

	return *this;
}

/******************************************************************************
* Throws an exception to report an I/O error.
******************************************************************************/
void CompressedTextWriterStream::reportWriteError()
{
	throw Exception(tr("Failed to write to output file '%1': %2").arg(filename()).arg(_stream->errorString()));
}


};
