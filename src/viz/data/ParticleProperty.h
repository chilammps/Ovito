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

/**
 * \file ParticleProperty.h
 * \brief Contains the definition of the Viz::ParticleProperty class.
 */

#ifndef __OVITO_PARTICLE_PROPERTY_H
#define __OVITO_PARTICLE_PROPERTY_H

#include <core/Core.h>

namespace Viz {

using namespace Ovito;

/**
 * \brief Memory storage for a per-particle property.
 */
class ParticleProperty
{
public:

	/// \brief The standard types of properties.
	enum Type {
		UserProperty = 0,	//< This is reserved for user-defined properties.
		ParticleTypeProperty = -1,
		PositionProperty = -2,
		SelectionProperty = -3,
		ColorProperty = -4,
		DisplacementProperty = -5,
		PotentialEnergyProperty = -6,
		KineticEnergyProperty = -7,
		TotalEnergyProperty = -8,
		VelocityProperty = -9,
		RadiusProperty = -10,
		ClusterProperty = -11,
		CoordinationProperty = -12,
		StructureTypeProperty = -13,
		IndexProperty = -14,
		StressTensorProperty = -15,
		StrainTensorProperty = -16,
		DeformationGradientProperty = -17,
		OrientationProperty = -18,
		ForceProperty = -19,
		MassProperty = -20,
		PeriodicImageProperty = -21,
		TransparencyProperty = -22,
	};
	Q_ENUMS(Type)

public:

	/// \brief Default constructor that creates an empty, uninitialized storage.
	ParticleProperty();

	/// \brief Constructor that creates a standard property storage.
	/// \param type Specifies which standard property should be created.
	///             This must not be ParticleProperty::Type::UserProperty.
	/// \param componentCount The component count if this type of property
	///                       has a variable component count; otherwise 0 to use the
	///                       default number of components.
	///
	/// Data type, component count and property name are automatically set by this
	/// constructor.
	ParticleProperty(Type type, size_t componentCount = 0);

	/// \brief Constructor that creates a user-defined property storage.
	/// \param dataType Specifies the data type (integer, floating-point, ...) of the per-particle elements.
	///                 The data type is specified as identifier according to the Qt metatype system.
	/// \param dataTypeSize The size of the data type given by \a dataType in bytes.
	///                     This is necessary because the Qt type system has no function to query
	///                     the size of a data type at runtime.
	/// \param componentCount The number of components per particle of type \a dataType.
	ParticleProperty(int dataType, size_t dataTypeSize, size_t componentCount);

	/// \brief Gets the property's name.
	/// \return The name of property.
	const QString& name() const { return _name; }

	/// \brief Sets the property's name if this is a user-defined property.
	/// \param name The new name string.
	void setName(const QString& name) { _name = name; }

	/// \brief Returns the number of particles for which this object stores the properties.
	/// \return The total number of data elements in this storage divided by the
	///         number of elements per particle.
	size_t size() const { return _numParticles; }

	/// \brief Resizes the property storage.
	/// \param newSize The new number of particles.
	void resize(size_t newSize);

	/// \brief Returns the type of this property.
	Type type() const { return _type; }

	/// \brief Returns the data type of the property.
	/// \return The identifier of the data type used for the elements stored in
	///         this property storage according to the Qt meta type system.
	int dataType() const { return _dataType; }

	/// \brief Returns the number of bytes per value.
	/// \return Number of bytes used to store a single value of the data type
	///         specified by type().
	size_t dataTypeSize() const { return _dataTypeSize; }

	/// \brief Returns the number of bytes used per particle.
	/// \return The size of the property' data type multiplied by the component count.
	size_t perParticleSize() const { return _perParticleSize; }

	/// \brief Returns the number of array elements per particle.
	/// \return The number of data values stored per particle in this storage object.
	size_t componentCount() const { return _componentCount; }

	/// \brief Changes the number of components per particle.
	/// \param count The new number of data values stored per particle in this storage object.
	/// \note Calling this function will destroy all data stored in the property storage.
	void setComponentCount(size_t count);

	/// \brief Returns the human-readable names for the vector components if this is a vector property.
	/// \return The names of the vector components if this property contains more than one value per atom.
	///         If this is only a single-valued property then an empty list is returned by this method.
	const QStringList& componentNames() const { return _componentNames; }

	/// \brief Returns a read-only pointer to the raw elements stored in this property object.
	const char* constData() const {
		return _data.get();
	}

	/// \brief Returns a read-only pointer to the first integer element stored in this object..
	/// \note This method may only be used if this property is of data type integer.
	const int* constDataInt() const {
		OVITO_ASSERT(dataType() == qMetaTypeId<int>());
		return reinterpret_cast<const int*>(constData());
	}

	/// \brief Returns a read-only pointer to the first float element in the property storage.
	/// \note This method may only be used if this property is of data type float.
	const FloatType* constDataFloat() const {
		OVITO_ASSERT(dataType() == qMetaTypeId<FloatType>());
		return reinterpret_cast<const FloatType*>(constData());
	}

	/// \brief Returns a read-only pointer to the first vector element in the property storage.
	/// \note This method may only be used if this property is of data type Vector3 or a FloatType channel with 3 components.
	const Vector3* constDataVector3() const {
		OVITO_ASSERT(dataType() == qMetaTypeId<Vector3>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 3));
		return reinterpret_cast<const Vector3*>(constData());
	}

	/// \brief Returns a read-only pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Point3 or a FloatType channel with 3 components.
	const Point3* constDataPoint3() const {
		OVITO_ASSERT(dataType() == qMetaTypeId<Point3>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 3));
		return reinterpret_cast<const Point3*>(constData());
	}

	/// \brief Returns a read-only pointer to the first tensor element in the property storage.
	/// \note This method may only be used if this property is of data type Tensor2 or a FloatType channel with 9 components.
	const Tensor2* constDataTensor2() const {
		OVITO_ASSERT(dataType() == qMetaTypeId<Tensor2>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 9));
		return reinterpret_cast<const Tensor2*>(constData());
	}

	/// \brief Returns a read-only pointer to the first symmetric tensor element in the property storage.
	/// \note This method may only be used if this property is of data type SymmetricTensor2 or a FloatType channel with 6 components.
	const SymmetricTensor2* constDataSymmetricTensor2() const {
		OVITO_ASSERT(dataType() == qMetaTypeId<SymmetricTensor2>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 6));
		return reinterpret_cast<const SymmetricTensor2*>(constData());
	}

	/// \brief Returns a read-only pointer to the first quaternion element in the property storage.
	/// \note This method may only be used if this property is of data type Quaternion or a FloatType channel with 4 components.
	const Quaternion* constDataQuaternion() const {
		OVITO_ASSERT(dataType() == qMetaTypeId<Quaternion>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 4));
		return reinterpret_cast<const Quaternion*>(constData());
	}

	/// Returns a read-write pointer to the raw elements in the property storage.
	char* data() {
		return _data.get();
	}

	/// \brief Returns a read-write pointer to the first integer element stored in this object..
	/// \note This method may only be used if this property is of data type integer.
	int* dataInt() {
		OVITO_ASSERT(dataType() == qMetaTypeId<int>());
		return reinterpret_cast<int*>(data());
	}

	/// \brief Returns a read-only pointer to the first float element in the property storage.
	/// \note This method may only be used if this property is of data type float.
	FloatType* dataFloat() {
		OVITO_ASSERT(dataType() == qMetaTypeId<FloatType>());
		return reinterpret_cast<FloatType*>(data());
	}

	/// \brief Returns a read-write pointer to the first vector element in the property storage.
	/// \note This method may only be used if this property is of data type Vector3 or a FloatType channel with 3 components.
	Vector3* dataVector3() {
		OVITO_ASSERT(dataType() == qMetaTypeId<Vector3>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 3));
		return reinterpret_cast<Vector3*>(data());
	}

	/// \brief Returns a read-write pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Point3 or a FloatType channel with 3 components.
	Point3* dataPoint3() {
		OVITO_ASSERT(dataType() == qMetaTypeId<Point3>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 3));
		return reinterpret_cast<Point3*>(data());
	}

	/// \brief Returns a read-write pointer to the first tensor element in the property storage.
	/// \note This method may only be used if this property is of data type Tensor2 or a FloatType channel with 9 components.
	Tensor2* dataTensor2() {
		OVITO_ASSERT(dataType() == qMetaTypeId<Tensor2>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 9));
		return reinterpret_cast<Tensor2*>(data());
	}

	/// \brief Returns a read-write pointer to the first symmetric tensor element in the property storage.
	/// \note This method may only be used if this property is of data type SymmetricTensor2 or a FloatType channel with 6 components.
	SymmetricTensor2* dataSymmetricTensor2() {
		OVITO_ASSERT(dataType() == qMetaTypeId<SymmetricTensor2>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 6));
		return reinterpret_cast<SymmetricTensor2*>(data());
	}

	/// \brief Returns a read-write pointer to the first quaternion element in the property storage.
	/// \note This method may only be used if this property is of data type Quaternion or a FloatType channel with 4 components.
	Quaternion* dataQuaternion() {
		OVITO_ASSERT(dataType() == qMetaTypeId<Quaternion>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 4));
		return reinterpret_cast<Quaternion*>(data());
	}

	/// \brief Returns an integer element at the given index (if this is an integer property).
	int getInt(size_t particleIndex) const {
		OVITO_ASSERT(particleIndex < size() && componentCount() == 1);
		return constDataInt()[particleIndex];
	}

	/// Returns a float element at the given index (if this is a float property).
	FloatType getFloat(size_t particleIndex) const {
		OVITO_ASSERT(particleIndex < size() && componentCount() == 1);
		return constDataFloat()[particleIndex];
	}

	/// Returns an integer element at the given index (if this is an integer property).
	int getIntComponent(size_t particleIndex, size_t componentIndex) const {
		OVITO_ASSERT(particleIndex < size() && componentIndex < componentCount());
		return constDataInt()[particleIndex*componentCount() + componentIndex];
	}

	/// Returns a float element at the given index (if this is a float property).
	FloatType getFloatComponent(size_t particleIndex, size_t componentIndex) const {
		OVITO_ASSERT(particleIndex < size() && componentIndex < componentCount());
		return constDataFloat()[particleIndex*componentCount() + componentIndex];
	}

	/// Returns a Vector3 element at the given index (if this is a vector property).
	const Vector3& getVector3(size_t particleIndex) const {
		OVITO_ASSERT(particleIndex < size());
		return constDataVector3()[particleIndex];
	}

	/// Returns a Point3 element at the given index (if this is a point property).
	const Point3& getPoint3(size_t particleIndex) const {
		OVITO_ASSERT(particleIndex < size());
		return constDataPoint3()[particleIndex];
	}

	/// Returns a Tensor2 element stored for the given particle.
	const Tensor2& getTensor2(size_t particleIndex) const {
		OVITO_ASSERT(particleIndex < size());
		return constDataTensor2()[particleIndex];
	}

	/// Returns a SymmetricTensor2 element stored for the given particle.
	const SymmetricTensor2& getSymmetricTensor2(size_t particleIndex) const {
		OVITO_ASSERT(particleIndex < size());
		return constDataSymmetricTensor2()[particleIndex];
	}

	/// Returns a Quaternion element stored for the given particle.
	const Quaternion& getQuaternion(size_t particleIndex) const {
		OVITO_ASSERT(particleIndex < size());
		return constDataQuaternion()[particleIndex];
	}

	/// Sets the value of an integer element at the given index (if this is an integer property).
	void setInt(size_t particleIndex, int newValue) {
		OVITO_ASSERT(particleIndex < size());
		dataInt()[particleIndex] = newValue;
	}

	/// Sets the value of a float element at the given index (if this is a float property).
	void setFloat(size_t particleIndex, FloatType newValue) {
		OVITO_ASSERT(particleIndex < size());
		dataFloat()[particleIndex] = newValue;
	}

	/// Sets the value of an integer element at the given index (if this is an integer property).
	void setIntComponent(size_t particleIndex, size_t componentIndex, int newValue) {
		OVITO_ASSERT(particleIndex < size() && componentIndex < componentCount());
		dataInt()[particleIndex*componentCount() + componentIndex] = newValue;
	}

	/// Sets the value of a float element at the given index (if this is a float property).
	void setFloatComponent(size_t particleIndex, size_t componentIndex, FloatType newValue) {
		OVITO_ASSERT(particleIndex < size() && componentIndex < componentCount());
		dataFloat()[particleIndex*componentCount() + componentIndex] = newValue;
	}

	/// Sets the value of a Vector3 element at the given index (if this is a vector property).
	void setVector3(size_t particleIndex, const Vector3& newValue) {
		OVITO_ASSERT(particleIndex < size());
		dataVector3()[particleIndex] = newValue;
	}

	/// Sets the value of a Point3 element at the given index (if this is a point property).
	void setPoint3(size_t particleIndex, const Point3& newValue) {
		OVITO_ASSERT(particleIndex < size());
		dataPoint3()[particleIndex] = newValue;
	}

	/// Sets the value of a Tensor2 element for the given particle.
	void setTensor2(size_t particleIndex, const Tensor2& newValue) {
		OVITO_ASSERT(particleIndex < size());
		dataTensor2()[particleIndex] = newValue;
	}

	/// Sets the value of a SymmetricTensor2 element for the given particle.
	void setSymmetricTensor2(size_t particleIndex, const SymmetricTensor2& newValue) {
		OVITO_ASSERT(particleIndex < size());
		dataSymmetricTensor2()[particleIndex] = newValue;
	}

	/// Sets the value of a Quaternion element for the given particle.
	void setQuaternion(size_t particleIndex, const Quaternion& newValue) {
		OVITO_ASSERT(particleIndex < size());
		dataQuaternion()[particleIndex] = newValue;
	}

public:

	/// \brief Returns the default name used by the given type of standard property.
	/// \param which Any of the standard property types except Type::UserProperty.
	/// \return The name string used for the given standard property by default.
	static QString standardPropertyName(Type which);

	/// Returns the data type used by the given standard property type.
	static int standardPropertyDataType(Type which);

	/// \brief Returns the number of vector components per particle used by the given standard property.
	/// \param which The standard property type for which the number of components should be returned.
	/// \return The number of fixed components or 0 if this kind of property has a variable number of components.
	static size_t standardPropertyComponentCount(Type which);

	/// \brief Returns the list of component names for the given standard property type.
	/// \param which The standard property type for which the component names should be returned.
	/// \param componentCount Optional number of actual components if the standard property has a variable number of components.
	static QStringList standardPropertyComponentNames(Type which, size_t componentCount = 0);

	/// \brief Returns a list with the names and types of all defined standard property types.
	static QMap<QString, Type> standardPropertyList();

protected:

	/// The type of this property.
	Type _type;

	/// The data type of the property (a Qt metadata type identifier).
	int _dataType;

	/// The number of bytes per data type value.
	size_t _dataTypeSize;

	/// The number of per-particle elements in the property storage.
	size_t _numParticles;

	/// The number of bytes per element.
	/// This is the size of the data type multiplied by the component count.
	size_t _perParticleSize;

	/// The number of array elements per particle.
	size_t _componentCount;

	/// The names of the vector components if this property consists of more than one value per particle.
	QStringList _componentNames;

	/// The internal data array that holds the elements.
	std::unique_ptr<char[]> _data;

	friend SaveStream& operator<<(SaveStream& stream, const ParticleProperty& s);
	friend LoadStream& operator>>(LoadStream& stream, ParticleProperty& s);
};

/// Writes a ParticleProperty to an output stream.
SaveStream& operator<<(SaveStream& stream, const ParticleProperty& s);

/// Reads a ParticleProperty from an input stream.
LoadStream& operator>>(LoadStream& stream, ParticleProperty& s);

};	// End of namespace

#endif // __OVITO_PARTICLE_PROPERTY_H
