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

#include <core/Core.h>
#include "InputColumnMapping.h"

namespace Viz {

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
	_columns[columnIndex].vectorComponent = vectorComponent;
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
 * Initializes the object.
 *****************************************************************************/
InputColumnReader::InputColumnReader(const InputColumnMapping& mapping, ParticleImportData& destination, size_t particleCount)
	: _mapping(mapping), _destination(destination),
	  _intMetaTypeId(qMetaTypeId<int>()), _floatMetaTypeId(qMetaTypeId<FloatType>())
{
	// Create particle properties as defined by the mapping.
	for(int i = 0; i < mapping.columnCount(); i++) {

		QExplicitlySharedDataPointer<ParticleProperty> property;

		int vectorComponent = mapping.vectorComponent(i);
		int dataType = mapping.dataType(i);

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
						property = p;
						break;
					}
				}
				if(!property) {
					// Create standard property.
					property = new ParticleProperty(propertyType);
					property->resize(particleCount);
					destination.addParticleProperty(property);
				}
			}
			else {
				// Look for existing user-defined property with the same name.
				for(int j = 0; j < destination.particleProperties().size(); j++) {
					ParticleProperty* p = destination.particleProperties()[j].data();
					if(p->name() == propertyName) {
						if(property->dataType() == dataType && property->componentCount() > vectorComponent)
							property = p;
						else
							destination.removeParticleProperty(j);
						break;
					}
				}
				if(!property) {
					// Create a new user-defined property for the column.
					property = new ParticleProperty(dataType, dataTypeSize, vectorComponent + 1);
					property->resize(particleCount);
					destination.addParticleProperty(property);
				}
			}
			if(property)
				property->setName(propertyName);
		}

		// Build list of property objects for fast look up during parsing.
		_properties.push_back(property.data());
	}
}

/******************************************************************************
 * Parses the string tokens from one line of the input file and stores the values
 * in the data channels of the destination AtomsObject.
 *****************************************************************************/
void InputColumnReader::readParticle(size_t particleIndex, char* dataLine)
{
	if(_tokens.isNull())
		_tokens.reset(new const char*[_properties.size()]);

	// Divide string into tokens.
	int ntokens = 0;
	while(ntokens < _properties.size()) {
		while(*dataLine != '\0' && std::isblank(*dataLine))
			++dataLine;
		_tokens[ntokens] = dataLine;
		while(*dataLine != '\0' && !std::isspace(*dataLine))
			++dataLine;
		if(*dataLine == '\n' || *dataLine == '\r') *dataLine = '\0';
		if(dataLine != _tokens[ntokens]) ntokens++;
		if(*dataLine == '\0') break;
		*dataLine = '\0';
		dataLine++;
	}

	readParticle(particleIndex, ntokens, _tokens.data());
}

/******************************************************************************
 * Parses the string tokens from one line of the input file and stores the values
 * in the data channels of the destination AtomsObject.
 *****************************************************************************/
void InputColumnReader::readParticle(size_t particleIndex, int ntokens, const char* tokens[])
{
	OVITO_ASSERT(_properties.size() == _mapping.columnCount());
	if(ntokens < _properties.size())
		throw Exception(tr("Data line in input file contains not enough items. Expected %1 file columns but found only %2.").arg(_properties.size()).arg(ntokens));

	auto propertyIterator = _properties.cbegin();
	const char** token = tokens;

	int d;
	char* endptr;
	for(int columnIndex = 0; propertyIterator != _properties.cend(); ++columnIndex, ++token, ++propertyIterator) {
		ParticleProperty* property = *propertyIterator;
		if(!property) continue;

		if(particleIndex >= property->size())
			throw Exception(tr("Too many data lines in input file. Expected only %1 lines.").arg(property->size()));

		if(property->dataType() == _floatMetaTypeId) {
			double f = strtod(*token, &endptr);
			if(*endptr)
				throw Exception(tr("Invalid floating-point value in column %1 (%2): \"%3\"").arg(columnIndex+1).arg(property->name()).arg(*token));
			property->setFloatComponent(particleIndex, _mapping.vectorComponent(columnIndex), (FloatType)f);
		}
		else if(property->dataType() == _intMetaTypeId) {
			d = strtol(*token, &endptr, 10);
			if(property->type() != ParticleProperty::ParticleTypeProperty) {
				if(*endptr)
					throw Exception(tr("Invalid integer value in column %1 (%2): \"%3\"").arg(columnIndex+1).arg(property->name()).arg(*token));
			}
			else {
				// Automatically register a new particle type if a new type identifier is encountered.
				if(!*endptr) {
					_destination.addParticleType(d);
				}
				else {
					d = _destination.particleTypeFromName(*token);
					if(d == -1) {
						d = _destination.particleTypes().size() + 1;
						_destination.addParticleType(d, *token);
					}
				}
			}
			property->setIntComponent(particleIndex, _mapping.vectorComponent(columnIndex), d);
		}
	}
}

};	// End of namespace
