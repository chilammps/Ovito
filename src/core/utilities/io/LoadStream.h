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
 * \brief Contains the definition of the Ovito::IO::LoadStream class.
 */

#ifndef __OVITO_LOADSTREAM_H
#define __OVITO_LOADSTREAM_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(IO)

/**
 * \brief An input stream that reads binary data from a file in a platform-independent way.
 *
 * The LoadStream class is wrapper for a Qt \c QDataStream.
 * That that this class can only be used to deserialize data that was written by a SaveStream.
 *
 * \sa SaveStream
 */
class OVITO_CORE_EXPORT LoadStream : public QObject
{
	Q_OBJECT

public:

	/// \brief Opens the stream for reading.
	/// \param source The data stream from which the binary data will be read. This must be
	///               stream that supports random access.
	/// \throw Exception if the QDataStream supports only sequential access or if an I/O error occurs.
	LoadStream(QDataStream& source);

	/// Automatically calls close() to close the LoadStream.
	/// \sa close()
	virtual ~LoadStream() { close(); }

	/// Closes the LoadStream,  but not the underlying QDataStream that was passed to the constructor.
	/// \sa isOpen()
	virtual void close();

	/// \brief Returns the open status of the stream.
	/// \return \c true if the output stream is still open and ready for reading data.
	/// \sa close()
	bool isOpen() const { return _isOpen; }

	/// \brief Reads a sequence of raw bytes from the input stream.
	/// \param buffer A pointer to the beginning of the destination buffer.
	/// \param numBytes The number of bytes to be read from the input stream.
	/// \note No data type conversion is done for the data read from the stream.
	/// \throw Exception if an I/O error has occurred.
	void read(void* buffer, size_t numBytes);

	/// \brief Opens the next chunk in the input stream.
	/// \return The identifier of the chunk. This identifier can be used to interpret the data contained in the chunk.
	/// \throw Exception if an I/O error has occurred.
	/// \sa closeChunk(), expectChunk()
	quint32 openChunk();
    
	/// \brief Opens the next chunk and throws an exception if the chunk ID is wrong.
	/// \param chunkId The expected ID of the chunk. If the actual chunk ID does not match, an exception is thrown.
	/// \throw Exception if an I/O error has occurred or the chunk identifier does not match \a chunkId.
	/// \sa closeChunk(), openChunk(), expectChunkRange()
	void expectChunk(quint32 chunkId);

	/// \brief Opens the next chunk and throws an exception if the actual chunk ID is not in a expected range.
	/// \param chunkBaseId The base identifier of the expected chunk. If the actual chunk ID is lower, then an exception is thrown.
	/// \param maxVersion The maximum file format version number for this chunk. If the actual chunk ID minus the base ID is greater than this value, then an exception is thrown.
	/// \return The actual chunk ID minus the base chunk ID. This is the version number.
	/// \throw Exception if an I/O error has occurred or the chunk identifier is out of range.
	/// \sa closeChunk(), openChunk()
	quint32 expectChunkRange(quint32 chunkBaseId, quint32 maxVersion);

	/// \brief Closes the current chunk.
	///
    /// This method is used to close a chunk previously opened by openChunk(), expectChunk(), or expectChunkRange().
	/// \throw Exception if an I/O error has occurred.
	void closeChunk();

	/// \brief Returns the current reading position in the input stream in bytes.
	qint64 filePosition() { return _is.device()->pos(); }

	/// \brief Changes the current stream reading position.
	/// \param pos The position in the input stream (from the beginning of the file).
	/// \throw Exception if an I/O error has occurred.
	void setFilePosition(qint64 pos) {
		if(!_is.device()->seek(pos))
			throw Exception(tr("Failed to seek in input file."));
	}

	/// \brief Reads a platform dependent value (32 or 64 bit, depending on your compiler settings) from the stream.
	/// \param[out] value The value read from the stream.
	/// \throw Exception if an I/O error has occurred.
	void readSizeT(size_t& value) { quint64 temp; _is >> temp; value = (size_t)temp; checkErrorCondition(); }

	/// \brief Reads a pointer to an object of type \c T from the input stream.
	/// \param patchPointer The address of the pointer variable.
	/// 
	/// This method will patch the address immediately if it is available, 
	/// otherwise it will happen later when it is known.
	///
	/// \return The identifier of the pointer used in the file.
	/// \throw Exception if an I/O error has occurred.
	/// \sa SaveStream::writePointer()
	quint64 readPointer(void** patchPointer);

	/// \brief The templated version of the function above.
	/// \param patchPointer The address of the pointer variable.
	/// \throw Exception if an I/O error has occurred.
	/// \sa SaveStream::writePointer()
	template<typename T>
	quint64 readPointer(T** patchPointer) { return readPointer((void**)patchPointer); }

	/// \brief Resolves an ID with the real pointer.
	/// \param id The identifier of the pointer that should be associated with a real physical memory address.
	/// \param pointer The physical memory address.
	///
	/// This method will backpatch all registered pointers with the given \a id.
	void resolvePointer(quint64 id, void* pointer);

	/// \brief Returns the underlying Qt data stream.
	QDataStream& dataStream() { return _is; }
	
	/// \brief Returns the floating-point precision used in the input file.
	/// \return The number of bytes used to represent a single floating-point value. This is either
	///         4 for single precision (32 bit) or 8 for double precision (64 bit) numbers.
	quint32 floatingPointPrecision() const { return _fpPrecision; }

	/// \brief Returns the file format version of the current file.
	quint32 formatVersion() const { return _fileFormat; }

	/// \brief Returns the name of the application that wrote the current file.
	const QString& applicationName() const { return _applicationName; }

	/// \brief Returns the major version number of the program that wrote the file.
	quint32 applicationMajorVersion() const { return _applicationMajorVersion; }

	/// \brief Returns the minor version number of the program that wrote the current file.
	quint32 applicationMinorVersion() const { return _applicationMinorVersion; }

	/// \brief Returns the revision version number of the program that wrote the current file.
	quint32 applicationRevisionVersion() const { return _applicationRevisionVersion; }

private:

	/// Checks the status of the underlying input stream and throws an exception if an error has occurred.
	void checkErrorCondition();

	// This reads a non-enum value.
	template<typename T>
	void loadValue(T& v, const std::false_type&) {
		dataStream() >> v;
		checkErrorCondition();
	}

	// This reads an enum value.
	template<typename T>
	void loadValue(T& enumValue, const std::true_type&) {
		qint32 v;
		dataStream() >> v;
		enumValue = (T)v;
		checkErrorCondition();
	}

	template<typename T> friend	LoadStream& operator>>(LoadStream& stream, T& v);

private:

	/// Indicates the input stream is still open.
	bool _isOpen;

	/// The internal input stream.
	QDataStream& _is;

	/// The version of the file format.
	quint32 _fileFormat;
	
	/// The floating-point precision (4 or 8 bytes).
	quint32 _fpPrecision;

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


/// \brief Reads a value from a LoadStream.
/// \relates LoadStream
///
/// \param stream The source stream.
/// \param v The variable which will receive the loaded value. This must be a value type supported by the QDataStream class of Qt.
/// \return The source stream.
/// \throw Exception if an I/O error has occurred.
///
/// This operator calls the underlying Qt \c QDataStream class to read the value.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, T& v)
{
	OVITO_ASSERT(stream.isOpen());
	stream.loadValue(v, std::is_enum<T>());
	return stream;
}

/// \brief Reads a vector with a variable number of values from the input stream.
/// \relates LoadStream
///
/// \param stream The source stream.
/// \param a The vector the will receive the loaded data.
/// \return The source stream.
/// \throw Exception if an I/O error has occurred.
///
/// The vector will automatically be resized to the number of elements stored in the stream.
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

/// \brief Reads an array with a fixed number of values from the input stream.
/// \relates LoadStream
///
/// \param stream The source stream.
/// \param a The array into which the data will be stored.
/// \return The source stream.
/// \throw Exception if an I/O error has occurred.
///
/// \note The destination array's size must exactly match the size of the array read from the file.
template<typename T, std::size_t N>
inline LoadStream& operator>>(LoadStream& stream, std::array<T, N>& a)
{
	for(typename std::array<T, N>::size_type i = 0; i < N; ++i)
		stream >> a[i];
	return stream;
}

/// \brief Reads a floating-point value from the input stream.
/// \relates LoadStream
///
/// \param stream The source stream.
/// \param v The variable which receives the loaded value.
/// \return The source stream.
/// \throw Exception if an I/O error has occurred.
inline LoadStream& operator>>(LoadStream& stream, FloatType& v)
{
	OVITO_ASSERT(stream.isOpen());
	stream.dataStream() >> v;
	return stream;
}

/// \brief Reads a Qt flag from the input stream.
/// \relates LoadStream
///
/// \param stream The source stream.
/// \param v The variable which receives the loaded value.
/// \return The source stream.
/// \throw Exception if an I/O error has occurred.
template<typename Enum>
inline LoadStream& operator>>(LoadStream& stream, QFlags<Enum>& v)
{
	typename QFlags<Enum>::enum_type e;
	stream >> e;
	v = e;
	return stream;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_LOADSTREAM_H
