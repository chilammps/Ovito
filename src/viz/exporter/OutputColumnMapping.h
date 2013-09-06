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

#ifndef __OVITO_OUTPUT_COLUMN_MAPPING_H
#define __OVITO_OUTPUT_COLUMN_MAPPING_H

#include <core/Core.h>

namespace Viz {

/**
 * \brief This class defines how available particle properties should be written to data columns in an output file.
 */
class OutputColumnMapping
{
public:

	/// \brief Returns the number of output columns.
	int columnCount() const { return _columns.size(); }

	/// \brief Inserts a column that will be written to the output data file.
	/// \param columnIndex The column number starting at 0. This specifies the insertion position.
	/// \param channelId The identifier of the DataChannel to be associated with the column.
	/// \param channelName Specifies the name of DataChannel to be associated with the column..
	/// \param vectorComponent The component of the vector if the channel contains multiple values per atom.
	/// \sa removeColumn(), columnCount()
	void insertColumn(int columnIndex, DataChannel::DataChannelIdentifier channelId, const QString& channelName, size_t vectorComponent = 0);

	/// \brief Removes the definition of a column.
	/// \param columnIndex The column number starting at 0.
	/// \sa insertColumn(), columnCount()
	void removeColumn(int columnIndex);

	/// \brief Returns the identifier of the DataChannel that is associated with the given column of the output file.
	DataChannel::DataChannelIdentifier getChannelId(int columnIndex) const { return (columnIndex < columns.size()) ? columns[columnIndex].dataChannelId : DataChannel::UserDataChannel; }

	/// \brief Returns the name of the DataChannel that is associated with the given column of the output file.
	QString getChannelName(int columnIndex) const { return (columnIndex < columns.size()) ? columns[columnIndex].dataChannelName : QString(); }

	/// \brief Returns the vector component for a column when it is associated with a DataChannel that contains multiple values per atom.
	/// \return The non-negative vector component index.
	size_t getVectorComponent(int columnIndex) const { return (columnIndex < columns.size()) ? columns[columnIndex].vectorComponent : 0; }

	/// \brief Saves the mapping to the given stream.
	void saveToStream(SaveStream& stream) const;
	/// \brief Loads the mapping from the given stream.
	void loadFromStream(LoadStream& stream);

	/// \brief Saves the mapping into a byte array.
	QByteArray toByteArray() const;
	/// \brief Loads the mapping from a byte array.
	void fromByteArray(const QByteArray& array);

	/// \brief Saves the mapping the application's settings store.
	/// \param presetName The name under which the mapping is saved.
	/// \sa loadPreset(), listPresets()
	void savePreset(const QString& presetName) const;

	/// \brief Loads a mapping from the application's settings store.
	/// \param presetName The name of the mapping to load.
	/// \throws Exception if there is no preset with the given name.
	/// \sa savePreset(), listPresets()
	void loadPreset(const QString& presetName);

	/// \brief Returns a list of all presets found in the
	///        application's settings store.
	/// \sa loadPreset(), deletePreset()
	static QStringList listPresets();

	/// \brief Deletes a mapping from the application's settings store.
	/// \param presetName The name of the mapping to delete.
	/// \throws Exception if there is no preset with the given name.
	/// \sa savePreset(), loadPreset(), listPresets()
	static void deletePreset(const QString& presetName);

	/// Makes a copy of the mapping object.
	OutputColumnMapping& operator=(const OutputColumnMapping& other);

private:

	/** Stores information about a single column in the output file. */
	struct Column {
		DataChannel::DataChannelIdentifier dataChannelId;					///< The identifier of the corresponding DataChannel in the AtomsObject.
		QString dataChannelName;			///< The name of the DataChannel if it is a user-defined channel.
		size_t vectorComponent;				///< The vector component if the channel's data type is DataChannel::TypeVector3.
	};

	/// Contains one entry for each column in the data file.
	QVector<MapEntry> _columns;
};

/**
 * \brief Writes the data columns to the output file as specified by a OutputColumnMapping.
 */
class DataRecordWriterHelper : public QObject
{
public:
	/// \brief Initializes the helper object.
	/// \param mapping Defines the mapping between the data channels in the source AtomsObject
	///                the columns in the output file.
	/// \param source The helper object will retrieve the atomic data from this source object.
	/// \throws Exception if the mapping is not valid.
	///
	/// This constructor checks that all necessary data channels referenced in the OutputColumnMapping
	/// are present in the source AtomsObject.
	DataRecordWriterHelper(const OutputColumnMapping* mapping, AtomsObject* source);

	/// \brief Returns the number of actual columns that will be written to the
	///        output file by helper object.
	///
	/// The number of actual columns might be less than the number of columns defined in the
	/// OutputColumnMapping object since some of them can be empty.
	int actualColumnCount() const { return channels.size(); }

	/// \brief Writes the data record for a single atom to the output stream.
	/// \param atomIndex The index of the atom to write (starting at 0).
	/// \param stream An output text stream.
	///
	/// This methods writes all data fields of an atom as defined by the OutputColumnMapping to the
	/// output stream. The number of fields written is given by actualColumnCount() and
	/// each field is delimited by a single space character.
	/// No newline character is written at the end of the line.
	void writeAtom(int atomIndex, QIODevice& stream);

	/// \brief Writes the data record for a single atom to the output stream.
	/// \param atomIndex The index of the atom to write (starting at 0).
	/// \param stream An output text stream.
	///
	/// This methods writes all data fields of an atom as defined by the OutputColumnMapping to the
	/// output stream. The number of fields written is given by actualColumnCount() and
	/// each field is delimited by a single space character.
	/// No newline character is written at the end of the line.
	void writeAtom(int atomIndex, std::ostream& stream);

	/// \brief Stores the data channels values for one atom in the given buffer according to the OutputColumnMapping.
	/// \param atomIndex The index of the atom to write (starting at 0).
	/// \param buffer A pointer to a preallocated buffer that can hold at least as many \a double values as returned by actualColumnCount().
	///
	/// This methods stores actualColumnCount() values in the supplied buffer.
	void writeAtom(int atomIndex, double* buffer);

private:

	/// Determines which data channels are written to which data columns in the output file.
	const OutputColumnMapping* mapping;

	/// The source object.
	AtomsObject* source;

	/// Stores the source data channel for each column in the output file.
	/// If one entry in the vector is NULL then this is the special
	/// atom index column.
	QVector<DataChannel*> channels;

	/// Stores the sub-component for each data column if the DataChannel contains multiple values per atom.
	QVector<size_t> vectorComponents;

	/// Internal buffer used for number -> string conversion.
	QByteArray buffer;
};

};	// End of namespace

#endif // __OVITO_OUTPUT_COLUMN_MAPPING_H
