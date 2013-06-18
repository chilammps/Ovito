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
 * \file ParticlePropertyObject.h
 * \brief Contains the definition of the Viz::ParticlePropertyObject class.
 */

#ifndef __OVITO_PARTICLE_PROPERTY_OBJECT_H
#define __OVITO_PARTICLE_PROPERTY_OBJECT_H

#include <core/Core.h>
#include <core/scene/objects/SceneObject.h>
#include "ParticlePropertyStorage.h"

namespace Viz {

using namespace Ovito;

/**
 * \brief Storage for a per-particle property.
 */
class ParticlePropertyObject : public SceneObject
{
public:

	/// \brief Creates an empty property object.
	Q_INVOKABLE ParticlePropertyObject();

	/// \brief Constructor that creates a user-defined property object.
	/// \param dataType Specifies the data type (integer, floating-point, ...) of the per-particle elements
	///                 in the new property storage. The data type is specified as identifier according to the
	///                 Qt metatype system.
	/// \param dataTypeSize The size of the data type given by \a dataType in bytes.
	///                     This is necessary because the Qt type system has no function to query
	///                     the size of a data type at runtime.
	/// \param componentCount The number of components per particle of type \a dataType.
	ParticlePropertyObject(int dataType, size_t dataTypeSize, size_t componentCount);

	/// \brief Constructor that creates a standard property storage.
	/// \param which Specifies which standard property should be created.
	///              This must not be ParticlePropertyIdentifier::UserProperty.
	/// \param componentCount The component count if this type of property
	///                       has a variable component count; otherwise 0 to use the
	///                       default number of components.
	///
	/// Data type, component count and property name are automatically set by this
	/// constructor.
	ParticlePropertyObject(ParticleProperty::Type which, size_t componentCount = 0);

	/// \brief Gets the property's name.
	/// \return The name of property, which is shown to the user.
	const QString& name() const { return _name; }

	/// \brief Sets the property's name.
	/// \param name The new name string.
	/// \undoable
	void setName(const QString& name) { _name = name; }

	/// \brief Returns the number of particles for which this object stores the properties.
	/// \return The total number of data elements in this channel divided by the
	///         number of elements per particle.
	size_t size() const { return _numParticles; }

	/// \brief Resizes the property storage.
	/// \param newSize The new number of particles.
	void setSize(size_t newSize);

	/// \brief Returns the type of this property.
	ParticleProperty::Type type() const { return _type; }

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

	/// \brief Returns the human-readable names for the components stored per atom.
	/// \return The names of the vector components if this channel contains more than one value per atom.
	///         If this is only a single valued channel then an empty list is returned by this method because
	///         then no naming is necessary.
	const QStringList& componentNames() const { return _componentNames; }

	/// \brief Returns a read-only pointer to the raw elements stored in this property object.
	const char* constData() const {
		return _data.constData();
	}

	/// \brief Returns a read-only pointer to the first integer element stored in this object..
	/// \note This method may only be used if this property is of data type integer.
	const int* constDataInt() const {
		OVITO_ASSERT(type() == qMetaTypeId<int>());
		return reinterpret_cast<const int*>(_data.constData());
	}

	/// \brief Returns a read-only pointer to the first float element in the property storage.
	/// \note This method may only be used if this property is of data type float.
	const FloatType* constDataFloat() const {
		OVITO_ASSERT(type() == qMetaTypeId<FloatType>());
		return reinterpret_cast<const FloatType*>(_data.constData());
	}

	/// \brief Returns a read-only pointer to the first vector element in the property storage.
	/// \note This method may only be used if this property is of data type Vector3 or a FloatType channel with 3 components.
	const Vector3* constDataVector3() const {
		OVITO_ASSERT(type() == qMetaTypeId<Vector3>() || (type() == qMetaTypeId<FloatType>() && componentCount() == 3));
		return reinterpret_cast<const Vector3*>(_data.constData());
	}

	/// \brief Returns a read-only pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Point3 or a FloatType channel with 3 components.
	const Point3* constDataPoint3() const {
		OVITO_ASSERT(type() == qMetaTypeId<Point3>() || (type() == qMetaTypeId<FloatType>() && componentCount() == 3));
		return reinterpret_cast<const Point3*>(_data.constData());
	}

	/// \brief Returns a read-only pointer to the first tensor element in the property storage.
	/// \note This method may only be used if this property is of data type Tensor2 or a FloatType channel with 9 components.
	const Tensor2* constDataTensor2() const {
		OVITO_ASSERT(type() == qMetaTypeId<Tensor2>() || (type() == qMetaTypeId<FloatType>() && componentCount() == 9));
		return reinterpret_cast<const Tensor2*>(_data.constData());
	}

	/// \brief Returns a read-only pointer to the first symmetric tensor element in the property storage.
	/// \note This method may only be used if this property is of data type SymmetricTensor2 or a FloatType channel with 6 components.
	const SymmetricTensor2* constDataSymmetricTensor2() const {
		OVITO_ASSERT(type() == qMetaTypeId<SymmetricTensor2>() || (type() == qMetaTypeId<FloatType>() && componentCount() == 6));
		return reinterpret_cast<const SymmetricTensor2*>(_data.constData());
	}

	/// \brief Returns a read-only pointer to the first quaternion element in the property storage.
	/// \note This method may only be used if this property is of data type Quaternion or a FloatType channel with 4 components.
	const Quaternion* constDataQuaternion() const {
		OVITO_ASSERT(type() == qMetaTypeId<Quaternion>() || (type() == qMetaTypeId<FloatType>() && componentCount() == 4));
		return reinterpret_cast<const Quaternion*>(_data.constData());
	}

	/// Returns a read-write pointer to the raw elements in the property storage.
	char* data() {
		return _data.data();
	}

	/// \brief Returns a read-write pointer to the first integer element stored in this object..
	/// \note This method may only be used if this property is of data type integer.
	int* dataInt() {
		OVITO_ASSERT(type() == qMetaTypeId<int>());
		return reinterpret_cast<int*>(_data.data());
	}

	/// \brief Returns a read-only pointer to the first float element in the property storage.
	/// \note This method may only be used if this property is of data type float.
	FloatType* dataFloat() {
		OVITO_ASSERT(type() == qMetaTypeId<FloatType>());
		return reinterpret_cast<FloatType*>(_data.data());
	}

	/// \brief Returns a read-write pointer to the first vector element in the property storage.
	/// \note This method may only be used if this property is of data type Vector3 or a FloatType channel with 3 components.
	Vector3* dataVector3() {
		OVITO_ASSERT(type() == qMetaTypeId<Vector3>() || (type() == qMetaTypeId<FloatType>() && componentCount() == 3));
		return reinterpret_cast<Vector3*>(_data.data());
	}

	/// \brief Returns a read-write pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Point3 or a FloatType channel with 3 components.
	Point3* dataPoint3() {
		OVITO_ASSERT(type() == qMetaTypeId<Point3>() || (type() == qMetaTypeId<FloatType>() && componentCount() == 3));
		return reinterpret_cast<Point3*>(_data.data());
	}

	/// \brief Returns a read-write pointer to the first tensor element in the property storage.
	/// \note This method may only be used if this property is of data type Tensor2 or a FloatType channel with 9 components.
	Tensor2* dataTensor2() {
		OVITO_ASSERT(type() == qMetaTypeId<Tensor2>() || (type() == qMetaTypeId<FloatType>() && componentCount() == 9));
		return reinterpret_cast<Tensor2*>(_data.data());
	}

	/// \brief Returns a read-write pointer to the first symmetric tensor element in the property storage.
	/// \note This method may only be used if this property is of data type SymmetricTensor2 or a FloatType channel with 6 components.
	SymmetricTensor2* dataSymmetricTensor2() {
		OVITO_ASSERT(type() == qMetaTypeId<SymmetricTensor2>() || (type() == qMetaTypeId<FloatType>() && componentCount() == 6));
		return reinterpret_cast<SymmetricTensor2*>(_data.data());
	}

	/// \brief Returns a read-write pointer to the first quaternion element in the property storage.
	/// \note This method may only be used if this property is of data type Quaternion or a FloatType channel with 4 components.
	Quaternion* dataQuaternion() {
		OVITO_ASSERT(type() == qMetaTypeId<Quaternion>() || (type() == qMetaTypeId<FloatType>() && componentCount() == 4));
		return reinterpret_cast<Quaternion*>(_data.data());
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

	/// \brief Sets the object title of this property object.
	void setObjectTitle(const QString& title) {
		_objectTitle = title;
		notifyDependents(ReferenceEvent::TitleChanged);
	}

	//////////////////////////////// from RefTarget //////////////////////////////

	/// \brief Returns whether this object, when returned as an editable sub-object by another object,
	///        should be displayed in the modification stack.
	///
	/// Default data channels cannot be edited and are hidden in the modifier stack.
	virtual bool isSubObjectEditable() const override { return false; }

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override {
		if(_objectTitle.value().isEmpty() == false) return _objectTitle;
		else return RefTarget::objectTitle();
	}

public:

	Q_PROPERTY(QString name READ name WRITE setName)
	Q_PROPERTY(size_t size READ size)
	Q_PROPERTY(Identifier id READ id)
	Q_PROPERTY(int type READ type)

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

	/// Copies the contents from the given source into this storage.
	/// Particles for which the bit in the given mask is set are skipped.
	virtual void filterCopy(const ParticleProperty& source, const std::vector<bool>& mask);

	/// The name of this property.
	PropertyField<QString, QString, ReferenceEvent::TitleChanged> _name;

	/// The title of this property.
	PropertyField<QString, QString, ReferenceEvent::TitleChanged> _objectTitle;

	/// The internal storage object that holds the elements.
	std::shared_ptr<ParticlePropertyStorage> _storage;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_name);
	DECLARE_PROPERTY_FIELD(_objectTitle);
};

/**
 * \brief A reference to a ParticleProperty
 *
 * This small helper class can be used to store a reference to a
 * particular property.
 */
class ParticlePropertyReference
{
public:

	/// \brief Default constructor.
	ParticlePropertyReference() : _id(ParticleProperty::UserProperty) {}
	/// \brief Constructor for references to standard property.
	ParticlePropertyReference(ParticleProperty::Identifier id) : _id(id), _name(ParticleProperty::standardPropertyName(id)) {}
	/// \brief Constructor for references to a property.
	ParticlePropertyReference(ParticleProperty::Identifier id, const QString& name) : _id(id), _name(name) {}
	/// \brief Constructor for references to user-defined properties.
	ParticlePropertyReference(const QString& name) : _id(ParticleProperty::UserProperty), _name(name) {}
	/// \brief Constructor for references to an existing property instance.
	ParticlePropertyReference(ParticleProperty* property) : _id(property->id()), _name(property->name()) {}

	/// \brief Gets the identifier of the referenced property.
	/// \return The property identifier.
	ParticleProperty::Identifier id() const { return _id; }

	/// \brief Sets the referenced channel.
	void setId(ParticleProperty::Identifier id) {
		_id = id;
		if(id != ParticleProperty::UserProperty)
			_name = ParticleProperty::standardPropertyName(id);
	}

	/// \brief Gets the human-readable name of the referenced property.
	/// \return The property name.
	const QString& name() const { return _name; }

	/// \brief Compares two references for equality.
	bool operator==(const ParticlePropertyReference& other) const {
		if(id() != other.id()) return false;
		if(id() != ParticleProperty::UserProperty) return true;
		return name() == other.name();
	}

	/// \brief Returns whether this reference object does not point to a ParticleProperty.
	bool isNull() const { return id() == ParticleProperty::UserProperty && name().isEmpty(); }

private:

	/// The identifier of the property.
	ParticleProperty::Identifier _id;

	/// The human-readable name of the property.
	/// It is only used for user-defined properties.
	QString _name;
};

/// Writes a ParticlePropertyReference to an output stream.
inline SaveStream& operator<<(SaveStream& stream, const ParticlePropertyReference& r)
{
	stream.writeEnum(r.id());
	stream << r.name();
	return stream;
}

/// Reads a ParticlePropertyReference from an input stream.
inline LoadStream& operator>>(LoadStream& stream, ParticlePropertyReference& r)
{
	ParticleProperty::Identifier id;
	QString name;
	stream.readEnum(id);
	stream >> name;
	if(id != ParticleProperty::UserProperty)
		r = ParticlePropertyReference(id);
	else
		r = ParticlePropertyReference(name);
	return stream;
}

};	// End of namespace

Q_DECLARE_METATYPE(Viz::ParticlePropertyReference)

#endif // __OVITO_PARTICLE_PROPERTY_OBJECT_H
