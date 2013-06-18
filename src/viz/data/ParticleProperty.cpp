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

/******************************************************************************
* Default constructor.
******************************************************************************/
ParticleProperty::ParticleProperty()
	: _perParticleSize(0), _dataTypeSize(0), _type(UserProperty),
	_numParticles(0), _dataType(QMetaType::Void), _componentCount(0)
{
}

/******************************************************************************
* Constructor.
******************************************************************************/
ParticlePropertyStorage::ParticlePropertyStorage(int dataType, size_t dataTypeSize, size_t componentCount)
	: _numParticles(0), _dataType(dataType), _dataTypeSize(dataTypeSize),
	_perParticleSize(dataTypeSize*componentCount),
	_componentCount(componentCount) _type(UserProperty)
{
	OVITO_ASSERT(_dataTypeSize > 0);
	OVITO_ASSERT(_componentCount > 0);
	if(componentCount > 1) {
		for(size_t i = 1; i <= componentCount; i++)
			_componentNames << QString::number(i);
	}
}

/******************************************************************************
* Constructor for a standard property.
******************************************************************************/
ParticleProperty::ParticleProperty(Type type, size_t componentCount)
	: _numParticles(0), _type(type)
{
	switch(type) {
	case ParticleTypeProperty:
	case StructureTypeProperty:
	case SelectionProperty:
	case ClusterProperty:
	case CoordinationProperty:
	case IndexProperty:
		_dataType = qMetaTypeId<int>();
		_dataTypeSize = sizeof(int);
		_componentCount = 1;
		break;
	case PositionProperty:
	case ColorProperty:
	case DisplacementProperty:
	case VelocityProperty:
	case ForceProperty:
		_dataType = qMetaTypeId<FloatType>();
		_dataTypeSize = sizeof(FloatType);
		_componentCount = 3;
		OVITO_ASSERT(_dataTypeSize * _componentCount == sizeof(Vector3));
		break;
	case PotentialEnergyProperty:
	case KineticEnergyProperty:
	case TotalEnergyProperty:
	case RadiusProperty:
	case MassProperty:
	case TransparencyProperty:
		_dataType = qMetaTypeId<FloatType>();
		_dataTypeSize = sizeof(FloatType);
		_componentCount = 1;
		break;
	case StressTensorProperty:
	case StrainTensorProperty:
		_dataType = qMetaTypeId<FloatType>();
		_dataTypeSize = sizeof(FloatType);
		_componentCount = 6;
		OVITO_ASSERT(_dataTypeSize * _componentCount == sizeof(SymmetricTensor2));
		break;
	case DeformationGradientProperty:
		_dataType = qMetaTypeId<FloatType>();
		_dataTypeSize = sizeof(FloatType);
		_componentCount = 9;
		OVITO_ASSERT(_dataTypeSize * _componentCount == sizeof(Matrix3));
		break;
	case OrientationProperty:
		_dataType = qMetaTypeId<FloatType>();
		_dataTypeSize = sizeof(FloatType);
		_componentCount = 4;
		OVITO_ASSERT(_dataTypeSize * _componentCount == sizeof(Quaternion));
		break;
	case PeriodicImageProperty:
		_dataType = qMetaTypeId<int>();
		_dataTypeSize = sizeof(int);
		_componentCount = 3;
		break;
	default:
		OVITO_ASSERT_MSG(false, "ParticleProperty constructor", "Invalid standard property type");
		throw Exception(tr("This is not a valid standard property type: %1").arg(type));
	}
	OVITO_ASSERT_MSG(componentCount == 0, "ParticleProperty::ParticleProperty(type)", "Cannot specify component count for a standard property with a fixed component count.");

	_perParticleSize = _componentCount * _dataTypeSize;
	_componentNames = standardPropertyComponentNames(type, _componentCount);
	_name = standardPropertyName(type);
}

/******************************************************************************
* Changes the number of components per particle.
******************************************************************************/
void ParticlePropertyStorage::setComponentCount(size_t count)
{
	if(count == _componentCount)
		return;

	size_t oldSize = size();
	resize(0);

	_componentCount = count;
	_perParticleSize = _componentCount * _dataTypeSize;
	// Resize component names array.
	if(_componentNames.size() > _componentCount)
		_componentNames = _componentNames.mid(0, _componentCount);
	else {
		while(_componentNames.size() < _componentCount)
			_componentNames.append(QString());
	}

	// Re-allocate memory.
	resize(oldSize);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
SaveStream& operator<<(SaveStream& stream, const ParticlePropertyStorage& s)
{
	stream.beginChunk(0x01);
	stream.writeEnum(s._type);
	stream << QByteArray(QMetaType::typeName(s._type));
	stream.writeSizeT(s._dataTypeSize);
	stream.writeSizeT(s._perParticleSize);
	stream.writeSizeT(s._numParticles);
	stream.writeSizeT(s._componentCount);
	stream << s._componentNames;
	stream.write(s._data.get(), s._perParticleSize * s._numParticles);
	stream.endChunk();
	return stream;
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
LoadStream& operator>>(LoadStream& stream, ParticlePropertyStorage& s)
{
	stream.expectChunk(0x01);
	stream.readEnum(s._type);
	QByteArray typeName;
	stream >> typeName;
	s._type = QMetaType::type(typeName.constData());
	OVITO_ASSERT_MSG(s._type != 0, "ParticlePropertyStorage LoadStream operator", QString("The meta data type '%1' seems to be no longer defined.").arg(QString(typeName)).toLocal8Bit().constData());
	OVITO_ASSERT(typeName == QMetaType::typeName(s._type));
	stream.readSizeT(s._dataTypeSize);
	stream.readSizeT(s._perParticleSize);
	stream.readSizeT(s._numParticles);
	stream.readSizeT(s._componentCount);
	stream >> s._componentNames;
	s._data.reset(new char[s._perParticleSize * s._numParticles]);
	stream.read(s._data.get(), s._perParticleSize * s._numParticles);
	stream.closeChunk();

	// Do floating-point precision conversion from single to double precision.
	if(s._type == qMetaTypeId<float>() && qMetaTypeId<FloatType>() == qMetaTypeId<double>()) {
		OVITO_ASSERT(sizeof(FloatType) == sizeof(double));
		OVITO_ASSERT(s._dataTypeSize == sizeof(float));
		s._perParticleSize *= sizeof(double) / sizeof(float);
		s._dataTypeSize = sizeof(double);
		s._type = qMetaTypeId<FloatType>();
		std::unique_ptr<char[]> newBuffer(new char[s._perParticleSize * s._numParticles]);
		double* dst = reinterpret_cast<double*>(newBuffer.get());
		const float* src = reinterpret_cast<const float*>(s._data.get());
		for(size_t c = s._numParticles * s._componentCount; c--; )
			*dst++ = (double)*src++;
		s._data.swap(newBuffer);
	}

	// Do floating-point precision conversion from double to single precision.
	if(s._type == qMetaTypeId<double>() && qMetaTypeId<FloatType>() == qMetaTypeId<float>()) {
		OVITO_ASSERT(sizeof(FloatType) == sizeof(float));
		OVITO_ASSERT(s._dataTypeSize == sizeof(double));
		s._perParticleSize /= sizeof(double) / sizeof(float);
		s._dataTypeSize = sizeof(float);
		s._type = qMetaTypeId<FloatType>();
		std::unique_ptr<char[]> newBuffer(new char[s._perParticleSize * s._numParticles]);
		float* dst = reinterpret_cast<float*>(newBuffer.get());
		const double* src = reinterpret_cast<const double*>(s._data.get());
		for(size_t c = s._numParticles * s._componentCount; c--; )
			*dst++ = (float)*src++;
		s._data.swap(newBuffer);
	}

	return stream;
}

/******************************************************************************
* Resizes the array to the given size.
******************************************************************************/
void ParticlePropertyStorage::resize(size_t newSize)
{
	OVITO_ASSERT(newSize >= 0 && newSize < 0xFFFFFFFF);
	std::unique_ptr<char[]> newBuffer(new char[newSize * _perParticleSize]);
	memcpy(newBuffer.get(), _data.get(), _perParticleSize * std::min(_numParticles, newSize));
	_data.swap(newBuffer);

	// Initialize new elements to zero.
	if(newSize > _numParticles) {
		memset(_data.get() + _numParticles * _perParticleSize, 0, (newSize - _numParticles) * _perParticleSize);
	}
	_numParticles = newSize;
}

};	// End of namespace
