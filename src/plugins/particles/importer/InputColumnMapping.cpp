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

/******************************************************************************
 * Resizes the mapping array to include the specified number of file columns.
 *****************************************************************************/
void InputColumnMapping::setColumnCount(int numberOfColumns, const QStringList& columnNames)
{
	// Expand column array if necessary and initialize all new columns to their default values.
	while(numberOfColumns >= columnCount()) {
		Column col;
		col.propertyType = ParticleProperty::UserProperty;
		col.dataType = QMetaType::Void;
		col.vectorComponent = 0;
		_columns.append(col);
	}
	_columns.resize(numberOfColumns);

	for(int i = 0; i < columnNames.size() && i < numberOfColumns; i++)
		_columns[i].columnName = columnNames[i];
}

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

/******************************************************************************
 * Saves the mapping to the given stream.
 *****************************************************************************/
void InputColumnMapping::saveToStream(SaveStream& stream) const
{
	stream.beginChunk(0x01);
	stream << (int)_columns.size();
	for(const Column& col : _columns) {
		stream << col.columnName;
		stream.writeEnum(col.propertyType);
		stream << col.propertyName;
		stream.writeEnum(col.dataType);
		stream << col.vectorComponent;
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
	_columns.resize(numColumns);
	for(Column& col : _columns) {
		stream >> col.columnName;
		stream.readEnum(col.propertyType);
		stream >> col.propertyName;
		stream.readEnum(col.dataType);
		if(col.dataType == qMetaTypeId<float>() || col.dataType == qMetaTypeId<double>())
			col.dataType = qMetaTypeId<FloatType>();
		stream >> col.vectorComponent;
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
	bool posPropertyPresent = false;
	for(int i = 0; i < columnCount(); i++) {
		if(propertyType(i) == ParticleProperty::PositionProperty) {
			posPropertyPresent = true;
			break;
		}
	}
	if(!posPropertyPresent)
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
	for(int i = 0; i < mapping.columnCount(); i++) {

		ParticleProperty* property = nullptr;

		int vectorComponent = mapping.vectorComponent(i);
		int dataType = mapping.dataType(i);
		if(vectorComponent < 0) vectorComponent = 0;

		if(dataType != QMetaType::Void) {
			ParticleProperty::Type propertyType = mapping.propertyType(i);
			QString propertyName = mapping.propertyName(i);

			size_t dataTypeSize;
			if(dataType == qMetaTypeId<int>())
				dataTypeSize = sizeof(int);
			else if(dataType == qMetaTypeId<FloatType>())
				dataTypeSize = sizeof(FloatType);
			else
				throw Exception(tr("Invalid custom particle property (data type %1) for input file column %2").arg(dataType).arg(i+1));

			if(propertyType != ParticleProperty::UserProperty) {
				// Look for existing standard property.
				for(const auto& p : destination.particleProperties()) {
					if(p->type() == propertyType) {
						property = p.get();
						break;
					}
				}
				if(!property) {
					// Create standard property.
					property = new ParticleProperty(particleCount, propertyType);
					destination.addParticleProperty(property);
				}
			}
			else {
				// Look for existing user-defined property with the same name.
				for(int j = 0; j < destination.particleProperties().size(); j++) {
					const auto& p = destination.particleProperties()[j];
					if(p->name() == propertyName) {
						if(property->dataType() == dataType && (int)property->componentCount() > vectorComponent)
							property = p.get();
						else
							destination.removeParticleProperty(j);
						break;
					}
				}
				if(!property) {
					// Create a new user-defined property for the column.
					property = new ParticleProperty(particleCount, dataType, dataTypeSize, vectorComponent + 1, propertyName);
					destination.addParticleProperty(property);
				}
			}
			if(property)
				property->setName(propertyName);

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
 * Parses the string tokens from one line of the input file and stores the values
 * in the particle properties.
 *****************************************************************************/
void InputColumnReader::readParticle(size_t particleIndex, int ntokens, const char* tokens[])
{
	OVITO_ASSERT(_properties.size() == _mapping.columnCount());
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
		OVITO_ASSERT_MSG(_mapping.vectorComponent(columnIndex) < (int)property->componentCount(), "InputColumnReader::readParticle", "Component index is out of range.");

		if(property->dataType() == _floatMetaTypeId) {
			FloatType f;
			if(!parseFloatType(*token, f))
				throw Exception(tr("Invalid floating-point value in column %1 (%2): \"%3\"").arg(columnIndex+1).arg(property->name()).arg(*token));
			property->setFloatComponent(particleIndex, _mapping.vectorComponent(columnIndex), f);
		}
		else if(property->dataType() == _intMetaTypeId) {
			bool ok = parseInt(*token, d);
			if(property->type() != ParticleProperty::ParticleTypeProperty) {
				if(!ok)
					throw Exception(tr("Invalid integer value in column %1 (%2): \"%3\"").arg(columnIndex+1).arg(property->name()).arg(*token));
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
			property->setIntComponent(particleIndex, _mapping.vectorComponent(columnIndex), d);
		}
	}
}

/******************************************************************************
 * Processes the values from one line of the input file and stores them
 * in the particle properties.
 *****************************************************************************/
void InputColumnReader::readParticle(size_t particleIndex, const double* values, int nvalues)
{
	OVITO_ASSERT(_properties.size() == _mapping.columnCount());
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
		OVITO_ASSERT_MSG(_mapping.vectorComponent(columnIndex) < (int)property->componentCount(), "InputColumnReader::readParticle", "Component index is out of range.");

		if(property->dataType() == _floatMetaTypeId) {
			property->setFloatComponent(particleIndex, _mapping.vectorComponent(columnIndex), *token);
		}
		else if(property->dataType() == _intMetaTypeId) {
			int ival = (int)*token;
			if(property->type() == ParticleProperty::ParticleTypeProperty) {
				// Automatically register a new particle type if a new type identifier is encountered.
				_destination.addParticleTypeId(ival);
			}
			property->setIntComponent(particleIndex, _mapping.vectorComponent(columnIndex), ival);
		}
	}
}

};	// End of namespace
