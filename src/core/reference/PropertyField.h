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
#include <core/gui/undo/UndoManager.h>
#include <core/reference/PropertyFieldDescriptor.h>
#include <core/reference/ReferenceEvent.h>

namespace Ovito {

class RefTarget;					// defined in RefTarget.h
class RefMaker;						// defined in RefMaker.h

/******************************************************************************
* RefMaker derived classes use this helper class to store their properties.
******************************************************************************/
class PropertyFieldBase
{
public:

	/// One has to call init() once to fully setup this property field.
	PropertyFieldBase() : _owner(NULL), _descriptor(NULL) {}

	/// Connects the property field to its owning RefMaker derived class.
	/// This function must be called in the constructor of the RefMaker derived class for each of its property fields.
	inline void init(RefMaker* owner, PropertyFieldDescriptor* descriptor) {
		OVITO_ASSERT_MSG(owner != NULL, "PropertyFieldBase::init()", "The PropertyField must be initialized with a non-NULL owner.");
		OVITO_ASSERT_MSG(descriptor != NULL, "PropertyFieldBase::init()", "The PropertyField must be initialized with a non-NULL descriptor.");
		OVITO_ASSERT_MSG(this->_owner == NULL, "PropertyFieldBase::init()", "The PropertyField has already been initialized.");
		this->_owner = owner;
		this->_descriptor = descriptor;
	}

	/// Returns the owner of this property field.
	inline RefMaker* owner() const {
		OVITO_ASSERT_MSG(_owner != NULL, "PropertyFieldBase", "The PropertyField object has not been initialized yet.");
		return _owner;
	}

	/// Returns descriptor of this property field.
	inline PropertyFieldDescriptor* descriptor() const {
		OVITO_ASSERT_MSG(_descriptor != NULL, "PropertyFieldBase", "The PropertyField object has not been initialized yet.");
		return _descriptor;
	}

protected:

	/// Generates a notification event to inform the dependents of the field's owner that it has changed.
	void generateTargetChangedEvent(ReferenceEvent::Type eventType = ReferenceEvent::TargetChanged);

	void generatePropertyChangedEvent() const;

private:

	/// The reference maker this object is a member of.
	/// This will be initialized after a call to init().
	RefMaker* _owner;

	/// The descriptor of this property field.
	PropertyFieldDescriptor* _descriptor;
};

/******************************************************************************
* An instance of this helper class stores a non-animatable property value
* with a primitive type.
******************************************************************************/
template<typename property_data_type, typename qvariant_data_type = property_data_type, int additionalChangeMessage = 0>
class PropertyField : public PropertyFieldBase
{
public:
	typedef property_data_type property_type;
	typedef qvariant_data_type qvariant_type;

	/// Default constructor that does not initialize the property value.
	PropertyField() : PropertyFieldBase() {}

	/// Constructor that assigns an initial value to the property.
	PropertyField(const property_type& value) : PropertyFieldBase(), _value(value) {}

	/// Constructor that calls the internal value constructor with one parameter.
	template<typename K1>
	PropertyField(K1 param1) : PropertyFieldBase(), _value(param1) {}

	/// Constructor that calls the internal value constructor with two parameters.
	template<typename K1, typename K2>
	PropertyField(K1 param1, K2 param2) : PropertyFieldBase(), _value(param1, param2) {}

	/// Constructor that calls the internal value constructor with three parameters.
	template<typename K1, typename K2, typename K3>
	PropertyField(K1 param1, K2 param2, K3 param3) : PropertyFieldBase(), _value(param1, param2, param3) {}

	/// Constructor that calls the internal value constructor with four parameters.
	template<typename K1, typename K2, typename K3, typename K4>
	PropertyField(K1 param1, K2 param2, K3 param3, K4 param4) : PropertyFieldBase(), _value(param1, param2, param3, param4) {}

	/// Cast the property field to the property value.
	inline operator const property_type&() const { return _value; }

	/// Changes the value of the property. Handles undo and sends a notification message.
	inline PropertyField& operator=(const property_type& newValue) {
		if(_value == newValue) return *this;
		if(UndoManager::instance().isRecording() && descriptor()->automaticUndo())
			UndoManager::instance().push(new PropertyChangeOperation(*this));
		else
			setPropertyValue(newValue);
		return *this;
	}

	/// Changes the value of the property. Handles undo and sends a notification message.
	inline PropertyField& operator=(const QVariant& newValue) {
		OVITO_ASSERT_MSG(newValue.canConvert<qvariant_type>(), "PropertyField assignment", "The assigned QVariant value cannot be converted to the data type of the property field.");
		return ((*this) = (property_type)newValue.value<qvariant_type>());
	}

	/// Returns the internal value stored in this property field.
	inline const property_type& value() const { return _value; }

	/// Returns the internal value stored in this property field as a QVariant.
	inline operator QVariant() const { return qVariantFromValue<qvariant_type>((qvariant_type)_value); }

	/// Saves the property's value to a stream.
	inline void saveToStream(SaveStream& stream) const { stream << _value; }

	/// Loads the property's value from a stream.
	inline void loadFromStream(LoadStream& stream) { stream >> _value; }

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

	/// This undo class records a change to the property value.
	class PropertyChangeOperation : public QUndoCommand {
	public:
		PropertyChangeOperation(PropertyField& field) : _field(field), _object(field.owner()) {
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
		/// Re-apply the change, assuming that it has been undone.
		virtual void redo() override { PropertyChangeOperation::undo(); }

	private:
		/// The object whose property has been changed.
		OORef<OvitoObject> _object;
		/// The property field that has been changed.
		PropertyField& _field;
		/// The old value of the property.
		property_type _oldValue;
	};

	/// The internal property value.
	property_type _value;
};

/******************************************************************************
* An instance of this helper class stores a pointer to a RefTarget derived class.
******************************************************************************/
class SingleReferenceFieldBase : public PropertyFieldBase
{
public:
	/// Default constructor that initializes the internal pointer to NULL.
	/// One has to call init() once to fully setup this reference field.
	SingleReferenceFieldBase() : PropertyFieldBase() {}

	/// Destructor that resets the reference before the object dies.
	~SingleReferenceFieldBase() {
		OVITO_ASSERT(UndoManager::instance().isRecording() == false);
		// The internal intrusive pointer must be cleared first before the reference count
		// to the target goes to zero to prevent infinite recursion.
		OORef<RefTarget> a_null_ptr;
		swapReference(a_null_ptr, false);
	}

	/// Returns the RefTarget pointer.
	inline operator RefTarget*() const {
		OVITO_ASSERT_MSG(owner() != NULL, "ReferenceField pointer operator", "The ReferenceField object has not been initialized yet.");
		return _pointer.get();
	}

protected:

	/// Replaces the reference target.
	void setValue(RefTarget* newTarget);

	/// Replaces the target stored in the reference field.
	void swapReference(OORef<RefTarget>& inactiveTarget, bool generateNotificationEvents = true);

	/// The actual pointer to the reference target.
	OORef<RefTarget> _pointer;

	friend class RefMaker;
	friend class RefTarget;

protected:

	class SetReferenceOperation : public QUndoCommand
	{
	private:
		OORef<RefTarget> inactiveTarget;
		SingleReferenceFieldBase& reffield;
	public:
		SetReferenceOperation(RefTarget* _oldTarget, SingleReferenceFieldBase& _reffield)
			: inactiveTarget(_oldTarget), reffield(_reffield) {}

		virtual void undo() override { reffield.swapReference(inactiveTarget); }
		virtual void redo() override { reffield.swapReference(inactiveTarget); }
	};
};

/******************************************************************************
* This is the templated version of the SingleReferenceFieldBase helper class.
* It stores a pointer to a RefTarget derived class given as the template
* parameter.
******************************************************************************/
template<typename RefTargetType>
class ReferenceField : public SingleReferenceFieldBase
{
public:
	/// Constructor that initializes the internal pointer to NULL.
	ReferenceField() : SingleReferenceFieldBase() {}

	/// Read access to the RefTarget derived pointer.
	operator RefTargetType*() const { return (RefTargetType*)_pointer.get(); }

	/// Write access to the RefTarget pointer. Changes the value of the reference field.
	/// The old reference target will be released and the new reference target
	/// will be bound to this reference field.
	/// This operator automatically handles undo so the value change can be undone.
	RefTargetType* operator=(RefTargetType* newPointer) {
		setValue(newPointer);
		OVITO_ASSERT(_pointer.get() == newPointer);
		return (RefTargetType*)_pointer.get();
	}

	/// Write access to the RefTarget pointer. Changes the value of the reference field.
	/// The old reference target will be released and the new reference target
	/// will be bound to this reference field.
	/// This operator automatically handles undo so the value change can be undone.
	RefTargetType* operator=(const OORef<RefTargetType>& newPointer) {
		setValue(newPointer.get());
		OVITO_ASSERT(_pointer == newPointer);
		return (RefTargetType*)_pointer.get();
	}

	/// Overloaded arrow operator; implements pointer semantics.
	/// Just use this operator as you would with a normal C++ pointer.
	RefTargetType* operator->() const {
		OVITO_ASSERT_MSG(_pointer, "ReferenceField operator->", QString("Tried to make a call to a NULL pointer. Reference field '%1' of class %2").arg(QString(descriptor()->identifier()), descriptor()->definingClass()->name()).toLocal8Bit().constData());
		return (RefTargetType*)_pointer.get();
	}

	/// Dereference operator; implements pointer semantics.
	/// Just use this operator as you would with a normal C++ pointer.
	RefTargetType& operator*() const {
		OVITO_ASSERT_MSG(_pointer, "ReferenceField operator*", QString("Tried to dereference a NULL pointer. Reference field '%1' of class %2").arg(QString(descriptor()->identifier()), descriptor()->definingClass()->name()).toLocal8Bit().constData());
		return *(RefTargetType*)_pointer.get();
	}

	/// Returns true if the internal is non-NULL.
	operator bool() const { return _pointer; }
};


/// \brief Dynamic casting function for reference fields.
///
/// Returns the given object cast to type \c T if the object is of type \c T
/// (or of a subclass); otherwise returns \c NULL.
template<class T, class U>
inline T* dynamic_object_cast(const ReferenceField<U>& field) {
	return dynamic_object_cast<T,U>((U*)field);
}

/******************************************************************************
* An instance of this helper class stores a list of pointers to a RefTarget derived classes.
* RefMaker derived classes must use this helper class to store their
* vector reference lists.
******************************************************************************/
class VectorReferenceFieldBase : public PropertyFieldBase
{
public:

	/// Default constructor that initializes the internal pointer list to an empty list.
	/// One has to call init() once to fully setup this reference field.
	VectorReferenceFieldBase() : PropertyFieldBase() {}

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

	class InsertReferenceOperation : public QUndoCommand
	{
	private:
	    OORef<RefTarget> target;
		VectorReferenceFieldBase& reffield;
		int index;
	public:
    	InsertReferenceOperation(RefTarget* _target, VectorReferenceFieldBase& _reffield, int _index)
			: target(_target), reffield(_reffield), index(_index) {}

		virtual void undo() override {
			OVITO_ASSERT(!target);
			reffield.removeReference(index);
		}

		virtual void redo() override {
			index = reffield.addReference(target, index);
			target = NULL;
		}

		int getInsertionIndex() const { return index; }
	};

	class RemoveReferenceOperation : public QUndoCommand
	{
	private:
	    OORef<RefTarget> target;
		VectorReferenceFieldBase& reffield;
		int index;
	public:
    	RemoveReferenceOperation(VectorReferenceFieldBase& _reffield, int _index)
			: reffield(_reffield), index(_index) {}

		virtual void undo() override {
			index = reffield.addReference(target, index);
			target = NULL;
		}

		virtual void redo() override {
			OVITO_ASSERT(!target);
			reffield.removeReference(index);
		}
	};

	friend class RefMaker;
	friend class RefTarget;
};

/******************************************************************************
* This is the templated version of the VectorReferenceFieldBase helper class.
* It stores a list of pointers to a RefTarget derived objects given as the template
* parameter.
******************************************************************************/
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

	/// Constructor that initializes the internal pointer to NULL.
	VectorReferenceField() : VectorReferenceFieldBase() {}

	/// Returns the stored references as a QVector.
	operator const RefTargetVector&() const { return targets(); }

	/// Returns the reference target at index position i.
	RefTargetType* operator[](int i) const { return static_object_cast<RefTargetType>(pointers[i]); }

	/// Inserts a reference at the end of the vector.
	void push_back(RefTargetType* object) { insertInternal(object); }

	/// Inserts a reference at the end of the vector.
	void push_back(const OORef<RefTargetType>& object) { insertInternal(object.get()); }

	/// Inserts a reference at index position i in the vector.
	/// If i is 0, the value is prepended to the vector.
	/// If i is size() or negative, the value is appended to the vector.
	void insert(int i, RefTargetType* object) { insertInternal(object, i); }

	/// Inserts a reference at index position i in the vector.
	/// If i is 0, the value is prepended to the vector.
	/// If i is size() or negative, the value is appended to the vector.
	void insert(int i, const OORef<RefTargetType>& object) { insertInternal(object.get(), i); }

	/// Replaces a reference in the vector.
	/// This method removes the reference at index i and inserts the new reference at the same index.
	void set(int i, RefTargetType* object) { remove(i); insert(i, object); }

	/// Replaces a reference in the vector.
	/// This method removes the reference at index i and inserts the new reference at the same index.
	void set(int i, const OORef<RefTargetType>& object) { remove(i); insert(i, object); }

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

	/// Returns the stored references as a QVector.
	const RefTargetVector& targets() const { return reinterpret_cast<const RefTargetVector&>(pointers); }
};

};	// End of namespace

#endif // __OVITO_PROPERTY_FIELD_H
