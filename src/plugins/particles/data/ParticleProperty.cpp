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
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include "ParticleProperty.h"

namespace Ovito { namespace Particles {

/******************************************************************************
* Default constructor.
******************************************************************************/
ParticleProperty::ParticleProperty()
	: _stride(0), _dataTypeSize(0), _type(UserProperty),
	_numParticles(0), _dataType(QMetaType::Void), _componentCount(0)
{
}

/******************************************************************************
* Constructor.
******************************************************************************/
ParticleProperty::ParticleProperty(size_t particleCount, int dataType, size_t dataTypeSize, size_t componentCount, size_t stride, const QString& name, bool initializeMemory)
	: _numParticles(0), _dataType(dataType), _dataTypeSize(dataTypeSize),
	  _stride(stride), _componentCount(componentCount), _type(UserProperty), _name(name)
{
	OVITO_ASSERT(_dataTypeSize > 0);
	OVITO_ASSERT(_componentCount > 0);
	OVITO_ASSERT(_stride >= _dataTypeSize * _componentCount);
	OVITO_ASSERT((_stride % _dataTypeSize) == 0);
	if(componentCount > 1) {
		for(size_t i = 1; i <= componentCount; i++)
			_componentNames << QString::number(i);
	}
	resize(particleCount, initializeMemory);
}

/******************************************************************************
* Constructor for a standard property.
******************************************************************************/
ParticleProperty::ParticleProperty(size_t particleCount, Type type, size_t componentCount, bool initializeMemory)
	: _numParticles(0), _type(type)
{
	switch(type) {
	case ParticleTypeProperty:
	case StructureTypeProperty:
	case SelectionProperty:
	case ClusterProperty:
	case CoordinationProperty:
	case IdentifierProperty:
	case MoleculeProperty:
		_dataType = qMetaTypeId<int>();
		_dataTypeSize = sizeof(int);
		_componentCount = 1;
		_stride = _dataTypeSize;
		break;
	case PositionProperty:
	case DisplacementProperty:
	case VelocityProperty:
	case ForceProperty:
	case DipoleOrientationProperty:
	case AngularVelocityProperty:
	case AngularMomentumProperty:
	case TorqueProperty:
	case AsphericalShapeProperty:
		_dataType = qMetaTypeId<FloatType>();
		_dataTypeSize = sizeof(FloatType);
		_componentCount = 3;
		_stride = sizeof(Vector3);
		OVITO_ASSERT(_stride == sizeof(Point3));
		break;
	case ColorProperty:
		_dataType = qMetaTypeId<FloatType>();
		_dataTypeSize = sizeof(FloatType);
		_componentCount = 3;
		_stride = _componentCount * _dataTypeSize;
		OVITO_ASSERT(_stride == sizeof(Color));
		break;
	case PotentialEnergyProperty:
	case KineticEnergyProperty:
	case TotalEnergyProperty:
	case RadiusProperty:
	case MassProperty:
	case ChargeProperty:
	case TransparencyProperty:
	case SpinProperty:
	case DipoleMagnitudeProperty:
	case CentroSymmetryProperty:
	case DisplacementMagnitudeProperty:
	case VelocityMagnitudeProperty:
    case NonaffineSquaredDisplacementProperty:
		_dataType = qMetaTypeId<FloatType>();
		_dataTypeSize = sizeof(FloatType);
		_componentCount = 1;
		_stride = _dataTypeSize;
		break;
	case StressTensorProperty:
	case StrainTensorProperty:
		_dataType = qMetaTypeId<FloatType>();
		_dataTypeSize = sizeof(FloatType);
		_componentCount = 6;
		_stride = _componentCount * _dataTypeSize;
		OVITO_ASSERT(_stride == sizeof(SymmetricTensor2));
		break;
	case DeformationGradientProperty:
		_dataType = qMetaTypeId<FloatType>();
		_dataTypeSize = sizeof(FloatType);
		_componentCount = 9;
		_stride = _componentCount * _dataTypeSize;
		break;
	case OrientationProperty:
		_dataType = qMetaTypeId<FloatType>();
		_dataTypeSize = sizeof(FloatType);
		_componentCount = 4;
		_stride = _componentCount * _dataTypeSize;
		OVITO_ASSERT(_stride == sizeof(Quaternion));
		break;
	case PeriodicImageProperty:
		_dataType = qMetaTypeId<int>();
		_dataTypeSize = sizeof(int);
		_componentCount = 3;
		_stride = _componentCount * _dataTypeSize;
		break;
	default:
		OVITO_ASSERT_MSG(false, "ParticleProperty constructor", "Invalid standard property type");
		throw Exception(ParticlePropertyObject::tr("This is not a valid standard property type: %1").arg(type));
	}
	OVITO_ASSERT_MSG(componentCount == 0 || componentCount == _componentCount, "ParticleProperty::ParticleProperty(type)", "Cannot specify component count for a standard property with a fixed component count.");
	OVITO_ASSERT(_stride >= _dataTypeSize * _componentCount);
	OVITO_ASSERT((_stride % _dataTypeSize) == 0);

	_componentNames = standardPropertyComponentNames(type, _componentCount);
	_name = standardPropertyName(type);
	resize(particleCount, initializeMemory);
}

/******************************************************************************
* Copy constructor.
******************************************************************************/
ParticleProperty::ParticleProperty(const ParticleProperty& other)
	: _type(other._type), _name(other._name), _dataType(other._dataType),
	  _dataTypeSize(other._dataTypeSize), _numParticles(other._numParticles),
	  _stride(other._stride), _componentCount(other._componentCount),
	  _componentNames(other._componentNames),
	  _data(new uint8_t[_numParticles * _stride])
{
	memcpy(_data.get(), other._data.get(), _numParticles * _stride);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void ParticleProperty::saveToStream(SaveStream& stream, bool onlyMetadata) const
{
	stream.beginChunk(0x01);
	stream << _name;
	stream << _type;
	stream << QByteArray(QMetaType::typeName(_dataType));
	stream.writeSizeT(_dataTypeSize);
	stream.writeSizeT(_stride);
	stream.writeSizeT(_componentCount);
	stream << _componentNames;
	if(onlyMetadata) {
		stream.writeSizeT(0);
	}
	else {
		stream.writeSizeT(_numParticles);
		stream.write(_data.get(), _stride * _numParticles);
	}
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void ParticleProperty::loadFromStream(LoadStream& stream)
{
	stream.expectChunk(0x01);
	stream >> _name;
	stream >> _type;
	QByteArray dataTypeName;
	stream >> dataTypeName;
	_dataType = QMetaType::type(dataTypeName.constData());
	OVITO_ASSERT_MSG(_dataType != 0, "ParticleProperty LoadStream operator", QString("The meta data type '%1' seems to be no longer defined.").arg(QString(dataTypeName)).toLocal8Bit().constData());
	OVITO_ASSERT(dataTypeName == QMetaType::typeName(_dataType));
	stream.readSizeT(_dataTypeSize);
	stream.readSizeT(_stride);
	stream.readSizeT(_componentCount);
	stream >> _componentNames;
	stream.readSizeT(_numParticles);
	_data.reset(new uint8_t[_numParticles * _stride]);
	stream.read(_data.get(), _stride * _numParticles);
	stream.closeChunk();

	// Do floating-point precision conversion from single to double precision.
	if(_dataType == qMetaTypeId<float>() && qMetaTypeId<FloatType>() == qMetaTypeId<double>()) {
		OVITO_ASSERT(sizeof(FloatType) == sizeof(double));
		OVITO_ASSERT(_dataTypeSize == sizeof(float));
		_stride *= sizeof(double) / sizeof(float);
		_dataTypeSize = sizeof(double);
		_dataType = qMetaTypeId<FloatType>();
		std::unique_ptr<uint8_t[]> newBuffer(new uint8_t[_stride * _numParticles]);
		double* dst = reinterpret_cast<double*>(newBuffer.get());
		const float* src = reinterpret_cast<const float*>(_data.get());
		for(size_t c = _numParticles * _componentCount; c--; )
			*dst++ = (double)*src++;
		_data.swap(newBuffer);
	}

	// Do floating-point precision conversion from double to single precision.
	if(_dataType == qMetaTypeId<double>() && qMetaTypeId<FloatType>() == qMetaTypeId<float>()) {
		OVITO_ASSERT(sizeof(FloatType) == sizeof(float));
		OVITO_ASSERT(_dataTypeSize == sizeof(double));
		_stride /= sizeof(double) / sizeof(float);
		_dataTypeSize = sizeof(float);
		_dataType = qMetaTypeId<FloatType>();
		std::unique_ptr<uint8_t[]> newBuffer(new uint8_t[_stride * _numParticles]);
		float* dst = reinterpret_cast<float*>(newBuffer.get());
		const double* src = reinterpret_cast<const double*>(_data.get());
		for(size_t c = _numParticles * _componentCount; c--; )
			*dst++ = (float)*src++;
		_data.swap(newBuffer);
	}
}

/******************************************************************************
* Resizes the array to the given size.
******************************************************************************/
void ParticleProperty::resize(size_t newSize, bool preserveData)
{
	OVITO_ASSERT(newSize >= 0 && newSize < 0xFFFFFFFF);
	std::unique_ptr<uint8_t[]> newBuffer(new uint8_t[newSize * _stride]);
	if(preserveData)
		memcpy(newBuffer.get(), _data.get(), _stride * std::min(_numParticles, newSize));
	_data.swap(newBuffer);

	// Initialize new elements to zero.
	if(newSize > _numParticles && preserveData) {
		memset(_data.get() + _numParticles * _stride, 0, (newSize - _numParticles) * _stride);
	}
	_numParticles = newSize;
}

/******************************************************************************
* Copies the contents from the given source into this property storage.
* Particles for which the bit in the given mask is set are skipped.
******************************************************************************/
void ParticleProperty::filterCopy(const ParticleProperty& source, const boost::dynamic_bitset<>& mask)
{
	OVITO_ASSERT(source.size() == mask.size());
	OVITO_ASSERT(stride() == source.stride());
	OVITO_ASSERT(source.size() == mask.count() + this->size());
	size_t oldParticleCount = source.size();

	// Optimize filter operation for the most common property types.
	if(stride() == sizeof(FloatType)) {
		// Single float
		const FloatType* src = reinterpret_cast<const FloatType*>(source.constData());
		FloatType* dst = reinterpret_cast<FloatType*>(data());
		for(size_t i = 0; i < oldParticleCount; ++i, ++src) {
			if(!mask.test(i)) *dst++ = *src;
		}
	}
	else if(stride() == sizeof(int)) {
		// Single integer
		const int* src = reinterpret_cast<const int*>(source.constData());
		int* dst = reinterpret_cast<int*>(data());
		for(size_t i = 0; i < oldParticleCount; ++i, ++src) {
			if(!mask.test(i)) *dst++ = *src;
		}
	}
	else if(stride() == sizeof(Point3)) {
		// Triple float (may actually be four floats when SSE instructions are enabled).
		const Point3* src = reinterpret_cast<const Point3*>(source.constData());
		Point3* dst = reinterpret_cast<Point3*>(data());
		for(size_t i = 0; i < oldParticleCount; ++i, ++src) {
			if(!mask.test(i)) *dst++ = *src;
		}
	}
	else if(stride() == sizeof(Color)) {
		// Triple float
		const Color* src = reinterpret_cast<const Color*>(source.constData());
		Color* dst = reinterpret_cast<Color*>(data());
		for(size_t i = 0; i < oldParticleCount; ++i, ++src) {
			if(!mask.test(i)) *dst++ = *src;
		}
	}
	else {
		// General case:
		const uint8_t* src = source._data.get();
		uint8_t* dst = _data.get();
		for(size_t i = 0; i < oldParticleCount; i++, src += stride()) {
			if(!mask.test(i)) {
				memcpy(dst, src, stride());
				dst += stride();
			}
		}
	}
}

/******************************************************************************
* Returns the name of a standard property.
******************************************************************************/
QString ParticleProperty::standardPropertyName(Type which)
{
	switch(which) {
	case ParticleTypeProperty: return ParticlePropertyObject::tr("Particle Type");
	case SelectionProperty: return ParticlePropertyObject::tr("Selection");
	case ClusterProperty: return ParticlePropertyObject::tr("Cluster");
	case CoordinationProperty: return ParticlePropertyObject::tr("Coordination");
	case PositionProperty: return ParticlePropertyObject::tr("Position");
	case ColorProperty: return ParticlePropertyObject::tr("Color");
	case DisplacementProperty: return ParticlePropertyObject::tr("Displacement");
	case DisplacementMagnitudeProperty: return ParticlePropertyObject::tr("Displacement Magnitude");
	case VelocityProperty: return ParticlePropertyObject::tr("Velocity");
	case PotentialEnergyProperty: return ParticlePropertyObject::tr("Potential Energy");
	case KineticEnergyProperty: return ParticlePropertyObject::tr("Kinetic Energy");
	case TotalEnergyProperty: return ParticlePropertyObject::tr("Total Energy");
	case RadiusProperty: return ParticlePropertyObject::tr("Radius");
	case StructureTypeProperty: return ParticlePropertyObject::tr("Structure Type");
	case IdentifierProperty: return ParticlePropertyObject::tr("Particle Identifier");
	case StressTensorProperty: return ParticlePropertyObject::tr("Stress Tensor");
	case StrainTensorProperty: return ParticlePropertyObject::tr("Strain Tensor");
	case DeformationGradientProperty: return ParticlePropertyObject::tr("Deformation Gradient");
	case OrientationProperty: return ParticlePropertyObject::tr("Orientation");
	case ForceProperty: return ParticlePropertyObject::tr("Force");
	case MassProperty: return ParticlePropertyObject::tr("Mass");
	case ChargeProperty: return ParticlePropertyObject::tr("Charge");
	case PeriodicImageProperty: return ParticlePropertyObject::tr("Periodic Image");
	case TransparencyProperty: return ParticlePropertyObject::tr("Transparency");
	case DipoleOrientationProperty: return ParticlePropertyObject::tr("Dipole Orientation");
	case DipoleMagnitudeProperty: return ParticlePropertyObject::tr("Dipole Magnitude");
	case AngularVelocityProperty: return ParticlePropertyObject::tr("Angular Velocity");
	case AngularMomentumProperty: return ParticlePropertyObject::tr("Angular Momentum");
	case TorqueProperty: return ParticlePropertyObject::tr("Torque");
	case SpinProperty: return ParticlePropertyObject::tr("Spin");
	case CentroSymmetryProperty: return ParticlePropertyObject::tr("Centrosymmetry");
	case VelocityMagnitudeProperty: return ParticlePropertyObject::tr("Velocity Magnitude");
	case NonaffineSquaredDisplacementProperty: return ParticlePropertyObject::tr("Nonaffine Squared Displacement");
	case MoleculeProperty: return ParticlePropertyObject::tr("Molecule Identifier");
	case AsphericalShapeProperty: return ParticlePropertyObject::tr("Aspherical Shape");
	default:
		OVITO_ASSERT_MSG(false, "ParticleProperty::standardPropertyName", "Invalid standard particle property type");
		throw Exception(ParticlePropertyObject::tr("This is not a valid standard particle property type: %1").arg(which));
	}
}

/******************************************************************************
* Returns the display title used for a standard property object.
******************************************************************************/
QString ParticleProperty::standardPropertyTitle(Type which)
{
	switch(which) {
	case ParticleTypeProperty: return ParticlePropertyObject::tr("Particle types");
	case PositionProperty: return ParticlePropertyObject::tr("Particle positions");
	case ColorProperty: return ParticlePropertyObject::tr("Particle colors");
	case DisplacementProperty: return ParticlePropertyObject::tr("Displacements");
	case VelocityProperty: return ParticlePropertyObject::tr("Velocities");
	case RadiusProperty: return ParticlePropertyObject::tr("Radii");
	case StructureTypeProperty: return ParticlePropertyObject::tr("Structure types");
	case IdentifierProperty: return ParticlePropertyObject::tr("Particle identifiers");
	default:
		return standardPropertyName(which);
	}
}

/******************************************************************************
* Returns the data type used by the given standard property.
******************************************************************************/
int ParticleProperty::standardPropertyDataType(Type which)
{
	switch(which) {
	case ParticleTypeProperty:
	case StructureTypeProperty:
	case SelectionProperty:
	case ClusterProperty:
	case CoordinationProperty:
	case IdentifierProperty:
	case PeriodicImageProperty:
	case MoleculeProperty:
		return qMetaTypeId<int>();
	case PositionProperty:
	case ColorProperty:
	case DisplacementProperty:
	case DisplacementMagnitudeProperty:
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
	case ChargeProperty:
	case TransparencyProperty:
	case DipoleMagnitudeProperty:
	case SpinProperty:
	case DipoleOrientationProperty:
	case AngularVelocityProperty:
	case AngularMomentumProperty:
	case TorqueProperty:
	case CentroSymmetryProperty:
	case VelocityMagnitudeProperty:
	case NonaffineSquaredDisplacementProperty:
	case AsphericalShapeProperty:
		return qMetaTypeId<FloatType>();
	default:
		OVITO_ASSERT_MSG(false, "ParticleProperty::standardPropertyDataType", "Invalid standard particle property type");
		throw Exception(ParticlePropertyObject::tr("This is not a valid standard particle property type: %1").arg(which));
	}
}

/******************************************************************************
* Returns a list with the names and identifiers of all defined standard properties.
******************************************************************************/
QMap<QString, ParticleProperty::Type> ParticleProperty::standardPropertyList()
{
	static QMap<QString, Type> table;
	if(table.empty()) {
		table.insert(standardPropertyName(ParticleTypeProperty), ParticleTypeProperty);
		table.insert(standardPropertyName(SelectionProperty), SelectionProperty);
		table.insert(standardPropertyName(ClusterProperty), ClusterProperty);
		table.insert(standardPropertyName(CoordinationProperty), CoordinationProperty);
		table.insert(standardPropertyName(PositionProperty), PositionProperty);
		table.insert(standardPropertyName(ColorProperty), ColorProperty);
		table.insert(standardPropertyName(DisplacementProperty), DisplacementProperty);
		table.insert(standardPropertyName(DisplacementMagnitudeProperty), DisplacementMagnitudeProperty);
		table.insert(standardPropertyName(VelocityProperty), VelocityProperty);
		table.insert(standardPropertyName(PotentialEnergyProperty), PotentialEnergyProperty);
		table.insert(standardPropertyName(KineticEnergyProperty), KineticEnergyProperty);
		table.insert(standardPropertyName(TotalEnergyProperty), TotalEnergyProperty);
		table.insert(standardPropertyName(RadiusProperty), RadiusProperty);
		table.insert(standardPropertyName(StructureTypeProperty), StructureTypeProperty);
		table.insert(standardPropertyName(IdentifierProperty), IdentifierProperty);
		table.insert(standardPropertyName(StressTensorProperty), StressTensorProperty);
		table.insert(standardPropertyName(StrainTensorProperty), StrainTensorProperty);
		table.insert(standardPropertyName(DeformationGradientProperty), DeformationGradientProperty);
		table.insert(standardPropertyName(OrientationProperty), OrientationProperty);
		table.insert(standardPropertyName(ForceProperty), ForceProperty);
		table.insert(standardPropertyName(MassProperty), MassProperty);
		table.insert(standardPropertyName(ChargeProperty), ChargeProperty);
		table.insert(standardPropertyName(PeriodicImageProperty), PeriodicImageProperty);
		table.insert(standardPropertyName(TransparencyProperty), TransparencyProperty);
		table.insert(standardPropertyName(DipoleOrientationProperty), DipoleOrientationProperty);
		table.insert(standardPropertyName(DipoleMagnitudeProperty), DipoleMagnitudeProperty);
		table.insert(standardPropertyName(AngularVelocityProperty), AngularVelocityProperty);
		table.insert(standardPropertyName(AngularMomentumProperty), AngularMomentumProperty);
		table.insert(standardPropertyName(TorqueProperty), TorqueProperty);
		table.insert(standardPropertyName(SpinProperty), SpinProperty);
		table.insert(standardPropertyName(CentroSymmetryProperty), CentroSymmetryProperty);
		table.insert(standardPropertyName(VelocityMagnitudeProperty), VelocityMagnitudeProperty);
		table.insert(standardPropertyName(NonaffineSquaredDisplacementProperty), NonaffineSquaredDisplacementProperty);
		table.insert(standardPropertyName(MoleculeProperty), MoleculeProperty);
		table.insert(standardPropertyName(AsphericalShapeProperty), AsphericalShapeProperty);
	}
	return table;
}

/******************************************************************************
* Returns the number of vector components per atom used by the given standard data channel.
******************************************************************************/
size_t ParticleProperty::standardPropertyComponentCount(Type which)
{
	switch(which) {
	case ParticleTypeProperty:
	case StructureTypeProperty:
	case SelectionProperty:
	case ClusterProperty:
	case CoordinationProperty:
	case IdentifierProperty:
	case PotentialEnergyProperty:
	case KineticEnergyProperty:
	case TotalEnergyProperty:
	case RadiusProperty:
	case MassProperty:
	case ChargeProperty:
	case TransparencyProperty:
	case DipoleMagnitudeProperty:
	case SpinProperty:
	case CentroSymmetryProperty:
	case DisplacementMagnitudeProperty:
	case VelocityMagnitudeProperty:
    case NonaffineSquaredDisplacementProperty:
    case MoleculeProperty:
		return 1;
	case PositionProperty:
	case ColorProperty:
	case DisplacementProperty:
	case VelocityProperty:
	case ForceProperty:
	case PeriodicImageProperty:
	case DipoleOrientationProperty:
	case AngularVelocityProperty:
	case AngularMomentumProperty:
	case TorqueProperty:
	case AsphericalShapeProperty:
		return 3;
	case StressTensorProperty:
	case StrainTensorProperty:
		return 6;
	case DeformationGradientProperty:
		return 9;
	case OrientationProperty:
		return 4;
	default:
		OVITO_ASSERT_MSG(false, "ParticleProperty::standardPropertyComponentCount", "Invalid standard particle property type");
		throw Exception(ParticlePropertyObject::tr("This is not a valid standard particle property type: %1").arg(which));
	}
}

/******************************************************************************
* Returns the list of component names for the given standard property.
******************************************************************************/
QStringList ParticleProperty::standardPropertyComponentNames(Type which, size_t componentCount)
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
	case IdentifierProperty:
	case PotentialEnergyProperty:
	case KineticEnergyProperty:
	case TotalEnergyProperty:
	case RadiusProperty:
	case MassProperty:
	case ChargeProperty:
	case TransparencyProperty:
	case DipoleMagnitudeProperty:
	case SpinProperty:
	case CentroSymmetryProperty:
	case DisplacementMagnitudeProperty:
	case VelocityMagnitudeProperty:
    case NonaffineSquaredDisplacementProperty:
    case MoleculeProperty:
		return emptyList;
	case PositionProperty:
	case DisplacementProperty:
	case VelocityProperty:
	case ForceProperty:
	case PeriodicImageProperty:
	case DipoleOrientationProperty:
	case AngularVelocityProperty:
	case AngularMomentumProperty:
	case TorqueProperty:
	case AsphericalShapeProperty:
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
		OVITO_ASSERT_MSG(false, "ParticleProperty::standardPropertyComponentNames", "Invalid standard particle property type");
		throw Exception(ParticlePropertyObject::tr("This is not a valid standard particle property type: %1").arg(which));
	}
}

}	// End of namespace
}	// End of namespace
