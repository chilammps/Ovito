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
#include "OutputColumnMapping.h"

namespace Particles {

/******************************************************************************
 * Inserts a column that will be written to the output data file.
 *****************************************************************************/
void OutputColumnMapping::insertColumn(int columnIndex, ParticleProperty::Type propertyType, const QString& propertyName, int vectorComponent)
{
	OVITO_ASSERT(columnIndex >= 0);

	// Expand column array if necessary and initialize all new columns to their default values.
	while(columnIndex >= columnCount()) {
		Column newEntry;
		newEntry.propertyType = ParticleProperty::UserProperty;
		newEntry.vectorComponent = 0;
		_columns.append(newEntry);
	}

	_columns[columnIndex].propertyType = propertyType;
	_columns[columnIndex].propertyName = propertyName;
	_columns[columnIndex].vectorComponent = vectorComponent;
}

/******************************************************************************
 * Removes the definition of a column.
 *****************************************************************************/
void OutputColumnMapping::removeColumn(int columnIndex)
{
	OVITO_ASSERT(columnIndex >= 0);
	if(columnIndex < columnCount()) {
		_columns.remove(columnIndex);
	}
}

/******************************************************************************
 * Saves the mapping to the given stream.
 *****************************************************************************/
void OutputColumnMapping::saveToStream(SaveStream& stream) const
{
	stream.beginChunk(0x01);
	stream << (int)_columns.size();
	for(const Column& col : _columns) {
		stream.writeEnum(col.propertyType);
		stream << col.propertyName;
		stream << col.vectorComponent;
	}
	stream.endChunk();
}

/******************************************************************************
 * Loads the mapping from the given stream.
 *****************************************************************************/
void OutputColumnMapping::loadFromStream(LoadStream& stream)
{
	stream.expectChunk(0x01);
	int numColumns;
	stream >> numColumns;
	_columns.resize(numColumns);
	for(Column& col : _columns) {
		stream.readEnum(col.propertyType);
		stream >> col.propertyName;
		stream >> col.vectorComponent;
	}
	stream.closeChunk();
}

/******************************************************************************
 * Saves the mapping into a byte array.
 *****************************************************************************/
QByteArray OutputColumnMapping::toByteArray() const
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
void OutputColumnMapping::fromByteArray(const QByteArray& array)
{
	QDataStream dstream(array);
	LoadStream stream(dstream);
	loadFromStream(stream);
	stream.close();
}

/******************************************************************************
 * Initializes the helper object.
 *****************************************************************************/
OutputColumnWriter::OutputColumnWriter(const OutputColumnMapping& mapping, const PipelineFlowState& source)
	: _mapping(mapping), _source(source)
{
	// Gather the source properties.
	for(int i = 0; i < mapping.columnCount(); i++) {

		ParticleProperty::Type propertyType = mapping.propertyType(i);
		QString propertyName = mapping.propertyName(i);
		int vectorComponent = mapping.vectorComponent(i);

		ParticlePropertyObject* property = nullptr;
		for(const auto& o : source.objects()) {
			ParticlePropertyObject* p = dynamic_object_cast<ParticlePropertyObject>(o.get());
			if(p && p->type() == propertyType) {
				if(propertyType != ParticleProperty::UserProperty || p->name() == propertyName) {
					property = p;
					break;
				}
			}
		}

		if(property == nullptr && propertyType != ParticleProperty::IdentifierProperty) {
			throw Exception(tr("The defined data columns to be written to the output file are not valid. "
			                   "The source data does not contain a particle property named '%1'.").arg(propertyName));
		}
		if(property && property->componentCount() <= vectorComponent)
			throw Exception(tr("The vector component specified for column %1 exceeds the number of available vector components in the particle property '%2'.").arg(i).arg(propertyName));
		if(property && property->dataType() == QMetaType::Void)
			throw Exception(tr("The particle property '%1' cannot be written to the output file because it is empty.").arg(propertyName));

		// Build internal list of property objects for fast look up during writing.
		_properties.push_back(property);
		_vectorComponents.push_back(vectorComponent);
	}
}

/******************************************************************************
 * Writes the data record for a single atom to the output stream.
 *****************************************************************************/
void OutputColumnWriter::writeParticle(size_t particleIndex, QTextStream& stream)
{
	QVector<ParticlePropertyObject*>::const_iterator property = _properties.constBegin();
	QVector<int>::const_iterator vcomp = _vectorComponents.constBegin();
	for(; property != _properties.constEnd(); ++property, ++vcomp) {
		if(property != _properties.constBegin()) stream << QStringLiteral(" ");
		if(*property) {
			if((*property)->dataType() == qMetaTypeId<int>())
				stream << (*property)->getIntComponent(particleIndex, *vcomp);
			else if((*property)->dataType() == qMetaTypeId<FloatType>())
				stream << (*property)->getFloatComponent(particleIndex, *vcomp);
		}
		else {
			stream << (particleIndex + 1);
		}
	}
}

};	// End of namespace
