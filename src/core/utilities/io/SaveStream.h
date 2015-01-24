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

/** 
 * \file
 * \brief Contains the definition of the Ovito::IO::SaveStream class.
 */

#ifndef __OVITO_SAVESTREAM_H
#define __OVITO_SAVESTREAM_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(IO)

/**
 * \brief An output stream class that writes binary data to a file in a platform-independent way.
 *
 * The SaveStream class is wrapper for a Qt \c QDataStream object that receives the binary data after conversion to a platform-independent representation.
 * The SaveStream class writes a file header to the output data stream that contains
 * meta information about the platform architecture (32 or 64 bit memory model), the default floating-point precision (single or double)
 * and the endian type used by the application. When reading the data on a different computer, the corresponding LoadStream class
 * will take care of converting the stored data to the architecture-dependent representation used by the local computer.
 *
 * The SaveStream class allows to structure the data using chunks. A new data chunk can be created using
 * the beginChunk() method. Any data subsequently written to the stream before a closing call to endChunk() will
 * become part of the current chunk. Chunks can be nested by calling beginChunk() multiple times
 * before calling endChunk() an equal number of times.
 * 
 * Each chunk possesses an integer ID, which is specified on the call to beginChunk(). The ID is stored in the
 * output file and can be used as meta information to describe the contents of the chunk.
 * The interpretation of the chunk ID is completely left to the user of the class.
 *
 * The writePointer() method allows to serialize C++ pointers. The SaveStream class generates a unique
 * ID for each unique pointer written to the stream using this method. On subsequent calls to writePointer()
 * with the same C++ pointer, the same ID will be written to the stream.
 *
 * \sa LoadStream
 */
class OVITO_CORE_EXPORT SaveStream : public QObject
{
	Q_OBJECT

public:

	/// \brief Constructs the stream wrapper.
	/// \param destination The sink that will receive the data. This Qt output stream must support random access.
	/// \throw Exception if the given data stream supports only sequential access or if an I/O error occured while writing the file header.
	SaveStream(QDataStream& destination);

	/// \brief Automatically closes the stream by calling close().
	virtual ~SaveStream() { close(); }

	/// \brief Closes this stream, but not the underlying output stream passed to the constructor.
	/// \throw Exception if an I/O error has occurred.
	/// \sa isOpen()
	virtual void close();

	/// \brief Returns whether the stream is still open for write operations. Returns \c false after close() has been called.
	/// \sa close()
	bool isOpen() const { return _isOpen; }

	/// \brief Writes an array of raw bytes to the output stream.
	/// \param buffer A pointer to the beginning of the data.
	/// \param numBytes The number of bytes to be written.
	/// \note No conversion is done for the data written to the stream, i.e. the data will not be stored in a platform-independent format.
	/// \throw Exception if an I/O error has occurred.
	void write(const void* buffer, size_t numBytes);

	/// \brief Start a new chunk with the given identifier.
	/// \param chunkId A identifier for this chunk. This identifier can be used
	///                to identify the type of data contained in the chunk during file loading.
	/// The chunk must be closed using endChunk().
	/// \throw Exception if an I/O error has occurred.
	/// \sa endChunk()
    void beginChunk(quint32 chunkId);
    
	/// \brief Closes the current chunk.
    /// 
    /// This method closes the last chunk previously opened using beginChunk().
	/// \throw Exception if an I/O error has occurred.
    /// \sa beginChunk()
	void endChunk();

	/// \brief Returns the current writing position of the underlying output stream in bytes.
	qint64 filePosition() const { return _os.device()->pos(); }
	
	/// \brief Writes a platform-dependent unsigned integer value (can be 32 or 64 bits) to the stream.
	/// \param value The value to be written to the stream.
	/// \throw Exception when the I/O error has occurred.
	void writeSizeT(size_t value) { _os << (quint64)value; }

	/// \brief Writes a pointer to the stream.
	/// \param pointer The pointer to be written to the stream (can be \c nullptr).
	///
	/// This method generates a unique ID for the pointer and writes the ID to the stream
	/// instead of the pointer itself.
	///
	/// \throw Exception if an I/O error has occurred.
	/// \sa pointerID()
	/// \sa LoadStream::readPointer()
	void writePointer(void* pointer);

	/// \brief Returns the ID for a pointer that was previously written to the stream using writePointer().
	/// \param pointer A pointer.
	/// \return The ID the given pointer was mapped to, or 0 if the pointer hasn't been written to the stream yet.
	/// \sa writePointer()
	quint64 pointerID(void* pointer) const;
	
	/// Provides direct access to the underlying Qt data stream.
	QDataStream& dataStream() { return _os; }

private:

	/// Checks the status of the underlying output stream and throws an exception if an error has occurred.
	void checkErrorCondition();

	/// Writes a C++ enum to the stream.
	template<typename T>
	void writeValue(T enumValue, const std::true_type&) {
		dataStream() << (qint32)enumValue;
		checkErrorCondition();
	}

	/// Writes a non-enum to the stream.
	template<typename T>
	void writeValue(T v, const std::false_type&) {
		dataStream() << v;
		checkErrorCondition();
	}

	template<typename T> friend	SaveStream& operator<<(SaveStream& stream, const T& v);

private:

	/// Indicates the output stream is still open.
	bool _isOpen;

	/// The output stream.
	QDataStream& _os;

	/// The stack of open chunks.
	std::stack<qint64> _chunks;

	/// Maps pointers to IDs.
	std::map<void*, quint64> _pointerMap;
};

/// \brief Writes a value to a SaveStream.
/// \relates SaveStream
///
/// \param stream The destination stream.
/// \param v The value to write to the stream. This must be a value type supported by the QDataStream class of Qt.
/// \return The destination stream.
/// \throw Exception if an I/O error has occurred.
///
/// This operator forwards the value to the output operator of the underlying Qt \c QDataStream class.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const T& v)
{
	stream.writeValue(v, std::is_enum<T>());
	return stream;
}

/// \brief Writes a vector container to a SaveStream.
/// \relates SaveStream
///
/// \param stream The destination stream.
/// \param v The vector to write to the stream.
/// \return The destination stream.
/// \throw Exception if an I/O error has occurred.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const QVector<T>& v)
{	
	stream.writeSizeT(v.size());
	for(const auto& el : v)
		stream << el;
	return stream;
}

/// \brief Writes an array of values to the output stream.
/// \relates SaveStream
///
/// \param stream The destination stream.
/// \param a The array to be written to the stream.
/// \return The destination stream.
/// \throw Exception if an I/O error has occurred.
template<typename T, std::size_t N>
inline SaveStream& operator<<(SaveStream& stream, const std::array<T, N>& a)
{
	for(typename std::array<T, N>::size_type i = 0; i < N; ++i)
		stream << a[i];
	return stream;
}

/// \brief Writes Qt flag to the output stream.
/// \relates SaveStream
///
/// \param stream The destination stream.
/// \param a The flag value
/// \return The destination stream.
/// \throw Exception if an I/O error has occurred.
template<typename Enum>
inline SaveStream& operator<<(SaveStream& stream, const QFlags<Enum>& a)
{
	return stream << (typename QFlags<Enum>::enum_type)(typename QFlags<Enum>::Int)a;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_SAVESTREAM_H
