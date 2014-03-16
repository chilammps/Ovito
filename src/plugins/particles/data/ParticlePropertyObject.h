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
 * \brief Contains the definition of the Particles::ParticlePropertyObject class.
 */

#ifndef __OVITO_PARTICLE_PROPERTY_OBJECT_H
#define __OVITO_PARTICLE_PROPERTY_OBJECT_H

#include <plugins/particles/Particles.h>
#include <core/scene/objects/SceneObject.h>
#include <core/scene/pipeline/PipelineFlowState.h>
#include "ParticleProperty.h"

namespace Particles {

using namespace Ovito;

/**
 * \brief Storage for a per-particle property.
 */
class OVITO_PARTICLES_EXPORT ParticlePropertyObject : public SceneObject
{
public:

	/// \brief Creates an property object.
	Q_INVOKABLE ParticlePropertyObject(DataSet* dataset, ParticleProperty* storage = nullptr);

	/// \brief Factory function that creates a user-defined property object.
	/// \param particleCount The number of particles.
	/// \param dataType Specifies the data type (integer, floating-point, ...) of the per-particle elements
	///                 in the new property storage. The data type is specified as identifier according to the
	///                 Qt metatype system.
	/// \param dataTypeSize The size of the data type given by \a dataType in bytes.
	///                     This is necessary because the Qt type system has no function to query
	///                     the size of a data type at runtime.
	/// \param componentCount The number of components per particle of type \a dataType.
	/// \param name The name assigned to the property.
	static OORef<ParticlePropertyObject> create(DataSet* dataset, size_t particleCount, int dataType, size_t dataTypeSize, size_t componentCount, const QString& name);

	/// \brief Factory function that creates a standard property object.
	/// \param particleCount The number of particles.
	/// \param which Specifies which standard property should be created.
	///              This must not be ParticlePropertyIdentifier::UserProperty.
	/// \param componentCount The component count if this type of property
	///                       has a variable component count; otherwise 0 to use the
	///                       default number of components.
	static OORef<ParticlePropertyObject> create(DataSet* dataset, size_t particleCount, ParticleProperty::Type which, size_t componentCount = 0);

	/// \brief Factory function that creates a property object based on an existing storage.
	static OORef<ParticlePropertyObject> create(DataSet* dataset, ParticleProperty* storage);

	/// \brief Gets the property's name.
	/// \return The name of property, which is shown to the user.
	const QString& name() const { return _storage->name(); }

	/// \brief Sets the property's name.
	/// \param name The new name string.
	/// \undoable
	void setName(const QString& name);

	/// \brief Replaces the internal storage object with the given one.
	void setStorage(ParticleProperty* storage);

	/// \brief Returns the internal storage object.
	ParticleProperty* storage() const { return _storage.data(); }

	/// \brief This must be called every time the contents of the property are changed.
	///        It generates a ReferenceEvent::TargetChanged event.
	void changed() { notifyDependents(ReferenceEvent::TargetChanged); }

	/// \brief Returns the number of particles for which this object stores the properties.
	/// \return The total number of data elements in this channel divided by the
	///         number of elements per particle.
	size_t size() const { return _storage->size(); }

	/// \brief Resizes the property storage.
	/// \param newSize The new number of particles.
	void resize(size_t newSize) {
		_storage.detach();
		_storage->resize(newSize);
		changed();
	}

	/// \brief Returns the type of this property.
	ParticleProperty::Type type() const { return _storage->type(); }

	/// \brief Changes the type of this property.
	/// \note The type may only be changed if the new property has the same
	///       data type and component count as the old one.
	void setType(ParticleProperty::Type newType) {
		if(newType == type()) return;
		_storage.detach();
		_storage->setType(newType);
		changed();
	}

	/// \brief Returns the data type of the property.
	/// \return The identifier of the data type used for the elements stored in
	///         this property storage according to the Qt meta type system.
	int dataType() const { return _storage->dataType(); }

	/// \brief Returns the number of bytes per value.
	/// \return Number of bytes used to store a single value of the data type
	///         specified by type().
	size_t dataTypeSize() const { return _storage->dataTypeSize(); }

	/// \brief Returns the number of bytes used per particle.
	/// \return The size of the property' data type multiplied by the component count.
	size_t perParticleSize() const { return _storage->perParticleSize(); }

	/// \brief Returns the number of array elements per particle.
	/// \return The number of data values stored per particle in this storage object.
	size_t componentCount() const { return _storage->componentCount(); }

	/// \brief Returns the human-readable names for the components stored per atom.
	/// \return The names of the vector components if this channel contains more than one value per atom.
	///         If this is only a single valued channel then an empty list is returned by this method because
	///         then no naming is necessary.
	const QStringList& componentNames() const { return _storage->componentNames(); }

	/// \brief Returns the display name of the property including the name of the given
	///        vector component.
	QString nameWithComponent(int vectorComponent) const {
		if(componentCount() <= 1 || vectorComponent < 0)
			return name();
		else if(vectorComponent < componentNames().size())
			return QString("%1.%2").arg(name()).arg(componentNames()[vectorComponent]);
		else
			return QString("%1.%2").arg(name()).arg(vectorComponent + 1);
	}

	/// Copies the contents from the given source into this storage.
	/// Particles for which the bit in the given mask is set are skipped.
	void filterCopy(ParticlePropertyObject* source, const std::vector<bool>& mask) {
		_storage.detach();
		_storage->filterCopy(*source->_storage.constData(), mask);
		changed();
	}

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
	/// \note This method may only be used if this property is of data type Point3I or an integer channel with 3 components.
	const Point3I* constDataPoint3I() const {
		return _storage->constDataPoint3I();
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

	/// \brief Returns a range of const iterators over the elements stored in this object.
	ParticleProperty::Range<const int*> constIntRange() const {
		return _storage->constIntRange();
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	ParticleProperty::Range<const FloatType*> constFloatRange() const {
		return _storage->constFloatRange();
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	ParticleProperty::Range<const Point3*> constPoint3Range() const {
		return _storage->constPoint3Range();
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	ParticleProperty::Range<const Vector3*> constVector3Range() const {
		return _storage->constVector3Range();
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	ParticleProperty::Range<const Color*> constColorRange() const {
		return _storage->constColorRange();
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	ParticleProperty::Range<const Point3I*> constPoint3IRange() const {
		return _storage->constPoint3IRange();
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	ParticleProperty::Range<const Tensor2*> constTensor2Range() const {
		return _storage->constTensor2Range();
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	ParticleProperty::Range<const SymmetricTensor2*> constSymmetricTensor2Range() const {
		return _storage->constSymmetricTensor2Range();
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	ParticleProperty::Range<const Quaternion*> constQuaternionRange() const {
		return _storage->constQuaternionRange();
	}

	/// Returns a read-write pointer to the raw elements in the property storage.
	void* data() {
		_storage.detach();
		return _storage->data();
	}

	/// \brief Returns a read-write pointer to the first integer element stored in this object..
	/// \note This method may only be used if this property is of data type integer.
	int* dataInt() {
		_storage.detach();
		return _storage->dataInt();
	}

	/// \brief Returns a read-only pointer to the first float element in the property storage.
	/// \note This method may only be used if this property is of data type float.
	FloatType* dataFloat() {
		_storage.detach();
		return _storage->dataFloat();
	}

	/// \brief Returns a read-write pointer to the first vector element in the property storage.
	/// \note This method may only be used if this property is of data type Vector3 or a FloatType channel with 3 components.
	Vector3* dataVector3() {
		_storage.detach();
		return _storage->dataVector3();
	}

	/// \brief Returns a read-write pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Point3 or a FloatType channel with 3 components.
	Point3* dataPoint3() {
		_storage.detach();
		return _storage->dataPoint3();
	}

	/// \brief Returns a read-write pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Point3I or an integer channel with 3 components.
	Point3I* dataPoint3I() {
		_storage.detach();
		return _storage->dataPoint3I();
	}

	/// \brief Returns a read-write pointer to the first point element in the property storage.
	/// \note This method may only be used if this property is of data type Color or a FloatType channel with 3 components.
	Color* dataColor() {
		_storage.detach();
		return _storage->dataColor();
	}

	/// \brief Returns a read-write pointer to the first tensor element in the property storage.
	/// \note This method may only be used if this property is of data type Tensor2 or a FloatType channel with 9 components.
	Tensor2* dataTensor2() {
		_storage.detach();
		return _storage->dataTensor2();
	}

	/// \brief Returns a read-write pointer to the first symmetric tensor element in the property storage.
	/// \note This method may only be used if this property is of data type SymmetricTensor2 or a FloatType channel with 6 components.
	SymmetricTensor2* dataSymmetricTensor2() {
		_storage.detach();
		return _storage->dataSymmetricTensor2();
	}

	/// \brief Returns a read-write pointer to the first quaternion element in the property storage.
	/// \note This method may only be used if this property is of data type Quaternion or a FloatType channel with 4 components.
	Quaternion* dataQuaternion() {
		_storage.detach();
		return _storage->dataQuaternion();
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	ParticleProperty::Range<int*> intRange() {
		_storage.detach();
		return _storage->intRange();
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	ParticleProperty::Range<FloatType*> floatRange() {
		_storage.detach();
		return _storage->floatRange();
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	ParticleProperty::Range<Point3*> point3Range() {
		_storage.detach();
		return _storage->point3Range();
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	ParticleProperty::Range<Vector3*> vector3Range() {
		_storage.detach();
		return _storage->vector3Range();
	}

	/// \brief Returns a range of const iterators over the elements stored in this object.
	ParticleProperty::Range<Color*> colorRange() {
		_storage.detach();
		return _storage->colorRange();
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	ParticleProperty::Range<Point3I*> point3IRange() {
		_storage.detach();
		return _storage->point3IRange();
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	ParticleProperty::Range<Tensor2*> tensor2Range() {
		_storage.detach();
		return _storage->tensor2Range();
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	ParticleProperty::Range<SymmetricTensor2*> symmetricTensor2Range() {
		_storage.detach();
		return _storage->symmetricTensor2Range();
	}

	/// \brief Returns a range of iterators over the elements stored in this object.
	ParticleProperty::Range<Quaternion*> quaternionRange() {
		_storage.detach();
		return _storage->quaternionRange();
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

	/// Returns a Point3I element at the given index (if this is a point property).
	const Point3I& getPoint3I(size_t particleIndex) const {
		return _storage->getPoint3I(particleIndex);
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
		_storage.detach();
		_storage->setInt(particleIndex, newValue);
	}

	/// Sets the value of a float element at the given index (if this is a float property).
	void setFloat(size_t particleIndex, FloatType newValue) {
		_storage.detach();
		_storage->setFloat(particleIndex, newValue);
	}

	/// Sets the value of an integer element at the given index (if this is an integer property).
	void setIntComponent(size_t particleIndex, size_t componentIndex, int newValue) {
		_storage.detach();
		_storage->setIntComponent(particleIndex, componentIndex, newValue);
	}

	/// Sets the value of a float element at the given index (if this is a float property).
	void setFloatComponent(size_t particleIndex, size_t componentIndex, FloatType newValue) {
		_storage.detach();
		_storage->setFloatComponent(particleIndex, componentIndex, newValue);
	}

	/// Sets the value of a Vector3 element at the given index (if this is a vector property).
	void setVector3(size_t particleIndex, const Vector3& newValue) {
		_storage.detach();
		_storage->setVector3(particleIndex, newValue);
	}

	/// Sets the value of a Point3 element at the given index (if this is a point property).
	void setPoint3(size_t particleIndex, const Point3& newValue) {
		_storage.detach();
		_storage->setPoint3(particleIndex, newValue);
	}

	/// Sets the value of a Point3I element at the given index (if this is a point property).
	void setPoint3I(size_t particleIndex, const Point3I& newValue) {
		_storage.detach();
		_storage->setPoint3I(particleIndex, newValue);
	}

	/// Sets the value of a Color element at the given index (if this is a point property).
	void setColor(size_t particleIndex, const Color& newValue) {
		_storage.detach();
		_storage->setColor(particleIndex, newValue);
	}

	/// Sets the value of a Tensor2 element for the given particle.
	void setTensor2(size_t particleIndex, const Tensor2& newValue) {
		_storage.detach();
		_storage->setTensor2(particleIndex, newValue);
	}

	/// Sets the value of a SymmetricTensor2 element for the given particle.
	void setSymmetricTensor2(size_t particleIndex, const SymmetricTensor2& newValue) {
		_storage.detach();
		_storage->setSymmetricTensor2(particleIndex, newValue);
	}

	/// Sets the value of a Quaternion element for the given particle.
	void setQuaternion(size_t particleIndex, const Quaternion& newValue) {
		_storage.detach();
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
		if(type() == ParticleProperty::UserProperty)
			return name();
		else
			return ParticleProperty::standardPropertyTitle(type());
	}

	/// This helper method returns a standard particle property (if present) from the given pipeline state.
	static ParticlePropertyObject* findInState(const PipelineFlowState& state, ParticleProperty::Type type);

	/// This helper method returns a specific user-defined particle property (if present) from the given pipeline state.
	static ParticlePropertyObject* findInState(const PipelineFlowState& state, const QString& name);

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
	QExplicitlySharedDataPointer<ParticleProperty> _storage;

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
	ParticlePropertyReference() : _type(ParticleProperty::UserProperty), _vectorComponent(-1) {}
	/// \brief Constructor for references to standard property.
	ParticlePropertyReference(ParticleProperty::Type type, int vectorComponent = -1) : _type(type), _name(ParticleProperty::standardPropertyName(type)), _vectorComponent(vectorComponent) {}
	/// \brief Constructor for references to a property.
	ParticlePropertyReference(ParticleProperty::Type type, const QString& name, int vectorComponent = -1) : _type(type), _name(name), _vectorComponent(vectorComponent) {}
	/// \brief Constructor for references to user-defined properties.
	ParticlePropertyReference(const QString& name, int vectorComponent = -1) : _type(ParticleProperty::UserProperty), _name(name), _vectorComponent(vectorComponent) {}
	/// \brief Constructor for references to an existing property instance.
	ParticlePropertyReference(ParticleProperty* property, int vectorComponent = -1) : _type(property->type()), _name(property->name()), _vectorComponent(vectorComponent) {}
	/// \brief Constructor for references to an existing property instance.
	ParticlePropertyReference(ParticlePropertyObject* property, int vectorComponent = -1) : _type(property->type()), _name(property->name()), _vectorComponent(vectorComponent) {}

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

	/// Returns the selected component index if the property is a vector property.
	int vectorComponent() const { return _vectorComponent; }

	/// Selects a component index if the property is a vector property.
	void setVectorComponent(int index) { _vectorComponent = index; }

	/// \brief Compares two references for equality.
	bool operator==(const ParticlePropertyReference& other) const {
		if(type() != other.type()) return false;
		if(vectorComponent() != other.vectorComponent()) return false;
		if(type() != ParticleProperty::UserProperty) return true;
		return name() == other.name();
	}

	/// \brief Returns whether this reference object does not point to a ParticleProperty.
	bool isNull() const { return type() == ParticleProperty::UserProperty && name().isEmpty(); }

	/// This helper method find the particle property referenced by this ParticlePropertyReference
	/// in the given pipeline state.
	ParticlePropertyObject* findInState(const PipelineFlowState& state) const;

private:

	/// The type identifier of the property.
	ParticleProperty::Type _type;

	/// The human-readable name of the property.
	/// It is only used for user-defined properties.
	QString _name;

	/// The component index if the property is a vector property.
	int _vectorComponent;
};

/// Writes a ParticlePropertyReference to an output stream.
inline SaveStream& operator<<(SaveStream& stream, const ParticlePropertyReference& r)
{
	stream.writeEnum(r.type());
	stream << r.name();
	stream << r.vectorComponent();
	return stream;
}

/// Reads a ParticlePropertyReference from an input stream.
inline LoadStream& operator>>(LoadStream& stream, ParticlePropertyReference& r)
{
	ParticleProperty::Type type;
	QString name;
	stream.readEnum(type);
	stream >> name;
	int vecComponent;
	stream >> vecComponent;
	if(type != ParticleProperty::UserProperty)
		r = ParticlePropertyReference(type, vecComponent);
	else
		r = ParticlePropertyReference(name, vecComponent);
	return stream;
}

};	// End of namespace

Q_DECLARE_METATYPE(Particles::ParticlePropertyReference);

#endif // __OVITO_PARTICLE_PROPERTY_OBJECT_H
