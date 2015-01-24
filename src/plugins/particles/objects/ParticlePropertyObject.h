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
 * \file
 * \brief Contains the definition of the Particles::ParticlePropertyObject class.
 */

#ifndef __OVITO_PARTICLE_PROPERTY_OBJECT_H
#define __OVITO_PARTICLE_PROPERTY_OBJECT_H

#include <plugins/particles/Particles.h>
#include <core/scene/objects/DataObject.h>
#include <core/scene/pipeline/PipelineFlowState.h>
#include <plugins/particles/data/ParticleProperty.h>

namespace Ovito { namespace Particles {

/**
 * \brief Stores one particle property.
 *
 * The ParticlePropertyObject class stores the data of one particle property (which may consist
 * of multiple values per particle if it is a vector property).
 *
 * An entire particle dataset usually consists of multiple ParticlePropertyObject instances, each storing a
 * different property such as position, type, identifier etc. A particle dataset is normally kept
 * in a PipelineFlowState structure, which consists of a collection of data objects (with some of them
 * being ParticlePropertyObject instances and perhaps a instance of the SimulationCellObject class).
 *
 * The ParticlePropertyObject class keeps the actual per-particle data in an internal storage object
 * (see ParticleProperty class). The reason is that ParticlePropertyObject instances can only be created and
 * accessed from the main thread while ParticleProperty storage objects can be used by background threads
 * too (e.g. when loading data from a file).
 *
 */
class OVITO_PARTICLES_EXPORT ParticlePropertyObject : public DataObject
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
	/// \param stride The number of bytes per particle.
	/// \param name The name assigned to the property.
	/// \param initializeMemory Controls whether the newly allocated memory is initialized with zeros.
	static OORef<ParticlePropertyObject> createUserProperty(DataSet* dataset, size_t particleCount, int dataType, size_t dataTypeSize, size_t componentCount, size_t stride, const QString& name, bool initializeMemory);

	/// \brief Factory function that creates a standard property object.
	/// \param particleCount The number of particles.
	/// \param which Specifies which standard property should be created.
	///              This must not be ParticlePropertyIdentifier::UserProperty.
	/// \param componentCount The component count if this type of property
	///                       has a variable component count; otherwise 0 to use the
	///                       default number of components.
	/// \param initializeMemory Controls whether the newly allocated memory is initialized with zeros.
	static OORef<ParticlePropertyObject> createStandardProperty(DataSet* dataset, size_t particleCount, ParticleProperty::Type which, size_t componentCount, bool initializeMemory);

	/// \brief Factory function that creates a property object based on an existing storage.
	static OORef<ParticlePropertyObject> createFromStorage(DataSet* dataset, ParticleProperty* storage);

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
	/// \param preserveData Controls whether the existing per-particle data is preserved.
	///                     This also determines whether newly allocated memory is initialized to zero.
	void resize(size_t newSize, bool preserveData);

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
	size_t stride() const { return _storage->stride(); }

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
	void filterCopy(ParticlePropertyObject* source, const boost::dynamic_bitset<>& mask) {
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
 * \brief A reference to a particle property.
 *
 * This class is a reference to a particle property. For instance, it is used by modifiers
 * to store the input property selected by the user, which they will act upon. When the modifier
 * is evaluated, the particle property reference is resolved by looking up the corresponding ParticlePropertyObject
 * from the current input dataset, which contains the actual per-particle data.
 *
 * A ParticlePropertyReference consists of the ParticleProperty::Type identifier, the name of the property
 * (only used for user-defined properties), and an optional vector component (can be -1 to indicate that the entire
 * vector property is referenced).
 *
 * \sa ParticleProperty, ParticlePropertyObject
 */
class ParticlePropertyReference
{
public:

	/// \brief Default constructor. Creates an empty reference.
	ParticlePropertyReference() : _type(ParticleProperty::UserProperty), _vectorComponent(-1) {}
	/// \brief Constructs a reference to a standard property.
	ParticlePropertyReference(ParticleProperty::Type type, int vectorComponent = -1) : _type(type), _name(ParticleProperty::standardPropertyName(type)), _vectorComponent(vectorComponent) {}
	/// \brief Constructs a reference to a property.
	ParticlePropertyReference(ParticleProperty::Type type, const QString& name, int vectorComponent = -1) : _type(type), _name(name), _vectorComponent(vectorComponent) {}
	/// \brief Constructs a reference to a user-defined property.
	ParticlePropertyReference(const QString& name, int vectorComponent = -1) : _type(ParticleProperty::UserProperty), _name(name), _vectorComponent(vectorComponent) {}
	/// \brief Constructs a reference based on an existing ParticleProperty.
	ParticlePropertyReference(ParticleProperty* property, int vectorComponent = -1) : _type(property->type()), _name(property->name()), _vectorComponent(vectorComponent) {}
	/// \brief Constructs a reference based on an existing ParticlePropertyObject.
	ParticlePropertyReference(ParticlePropertyObject* property, int vectorComponent = -1) : _type(property->type()), _name(property->name()), _vectorComponent(vectorComponent) {}

	/// \brief Returns the type of property being referenced.
	ParticleProperty::Type type() const { return _type; }

	/// \brief Sets the type of property being referenced.
	void setType(ParticleProperty::Type type) {
		_type = type;
		if(type != ParticleProperty::UserProperty)
			_name = ParticleProperty::standardPropertyName(type);
	}

	/// \brief Gets the human-readable name of the referenced property.
	/// \return The property name.
	const QString& name() const { return _name; }

	/// Returns the selected component index.
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

	/// \brief Compares two references for inequality.
	bool operator!=(const ParticlePropertyReference& other) const { return !(*this == other); }

	/// \brief Returns true if this reference does not point to any particle property.
	/// \return true if this is a default constructed ParticlePropertyReference.
	bool isNull() const { return type() == ParticleProperty::UserProperty && name().isEmpty(); }

	/// \brief This method retrieves the actual particle property from a pipeline state.
	/// \return The actual particle property after resolving this reference; or NULL if the property does not exist.
	ParticlePropertyObject* findInState(const PipelineFlowState& state) const;

	/// \brief Returns the display name of the referenced property including the optional vector component.
	QString nameWithComponent() const {
		if(type() != ParticleProperty::UserProperty) {
			if(vectorComponent() < 0 || ParticleProperty::standardPropertyComponentCount(type()) <= 1) {
				return name();
			}
			else {
				QStringList names = ParticleProperty::standardPropertyComponentNames(type());
				if(vectorComponent() < names.size())
					return QString("%1.%2").arg(name()).arg(names[vectorComponent()]);
			}
		}
		if(vectorComponent() < 0)
			return name();
		else
			return QString("%1.%2").arg(name()).arg(vectorComponent() + 1);
	}

private:

	/// The type of the property.
	ParticleProperty::Type _type;

	/// The human-readable name of the property.
	QString _name;

	/// The zero-based component index if the property is a vector property (or zero if not a vector property).
	int _vectorComponent;
};

/// Writes a ParticlePropertyReference to an output stream.
/// \relates ParticlePropertyReference
inline SaveStream& operator<<(SaveStream& stream, const ParticlePropertyReference& r)
{
	stream << r.type();
	stream << r.name();
	stream << r.vectorComponent();
	return stream;
}

/// Reads a ParticlePropertyReference from an input stream.
/// \relates ParticlePropertyReference
inline LoadStream& operator>>(LoadStream& stream, ParticlePropertyReference& r)
{
	ParticleProperty::Type type;
	QString name;
	stream >> type;
	stream >> name;
	int vecComponent;
	stream >> vecComponent;
	if(type != ParticleProperty::UserProperty)
		r = ParticlePropertyReference(type, vecComponent);
	else
		r = ParticlePropertyReference(name, vecComponent);
	return stream;
}

}	// End of namespace
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Particles::ParticlePropertyReference);

#endif // __OVITO_PARTICLE_PROPERTY_OBJECT_H
