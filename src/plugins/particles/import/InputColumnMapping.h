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
#include <plugins/particles/import/ParticleImporter.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import)

/**
 * \brief Defines the mapping between one column of an particle input file and
 *        one of OVITO's particle properties.
 *
 * An InputColumnMapping is composed of a list of these structures, one for each
 * column in the input file.
 */
class OVITO_PARTICLES_EXPORT InputColumnInfo
{
public:

	/// \brief Constructor, which sets the column to an unmapped state.
	InputColumnInfo() : dataType(QMetaType::Void) {}

	/// \brief Maps this column to a custom particle property.
	/// \param propertyName The name of target particle property.
	/// \param dataType The data type of the property to create.
	/// \param vectorComponent The component of the per-particle vector.
	/// \param type The type of the ParticleProperty (should be ParticleProperty::UserProperty).
	void mapCustomColumn(const QString& propertyName, int dataType, int vectorComponent = 0, ParticleProperty::Type type = ParticleProperty::UserProperty) {
		this->property = ParticlePropertyReference(type, propertyName, vectorComponent);
		this->dataType = dataType;
	}

	/// \brief Maps this column to a standard particle property.
	/// \param type Specifies the standard property.
	/// \param vectorComponent The component in the per-particle vector.
	void mapStandardColumn(ParticleProperty::Type type, int vectorComponent = 0) {
		OVITO_ASSERT(type != ParticleProperty::UserProperty);
		this->property = ParticlePropertyReference(type, vectorComponent);
		this->dataType = ParticleProperty::standardPropertyDataType(type);
	}

	/// \brief Returns true if the file column is mapped to a particle property; false otherwise (file column will be ignored during import).
	bool isMapped() const { return dataType != QMetaType::Void; }

	/// The target particle property this column is mapped to.
	ParticlePropertyReference property;

	/// The data type of the particle property if this column is mapped to a user-defined property.
	/// This field can be set to QMetaType::Void to indicate that the column should be ignored during file import.
	int dataType;

	/// The name of the column in the input file. This information is
	/// read from the input file (if available).
	QString columnName;
};

/**
 * \brief Defines a mapping between the columns in a column-based input particle file
 *        and OVITO's internal particle properties.
 */
class OVITO_PARTICLES_EXPORT InputColumnMapping : public std::vector<InputColumnInfo>
{
public:

	/// \brief Saves the mapping to a stream.
	void saveToStream(SaveStream& stream) const;

	/// \brief Loads the mapping from a stream.
	void loadFromStream(LoadStream& stream);

	/// \brief Saves the mapping into a byte array.
	QByteArray toByteArray() const;

	/// \brief Loads the mapping from a byte array.
	void fromByteArray(const QByteArray& array);

	/// \brief Checks if the mapping is valid; throws an exception if not.
	void validate() const;

	/// \brief Returns the first few lines of the file, which can help the user to figure out
	///        the column mapping.
	const QString& fileExcerpt() const { return _fileExcerpt; }

	/// \brief Stores the first few lines of the file, which can help the user to figure out
	///        the column mapping.
	void setFileExcerpt(const QString& text) { _fileExcerpt = text; }

private:

	/// A string with the first few lines of the file, which is meant as a hint for the user to figure out
	/// the column mapping.
	QString _fileExcerpt;
};


/**
 * \brief Helper class that reads column-based data from an input file and
 *        stores the parsed values in particles properties according to an InputColumnMapping.
 */
class OVITO_PARTICLES_EXPORT InputColumnReader : public QObject
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
	InputColumnReader(const InputColumnMapping& mapping, ParticleFrameLoader& destination, size_t particleCount);

	/// \brief Parses the string tokens from one line of the input file and stores the values
	///        in the property objects.
	/// \param particleIndex The line index starting at 0 that specifies the particle whose properties
	///                  are read from the input file.
	/// \param dataLine The text line read from the input file containing the field values.
	void readParticle(size_t particleIndex, const char* dataLine);

	/// \brief Parses the string tokens from one line of the input file and stores the values
	///        in the property objects.
	/// \param particleIndex The line index starting at 0 that specifies the particle whose properties
	///                  are read from the input file.
	/// \param dataLine The text line read from the input file containing the field values.
	const char* readParticle(size_t particleIndex, const char* dataLine, const char* dataLineEnd);

	/// \brief Processes the values from one line of the input file and stores them in the particle properties.
	void readParticle(size_t particleIndex, const double* values, int nvalues);

	/// Returns whether particle types were specified in the file as strings instead of numeric IDs.
	bool usingNamedParticleTypes() const { return _usingNamedParticleTypes; }

private:

	/// Parse a single field from a text line.
	void parseField(size_t particleIndex, int columnIndex, const char* token, const char* token_end);

	/// Determines which input data columns are stored in what properties.
	InputColumnMapping _mapping;

	/// The data container.
	ParticleFrameLoader& _destination;

	struct TargetPropertyRecord {
		ParticleProperty* property;
		uint8_t* data;
		size_t stride;
		size_t count;
		int vectorComponent;
		bool isTypeProperty;
		bool isInt;
	};

	/// Stores the destination particle properties.
	QVector<TargetPropertyRecord> _properties;

	/// The Qt data type identifiers.
	int _intMetaTypeId, _floatMetaTypeId;

	/// Indicates that particle types were specified in the file as strings instead of numeric IDs.
	bool _usingNamedParticleTypes;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Particles::InputColumnInfo);
Q_DECLARE_METATYPE(Ovito::Particles::InputColumnMapping);

#endif // __OVITO_INPUT_COLUMN_MAPPING_H
