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

#ifndef __OVITO_INPUT_COLUMN_MAPPING_H
#define __OVITO_INPUT_COLUMN_MAPPING_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/importer/ParticleImporter.h>

namespace Particles {

/**
 * \brief Describes the mapping between data columns in a column-based input file
 *        and the internal particle properties.
 */
class OVITO_PARTICLES_EXPORT InputColumnMapping
{
public:

	/// \brief Returns the number of columns that have been mapped.
	int columnCount() const { return _columns.size(); }

	/// \brief Resizes the mapping array to include the specified number of file columns.
	void setColumnCount(int numberOfColumns, const QStringList& columnNames = QStringList());

	/// \brief Removes unmapped columns from the end of the list.
	void shrink() {
		while(columnCount() > 0 && dataType(columnCount() - 1) == QMetaType::Void)
			setColumnCount(columnCount() - 1);
	}

	/// \brief Map a column in the data file to a custom ParticleProperty.
	/// \param columnIndex The column number starting at 0.
	/// \param propertyName The name of ParticleProperty to be created.
	/// \param dataType The data type of the property to create.
	/// \param vectorComponent The component of the per-particle vector.
	/// \param property The type of the ParticleProperty (usually ParticleProperty::UserProperty).
	/// \param columnName Specifies the name of the column in the input file if known.
	void mapCustomColumn(int columnIndex, const QString& propertyName, int dataType, int vectorComponent = 0, ParticleProperty::Type property = ParticleProperty::UserProperty, const QString& columnName = QString());

	/// \brief Map a column in the data file to a standard ParticleProperty.
	/// \param columnIndex The column number starting at 0.
	/// \param property The type of the standard property.
	/// \param vectorComponent The component in the per-particle vector.
	/// \param columnName Specifies the name of the column in the input file if known.
	void mapStandardColumn(int columnIndex, ParticleProperty::Type property, int vectorComponent = 0, const QString& columnName = QString());

	/// \brief Ignores a column in the data file and removes any mapping to a particle property.
	/// \param columnIndex The column number starting at 0.
	/// \param columnName Assigns this name to the column.
	void unmapColumn(int columnIndex, const QString& columnName = QString());

	/// \brief Returns the assigned name of a column in the input file.
	/// \return The name of the column or an empty string if the input file does not contain column name information.
	QString columnName(int columnIndex) const { return (columnIndex < _columns.size()) ? _columns[columnIndex].columnName : QString(); }

	/// \brief Sets the assigned name of a column in the input file.
	void setColumnName(int columnIndex, const QString& name) { if(columnIndex < _columns.size()) _columns[columnIndex].columnName  = name; }

	/// \brief Resets the assigned column names.
	void resetColumnNames() {
		for(Column& col : _columns)
			col.columnName.clear();
	}

	/// \brief Returns the type of the ParticleProperty to which the given column of the input file has been mapped.
	ParticleProperty::Type propertyType(int columnIndex) const { return (columnIndex < _columns.size()) ? _columns[columnIndex].propertyType : ParticleProperty::UserProperty; }

	/// \brief Returns the name of the particle property to which the given column of the input file has been mapped.
	QString propertyName(int columnIndex) const { return (columnIndex < _columns.size()) ? _columns[columnIndex].propertyName : QString(); }

	/// \brief Returns the data type of the property to which the given column of the input file has been mapped.
	int dataType(int columnIndex) const { return (columnIndex < _columns.size()) ? _columns[columnIndex].dataType : QMetaType::Void; }

	/// \brief Returns true if the given file column is mapped to a particle property; false otherwise.
	bool isMapped(int columnIndex) const { return columnIndex < columnCount() && dataType(columnIndex) != QMetaType::Void; }

	/// \brief Returns the vector component for a column when it has been mapped to a vector particle property.
	int vectorComponent(int columnIndex) const { return (columnIndex < _columns.size()) ? _columns[columnIndex].vectorComponent : 0; }

	/// \brief Saves the mapping to a stream.
	void saveToStream(SaveStream& stream) const;

	/// \brief Loads the mapping from a stream.
	void loadFromStream(LoadStream& stream);

	/// \brief Saves the mapping into a byte array.
	QByteArray toByteArray() const;

	/// \brief Loads the mapping from a byte array.
	void fromByteArray(const QByteArray& array);

	/// \brief Checks if the mapping is valid; throws an exception if not.
	void validate();

private:

	/// Stores information about a single column in the data file.
	struct Column {

		/// The name of the column in the input file.
		QString columnName;

		/// The type of the particle property the column is mapped to.
		ParticleProperty::Type propertyType;

		/// The name of the particle property the column is mapped to.
		QString propertyName;

		/// The data type of the particle property if this is a user-defined property. If this is QMetaType::Void, the column will be ignored completely.
		int dataType;

		/// The component for vector properties.
		int vectorComponent;
	};

	/// Stores the mapping of each column in the input file.
	QVector<Column> _columns;
};


/**
 * \brief Helper class that parses the data columns in a input file and
 *        maps them to internal particles properties according to a InputColumnMapping.
 */
class InputColumnReader : public QObject
{
public:

	/// \brief Initializes the object.
	/// \param mapping Defines the mapping between the columns in the input file
	///        and the internal particle properties.
	/// \param destination The object where the read data will be stored in.
	/// \param particleCount The number of particles that will be read from the input file.
	/// \throws Exception if the mapping is not valid.
	///
	/// This constructor creates all necessary data channels in the destination object as defined
 	/// by the column to channel mapping.
	InputColumnReader(const InputColumnMapping& mapping, ParticleImportTask& destination, size_t particleCount);

	/// \brief Parses the string tokens from one line of the input file and stores the values
	///        in the property objects.
	/// \param particleIndex The line index starting at 0 that specifies the particle whose properties
	///                  are read from the input file.
	/// \param dataLine The text line read from the input file that contains the data field values.
	///                 The contents of this string may be destroyed by the parsing method.
	void readParticle(size_t particleIndex, char* dataLine);

	/// \brief Parses the string tokens from one line of the input file and stores the values
	///        in the property objects.
	/// \param particleIndex The line index starting at 0 that specifies the particle whose properties
	///                  are read from the input file.
	/// \param ntokens The number of tokens parsed from the input file line.
	/// \param tokens The list of parsed tokens.
	void readParticle(size_t particleIndex, int ntokens, const char* tokens[]);

private:

	/// Determines which input data columns are stored in what properties.
	InputColumnMapping _mapping;

	/// The data container.
	ParticleImportTask& _destination;

	/// Stores the destination particle properties.
	QVector<ParticleProperty*> _properties;

	/// The Qt data type identifiers.
	int _intMetaTypeId, _floatMetaTypeId;

	/// Array of pointers to the tokens in a text string.
	std::unique_ptr<const char*[]> _tokens;
};

};	// End of namespace

#endif // __OVITO_INPUT_COLUMN_MAPPING_H
