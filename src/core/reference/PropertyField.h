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

#ifndef __OVITO_PROPERTY_FIELD_H
#define __OVITO_PROPERTY_FIELD_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>
#include <core/dataset/UndoStack.h>
#include <core/reference/PropertyFieldDescriptor.h>
#include <core/reference/ReferenceEvent.h>
#include <core/plugins/Plugin.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

/**
 * \brief RefMaker derived classes use this implement properties and reference fields.
 */
class OVITO_CORE_EXPORT PropertyFieldBase
{
public:

#ifdef OVITO_DEBUG
	/// One has to call init() once to fully setup this property field.
	PropertyFieldBase() : _owner(nullptr), _descriptor(nullptr) {}
#endif

	/// Connects the property field to its owning RefMaker derived class.
	/// This function must be called in the constructor of the RefMaker derived class for each of its property fields.
	void init(RefMaker* owner, PropertyFieldDescriptor* descriptor);

	/// Returns the owner of this property field.
	inline RefMaker* owner() const {
		OVITO_ASSERT_MSG(_owner != nullptr, "PropertyFieldBase", "The PropertyField object has not been initialized yet.");
		return _owner;
	}

	/// Returns descriptor of this property field.
	inline PropertyFieldDescriptor* descriptor() const {
		OVITO_ASSERT_MSG(_descriptor != nullptr, "PropertyFieldBase", "The PropertyField object has not been initialized yet.");
		return _descriptor;
	}

protected:

	/// Generates a notification event to inform the dependents of the field's owner that it has changed.
	void generateTargetChangedEvent(ReferenceEvent::Type eventType = ReferenceEvent::TargetChanged);

	/// Generates a notification event to inform the dependents of the field's owner that it has changed.
	void generatePropertyChangedEvent() const;

private:

	/// The reference maker this object is a member of.
	/// This will be initialized after a call to init().
	RefMaker* _owner;

	/// The descriptor of this property field.
	PropertyFieldDescriptor* _descriptor;
};

/**
 * \brief Stores a non-animatable property of a RefTarget derived class.
 */
template<typename property_data_type, typename qvariant_data_type = property_data_type, int additionalChangeMessage = 0>
class PropertyField : public PropertyFieldBase
{
public:
	typedef property_data_type property_type;
	typedef qvariant_data_type qvariant_type;

	/// Forwarding constructor.
	template<class... Args>
	PropertyField(Args&&... args) : PropertyFieldBase(), _value(std::forward<Args>(args)...) {}

	/// Connects the property field to its owning RefMaker derived class.
	inline void init(RefMaker* owner, PropertyFieldDescriptor* descriptor) {
		PropertyFieldBase::init(owner, descriptor);

		// For enum types, the QVariant data type must always be set to 'int'.
		static_assert(!std::is_enum<property_data_type>::value || std::is_same<qvariant_data_type, int>::value,
				"When using PropertyField<> with an enum type, the QVariant data type must always be set to 'int'. For example: PropertyField<MyEnumType, int>");
	}

	/// Cast the property field to the property value.
	inline operator const property_type&() const { return _value; }

	/// Changes the value of the property. Handles undo and sends a notification message.
	PropertyField& operator=(const property_type& newValue) {
		if(_value == newValue) return *this;
		if(descriptor()->automaticUndo() && owner()->dataset()->undoStack().isRecording())
			owner()->dataset()->undoStack().push(new PropertyChangeOperation(*this));
		setPropertyValue(newValue);
		return *this;
	}

	/// Changes the value of the property. Handles undo and sends a notification message.
	PropertyField& operator=(const QVariant& newValue) {
		if(newValue.canConvert<qvariant_type>())
			return ((*this) = static_cast<property_type>(newValue.value<qvariant_type>()));
		OVITO_ASSERT_MSG(false, "PropertyField assignment", "The assigned QVariant value cannot be converted to the data type of the property field.");
		return *this;
	}

	/// Returns the internal value stored in this property field.
	inline const property_type& value() const { return _value; }

	/// Returns the internal value stored in this property field as a QVariant.
	inline operator QVariant() const { return qVariantFromValue<qvariant_type>(static_cast<qvariant_type>(_value)); }

	/// Saves the property's value to a stream.
	inline void saveToStream(SaveStream& stream) const {
		stream << _value;
	}

	/// Loads the property's value from a stream.
	inline void loadFromStream(LoadStream& stream) {
		stream >> _value;
	}

private:

	/// Internal helper function that changes the stored value and
	/// generates notification events.
	void setPropertyValue(const property_type& newValue) {
		_value = newValue;
		generatePropertyChangedEvent();
		generateTargetChangedEvent();
		if(additionalChangeMessage != 0)
			generateTargetChangedEvent((ReferenceEvent::Type)additionalChangeMessage);
	}

	// This undo class records a change to the property value.
	class PropertyChangeOperation : public UndoableOperation {
	public:
		PropertyChangeOperation(PropertyField& field) : _field(field), _owner(field.owner() != field.owner()->dataset() ? field.owner() : nullptr) {
			// Make a copy of the current property value.
			_oldValue = field;
		}
		/// Restores the old property value.
		virtual void undo() override {
			// Swap old value and current property value.
			property_type temp = _field;
			_field.setPropertyValue(_oldValue);
			_oldValue = temp;
		}

	private:

		/// The object whose property has been changed.
		/// This OORef is needed to keep the owner object alive as long as this undo record is on the undo stack.
		/// This is only used if the owner is not a DataSet, because that would create a circular reference.
		OORef<OvitoObject> _owner;
		/// The property field that has been changed.
		PropertyField& _field;
		/// The old value of the property.
		property_type _oldValue;
	};

	/// The internal property value.
	property_type _value;
};

/**
 * \brief Manages a pointer to a RefTarget derived class held by a RefMaker derived class.
 */
class OVITO_CORE_EXPORT SingleReferenceFieldBase : public PropertyFieldBase
{
public:

	/// Default constructor.
	SingleReferenceFieldBase() : _pointer(nullptr) {}

	/// Destructor that resets the reference before the object dies.
	~SingleReferenceFieldBase();

	/// Returns the RefTarget pointer.
	inline operator RefTarget*() const {
		OVITO_ASSERT_MSG(owner() != nullptr, "ReferenceField pointer operator", "The ReferenceField object has not been initialized yet.");
		return _pointer;
	}

protected:

	/// Replaces the reference target.
	void setValue(RefTarget* newTarget);

	/// Replaces the target stored in the reference field.
	void swapReference(OORef<RefTarget>& inactiveTarget, bool generateNotificationEvents = true);

	/// The actual pointer to the reference target.
	RefTarget* _pointer;

	friend class RefMaker;
	friend class RefTarget;

protected:

	class SetReferenceOperation : public UndoableOperation
	{
	private:
		/// The reference target that is currently not assigned to the reference field.
		/// This is stored here so that we can restore it on a call to undo().
		OORef<RefTarget> _inactiveTarget;
		/// The reference field whose value has changed.
		SingleReferenceFieldBase& _reffield;
		/// This reference is needed to keep the owner object alive as long as this undo record is on the stack.
		/// This is only used if the owner is not a DataSet, because that would create a circular reference.
		OORef<RefMaker> _owner;
	public:
		SetReferenceOperation(RefTarget* oldTarget, SingleReferenceFieldBase& reffield);
		virtual void undo() override { _reffield.swapReference(_inactiveTarget); }
	};
};

/**
 * \brief Templated version of the SingleReferenceFieldBase class.
 */
template<typename RefTargetType>
class ReferenceField : public SingleReferenceFieldBase
{
public:

	/// Read access to the RefTarget derived pointer.
	operator RefTargetType*() const { return reinterpret_cast<RefTargetType*>(_pointer); }

	/// Write access to the RefTarget pointer. Changes the value of the reference field.
	/// The old reference target will be released and the new reference target
	/// will be bound to this reference field.
	/// This operator automatically handles undo so the value change can be undone.
	RefTargetType* operator=(RefTargetType* newPointer) {
		setValue(newPointer);
		return newPointer;
	}

	/// Overloaded arrow operator; implements pointer semantics.
	/// Just use this operator as you would with a normal C++ pointer.
	RefTargetType* operator->() const {
		OVITO_ASSERT_MSG(_pointer, "ReferenceField operator->", QString("Tried to make a call to a NULL pointer. Reference field '%1' of class %2").arg(QString(descriptor()->identifier()), descriptor()->definingClass()->name()).toLocal8Bit().constData());
		return reinterpret_cast<RefTargetType*>(_pointer);
	}

	/// Dereference operator; implements pointer semantics.
	/// Just use this operator as you would with a normal C++ pointer.
	RefTargetType& operator*() const {
		OVITO_ASSERT_MSG(_pointer, "ReferenceField operator*", QString("Tried to dereference a NULL pointer. Reference field '%1' of class %2").arg(QString(descriptor()->identifier()), descriptor()->definingClass()->name()).toLocal8Bit().constData());
		return *reinterpret_cast<RefTargetType*>(_pointer);
	}

	/// Returns true if the internal pointer is non-NULL.
	operator bool() const { return _pointer != nullptr; }
};


/// \brief Dynamic casting function for reference fields.
///
/// Returns the given object cast to type \c T if the object is of type \c T
/// (or of a subclass); otherwise returns \c NULL.
///
/// \relates ReferenceField
template<class T, class U>
inline T* dynamic_object_cast(const ReferenceField<U>& field) {
	return dynamic_object_cast<T,U>(field.value());
}

/**
 * \brief Manages a list of references to RefTarget objects held by a RefMaker derived class.
 */
class OVITO_CORE_EXPORT VectorReferenceFieldBase : public PropertyFieldBase
{
public:

	/// Destructor that releases all referenced objects.
	~VectorReferenceFieldBase();

	/// Returns the stored references as a QVector.
	operator const QVector<RefTarget*>&() const { return pointers; }

	/// Returns the reference target at index position i.
	RefTarget* operator[](int i) const { return pointers[i]; }

	/// Returns the number of objects in the vector reference field.
	int size() const { return pointers.size(); }

	/// Returns true if the vector has size 0; otherwise returns false.
	bool empty() const { return pointers.empty(); }

	/// Returns true if the vector contains an occurrence of value; otherwise returns false.
	bool contains(RefTarget* value) const { return pointers.contains(value); }

	/// Returns the index position of the first occurrence of value in the vector,
	/// searching forward from index position from. Returns -1 if no item matched.
	int indexOf(RefTarget* value, int from = 0) const { return pointers.indexOf(value, from); }

	/// Clears all references at sets the vector size to zero.
	void clear();

	/// Removes the element at index position i.
	void remove(int i);

	/// Returns the stored references as a QVector.
	const QVector<RefTarget*>& targets() const { return pointers; }

protected:

	/// The actual pointer list to the reference targets.
	QVector<RefTarget*> pointers;

	/// Adds a reference target to the internal list.
	int insertInternal(RefTarget* newTarget, int index = -1);

	/// Removes a target from the list reference field.
	OORef<RefTarget> removeReference(int index, bool generateNotificationEvents = true);

	/// Adds the target to the list reference field.
	int addReference(const OORef<RefTarget>& target, int index);

protected:

	class InsertReferenceOperation : public UndoableOperation
	{
	public:
    	InsertReferenceOperation(RefTarget* target, VectorReferenceFieldBase& reffield, int index);

		virtual void undo() override {
			OVITO_ASSERT(!_target);
			_target = _reffield.removeReference(_index);
		}

		virtual void redo() override {
			_index = _reffield.addReference(_target, _index);
			_target.reset();
		}

		int insertionIndex() const { return _index; }
	private:
		/// The target that has been added into the vector reference field.
	    OORef<RefTarget> _target;
		/// The vector reference field to which the reference has been added.
		VectorReferenceFieldBase& _reffield;
		/// This reference is needed to keep the owner object alive as long as this undo record is on the stack.
		/// This is only used if the owner is not a DataSet, because that would create a circular reference.
		OORef<RefMaker> _owner;
		/// The position at which the target has been inserted into the vector reference field.
		int _index;
	};

	class RemoveReferenceOperation : public UndoableOperation
	{
	public:
    	RemoveReferenceOperation(VectorReferenceFieldBase& reffield, int index);

		virtual void undo() override {
			_index = _reffield.addReference(_target, _index);
			_target.reset();
		}

		virtual void redo() override {
			OVITO_ASSERT(!_target);
			_target = _reffield.removeReference(_index);
		}
	private:
		/// The target that has been removed from the vector reference field.
	    OORef<RefTarget> _target;
		/// The vector reference field from which the reference has been removed.
		VectorReferenceFieldBase& _reffield;
		/// This reference is needed to keep the owner object alive as long as this undo record is on the stack.
		/// This is only used if the owner is not a DataSet, because that would create a circular reference.
		OORef<RefMaker> _owner;
		/// The position at which the target has been removed from the vector reference field.
		int _index;
	};

	friend class RefMaker;
	friend class RefTarget;
};

/**
 * \brief Templated version of the VectorReferenceFieldBase class.
 */
template<typename RefTargetType>
class VectorReferenceField : public VectorReferenceFieldBase
{
public:

	typedef QVector<RefTargetType*> RefTargetVector;
	typedef typename RefTargetVector::ConstIterator ConstIterator;
	typedef typename RefTargetVector::const_iterator const_iterator;
	typedef typename RefTargetVector::const_pointer const_pointer;
	typedef typename RefTargetVector::const_reference const_reference;
	typedef typename RefTargetVector::difference_type difference_type;
	typedef typename RefTargetVector::size_type size_type;
	typedef typename RefTargetVector::value_type value_type;

	/// Returns the stored references as a QVector.
	operator const RefTargetVector&() const { return targets(); }

	/// Returns the reference target at index position i.
	RefTargetType* operator[](int i) const { return static_object_cast<RefTargetType>(pointers[i]); }

	/// Inserts a reference at the end of the vector.
	void push_back(RefTargetType* object) { insertInternal(object); }

	/// Inserts a reference at index position i in the vector.
	/// If i is 0, the value is prepended to the vector.
	/// If i is size() or negative, the value is appended to the vector.
	void insert(int i, RefTargetType* object) { insertInternal(object, i); }

	/// Replaces a reference in the vector.
	/// This method removes the reference at index i and inserts the new reference at the same index.
	void set(int i, RefTargetType* object) { remove(i); insert(i, object); }

	/// Returns an STL-style iterator pointing to the first item in the vector.
	const_iterator begin() const { return targets().begin(); }

	/// Returns an STL-style iterator pointing to the imaginary item after the last item in the vector.
	const_iterator end() const { return targets().end(); }

	/// Returns an STL-style iterator pointing to the first item in the vector.
	const_iterator constBegin() const { return begin(); }

	/// Returns a const STL-style iterator pointing to the imaginary item after the last item in the vector.
	const_iterator constEnd() const { return end(); }

	/// Returns the first reference stored in this vector reference field.
	RefTargetType* front() const { return static_object_cast<RefTargetType>(pointers.front()); }

	/// Returns the last reference stored in this vector reference field.
	RefTargetType* back() const { return static_object_cast<RefTargetType>(pointers.back()); }

	/// Finds the first object stored in this vector reference field that is of the given type.
	/// or can be cast to the given type. Returns NULL if no such object is in the list.
	template<class Clazz>
	Clazz* firstOf() const {
		for(const_iterator i = constBegin(); i != constEnd(); ++i) {
			Clazz* obj = dynamic_object_cast<Clazz>(*i);
			if(obj) return obj;
		}
		return NULL;
	}

	/// Copies the references of another vector reference field.
	VectorReferenceField& operator=(const VectorReferenceField& other) {
		clear();
		for(const_iterator o = other.begin(); o != other.end(); ++o)
			push_back(*o);
		return *this;
	}

	/// Assigns the given list of targets to this vector reference field.
	VectorReferenceField& operator=(const RefTargetVector& other) {
		clear();
		for(const_iterator o = other.begin(); o != other.end(); ++o)
			push_back(*o);
		return *this;
	}

	/// Returns the stored references as a QVector.
	const RefTargetVector& targets() const { return reinterpret_cast<const RefTargetVector&>(pointers); }
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#include <core/dataset/DataSet.h>

#endif // __OVITO_PROPERTY_FIELD_H
