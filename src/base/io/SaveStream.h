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
 * \brief Contains the definition of the Ovito::Util::SaveStream class.
 */

#ifndef __OVITO_SAVESTREAM_H
#define __OVITO_SAVESTREAM_H

#include <base/Base.h>

namespace Ovito { namespace Util {

/**
 * \brief An output stream that is used to write binary data to a file in a platform-independent way.
 *
 * The SaveStream class is wrapper for a Qt \c QDataStream object that receives the binary data.
 * 
 * Various methods and operators can be used to store data types in the stream in a platform-independent
 * manner. The SaveStream class writes an header to the output data stream that contains
 * information about the platform architecture (32 or 64 bit), the floating-point precision (float or double)
 * and the endian type used by the application.
 * 
 * When reading the written data from the file the LoadStream class will take care of the data type conversion
 * needed to match the architecture present on the actual computer.
 * 
 * \sa LoadStream
 */
class OVITO_BASE_EXPORT SaveStream : public QObject
{
	Q_OBJECT

public:

	/// \brief Opens the stream for writing.
	/// \param destination The data stream to which the binary data is written. This must be 
	///                    stream that supports random access.
	/// \throw Exception when the given data stream does only support sequential access. 
	SaveStream(QDataStream& destination);

	/// \brief The destructor closes the stream.
	/// \sa close()
	virtual ~SaveStream() { close(); }

	/// \brief Closes the stream.
	/// \note The underlying data stream is not closed by this method.
	/// \sa isOpen()
	virtual void close();

	/// \brief Returns the open status of the save stream.
	/// \return \c true if the output stream is still open and ready for writing.
	/// \sa close()
	bool isOpen() const { return _isOpen; }

	/// \brief Writes an array of raw bytes to the output stream.
	/// \param buffer A pointer to the beginning of the data.
	/// \param numBytes The number of bytes to be written to the data.
	/// \note No data type conversion is done for the data written to the stream, i.e. the data will not 
	///       be platform independent.
	/// \throw Exception when the I/O error has occurred.
	void write(const void* buffer, size_t numBytes);

	/// \brief Start a new chunk with the given identifier.
	/// \param chunkId A identifier for this chunk. This identifier can be used
	///                to identify the type of data contained in the chunk during file loading.
	/// The chunk must be closed using endChunk().
	/// \throw Exception when the I/O error has occurred.
	/// \sa endChunk()
    void beginChunk(quint32 chunkId);
    
	/// \brief Closes the current chunk.
    /// 
    /// This method is used to close a chunk previously opened using beginChunk().
	/// \throw Exception when the I/O error has occurred.
    /// \sa beginChunk()
	void endChunk();

	/// \brief Returns the current writing position in the output file in bytes.
	/// \return The current writing position in the output stream.
	qint64 filePosition() { return _os.device()->pos(); }
	
	/// \brief Writes an enumeration value to the stream.
	/// \param enumValue A value from an enumeration defined with \c enum.
	/// \throw Exception when the I/O error has occurred.
	template<typename T>
	void writeEnum(T enumValue) { _os << (qint32)enumValue; }
	
	/// \brief Writes a platform-dependent unsigned integer value (32 or 64 bit, depending on your compiler settings) to the stream.
	/// \param value The value to write to the stream.
	/// \throw Exception when the I/O error has occurred.
	void writeSizeT(size_t value) { _os << (quint64)value; }

	/// \brief Writes a pointer to the stream.
	/// \param pointer The pointer to be written to the stream (can be a \c NULL pointer).
	///
	/// This method generates a unique ID for the given pointer that
	/// is written to the stream instead of the pointer itself.
	///
	/// \throw Exception when the I/O error has occurred.
	void writePointer(void* pointer);

	/// \brief Returns the ID for a pointer that was used to write the pointer to the stream.
	/// \param pointer A pointer.
	/// \return The ID for the given pointer or 0 if the given pointer hasn't been written to the stream yet.
	quint64 pointerID(void* pointer) const;
	
	/// \brief Accesses the internal data stream.
	/// \return The underlying data stream that is used as output stream.
	QDataStream& dataStream() { return _os; }

protected:

	/// Indicates the output stream is still open.
	bool _isOpen;

	/// The output stream.
	QDataStream& _os;

	/// The stack of open chunks.
	std::stack<qint64> _chunks;

	/// Maps pointers to IDs.
	std::map<void*, quint64> _pointerMap;
};

// These two template functions are necessary to handle the saving of enum types in the << operator.
namespace detail {

	/// This writes a non-enum value.
	template<typename T>
	void saveToSaveStream(SaveStream& stream, const T& v, const std::false_type&) { stream.dataStream() << v; }

	/// This writes an enum value.
	template<typename T>
	void saveToSaveStream(SaveStream& stream, const T& v, const std::true_type&) { stream.dataStream() << (qint32)v; }
};


/// \brief Writes an arbitrary value to a SaveStream.
///
/// \param stream The destination stream.
/// \param v The value to write to the stream.
/// \return The destination stream.
///
/// This operator just passes the data value on the output operator of the Qt \c QDataStream class.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const T& v)
{
	detail::saveToSaveStream(stream, v, std::is_enum<T>());
	return stream;
}

/// \brief Writes an array vector to a SaveStream.
///
/// \param stream The destination stream.
/// \param v The vector to write to the stream.
/// \return The destination stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const QVector<T>& v)
{	
	stream.writeSizeT(v.size());
	for(const auto& el : v)
		stream << el;
	return stream;
}

/// \brief Writes an array with a fixed number of values to the output stream.
///
/// \param stream The destination stream.
/// \param a The array to be written to the stream.
/// \return The destination stream.
template<typename T, std::size_t N>
inline SaveStream& operator<<(SaveStream& stream, const std::array<T, N>& a)
{
	for(typename std::array<T, N>::size_type i = 0; i < N; ++i)
		stream << a[i];
	return stream;
}

/// \brief Writes a floating-point value to a SaveStream.
///
/// \param stream The destination stream.
/// \param v The value to write to the stream.
/// \return The destination stream.
///
/// The floating-point value will be written to the stream using the
/// machine precision set when compiling the program. When loading the value
/// using LoadStream on another computer it will automatically be converted to the precision
/// used on that computer.
inline SaveStream& operator<<(SaveStream& stream, const FloatType& v)
{
	stream.dataStream() << v;
	return stream;
}

}}	// End of namespace

#endif // __OVITO_SAVESTREAM_H
