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
#include "ParticleProperty.h"

namespace Viz {

using namespace Ovito;

/**
 * \brief Storage for a per-particle property.
 */
class ParticlePropertyObject : public SceneObject
{
public:

	/// \brief Creates an property object.
	Q_INVOKABLE ParticlePropertyObject(ParticleProperty* storage = nullptr);

	/// \brief Factory function that creates a user-defined property object.
	/// \param particleCount The number of particles.
	/// \param dataType Specifies the data type (integer, floating-point, ...) of the per-particle elements
	///                 in the new property storage. The data type is specified as identifier according to the
	///                 Qt metatype system.
	/// \param dataTypeSize The size of the data type given by \a dataType in bytes.
	///                     This is necessary because the Qt type system has no function to query
	///                     the size of a data type at runtime.
	/// \param componentCount The number of components per particle of type \a dataType.
	static OORef<ParticlePropertyObject> create(size_t particleCount, int dataType, size_t dataTypeSize, size_t componentCount);

	/// \brief Factory function that creates a standard property object.
	/// \param particleCount The number of particles.
	/// \param which Specifies which standard property should be created.
	///              This must not be ParticlePropertyIdentifier::UserProperty.
	/// \param componentCount The component count if this type of property
	///                       has a variable component count; otherwise 0 to use the
	///                       default number of components.
	static OORef<ParticlePropertyObject> create(size_t particleCount, ParticleProperty::Type which, size_t componentCount = 0);

	/// \brief Factory function that creates a property object based on an existing storage.
	static OORef<ParticlePropertyObject> create(ParticleProperty* storage);

	/// \brief Gets the property's name.
	/// \return The name of property, which is shown to the user.
	const QString& name() const { return _storage->name(); }

	/// \brief Sets the property's name.
	/// \param name The new name string.
	/// \undoable
	void setName(const QString& name);

	/// \brief Replaces the internal storage object with the given one.
	void replaceStorage(ParticleProperty* storage);

	/// \brief Returns the number of particles for which this object stores the properties.
	/// \return The total number of data elements in this channel divided by the
	///         number of elements per particle.
	size_t size() const { return _storage->size(); }

	/// \brief Resizes the property storage.
	/// \param newSize The new number of particles.
	void resize(size_t newSize) { _storage->resize(newSize); }

	/// \brief Returns the type of this property.
	ParticleProperty::Type type() const { return _storage->type(); }

	/// \brief Returns the data type of the property.
	/// \return The identifier of the data type used for the elements stored in
	///         this property storage according to the Qt meta type system.
	int dataType() const { return _storage->dataType(); }

	/// \brief Returns the number of array elements per particle.
	/// \return The number of data values stored per particle in this storage object.
	size_t componentCount() const { return _storage->componentCount(); }

	/// \brief Returns the human-readable names for the components stored per atom.
	/// \return The names of the vector components if this channel contains more than one value per atom.
	///         If this is only a single valued channel then an empty list is returned by this method because
	///         then no naming is necessary.
	const QStringList& componentNames() const { return _storage->componentNames(); }

	/// \brief Returns a read-only pointer to the raw elements stored in this property object.
	const void* constData() const {
		return _storage->constData();
	}

	/// \brief Returns a read-only pointer to the first integer element stored in this object..
	/// \note This method may only be used if this property is of data type integer.
	const int* constDataInt() const {
		return _storage->constDataInt();
	}

	/// \brief Returns a read-only pointer to the first float element in the property storage.
	/// \note This method may only be used if this property is of data type float.
	const FloatType* constDataFloat() const {
		return _storage->constDataFloat();
	}

	/// \brief Returns a read-only pointer to the first vector element in the property storage.
	/// \note This method may only be used if this property is of data type Vector3 or a FloatType channel with 3 components.
	const Vector3* constDataVector3() const {
		return _storage->constDataVector3();
	}

	/// \brief Returns a read-only pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Point3 or a FloatType channel with 3 components.
	const Point3* constDataPoint3() const {
		return _storage->constDataPoint3();
	}

	/// \brief Returns a read-only pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Color or a FloatType channel with 3 components.
	const Color* constDataColor() const {
		return _storage->constDataColor();
	}

	/// \brief Returns a read-only pointer to the first tensor element in the property storage.
	/// \note This method may only be used if this property is of data type Tensor2 or a FloatType channel with 9 components.
	const Tensor2* constDataTensor2() const {
		return _storage->constDataTensor2();
	}

	/// \brief Returns a read-only pointer to the first symmetric tensor element in the property storage.
	/// \note This method may only be used if this property is of data type SymmetricTensor2 or a FloatType channel with 6 components.
	const SymmetricTensor2* constDataSymmetricTensor2() const {
		return _storage->constDataSymmetricTensor2();
	}

	/// \brief Returns a read-only pointer to the first quaternion element in the property storage.
	/// \note This method may only be used if this property is of data type Quaternion or a FloatType channel with 4 components.
	const Quaternion* constDataQuaternion() const {
		return _storage->constDataQuaternion();
	}

	/// Returns a read-write pointer to the raw elements in the property storage.
	void* data() {
		return _storage->data();
	}

	/// \brief Returns a read-write pointer to the first integer element stored in this object..
	/// \note This method may only be used if this property is of data type integer.
	int* dataInt() {
		return _storage->dataInt();
	}

	/// \brief Returns a read-only pointer to the first float element in the property storage.
	/// \note This method may only be used if this property is of data type float.
	FloatType* dataFloat() {
		return _storage->dataFloat();
	}

	/// \brief Returns a read-write pointer to the first vector element in the property storage.
	/// \note This method may only be used if this property is of data type Vector3 or a FloatType channel with 3 components.
	Vector3* dataVector3() {
		return _storage->dataVector3();
	}

	/// \brief Returns a read-write pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Point3 or a FloatType channel with 3 components.
	Point3* dataPoint3() {
		return _storage->dataPoint3();
	}

	/// \brief Returns a read-write pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Color or a FloatType channel with 3 components.
	Color* dataColor() {
		return _storage->dataColor();
	}

	/// \brief Returns a read-write pointer to the first tensor element in the property storage.
	/// \note This method may only be used if this property is of data type Tensor2 or a FloatType channel with 9 components.
	Tensor2* dataTensor2() {
		return _storage->dataTensor2();
	}

	/// \brief Returns a read-write pointer to the first symmetric tensor element in the property storage.
	/// \note This method may only be used if this property is of data type SymmetricTensor2 or a FloatType channel with 6 components.
	SymmetricTensor2* dataSymmetricTensor2() {
		return _storage->dataSymmetricTensor2();
	}

	/// \brief Returns a read-write pointer to the first quaternion element in the property storage.
	/// \note This method may only be used if this property is of data type Quaternion or a FloatType channel with 4 components.
	Quaternion* dataQuaternion() {
		return _storage->dataQuaternion();
	}

	/// \brief Returns an integer element at the given index (if this is an integer property).
	int getInt(size_t particleIndex) const {
		return _storage->getInt(particleIndex);
	}

	/// Returns a float element at the given index (if this is a float property).
	FloatType getFloat(size_t particleIndex) const {
		return _storage->getFloat(particleIndex);
	}

	/// Returns an integer element at the given index (if this is an integer property).
	int getIntComponent(size_t particleIndex, size_t componentIndex) const {
		return _storage->getIntComponent(particleIndex, componentIndex);
	}

	/// Returns a float element at the given index (if this is a float property).
	FloatType getFloatComponent(size_t particleIndex, size_t componentIndex) const {
		return _storage->getFloatComponent(particleIndex, componentIndex);
	}

	/// Returns a Vector3 element at the given index (if this is a vector property).
	const Vector3& getVector3(size_t particleIndex) const {
		return _storage->getVector3(particleIndex);
	}

	/// Returns a Point3 element at the given index (if this is a point property).
	const Point3& getPoint3(size_t particleIndex) const {
		return _storage->getPoint3(particleIndex);
	}

	/// Returns a Color element at the given index (if this is a point property).
	const Color& getColor(size_t particleIndex) const {
		return _storage->getColor(particleIndex);
	}

	/// Returns a Tensor2 element stored for the given particle.
	const Tensor2& getTensor2(size_t particleIndex) const {
		return _storage->getTensor2(particleIndex);
	}

	/// Returns a SymmetricTensor2 element stored for the given particle.
	const SymmetricTensor2& getSymmetricTensor2(size_t particleIndex) const {
		return _storage->getSymmetricTensor2(particleIndex);
	}

	/// Returns a Quaternion element stored for the given particle.
	const Quaternion& getQuaternion(size_t particleIndex) const {
		return _storage->getQuaternion(particleIndex);
	}

	/// Sets the value of an integer element at the given index (if this is an integer property).
	void setInt(size_t particleIndex, int newValue) {
		_storage->setInt(particleIndex, newValue);
	}

	/// Sets the value of a float element at the given index (if this is a float property).
	void setFloat(size_t particleIndex, FloatType newValue) {
		_storage->setFloat(particleIndex, newValue);
	}

	/// Sets the value of an integer element at the given index (if this is an integer property).
	void setIntComponent(size_t particleIndex, size_t componentIndex, int newValue) {
		_storage->setIntComponent(particleIndex, componentIndex, newValue);
	}

	/// Sets the value of a float element at the given index (if this is a float property).
	void setFloatComponent(size_t particleIndex, size_t componentIndex, FloatType newValue) {
		_storage->setFloatComponent(particleIndex, componentIndex, newValue);
	}

	/// Sets the value of a Vector3 element at the given index (if this is a vector property).
	void setVector3(size_t particleIndex, const Vector3& newValue) {
		_storage->setVector3(particleIndex, newValue);
	}

	/// Sets the value of a Point3 element at the given index (if this is a point property).
	void setPoint3(size_t particleIndex, const Point3& newValue) {
		_storage->setPoint3(particleIndex, newValue);
	}

	/// Sets the value of a Color element at the given index (if this is a point property).
	void setColor(size_t particleIndex, const Color& newValue) {
		_storage->setColor(particleIndex, newValue);
	}

	/// Sets the value of a Tensor2 element for the given particle.
	void setTensor2(size_t particleIndex, const Tensor2& newValue) {
		_storage->setTensor2(particleIndex, newValue);
	}

	/// Sets the value of a SymmetricTensor2 element for the given particle.
	void setSymmetricTensor2(size_t particleIndex, const SymmetricTensor2& newValue) {
		_storage->setSymmetricTensor2(particleIndex, newValue);
	}

	/// Sets the value of a Quaternion element for the given particle.
	void setQuaternion(size_t particleIndex, const Quaternion& newValue) {
		_storage->setQuaternion(particleIndex, newValue);
	}

	//////////////////////////////// from RefTarget //////////////////////////////

	/// \brief Returns whether this object, when returned as an editable sub-object by another object,
	///        should be displayed in the modification stack.
	///
	/// This implementation returns false because standard particle properties cannot be edited and
	/// are hidden in the modifier stack.
	virtual bool isSubObjectEditable() const override { return false; }

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override {
		return name();
	}

public:

	Q_PROPERTY(QString name READ name WRITE setName)
	Q_PROPERTY(size_t size READ size WRITE resize)
	Q_PROPERTY(int dataType READ dataType)

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

	/// The internal storage object that holds the elements.
	QSharedDataPointer<ParticleProperty> _storage;

private:

	Q_OBJECT
	OVITO_OBJECT
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
	ParticlePropertyReference() : _type(ParticleProperty::UserProperty) {}
	/// \brief Constructor for references to standard property.
	ParticlePropertyReference(ParticleProperty::Type type) : _type(type), _name(ParticleProperty::standardPropertyName(type)) {}
	/// \brief Constructor for references to a property.
	ParticlePropertyReference(ParticleProperty::Type type, const QString& name) : _type(type), _name(name) {}
	/// \brief Constructor for references to user-defined properties.
	ParticlePropertyReference(const QString& name) : _type(ParticleProperty::UserProperty), _name(name) {}
	/// \brief Constructor for references to an existing property instance.
	ParticlePropertyReference(ParticleProperty* property) : _type(property->type()), _name(property->name()) {}
	/// \brief Constructor for references to an existing property instance.
	ParticlePropertyReference(ParticlePropertyObject* property) : _type(property->type()), _name(property->name()) {}

	/// \brief Gets the type identifier of the referenced property.
	/// \return The property type.
	ParticleProperty::Type type() const { return _type; }

	/// \brief Sets the type of referenced property.
	void setType(ParticleProperty::Type type) {
		_type = type;
		if(type != ParticleProperty::UserProperty)
			_name = ParticleProperty::standardPropertyName(type);
	}

	/// \brief Gets the human-readable name of the referenced property.
	/// \return The property name.
	const QString& name() const { return _name; }

	/// \brief Compares two references for equality.
	bool operator==(const ParticlePropertyReference& other) const {
		if(type() != other.type()) return false;
		if(type() != ParticleProperty::UserProperty) return true;
		return name() == other.name();
	}

	/// \brief Returns whether this reference object does not point to a ParticleProperty.
	bool isNull() const { return type() == ParticleProperty::UserProperty && name().isEmpty(); }

private:

	/// The type identifier of the property.
	ParticleProperty::Type _type;

	/// The human-readable name of the property.
	/// It is only used for user-defined properties.
	QString _name;
};

/// Writes a ParticlePropertyReference to an output stream.
inline SaveStream& operator<<(SaveStream& stream, const ParticlePropertyReference& r)
{
	stream.writeEnum(r.type());
	stream << r.name();
	return stream;
}

/// Reads a ParticlePropertyReference from an input stream.
inline LoadStream& operator>>(LoadStream& stream, ParticlePropertyReference& r)
{
	ParticleProperty::Type type;
	QString name;
	stream.readEnum(type);
	stream >> name;
	if(type != ParticleProperty::UserProperty)
		r = ParticlePropertyReference(type);
	else
		r = ParticlePropertyReference(name);
	return stream;
}

};	// End of namespace

Q_DECLARE_METATYPE(Viz::ParticlePropertyReference)

#endif // __OVITO_PARTICLE_PROPERTY_OBJECT_H
