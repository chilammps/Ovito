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
 * \file LoadStream.h 
 * \brief Contains definition of the Ovito::LoadStream class.
 */

#ifndef __OVITO_LOADSTREAM_H
#define __OVITO_LOADSTREAM_H

#include <base/Base.h>  

namespace Ovito {

/**
 * \brief An input stream that is used to parse binary data from a file in a platform-independent way.
 *
 * The LoadStream class is wrapper for a Qt \c QDataStream object that provides the binary input data.
 * It is used to load data file that have previously been written using a SaveStream.
 * 
 * Various methods and operators can be used to load data types from the stream in a platform-independent
 * manner. When reading the written data from the file the LoadStream class will take care of the data type conversion
 * needed to match the architecture present on the actual computer.
 */
class LoadStream : public QObject
{
	Q_OBJECT

public:

	/// \brief Opens the stream for reading.
	/// \param source The data stream from which the binary data is read. This must be 
	///               stream that supports random access.
	/// \throw Exception when the given data stream \a source does only support sequential access. 	
	LoadStream(QDataStream& source);

	/// \brief The destructor closes the stream.
	/// \sa close()
	virtual ~LoadStream() { close(); }

	/// \brief Closes the stream.
	/// \note The underlying data stream is not closed by this method.
	/// \sa isOpen()
	virtual void close();

	/// \brief Returns the open status of the save stream.
	/// \return \c true if the output stream is still open and ready for writing.
	/// \sa close()
	bool isOpen() const { return _isOpen; }

	/// \brief Loads an array of raw bytes from the input stream.
	/// \param buffer A pointer to the beginning of the destination buffer.
	/// \param numBytes The number of bytes to be read from the input stream.
	/// \note No data type conversion is done for the data read from the stream, i.e. the data must be 
	///       platform-independent.
	/// \throw Exception when the I/O error has occurred.
	void read(void* buffer, size_t numBytes);

	/// \brief Opens the next chunk in the stream.
	/// \return The identifier of the opened chunk. This identifier can be used
	///         to identify the type of data contained in the chunk.
	/// \throw Exception when the I/O error has occurred.
	/// \sa closeChunk(), expectChunk()
	///
	/// The chunk must be closed using closeChunk().
	quint32 openChunk();
    
	/// \brief Opens the next chunk and throws an exception if the id doesn't match.
	/// \param chunkId The identifier of the expected chunk. If the actual chunk identifier does not match 
	///                this value then an exception is thrown.
	/// \throw Exception when the I/O error has occurred or the chunk identifier does not match \a chunkId.
	/// \sa closeChunk(), openChunk(), expectChunkRange()
	///
	/// The chunk must be closed using closeChunk().
	void expectChunk(quint32 chunkId);

	/// \brief Opens the next chunk and throws an exception if the id is not in a given range.
	/// \param chunkBaseId The base identifier of the expected chunk. If the actual chunk identifier is below this value then an exception is thrown.
	/// \param maxVersion The maximum file format version number for this chunk. If the actual chunk identifier minus the base identifier is above this value then an exception is thrown.
	/// \return The actual chunk id minus the base chunk id. This is the version number.
	/// \throw Exception when the I/O error has occurred or the chunk identifier is out of range.
	/// \sa closeChunk(), openChunk()
	///
	/// The chunk must be closed using closeChunk().
	quint32 expectChunkRange(quint32 chunkBaseId, quint32 maxVersion);

	/// \brief Closes the current chunk.
	///
    /// This method is used to close a chunk previously opened using openChunk() or expectChunk().
	/// \throw Exception when the I/O error has occurred.
    /// \sa openChunk()	
	void closeChunk();

	/// \brief Returns the current reading position in the input file in bytes.
	/// \return The current reading position in the input stream.
	qint64 filePosition() { return _is.device()->pos(); }

	/// \brief Changes the current stream position.
	/// \param pos The position in the input stream from the beginning. 
	/// \throw Exception when the I/O error has occurred.
	void setFilePosition(qint64 pos) {
		if(!_is.device()->seek(pos))
			throw Exception(tr("Failed to seek in input file."));
	}

	/// \brief Reads an enumeration value from the stream.
	/// \param[out] enumValue A value from an enumeration defined with \c enum that has been read from the stream.
	/// \throw Exception when the I/O error has occurred.
	template<typename T>
	void readEnum(T& enumValue) { qint32 v; _is >> v; enumValue = (T)v; }

	/// \brief Reads a platform dependent value (32 or 64 bit, depending on your compiler settings) from the stream.
	/// \param[out] value The value read from the stream.
	/// \throw Exception when the I/O error has occurred.
	void readSizeT(size_t& value) { quint64 temp; _is >> temp; value = (size_t)temp; }

	/// \brief Reads a pointer to an object of type \c T from the input stream.
	/// \param patchPointer The address of the pointer variable.
	/// 
	/// This method will patch the address immediately if it is available, 
	/// otherwise it will happen later when it is known.
	///
	/// \return The identifier of the pointer used in the file.
	quint64 readPointer(void** patchPointer);

	/// \brief The templated version of the function above.
	/// \param patchPointer The address of the pointer variable.
	template<typename T>
	quint64 readPointer(T** patchPointer) { return readPointer((void**)patchPointer); }

	/// \brief Resolves an ID with the real pointer.
	/// \param id The identifier of the pointer that should be associated with a real physical memory address.
	/// \param pointer The physical memory address.
	///
	/// This method will backpatch all registered pointers with the given \a id.
	void resolvePointer(quint64 id, void* pointer);

	/// \brief Returns the internal data stream.
	/// \return The underlying data stream that is used as input stream.
	QDataStream& dataStream() { return _is; }
	
	/// \brief Returns the floating-point precision used in the input file.
	/// \return The number of bytes used to represent one floating-point value. This is either
	///         4 for single precision (32 bit) or 8 for double precision (64 bit).
	quint32 floatingPointPrecision() const { return _numFPBits; }

	/// \brief Returns the file format version of the current file.
	quint32 formatVersion() const { return _fileFormat; }

protected:

	/// Indicates the input stream is still open.
	bool _isOpen;

	/// The internal input stream.
	QDataStream& _is;

	/// The version of the file format.
	quint32 _fileFormat;
	
	/// The floating-point precision.
	quint32 _numFPBits;

	/// The name of the application that wrote the current file.
	QString _applicationName;

	/// The major version number of the program that wrote the current file.
	quint32 _applicationMajorVersion;
	/// The minor version number of the program that wrote the current file.
	quint32 _applicationMinorVersion;
	/// The revision version number of the program that wrote the current file.
	quint32 _applicationRevisionVersion;

	/// The list of open chunks.
	std::vector<std::pair<int, qint64>> _chunks;

	/// Maps from IDs to real pointers.
	std::vector<void*> _pointerMap;

	/// Indicates for each pointer ID if it has been resolved to the real pointer.
	std::vector<bool> _resolvedPointers;

	/// Contains the pointers that have to be backpatched.
	std::multimap<quint64, void**> _backpatchPointers;
};

// These two template functions are necessary to handle the loading of enum types in the >> operator.
namespace detail {

	/// This reads a non-enum value.
	template<typename T>
	void loadFromLoadStream(Ovito::LoadStream& stream, T& v, const std::false_type&) { stream.dataStream() >> v; }

	/// This reads an enum value.
	template<typename T>
	void loadFromLoadStream(Ovito::LoadStream& stream, T& v, const std::true_type&) { stream.readEnum(v); }
};


/// \brief Reads an arbitrary value from a LoadStream.
///
/// \param stream The source stream.
/// \param v[out] The variable into which the parsed value will be stored.   
/// \return The source stream.
///
/// This operator just passes the reference on to the input operator of the Qt \c QDataStream class.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, T& v)
{
	OVITO_ASSERT(stream.isOpen());
	detail::loadFromLoadStream(stream, v, std::is_enum<T>());
	return stream;
}

/// \brief Loads an array vector with a variable number of values from the input stream.
///
/// \param stream The source stream.
/// \param a[out] The vector into which the data will be stored.
/// \return The source stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, QVector<T>& v)
{
	size_t n;
	stream.readSizeT(n);
	v.resize((int)n);
	for(auto& el : v)
		stream >> el;
	return stream;
}

/// \brief Loads an array with a fixed number of values from the input stream.
///
/// \param stream The source stream.
/// \param a[out] The array into which the data will be stored.
/// \return The source stream.
template<typename T, std::size_t N>
inline LoadStream& operator>>(LoadStream& stream, std::array<T, N>& a)
{
	for(size_t i = 0; i < N; ++i)
		stream >> a[i];
	return stream;
}

/// \brief Parses a floating-point value from a LoadStream.
///
/// \param stream The source stream.
/// \param v[out] The variable into which the parsed value will be stored.
/// \return The source stream.
///
/// The floating-point value will be read from the stream using the
/// lcal machine precision, independent of the
/// precision used for writing the value to the file.
inline LoadStream& operator>>(LoadStream& stream, FloatType& v)
{
	OVITO_ASSERT(stream.isOpen());
	stream.dataStream() >> v;
	return stream;
}

};	// End of namespace

#endif // __OVITO_LOADSTREAM_H
