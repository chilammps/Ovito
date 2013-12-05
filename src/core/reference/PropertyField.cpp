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
#include <core/reference/PropertyField.h>
#include <core/reference/PropertyFieldDescriptor.h>
#include <core/reference/RefTarget.h>
#include <core/plugins/Plugin.h>
#include <core/dataset/UndoStack.h>

namespace Ovito {

/******************************************************************************
* Generates a notification event to inform the dependents of the field's owner
* that it has changed.
******************************************************************************/
void PropertyFieldBase::generateTargetChangedEvent(ReferenceEvent::Type messageType)
{
	// Send change message.
	RefTarget* thisTarget = dynamic_object_cast<RefTarget>(owner());
	if(thisTarget && descriptor()->shouldGenerateChangeEvent())
		thisTarget->notifyDependents(messageType);
}

/******************************************************************************
* Generates a notification event to inform the dependents of the field's owner
* that it has changed.
******************************************************************************/
void PropertyFieldBase::generatePropertyChangedEvent() const
{
	owner()->propertyChanged(*descriptor());
}

/******************************************************************************
* Destructor that releases all referenced objects.
******************************************************************************/
VectorReferenceFieldBase::~VectorReferenceFieldBase()
{
	OVITO_ASSERT(!owner()->getOOType().isDerivedFrom(RefTarget::OOType) || owner()->dataset()->undoStack().isRecording() == false);
	while(!pointers.empty()) {
		removeReference(pointers.size() - 1, false);
	}
}

/******************************************************************************
* Destructor that resets the reference before the object dies.
******************************************************************************/
SingleReferenceFieldBase::~SingleReferenceFieldBase()
{
	OVITO_ASSERT(!owner()->getOOType().isDerivedFrom(RefTarget::OOType) || owner()->dataset()->undoStack().isRecording() == false);
	// The internal intrusive pointer must be cleared first before the reference count
	// to the target goes to zero to prevent infinite recursion.
	OORef<RefTarget> a_null_ptr;
	swapReference(a_null_ptr, false);
}

/******************************************************************************
* Replaces the target stored in the reference field.
******************************************************************************/
void SingleReferenceFieldBase::swapReference(OORef<RefTarget>& inactiveTarget, bool generateNotificationEvents)
{
	OVITO_CHECK_POINTER(this);
	OVITO_ASSERT(!descriptor()->isVector());

	RefMaker* refmaker = owner();
	OVITO_CHECK_OBJECT_POINTER(refmaker);

	// Check for cyclic references.
	if(inactiveTarget && refmaker->isReferencedBy(inactiveTarget.get())) {
		OVITO_ASSERT(!owner()->getOOType().isDerivedFrom(RefTarget::OOType) || !owner()->dataset()->undoStack().isUndoingOrRedoing());
		throw CyclicReferenceError();
	}

	_pointer.swap(inactiveTarget);

	// Remove the RefMaker from the old target's list of dependents if it has no
	// more references to it.
	if(inactiveTarget) {
		OVITO_ASSERT(inactiveTarget->_dependents.contains(refmaker));
		if(!refmaker->hasReferenceTo(inactiveTarget.get())) {
			inactiveTarget->_dependents.remove(refmaker);
		}
	}

	// Add the RefMaker to the list of dependents of the new target.
	if(_pointer) {
		if(_pointer->_dependents.contains(refmaker) == false)
			_pointer->_dependents.push_back(refmaker);
	}

	if(generateNotificationEvents) {

		// Inform derived classes.
		refmaker->referenceReplaced(*descriptor(), inactiveTarget.get(), _pointer.get());

		// Send auto change message.
		generateTargetChangedEvent();

	}
}

/******************************************************************************
* Replaces the reference target stored in a reference field.
* Creates an undo record so the old value can be restored at a later time.
******************************************************************************/
void SingleReferenceFieldBase::setValue(RefTarget* newTarget)
{
	if(_pointer == newTarget) return;	// Nothing has changed.

    // Check object type
	if(newTarget && !newTarget->getOOType().isDerivedFrom(*descriptor()->targetClass())) {
		OVITO_ASSERT_MSG(false, "SingleReferenceFieldBase::SetValue", "Tried to create a reference to an incompatible object for this reference field.");
		throw Exception(QString("Cannot set a reference field of type %1 to an incompatible object of type %2.").arg(descriptor()->targetClass()->name(), newTarget->getOOType().name()));
	}

	// Make sure automatic undo is disabled for a reference field of a class that is not derived from RefTarget.
	OVITO_ASSERT_MSG(descriptor()->automaticUndo() == false || owner()->getOOType().isDerivedFrom(RefTarget::OOType), "SingleReferenceFieldBase::setValue()",
			"PROPERTY_FIELD_NO_UNDO flag has not been set for reference field of non-RefTarget derived class.");

	if(descriptor()->automaticUndo() && owner()->dataset()->undoStack().isRecording()) {
		SetReferenceOperation* op = new SetReferenceOperation(newTarget, *this);
		owner()->dataset()->undoStack().push(op);
		op->redo();
		OVITO_ASSERT(_pointer.get() == newTarget);
	}
	else {
		OORef<RefTarget> newTargetRef(newTarget);
		swapReference(newTargetRef);
		OVITO_ASSERT(_pointer.get() == newTarget);
	}
}

/******************************************************************************
* Removes a target from the list reference field.
******************************************************************************/
OORef<RefTarget> VectorReferenceFieldBase::removeReference(int index, bool generateNotificationEvents)
{
	OVITO_CHECK_POINTER(this);
	OVITO_CHECK_OBJECT_POINTER(owner());
	OVITO_ASSERT(descriptor()->isVector());
	RefMaker* refmaker = owner();

	OVITO_ASSERT(index >= 0 && index < pointers.size());
	OORef<RefTarget> target = pointers.at(index);

	// Remove reference.
	pointers.remove(index);

	// Release old reference target if there are no more references to it.
	if(target) {
		target->decrementReferenceCount();

		// Remove the refmaker from the old target's list of dependents.
		OVITO_CHECK_OBJECT_POINTER(target);
		OVITO_ASSERT(target->_dependents.contains(refmaker));
		if(!refmaker->hasReferenceTo(target.get())) {
			target->_dependents.remove(refmaker);
		}
	}

	if(generateNotificationEvents) {
		try {
			// Inform derived classes.
			refmaker->referenceRemoved(*descriptor(), target.get(), index);

			// Send auto change message.
			generateTargetChangedEvent();
		}
		catch(...) {
			if(!owner()->getOOType().isDerivedFrom(RefTarget::OOType) || !owner()->dataset()->undoStack().isUndoingOrRedoing())
				throw;
			qDebug() << "Caught exception in VectorReferenceFieldBase::removeReference(). RefMaker is" << owner() << ". RefTarget is" << target;
		}
	}

	return target;
}

/******************************************************************************
* Adds the target to the list reference field.
******************************************************************************/
int VectorReferenceFieldBase::addReference(const OORef<RefTarget>& target, int index)
{
	OVITO_CHECK_POINTER(this);
	OVITO_CHECK_OBJECT_POINTER(owner());
	OVITO_ASSERT(descriptor()->isVector());

	RefMaker* refmaker = owner();

	// Check for cyclic references.
	if(target && refmaker->isReferencedBy(target.get())) {
		OVITO_ASSERT(!owner()->getOOType().isDerivedFrom(RefTarget::OOType) || !owner()->dataset()->undoStack().isUndoingOrRedoing());
		throw CyclicReferenceError();
	}

	// Add new reference to list field.
	if(index == -1) {
		index = pointers.size();
		pointers.push_back(target.get());
	}
	else {
		OVITO_ASSERT(index >= 0 && index <= pointers.size());
		pointers.insert(index, target.get());
	}
	if(target)
		target->incrementReferenceCount();

	// Add the RefMaker to the list of dependents of the new target.
	if(target && target->_dependents.contains(refmaker) == false)
		target->_dependents.push_back(refmaker);

	try {
		// Inform derived classes.
		refmaker->referenceInserted(*descriptor(), target.get(), index);

		// Send auto change message.
		generateTargetChangedEvent();
	}
	catch(...) {
		if(!owner()->getOOType().isDerivedFrom(RefTarget::OOType) || !owner()->dataset()->undoStack().isUndoingOrRedoing())
			throw;
		qDebug() << "Caught exception in VectorReferenceFieldBase::addReference(). RefMaker is" << refmaker << ". RefTarget is" << target.get();
	}

	return index;
}

/******************************************************************************
* Adds a reference target to the internal list.
* Creates an undo record so the insertion can be undone at a later time.
******************************************************************************/
int VectorReferenceFieldBase::insertInternal(RefTarget* newTarget, int index)
{
    // Check object type
	if(newTarget && !newTarget->getOOType().isDerivedFrom(*descriptor()->targetClass())) {
		OVITO_ASSERT_MSG(false, "VectorReferenceFieldBase::insert", "Cannot add incompatible object to this vector reference field.");
		throw Exception(QString("Cannot add an object to a reference field of type %1 that has the incompatible type %2.").arg(descriptor()->targetClass()->name(), newTarget->getOOType().name()));
	}

	// Make sure automatic undo is disabled for a reference field of a class that is not derived from RefTarget.
	OVITO_ASSERT_MSG(descriptor()->automaticUndo() == false || owner()->getOOType().isDerivedFrom(RefTarget::OOType), "VectorReferenceFieldBase::insertInternal()",
			"PROPERTY_FIELD_NO_UNDO flag has not been set for reference field of non-RefTarget derived class.");

	if(descriptor()->automaticUndo() && owner()->dataset()->undoStack().isRecording()) {
		InsertReferenceOperation* op = new InsertReferenceOperation(newTarget, *this, index);
		owner()->dataset()->undoStack().push(op);
		op->redo();
		return op->getInsertionIndex();
	}
	else {
		return addReference(newTarget, index);
	}
}

/******************************************************************************
* Removes the element at index position i.
* Creates an undo record so the removal can be undone at a later time.
******************************************************************************/
void VectorReferenceFieldBase::remove(int i)
{
	OVITO_ASSERT(i >= 0 && i < size());

	// Make sure automatic undo is disabled for a reference field of a class that is not derived from RefTarget.
	OVITO_ASSERT_MSG(descriptor()->automaticUndo() == false || owner()->getOOType().isDerivedFrom(RefTarget::OOType), "VectorReferenceFieldBase::remove()",
			"PROPERTY_FIELD_NO_UNDO flag has not been set for reference field of non-RefTarget derived class.");

	if(descriptor()->automaticUndo() && owner()->dataset()->undoStack().isRecording()) {
		RemoveReferenceOperation* op = new RemoveReferenceOperation(*this, i);
		owner()->dataset()->undoStack().push(op);
		op->redo();
	}
	else {
		removeReference(i);
	}
}

/// Clears all references at sets the vector size to zero.
void VectorReferenceFieldBase::clear()
{
	while(!pointers.empty())
		remove(pointers.size() - 1);
}

};
