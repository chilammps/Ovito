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
#include "ParticleProperty.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ParticleProperty, SceneObject)
DEFINE_PROPERTY_FIELD(ParticleProperty, _name, "Name");
DEFINE_PROPERTY_FIELD(ParticleProperty, _objectTitle, "ObjectTitle");
SET_PROPERTY_FIELD_LABEL(ParticleProperty, _name, "Name")
SET_PROPERTY_FIELD_LABEL(ParticleProperty, _objectTitle, "Object title")

/******************************************************************************
* Default constructor.
******************************************************************************/
ParticleProperty::ParticleProperty()
	: _perParticleSize(0), _id(UserProperty), _dataTypeSize(0),
	_numParticles(0), _type(QMetaType::Void), _componentCount(0)
{
	INIT_PROPERTY_FIELD(ParticleProperty::_name);
	INIT_PROPERTY_FIELD(ParticleProperty::_objectTitle);
}

/******************************************************************************
* Constructor.
******************************************************************************/
ParticleProperty::ParticleProperty(int dataType, size_t dataTypeSize, size_t componentCount)
	: _numParticles(0), _type(dataType), _dataTypeSize(dataTypeSize),
	_id(UserProperty), _perParticleSize(dataTypeSize*componentCount),
	_componentCount(componentCount)
{
	INIT_PROPERTY_FIELD(ParticleProperty::_name);
	INIT_PROPERTY_FIELD(ParticleProperty::_objectTitle);

	OVITO_ASSERT(_dataTypeSize > 0);
	OVITO_ASSERT(_componentCount > 0);
	if(componentCount > 1) {
		for(size_t i = 1; i <= componentCount; i++)
			_componentNames << QString::number(i);
	}
}


/******************************************************************************
* Changes the number of components per particle.
******************************************************************************/
void ParticleProperty::setComponentCount(size_t count)
{
	_componentCount = count;
	_perParticleSize = _componentCount * _dataTypeSize;
	// Resize component names array.
	if(_id != UserProperty) {
		OVITO_ASSERT_MSG(standardPropertyComponentCount(_id) == 0, "ParticleProperty::setComponentCount()", "Cannot change component count of a standard property with a fixed number of components.");
		_componentNames = standardPropertyComponentNames(_id, _componentCount);
	}
	else {
		if(_componentNames.size() > _componentCount)
			_componentNames = _componentNames.mid(0, _componentCount);
		else
			while(_componentNames.size() < _componentCount)
				_componentNames.append(QString());
	}

	// Re-allocate memory.
	setSize(size());
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void ParticleProperty::saveToStream(ObjectSaveStream& stream)
{
	SceneObject::saveToStream(stream);

	stream.beginChunk(0x01);
	stream.writeEnum(_id);
	stream.writeEnum(_type);
	stream << QByteArray(QMetaType::typeName(_type));
	stream.writeSizeT(_dataTypeSize);
	stream.writeSizeT(_perParticleSize);
	stream.writeSizeT(_numParticles);
	stream.writeSizeT(_componentCount);
	stream << _componentNames;
	stream << _data;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void ParticleProperty::loadFromStream(ObjectLoadStream& stream)
{
	SceneObject::loadFromStream(stream);

	stream.expectChunk(0x01);
	stream.readEnum(_id);
	stream.readEnum(_type);
	QByteArray typeName;
	stream >> typeName;
	_type = QMetaType::type(typeName.constData());
	OVITO_ASSERT_MSG(_type != 0, "ParticleProperty::loadFromStream", QString("The meta data type '%1' seems to be no longer defined.").arg(QString(typeName)).toLocal8Bit().constData());
	OVITO_ASSERT(typeName == QMetaType::typeName(_type));
	stream.readSizeT(_dataTypeSize);
	stream.readSizeT(_perParticleSize);
	stream.readSizeT(_numParticles);
	stream.readSizeT(_componentCount);
	stream >> _componentNames;
	stream >> _data;
	stream.closeChunk();

	// Do floating-point precision conversion from single to double precision.
	if(_type == qMetaTypeId<float>() && qMetaTypeId<FloatType>() == qMetaTypeId<double>()) {
		OVITO_ASSERT(sizeof(FloatType) == sizeof(double));
		OVITO_ASSERT(_dataTypeSize == sizeof(float));
		_perParticleSize *= sizeof(double) / sizeof(float);
		_dataTypeSize = sizeof(double);
		_type = qMetaTypeId<FloatType>();
		QByteArray newBuffer;
		newBuffer.resize(_perParticleSize * _numParticles);
		double* dst = (double*)newBuffer.data();
		const float* src = (const float*)_data.constData();
		for(size_t c = _numParticles * _componentCount; c--; )
			*dst++ = (double)*src++;
	}

	// Do floating-point precision conversion from double to single precision.
	if(_type == qMetaTypeId<double>() && qMetaTypeId<FloatType>() == qMetaTypeId<float>()) {
		OVITO_ASSERT(sizeof(FloatType) == sizeof(float));
		OVITO_ASSERT(_dataTypeSize == sizeof(double));
		_perParticleSize /= sizeof(double) / sizeof(float);
		_dataTypeSize = sizeof(float);
		_type = qMetaTypeId<FloatType>();
		QByteArray newBuffer;
		newBuffer.resize(_perParticleSize * _numParticles);
		float* dst = (float*)newBuffer.data();
		const double* src = (const double*)_data.constData();
		for(size_t c = _numParticles * _componentCount; c--; )
			*dst++ = (float)*src++;
	}

	OVITO_ASSERT(_data.size() == _numParticles * _perParticleSize);
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> ParticleProperty::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<ParticleProperty> clone = static_object_cast<ParticleProperty>(RefTarget::clone(deepCopy, cloneHelper));

	clone->_id = this->_id;
	clone->_type = this->_type;
	clone->_dataTypeSize = this->_dataTypeSize;
	clone->_perParticleSize = this->_perParticleSize;
	clone->_numParticles = this->_numParticles;
	clone->_componentCount = this->_componentCount;
	clone->_componentNames = this->_componentNames;
	clone->_data = this->_data;

	return clone;
}

/******************************************************************************
* Resizes the array to the given size.
******************************************************************************/
void ParticleProperty::setSize(size_t newSize)
{
	//OVITO_ASSERT_MSG(usageCount() <= 1, "ParticleProperty::setSize()", "The size of the property storage may only be changed when it is not references by multiple objects.");
	OVITO_ASSERT(newSize >= 0 && newSize < 0xFFFFFFFF);
	_data.resize(newSize * _perParticleSize);

	// Initialize new elements to zero.
	if(newSize > _numParticles) {
		memset(_data.data() + _numParticles*_perParticleSize, 0, (newSize-_numParticles)*_perParticleSize);
	}
	_numParticles = newSize;
}

/******************************************************************************
* Copies the contents from the given source into this property storage.
* Particles for which the bit in the given mask is set are skipped.
******************************************************************************/
void ParticleProperty::filterCopy(const ParticleProperty& source, const std::vector<bool>& mask)
{
	OVITO_ASSERT(source.size() == mask.size());
	OVITO_ASSERT(perParticleSize() == source.perParticleSize());
	size_t oldParticleCount = source.size();

	// Optimize filter operation for the most common channel types.
	if(perParticleSize() == sizeof(FloatType)) {
		// Single float
		const FloatType* src = source.constDataFloat();
		FloatType* dst = dataFloat();
		for(size_t i = 0; i < oldParticleCount; ++i, ++src) {
			if(!mask[i]) *dst++ = *src;
		}
	}
	else if(perParticleSize() == sizeof(int)) {
		// Single integer
		const int* src = source.constDataInt();
		int* dst = dataInt();
		for(size_t i = 0; i < oldParticleCount; ++i, ++src) {
			if(!mask[i]) *dst++ = *src;
		}
	}
	else if(perParticleSize() == sizeof(Point3)) {
		// Triple float
		const Point3* src = source.constDataPoint3();
		Point3* dst = dataPoint3();
		for(size_t i = 0; i < oldParticleCount; ++i, ++src) {
			if(!mask[i]) *dst++ = *src;
		}
	}
	else {
		// General case:
		const char* src = source.constData();
		char* dst = data();
		for(size_t i = 0; i < oldParticleCount; i++, src += _perParticleSize) {
			if(!mask[i]) {
				memcpy(dst, src, _perParticleSize);
				dst += _perParticleSize;
			}
		}
	}
}

/******************************************************************************
* Returns the name string used by default for the given standard property.
******************************************************************************/
QString ParticleProperty::standardPropertyName(Identifier which)
{
	switch(which) {
	case ParticleTypeProperty: return tr("Particle Type");
	case SelectionProperty: return tr("Selection");
	case ClusterProperty: return tr("Cluster");
	case CoordinationProperty: return tr("Coordination");
	case PositionProperty: return tr("Position");
	case ColorProperty: return tr("Color");
	case DisplacementProperty: return tr("Displacement");
	case VelocityProperty: return tr("Velocity");
	case PotentialEnergyProperty: return tr("Potential Energy");
	case KineticEnergyProperty: return tr("Kinetic Energy");
	case TotalEnergyProperty: return tr("Total Energy");
	case RadiusProperty: return tr("Radius");
	case StructureTypeProperty: return tr("Structure Type");
	case IndexProperty: return tr("Atom Index");
	case StressTensorProperty: return tr("Stress Tensor");
	case StrainTensorProperty: return tr("Strain Tensor");
	case DeformationGradientProperty: return tr("Deformation Gradient");
	case OrientationProperty: return tr("Orientation");
	case ForceProperty: return tr("Force");
	case MassProperty: return tr("Mass");
	case PeriodicImageProperty: return tr("Periodic Image");
	case TransparencyProperty: return tr("Transparency");
	default:
		OVITO_ASSERT_MSG(false, "ParticleProperty::standardChannelName", "Invalid standard particle property identifier");
		throw Exception(tr("This is not a valid standard particle property identifier: %1").arg(which));
	}
}

/******************************************************************************
* Returns the data type used by the given standard property.
******************************************************************************/
int ParticleProperty::standardPropertyDataType(Identifier which)
{
	switch(which) {
	case ParticleTypeProperty:
	case StructureTypeProperty:
	case SelectionProperty:
	case ClusterProperty:
	case CoordinationProperty:
	case IndexProperty:
	case PeriodicImageProperty:
		return qMetaTypeId<int>();
	case PositionProperty:
	case ColorProperty:
	case DisplacementProperty:
	case VelocityProperty:
	case PotentialEnergyProperty:
	case KineticEnergyProperty:
	case TotalEnergyProperty:
	case RadiusProperty:
	case StressTensorProperty:
	case StrainTensorProperty:
	case DeformationGradientProperty:
	case OrientationProperty:
	case ForceProperty:
	case MassProperty:
	case TransparencyProperty:
		return qMetaTypeId<FloatType>();
	default:
		OVITO_ASSERT_MSG(false, "ParticleProperty::standardPropertyDataType", "Invalid standard particle property identifier");
		throw Exception(tr("This is not a valid standard particle property identifier: %1").arg(which));
	}
}

/******************************************************************************
* Returns a list with the names and identifiers of all defined standard properties.
******************************************************************************/
QMap<QString, ParticleProperty::Identifier> ParticleProperty::standardPropertyList()
{
	static QMap<QString, Identifier> table;
	if(table.empty()) {
		table.insert(standardPropertyName(ParticleTypeProperty), ParticleTypeProperty);
		table.insert(standardPropertyName(SelectionProperty), SelectionProperty);
		table.insert(standardPropertyName(ClusterProperty), ClusterProperty);
		table.insert(standardPropertyName(CoordinationProperty), CoordinationProperty);
		table.insert(standardPropertyName(PositionProperty), PositionProperty);
		table.insert(standardPropertyName(ColorProperty), ColorProperty);
		table.insert(standardPropertyName(DisplacementProperty), DisplacementProperty);
		table.insert(standardPropertyName(VelocityProperty), VelocityProperty);
		table.insert(standardPropertyName(PotentialEnergyProperty), PotentialEnergyProperty);
		table.insert(standardPropertyName(KineticEnergyProperty), KineticEnergyProperty);
		table.insert(standardPropertyName(TotalEnergyProperty), TotalEnergyProperty);
		table.insert(standardPropertyName(RadiusProperty), RadiusProperty);
		table.insert(standardPropertyName(StructureTypeProperty), StructureTypeProperty);
		table.insert(standardPropertyName(IndexProperty), IndexProperty);
		table.insert(standardPropertyName(StressTensorProperty), StressTensorProperty);
		table.insert(standardPropertyName(StrainTensorProperty), StrainTensorProperty);
		table.insert(standardPropertyName(DeformationGradientProperty), DeformationGradientProperty);
		table.insert(standardPropertyName(OrientationProperty), OrientationProperty);
		table.insert(standardPropertyName(ForceProperty), ForceProperty);
		table.insert(standardPropertyName(MassProperty), MassProperty);
		table.insert(standardPropertyName(PeriodicImageProperty), PeriodicImageProperty);
		table.insert(standardPropertyName(TransparencyProperty), TransparencyProperty);
	}
	return table;
}

/******************************************************************************
* Returns the number of vector components per atom used by the given standard data channel.
******************************************************************************/
size_t ParticleProperty::standardPropertyComponentCount(Identifier which)
{
	switch(which) {
	case ParticleTypeProperty:
	case StructureTypeProperty:
	case SelectionProperty:
	case ClusterProperty:
	case CoordinationProperty:
	case IndexProperty:
	case PotentialEnergyProperty:
	case KineticEnergyProperty:
	case TotalEnergyProperty:
	case RadiusProperty:
	case MassProperty:
	case TransparencyProperty:
		return 1;
	case PositionProperty:
	case ColorProperty:
	case DisplacementProperty:
	case VelocityProperty:
	case ForceProperty:
	case PeriodicImageProperty:
		return 3;
	case StressTensorProperty:
	case StrainTensorProperty:
		return 6;
	case DeformationGradientProperty:
		return 9;
	case OrientationProperty:
		return 4;
	default:
		OVITO_ASSERT_MSG(false, "ParticleProperty::standardPropertyComponentCount", "Invalid standard particle property identifier");
		throw Exception(tr("This is not a valid standard particle property identifier: %1").arg(which));
	}
}

/******************************************************************************
* Returns the list of component names for the given standard property.
******************************************************************************/
QStringList ParticleProperty::standardPropertyComponentNames(Identifier which, size_t componentCount)
{
	const static QStringList emptyList;
	const static QStringList xyzList = QStringList() << "X" << "Y" << "Z";
	const static QStringList rgbList = QStringList() << "R" << "G" << "B";
	const static QStringList symmetricTensorList = QStringList() << "XX" << "YY" << "ZZ" << "XY" << "XZ" << "YZ";
	const static QStringList matrix3List = QStringList() << "11" << "21" << "31" << "12" << "22" << "32" << "13" << "23" << "33";
	const static QStringList quaternionList = QStringList() << "X" << "Y" << "Z" << "W";

	switch(which) {
	case ParticleTypeProperty:
	case StructureTypeProperty:
	case SelectionProperty:
	case ClusterProperty:
	case CoordinationProperty:
	case IndexProperty:
	case PotentialEnergyProperty:
	case KineticEnergyProperty:
	case TotalEnergyProperty:
	case RadiusProperty:
	case MassProperty:
	case TransparencyProperty:
		return emptyList;
	case PositionProperty:
	case DisplacementProperty:
	case VelocityProperty:
	case ForceProperty:
	case PeriodicImageProperty:
		return xyzList;
	case ColorProperty:
		return rgbList;
	case StressTensorProperty:
	case StrainTensorProperty:
		return symmetricTensorList;
	case DeformationGradientProperty:
		return matrix3List;
	case OrientationProperty:
		return quaternionList;
	default:
		OVITO_ASSERT_MSG(false, "ParticleProperty::standardPropertyComponentNames", "Invalid standard particle property identifier");
		throw Exception(tr("This is not a valid standard particle property identifier: %1").arg(which));
	}
}

};	// End of namespace
