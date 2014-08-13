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

#include <plugins/particles/Particles.h>
#include "InputColumnMapping.h"

namespace Particles {

#if 0
/******************************************************************************
 * Map a column in the data file to a custom ParticleProperty.
 *****************************************************************************/
void InputColumnMapping::mapCustomColumn(int columnIndex, const QString& propertyName, int dataType, int vectorComponent, ParticleProperty::Type property, const QString& columnName)
{
	OVITO_ASSERT(columnIndex >= 0);

	// Expand column array if necessary and initialize all new columns to their default values.
	if(columnIndex >= columnCount())
		setColumnCount(columnIndex + 1);

	_columns[columnIndex].propertyType = property;
	_columns[columnIndex].propertyName = propertyName;
	_columns[columnIndex].columnName = columnName;
	_columns[columnIndex].dataType = dataType;
	_columns[columnIndex].vectorComponent = std::max(0, vectorComponent);
}

/******************************************************************************
 * Map a column in the data file to a standard ParticleProperty.
 *****************************************************************************/
void InputColumnMapping::mapStandardColumn(int columnIndex, ParticleProperty::Type property, int vectorComponent, const QString& columnName)
{
	mapCustomColumn(columnIndex, ParticleProperty::standardPropertyName(property), ParticleProperty::standardPropertyDataType(property), vectorComponent, property, columnName);
}

/******************************************************************************
 * Ignores a column in the data file and removes any mapping to a particle property.
 *****************************************************************************/
void InputColumnMapping::unmapColumn(int columnIndex, const QString& columnName)
{
	OVITO_ASSERT(columnIndex >= 0);

	if(columnIndex < columnCount()) {
		_columns[columnIndex].propertyType = ParticleProperty::UserProperty;
		_columns[columnIndex].propertyName.clear();
		_columns[columnIndex].columnName = columnName;
		_columns[columnIndex].dataType = QMetaType::Void;
		_columns[columnIndex].vectorComponent = 0;
	}
	else {
		// Expand column array if necessary and initialize all new columns to their default values.
		setColumnCount(columnIndex + 1);
		_columns[columnIndex].columnName = columnName;
	}
}
#endif
/******************************************************************************
 * Saves the mapping to the given stream.
 *****************************************************************************/
void InputColumnMapping::saveToStream(SaveStream& stream) const
{
	stream.beginChunk(0x01);
	stream << (int)size();
	for(const InputColumnInfo& col : *this) {
		stream << col.columnName;
		stream.writeEnum(col.property.type());
		stream << col.property.name();
		stream.writeEnum(col.dataType);
		stream << col.property.vectorComponent();
	}
	stream.endChunk();
}

/******************************************************************************
 * Loads the mapping from the given stream.
 *****************************************************************************/
void InputColumnMapping::loadFromStream(LoadStream& stream)
{
	stream.expectChunk(0x01);
	int numColumns;
	stream >> numColumns;
	resize(numColumns);
	for(InputColumnInfo& col : *this) {
		stream >> col.columnName;
		ParticleProperty::Type propertyType;
		stream.readEnum(propertyType);
		QString propertyName;
		stream >> propertyName;
		stream.readEnum(col.dataType);
		if(col.dataType == qMetaTypeId<float>() || col.dataType == qMetaTypeId<double>())
			col.dataType = qMetaTypeId<FloatType>();
		int vectorComponent;
		stream >> vectorComponent;
		col.property = ParticlePropertyReference(propertyType, propertyName, vectorComponent);
	}
	stream.closeChunk();
}

/******************************************************************************
 * Saves the mapping into a byte array.
 *****************************************************************************/
QByteArray InputColumnMapping::toByteArray() const
{
	QByteArray buffer;
	QDataStream dstream(&buffer, QIODevice::WriteOnly);
	SaveStream stream(dstream);
	saveToStream(stream);
	stream.close();
	return buffer;
}

/******************************************************************************
 * Loads the mapping from a byte array.
 *****************************************************************************/
void InputColumnMapping::fromByteArray(const QByteArray& array)
{
	QDataStream dstream(array);
	LoadStream stream(dstream);
	loadFromStream(stream);
	stream.close();
}

/******************************************************************************
 * Checks if the mapping is valid; throws an exception if not.
 *****************************************************************************/
void InputColumnMapping::validate() const
{
	// Make sure that at least the particle positions are read from the input file.
	if(std::none_of(begin(), end(), [](const InputColumnInfo& column) { return column.property.type() == ParticleProperty::PositionProperty; }))
		throw Exception(InputColumnReader::tr("No file column has been mapped to the particle position property."));
}

/******************************************************************************
 * Initializes the object.
 *****************************************************************************/
InputColumnReader::InputColumnReader(const InputColumnMapping& mapping, ParticleImportTask& destination, size_t particleCount)
	: _mapping(mapping), _destination(destination),
	  _intMetaTypeId(qMetaTypeId<int>()), _floatMetaTypeId(qMetaTypeId<FloatType>()),
	  _usingNamedParticleTypes(false)
{
	mapping.validate();

	// Create particle properties as defined by the mapping.
	for(int i = 0; i < mapping.size(); i++) {

		ParticleProperty* property = nullptr;
		const ParticlePropertyReference& pref = mapping[i].property;

		int vectorComponent = std::max(0, pref.vectorComponent());
		int dataType = mapping[i].dataType;

		if(dataType != QMetaType::Void) {
			size_t dataTypeSize;
			if(dataType == qMetaTypeId<int>())
				dataTypeSize = sizeof(int);
			else if(dataType == qMetaTypeId<FloatType>())
				dataTypeSize = sizeof(FloatType);
			else
				throw Exception(tr("Invalid custom particle property (data type %1) for input file column %2").arg(dataType).arg(i+1));

			if(pref.type() != ParticleProperty::UserProperty) {
				// Look for existing standard property.
				for(const auto& p : destination.particleProperties()) {
					if(p->type() == pref.type()) {
						property = p.get();
						break;
					}
				}
				if(!property) {
					// Create standard property.
					property = new ParticleProperty(particleCount, pref.type());
					destination.addParticleProperty(property);
				}
			}
			else {
				// Look for existing user-defined property with the same name.
                ParticleProperty* oldProperty = nullptr;
                int oldPropertyIndex = -1;
				for(int j = 0; j < destination.particleProperties().size(); j++) {
					const auto& p = destination.particleProperties()[j];
					if(p->name() == pref.name()) {
						if(p->dataType() == dataType && (int)p->componentCount() > vectorComponent) {
							property = p.get();
                        }
						else {
                            oldProperty = p.get();
                            oldPropertyIndex = j;
                        }
						break;
					}
				}
				if(!property) {
					// Create a new user-defined property for the column.
					property = new ParticleProperty(particleCount, dataType, dataTypeSize, vectorComponent + 1, pref.name());
					destination.addParticleProperty(property);
					if(oldProperty) {
						// We need to replace all old properties with (lower vector component count) with this one.
						int indexOfOldProperty = _properties.indexOf(oldProperty);
						while(indexOfOldProperty != -1) {
							_properties.replace(indexOfOldProperty, property);
							indexOfOldProperty = _properties.indexOf(oldProperty);
						}
						// Remove here and not above because it is auto-released.
						destination.removeParticleProperty(oldPropertyIndex);
					}
				}
			}
			if(property)
				property->setName(pref.name());

			OVITO_ASSERT(vectorComponent < (int)property->componentCount());
		}

		// Build list of property objects for fast look up during parsing.
		_properties.push_back(property);
	}
}

/******************************************************************************
 * Parses the string tokens from one line of the input file and stores the values
 * in the data channels of the destination AtomsObject.
 *****************************************************************************/
void InputColumnReader::readParticle(size_t particleIndex, char* dataLine)
{
	if(!_tokens)
		_tokens.reset(new const char*[_properties.size()]);

	// Divide string into tokens.
	int ntokens = 0;
	while(ntokens < _properties.size()) {
		while(*dataLine == ' ' || *dataLine == '\t')
			++dataLine;
		_tokens[ntokens] = dataLine;
		while(*dataLine > ' ')
			++dataLine;
		if(*dataLine == '\n' || *dataLine == '\r') *dataLine = '\0';
		if(dataLine != _tokens[ntokens]) ntokens++;
		if(*dataLine == '\0') break;
		*dataLine = '\0';
		dataLine++;
	}

	readParticle(particleIndex, ntokens, _tokens.get());
}


/******************************************************************************
 * Helper function that converts a string to a floating-point number.
 *****************************************************************************/
inline bool parseFloatType(const char* s, float& f)
{
	// First use the atof() function to parse the number because it's fast.
	// However, atof() returns 0.0 to report a parsing error. Thus, if
	// we get 0.0, we need to use the slower strtof() function as a means to
	// discriminate invalid strings from an actual zero value.

	f = (float)std::atof(s);
	if(f != 0.0f)
		return true;
	else {
		char* endptr;
		f = std::strtof(s, &endptr);
		return !*endptr;
	}
}

/******************************************************************************
 * Helper function that converts a string to a floating-point number.
 *****************************************************************************/
inline bool parseFloatType(const char* s, double& f)
{
	// First use the atof() function to parse the number because it's fast.
	// However, atof() returns 0.0 to report a parsing error. Thus, if
	// we get 0.0, we need to use the slower strtod() function as a means to
	// discriminate invalid strings from an actual zero value.

	f = std::atof(s);
	if(f != 0.0)
		return true;
	else {
		char* endptr;
		f = std::strtod(s, &endptr);
		return !*endptr;
	}
}

/******************************************************************************
 * Helper function that converts a string to an integer number.
 *****************************************************************************/
inline bool parseInt(const char* s, int& i)
{
	char* endptr;
	i = std::strtol(s, &endptr, 10);
	return !*endptr;
}

/******************************************************************************
 * Helper function that converts a string repr. of a bool ('T' or 'F') to an int
 *****************************************************************************/
inline bool parseBool(const char* s, int& d)
{
	if(s[1] != '\0') return false;
	if(s[0] == 'T') {
		d = 1;
		return true;
	}
	else if(s[0] == 'F') {
		d = 0;
		return true;
	}
	return false;
}

/******************************************************************************
 * Parses the string tokens from one line of the input file and stores the values
 * in the particle properties.
 *****************************************************************************/
void InputColumnReader::readParticle(size_t particleIndex, int ntokens, const char* tokens[])
{
	OVITO_ASSERT(_properties.size() == _mapping.size());
	if(ntokens < _properties.size())
		throw Exception(tr("Data line in input file does not contain enough columns. Expected %1 file columns, but found only %2.").arg(_properties.size()).arg(ntokens));

	auto propertyIterator = _properties.cbegin();
	const char** token = tokens;

	int d;
	char* endptr;
	for(int columnIndex = 0; propertyIterator != _properties.cend(); ++columnIndex, ++token, ++propertyIterator) {
		ParticleProperty* property = *propertyIterator;
		if(!property) continue;

		if(particleIndex >= property->size())
			throw Exception(tr("Too many data lines in input file. Expected only %1 lines.").arg(property->size()));

		int vectorComponent = std::max(0, _mapping[columnIndex].property.vectorComponent());
		OVITO_ASSERT_MSG(vectorComponent < (int)property->componentCount(), "InputColumnReader::readParticle", "Component index is out of range.");

		if(property->dataType() == _floatMetaTypeId) {
			FloatType f;
			if(!parseFloatType(*token, f))
				throw Exception(tr("Invalid floating-point value in column %1 (%2): \"%3\"").arg(columnIndex+1).arg(property->name()).arg(*token));
			property->setFloatComponent(particleIndex, vectorComponent, f);
		}
		else if(property->dataType() == _intMetaTypeId) {
			bool ok = parseInt(*token, d);
			if(property->type() != ParticleProperty::ParticleTypeProperty) {
				if(!ok) {
					ok = parseBool(*token, d);
					if(!ok)
						throw Exception(tr("Invalid integer/bool value in column %1 (%2): \"%3\"").arg(columnIndex+1).arg(property->name()).arg(*token));
				}
			}
			else {
				// Automatically register a new particle type if a new type identifier is encountered.
				if(ok) {
					_destination.addParticleTypeId(d);
				}
				else {
					d = _destination.addParticleTypeName(*token);
					_usingNamedParticleTypes = true;
				}
			}
			property->setIntComponent(particleIndex, vectorComponent, d);
		}
	}
}

/******************************************************************************
 * Processes the values from one line of the input file and stores them
 * in the particle properties.
 *****************************************************************************/
void InputColumnReader::readParticle(size_t particleIndex, const double* values, int nvalues)
{
	OVITO_ASSERT(_properties.size() == _mapping.size());
	if(nvalues < _properties.size())
		throw Exception(tr("Data record in input file does not contain enough columns. Expected %1 file columns, but found only %2.").arg(_properties.size()).arg(nvalues));

	auto propertyIterator = _properties.cbegin();
	const double* token = values;

	int d;
	for(int columnIndex = 0; propertyIterator != _properties.cend(); ++columnIndex, ++token, ++propertyIterator) {
		ParticleProperty* property = *propertyIterator;
		if(!property) continue;

		if(particleIndex >= property->size())
			throw Exception(tr("Too many data lines in input file. Expected only %1 lines.").arg(property->size()));

		int vectorComponent = std::max(0, _mapping[columnIndex].property.vectorComponent());
		OVITO_ASSERT_MSG(vectorComponent < (int)property->componentCount(), "InputColumnReader::readParticle", "Component index is out of range.");

		if(property->dataType() == _floatMetaTypeId) {
			property->setFloatComponent(particleIndex, vectorComponent, *token);
		}
		else if(property->dataType() == _intMetaTypeId) {
			int ival = (int)*token;
			if(property->type() == ParticleProperty::ParticleTypeProperty) {
				// Automatically register a new particle type if a new type identifier is encountered.
				_destination.addParticleTypeId(ival);
			}
			property->setIntComponent(particleIndex, vectorComponent, ival);
		}
	}
}

};	// End of namespace
