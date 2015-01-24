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

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import)

/******************************************************************************
 * Saves the mapping to the given stream.
 *****************************************************************************/
void InputColumnMapping::saveToStream(SaveStream& stream) const
{
	stream.beginChunk(0x01);
	stream << (int)size();
	for(const InputColumnInfo& col : *this) {
		stream << col.columnName;
		stream << col.property.type();
		stream << col.property.name();
		stream << col.dataType;
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
		stream >> propertyType;
		QString propertyName;
		stream >> propertyName;
		stream >> col.dataType;
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
InputColumnReader::InputColumnReader(const InputColumnMapping& mapping, ParticleFrameLoader& destination, size_t particleCount)
	: _mapping(mapping), _destination(destination),
	  _intMetaTypeId(qMetaTypeId<int>()), _floatMetaTypeId(qMetaTypeId<FloatType>()),
	  _usingNamedParticleTypes(false)
{
	mapping.validate();

	// Create particle properties as defined by the mapping.
	for(int i = 0; i < (int)mapping.size(); i++) {

		ParticleProperty* property = nullptr;
		const ParticlePropertyReference& pref = mapping[i].property;

		int vectorComponent = std::max(0, pref.vectorComponent());
		int dataType = mapping[i].dataType;

		TargetPropertyRecord rec;

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
					property = new ParticleProperty(particleCount, pref.type(), 0, true);
					destination.addParticleProperty(property);
				}
			}
			else {
				// Look for existing user-defined property with the same name.
                ParticleProperty* oldProperty = nullptr;
                int oldPropertyIndex = -1;
				for(int j = 0; j < (int)destination.particleProperties().size(); j++) {
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
					property = new ParticleProperty(particleCount, dataType, dataTypeSize, vectorComponent + 1, dataTypeSize * (vectorComponent + 1), pref.name(), true);
					destination.addParticleProperty(property);
					if(oldProperty) {
						// We need to replace all old properties (with lower vector component count) with this one.
						for(TargetPropertyRecord& rec2 : _properties) {
							if(rec2.property == oldProperty)
								rec2.property = property;
						}
						// Remove old property.
						destination.removeParticleProperty(oldPropertyIndex);
					}
				}
			}
			if(property)
				property->setName(pref.name());

			OVITO_ASSERT(vectorComponent < (int)property->componentCount());
			rec.vectorComponent = vectorComponent;
		}

		// Build list of target properties for fast look up during parsing.
		rec.property = property;
		_properties.push_back(rec);
	}

	// Finalize the target property records.
	for(TargetPropertyRecord& rec : _properties) {
		if(rec.property) {
			rec.count = rec.property->size();
			rec.isTypeProperty = (rec.property->type() == ParticleProperty::ParticleTypeProperty);
			if(rec.property->dataType() == qMetaTypeId<FloatType>()) {
				rec.data = reinterpret_cast<uint8_t*>(rec.property->dataFloat() + rec.vectorComponent);
				rec.isInt = false;
			}
			else if(rec.property->dataType() == qMetaTypeId<int>()) {
				rec.data = reinterpret_cast<uint8_t*>(rec.property->dataInt() + rec.vectorComponent);
				rec.isInt = true;
			}
			else rec.data = nullptr;
			rec.stride = rec.property->stride();
		}
	}
}

/******************************************************************************
 * Helper function that converts a string to a floating-point number.
 *****************************************************************************/
inline bool parseFloatType(const char* s, const char* s_end, float& f)
{
	return boost::spirit::qi::parse(s, s_end, boost::spirit::qi::float_, f);
}

/******************************************************************************
 * Helper function that converts a string to a floating-point number.
 *****************************************************************************/
inline bool parseFloatType(const char* s, const char* s_end, double& f)
{
	return boost::spirit::qi::parse(s, s_end, boost::spirit::qi::double_, f);
}

/******************************************************************************
 * Helper function that converts a string to an integer number.
 *****************************************************************************/
inline bool parseInt(const char* s, const char* s_end, int& i)
{
	return boost::spirit::qi::parse(s, s_end, boost::spirit::qi::int_, i);
}

/******************************************************************************
 * Helper function that converts a string repr. of a bool ('T' or 'F') to an int
 *****************************************************************************/
inline bool parseBool(const char* s, const char* s_end, int& d)
{
	if(s_end != s + 1) return false;
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
 * in the data channels of the destination AtomsObject.
 *****************************************************************************/
const char* InputColumnReader::readParticle(size_t particleIndex, const char* s, const char* s_end)
{
	OVITO_ASSERT(_properties.size() == _mapping.size());

	int columnIndex = 0;
	while(columnIndex < _properties.size()) {
		// Skip initial whitespace.
		while(s != s_end && (*s == ' ' || *s == '\t' || *s == '\r'))
			++s;
		if(s == s_end || *s == '\n') break;
		const char* token = s;
		// Go to end of token.
		while(s != s_end && *s > ' ')
			++s;
		if(s != token) {
			parseField(particleIndex, columnIndex, token, s);
			columnIndex++;
		}
		if(s == s_end) break;
	}
	if(columnIndex < _properties.size())
		throw Exception(tr("Data line in input file does not contain enough columns. Expected %1 file columns, but found only %2.").arg(_properties.size()).arg(columnIndex));

	// Skip to end of line.
	while(s != s_end && *s != '\n')
		++s;
	if(s != s_end) ++s;
	return s;
}

/******************************************************************************
 * Parses the string tokens from one line of the input file and stores the values
 * in the data channels of the destination AtomsObject.
 *****************************************************************************/
void InputColumnReader::readParticle(size_t particleIndex, const char* s)
{
	OVITO_ASSERT(_properties.size() == _mapping.size());

	int columnIndex = 0;
	while(columnIndex < _properties.size()) {
		while(*s == ' ' || *s == '\t')
			++s;
		const char* token = s;
		while(*s > ' ')
			++s;
		if(s != token) {
			parseField(particleIndex, columnIndex, token, s);
			columnIndex++;
		}
		if(*s == '\0') break;
		s++;
	}
	if(columnIndex < _properties.size())
		throw Exception(tr("Data line in input file does not contain enough columns. Expected %1 file columns, but found only %2.").arg(_properties.size()).arg(columnIndex));
}

/******************************************************************************
 * Parse a single field from a text line.
 *****************************************************************************/
void InputColumnReader::parseField(size_t particleIndex, int columnIndex, const char* token, const char* token_end)
{
	const TargetPropertyRecord& prec = _properties[columnIndex];
	if(!prec.property || !prec.data) return;

	if(particleIndex >= prec.count)
		throw Exception(tr("Too many data lines in input file. Expected only %1 lines.").arg(prec.count));

	if(!prec.isInt) {
		if(!parseFloatType(token, token_end, *reinterpret_cast<FloatType*>(prec.data + particleIndex * prec.stride)))
			throw Exception(tr("Invalid floating-point value in column %1 (%2): \"%3\"").arg(columnIndex+1).arg(prec.property->name()).arg(QString::fromLocal8Bit(token, token_end - token)));
	}
	else {
		int& d = *reinterpret_cast<int*>(prec.data + particleIndex * prec.stride);
		bool ok = parseInt(token, token_end, d);
		if(!prec.isTypeProperty) {
			if(!ok) {
				ok = parseBool(token, token_end, d);
				if(!ok)
					throw Exception(tr("Invalid integer/bool value in column %1 (%2): \"%3\"").arg(columnIndex+1).arg(prec.property->name()).arg(QString::fromLocal8Bit(token, token_end - token)));
			}
		}
		else {
			// Automatically register a new particle type if a new type identifier is encountered.
			if(ok) {
				_destination.addParticleTypeId(d);
			}
			else {
				d = _destination.addParticleTypeName(token, token_end);
				_usingNamedParticleTypes = true;
			}
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

	auto prec = _properties.cbegin();
	const double* token = values;

	for(int columnIndex = 0; prec != _properties.cend(); ++columnIndex, ++token, ++prec) {
		if(!prec->property) continue;

		if(particleIndex >= prec->count)
			throw Exception(tr("Too many data lines in input file. Expected only %1 lines.").arg(prec->count));

		if(prec->data) {
			if(!prec->isInt) {
				*reinterpret_cast<FloatType*>(prec->data + particleIndex * prec->stride) = (FloatType)*token;
			}
			else {
				int ival = (int)*token;
				if(prec->isTypeProperty) {
					// Automatically register a new particle type if a new type identifier is encountered.
					_destination.addParticleTypeId(ival);
				}
				*reinterpret_cast<int*>(prec->data + particleIndex * prec->stride) = ival;
			}
		}
	}
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
