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

#ifndef __OVITO_PARTICLE_PROPERTY_H
#define __OVITO_PARTICLE_PROPERTY_H

#include <plugins/particles/Particles.h>

namespace Ovito { namespace Particles {

/**
 * \brief Memory storage for a per-particle property.
 */
class OVITO_PARTICLES_EXPORT ParticleProperty : public QSharedData
{
public:

	/// \brief The standard types of properties.
	enum Type {
		UserProperty = 0,	//< This is reserved for user-defined properties.
		ParticleTypeProperty,
		PositionProperty,
		SelectionProperty,
		ColorProperty,
		DisplacementProperty,
		DisplacementMagnitudeProperty,
		PotentialEnergyProperty,
		KineticEnergyProperty,
		TotalEnergyProperty,
		VelocityProperty,
		RadiusProperty,
		ClusterProperty,
		CoordinationProperty,
		StructureTypeProperty,
		IdentifierProperty,
		StressTensorProperty,
		StrainTensorProperty,
		DeformationGradientProperty,
		OrientationProperty,
		ForceProperty,
		MassProperty,
		ChargeProperty,
		PeriodicImageProperty,
		TransparencyProperty,
		DipoleOrientationProperty,
		DipoleMagnitudeProperty,
		AngularVelocityProperty,
		AngularMomentumProperty,
		TorqueProperty,
		SpinProperty,
		CentroSymmetryProperty,
		VelocityMagnitudeProperty,
        NonaffineSquaredDisplacementProperty,
		MoleculeProperty,
		AsphericalShapeProperty
	};
	Q_ENUMS(Type);

	/// Define our own iterator range type that can be used to iterate over
	/// the values stored in a ParticleProperty using C++11 range-based for loops.
	template<class T>
	class Range : public std::pair<T,T> {
	public:
		Q_DECL_CONSTEXPR Range(T begin, std::size_t size) : std::pair<T,T>(begin, begin + size) {}
		Q_DECL_CONSTEXPR T begin() const { return this->first; }
		Q_DECL_CONSTEXPR T end() const { return this->second; }
	};

public:

	/// \brief Default constructor that creates an empty, uninitialized storage.
	ParticleProperty();

	/// \brief Constructor that creates a standard property storage.
	/// \param particleCount The number of particles.
	/// \param type Specifies which standard property should be created.
	///             This must not be ParticleProperty::Type::UserProperty.
	/// \param componentCount The component count if this type of property
	///                       has a variable component count; otherwise 0 to use the
	///                       default number of components.
	/// \param initializeMemory Controls whether the newly allocated memory is initialized with zeros.
	///
	/// Data type, component count and property name are automatically set by this
	/// constructor.
	ParticleProperty(size_t particleCount, Type type, size_t componentCount, bool initializeMemory);

	/// \brief Constructor that creates a user-defined property storage.
	/// \param particleCount The number of particles.
	/// \param dataType Specifies the data type (integer, floating-point, ...) of the per-particle elements.
	///                 The data type is specified as identifier according to the Qt metatype system.
	/// \param dataTypeSize The size of the data type given by \a dataType in bytes.
	///                     This is necessary because the Qt type system has no function to query
	///                     the size of a data type at runtime.
	/// \param componentCount The number of components per particle of type \a dataType.
	/// \param stride The number of bytes per particle.
	/// \param name The name assigned to the property.
	/// \param initializeMemory Controls whether the newly allocated memory is initialized with zeros.
	ParticleProperty(size_t particleCount, int dataType, size_t dataTypeSize, size_t componentCount, size_t stride, const QString& name, bool initializeMemory);

	/// \brief Copy constructor.
	ParticleProperty(const ParticleProperty& other);

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
	/// \param preserveData Controls whether the existing per-particle data is preserved.
	///                     This also determines whether newly allocated memory is initialized to zero.
	void resize(size_t newSize, bool preserveData);

	/// \brief Returns the type of this property.
	Type type() const { return _type; }

	/// \brief Changes the type of this property.
	/// \note The type may only be changed if the new property has the same
	///       data type and component count as the old one.
	void setType(Type newType) {
		if(newType != UserProperty) {
			OVITO_ASSERT(dataType() == standardPropertyDataType(newType));
			OVITO_ASSERT(componentCount() == standardPropertyComponentCount(newType));
			setName(standardPropertyName(newType));
			_componentNames = standardPropertyComponentNames(newType);
		}
		_type = newType;
	}

	/// \brief Returns the data type of the property.
	/// \return The identifier of the data type used for the elements stored in
	///         this property storage according to the Qt meta type system.
	int dataType() const { return _dataType; }

	/// \brief Returns the number of bytes per value.
	/// \return Number of bytes used to store a single value of the data type
	///         specified by type().
	size_t dataTypeSize() const { return _dataTypeSize; }

	/// \brief Returns the number of bytes used per particle.
	size_t stride() const { return _stride; }

	/// \brief Returns the number of array elements per particle.
	/// \return The number of data values stored per particle in this storage object.
	size_t componentCount() const { return _componentCount; }

	/// \brief Returns the human-readable names for the vector components if this is a vector property.
	/// \return The names of the vector components if this property contains more than one value per atom.
	///         If this is only a single-valued property then an empty list is returned by this method.
	const QStringList& componentNames() const { return _componentNames; }

	/// \brief Returns a read-only pointer to the raw elements stored in this property object.
	const void* constData() const {
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

	/// \brief Returns a read-only pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Point3I or an integer channel with 3 components.
	const Point3I* constDataPoint3I() const {
		OVITO_ASSERT(dataType() == qMetaTypeId<Point3I>() || (dataType() == qMetaTypeId<int>() && componentCount() == 3));
		OVITO_STATIC_ASSERT(sizeof(Point3I) == sizeof(int) * 3);
		return reinterpret_cast<const Point3I*>(constData());
	}

	/// \brief Returns a read-only pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Color or a FloatType channel with 3 components.
	const Color* constDataColor() const {
		OVITO_ASSERT(dataType() == qMetaTypeId<Color>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 3));
		return reinterpret_cast<const Color*>(constData());
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

	/// \brief Returns a range of const iterators over the elements stored in this object.
	Range<const int*> constIntRange() const {
		OVITO_ASSERT(componentCount() == 1);
		return Range<const int*>(constDataInt(), size());
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	Range<const FloatType*> constFloatRange() const {
		OVITO_ASSERT(componentCount() == 1);
		return Range<const FloatType*>(constDataFloat(), size());
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	Range<const Point3*> constPoint3Range() const {
		return Range<const Point3*>(constDataPoint3(), size());
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	Range<const Vector3*> constVector3Range() const {
		return Range<const Vector3*>(constDataVector3(), size());
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	Range<const Color*> constColorRange() const {
		return Range<const Color*>(constDataColor(), size());
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	Range<const Point3I*> constPoint3IRange() const {
		return Range<const Point3I*>(constDataPoint3I(), size());
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	Range<const SymmetricTensor2*> constSymmetricTensor2Range() const {
		return Range<const SymmetricTensor2*>(constDataSymmetricTensor2(), size());
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	Range<const Quaternion*> constQuaternionRange() const {
		return Range<const Quaternion*>(constDataQuaternion(), size());
	}

	/// Returns a read-write pointer to the raw elements in the property storage.
	void* data() {
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

	/// \brief Returns a read-write pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Point3I or an integer channel with 3 components.
	Point3I* dataPoint3I() {
		OVITO_ASSERT(dataType() == qMetaTypeId<Point3I>() || (dataType() == qMetaTypeId<int>() && componentCount() == 3));
		OVITO_STATIC_ASSERT(sizeof(Point3I) == sizeof(int) * 3);
		return reinterpret_cast<Point3I*>(data());
	}

	/// \brief Returns a read-write pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Color or a FloatType channel with 3 components.
	Color* dataColor() {
		OVITO_ASSERT(dataType() == qMetaTypeId<Color>() || (dataType() == qMetaTypeId<FloatType>() && componentCount() == 3));
		return reinterpret_cast<Color*>(data());
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

	/// \brief Returns a range of iterators over the elements stored in this object.
	Range<int*> intRange() {
		OVITO_ASSERT(componentCount() == 1);
		return Range<int*>(dataInt(), size());
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	Range<FloatType*> floatRange() {
		OVITO_ASSERT(componentCount() == 1);
		return Range<FloatType*>(dataFloat(), size());
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	Range<Point3*> point3Range() {
		return Range<Point3*>(dataPoint3(), size());
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	Range<Vector3*> vector3Range() {
		return Range<Vector3*>(dataVector3(), size());
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	Range<Color*> colorRange() {
		return Range<Color*>(dataColor(), size());
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	Range<Point3I*> point3IRange() {
		return Range<Point3I*>(dataPoint3I(), size());
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	Range<SymmetricTensor2*> symmetricTensor2Range() {
		return Range<SymmetricTensor2*>(dataSymmetricTensor2(), size());
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	Range<Quaternion*> quaternionRange() {
		return Range<Quaternion*>(dataQuaternion(), size());
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

	/// Returns a Point3I element at the given index (if this is a point property).
	const Point3I& getPoint3I(size_t particleIndex) const {
		OVITO_ASSERT(particleIndex < size());
		return constDataPoint3I()[particleIndex];
	}

	/// Returns a Color element at the given index (if this is a color property).
	const Color& getColor(size_t particleIndex) const {
		OVITO_ASSERT(particleIndex < size());
		return constDataColor()[particleIndex];
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

	/// Sets the value of a Point3I element at the given index (if this is a point property).
	void setPoint3I(size_t particleIndex, const Point3I& newValue) {
		OVITO_ASSERT(particleIndex < size());
		dataPoint3I()[particleIndex] = newValue;
	}

	/// Sets the value of a Color element at the given index (if this is a color property).
	void setColor(size_t particleIndex, const Color& newValue) {
		OVITO_ASSERT(particleIndex < size());
		dataColor()[particleIndex] = newValue;
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

	/// Copies the contents from the given source into this storage.
	/// Particles for which the bit in the given mask is set are skipped.
	void filterCopy(const ParticleProperty& source, const boost::dynamic_bitset<>& mask);

	/// Writes the ParticleProperty to an output stream.
	void saveToStream(SaveStream& stream, bool onlyMetadata = false) const;

	/// Reads the ParticleProperty from an input stream.
	void loadFromStream(LoadStream& stream);

public:

	/// \brief Returns the name of a standard property.
	/// \param which Any of the standard property types except Type::UserProperty.
	/// \return The name string used for the given standard property by default.
	static QString standardPropertyName(Type which);

	/// \brief Returns the display title used for a standard property object.
	/// \param which Any of the standard property types except Type::UserProperty.
	/// \return The title string used for a property object.
	static QString standardPropertyTitle(Type which);

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

	/// The name of the property.
	QString _name;

	/// The data type of the property (a Qt metadata type identifier).
	int _dataType;

	/// The number of bytes per data type value.
	size_t _dataTypeSize;

	/// The number of per-particle elements in the property storage.
	size_t _numParticles;

	/// The number of bytes per particle.
	size_t _stride;

	/// The number of elements per particle.
	size_t _componentCount;

	/// The names of the vector components if this property consists of more than one value per particle.
	QStringList _componentNames;

	/// The internal data array that holds the elements.
	std::unique_ptr<uint8_t[]> _data;
};

}	// End of namespace
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Particles::ParticleProperty::Type);
Q_DECLARE_TYPEINFO(Ovito::Particles::ParticleProperty::Type, Q_PRIMITIVE_TYPE);

#endif // __OVITO_PARTICLE_PROPERTY_H
