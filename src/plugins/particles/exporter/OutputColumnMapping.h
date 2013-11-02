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
#include <core/scene/pipeline/PipelineFlowState.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/ParticlePropertyObject.h>

namespace Particles {

/**
 * \brief This class defines how particle properties should be written to data columns in an output file.
 */
class OutputColumnMapping
{
public:

	/// \brief Returns the number of output columns.
	int columnCount() const { return _columns.size(); }

	/// \brief Inserts a column that will be written to the output file.
	/// \param columnIndex The column number starting at 0. This specifies the insertion position.
	/// \param propertyType The particle property to be written into the column.
	/// \param propertyName Specifies the name of particle property.
	/// \param vectorComponent The component of the vector if the property contains multiple values per particle.
	void insertColumn(int columnIndex, ParticleProperty::Type propertyType, const QString& propertyName, int vectorComponent = 0);

	/// \brief Removes the definition of a column.
	/// \param columnIndex The column number starting at 0.
	void removeColumn(int columnIndex);

	/// \brief Returns the property type that is associated with the given column of the output file.
	ParticleProperty::Type propertyType(int columnIndex) const { return (columnIndex < _columns.size()) ? _columns[columnIndex].propertyType : ParticleProperty::UserProperty; }

	/// \brief Returns the name of the particle property that is associated with the given column of the output file.
	QString propertyName(int columnIndex) const { return (columnIndex < _columns.size()) ? _columns[columnIndex].propertyName : QString(); }

	/// \brief Returns the vector component for particle properties that contain multiple values per atom.
	/// \return The non-negative vector component index.
	int vectorComponent(int columnIndex) const { return (columnIndex < _columns.size()) ? _columns[columnIndex].vectorComponent : 0; }

	/// \brief Saves the mapping to the given stream.
	void saveToStream(SaveStream& stream) const;

	/// \brief Loads the mapping from the given stream.
	void loadFromStream(LoadStream& stream);

	/// \brief Saves the mapping into a byte array.
	QByteArray toByteArray() const;

	/// \brief Loads the mapping from a byte array.
	void fromByteArray(const QByteArray& array);

private:

	/** Stores information about a single column in the output file. */
	struct Column {

		/// The particle property to be written to the output file.
		ParticleProperty::Type propertyType;

		/// The name of the particle property if this is a user-defined property.
		QString propertyName;

		/// The component for vector properties.
		int vectorComponent;
	};

	/// Contains one entry for each column of the output file.
	QVector<Column> _columns;
};

/**
 * \brief Writes the data columns to the output file as specified by a OutputColumnMapping.
 */
class OutputColumnWriter : public QObject
{
public:

	/// \brief Initializes the helper object.
	/// \param mapping The mapping between the particle properties
	///                and the columns in the output file.
	/// \param source The data source for the particle properties.
	/// \throws Exception if the mapping is not valid.
	///
	/// This constructor checks that all necessary particle properties referenced in the OutputColumnMapping
	/// are present in the source object.
	OutputColumnWriter(const OutputColumnMapping& mapping, const PipelineFlowState& source);

	/// \brief Writes the output line for a single particle to the output stream.
	/// \param particleIndex The index of the particle to write (starting at 0).
	/// \param stream An output text stream.
	///
	/// No newline character is written at the end of the line.
	void writeParticle(size_t particleIndex, QTextStream& stream);

private:

	/// Determines how particle properties are written to which data columns of the output file.
	const OutputColumnMapping& _mapping;

	/// The data source.
	const PipelineFlowState& _source;

	/// Stores the source particle properties for each column in the output file.
	/// If an entry is NULL then the particle index will be written to the corresponding column.
	QVector<ParticlePropertyObject*> _properties;

	/// Stores the source vector component for each output column.
	QVector<int> _vectorComponents;

	/// Internal buffer used for number -> string conversion.
	QByteArray _buffer;
};

};	// End of namespace

#endif // __OVITO_OUTPUT_COLUMN_MAPPING_H
