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
#include <core/gui/undo/UndoManager.h>

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

void PropertyFieldBase::generatePropertyChangedEvent() const
{
	owner()->propertyChanged(*descriptor());
}

/******************************************************************************
* Destructor that releases all referenced objects.
******************************************************************************/
VectorReferenceFieldBase::~VectorReferenceFieldBase()
{
	// Make a copy of the pointer list.
	QVector<RefTarget*> old_pointers = pointers;

	// Now clear the original list.
	pointers.clear();

	// Release objects one by one.
	for(RefTarget* o : old_pointers)
		o->decrementReferenceCount();
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

	class SetReferenceOperation : public QUndoCommand
	{
	private:
	    OORef<RefTarget> inactiveTarget;
		SingleReferenceFieldBase& reffield;
	public:
    	SetReferenceOperation(RefTarget* _oldTarget, SingleReferenceFieldBase& _reffield)
			: inactiveTarget(_oldTarget), reffield(_reffield) {}

		virtual void undo() override { swapReferences(); }
		virtual void redo() override { swapReferences(); }

		/// Replaces the target stored in a reference field.
		void swapReferences() {
			OVITO_CHECK_POINTER(&reffield);
			OVITO_CHECK_OBJECT_POINTER(reffield.owner());
			OVITO_ASSERT(!reffield.descriptor()->isVector());

			RefMaker* refmaker = reffield.owner();

			// Check for cyclic references.
			if(inactiveTarget && refmaker->isReferencedBy(inactiveTarget.get())) {
				OVITO_ASSERT(!UndoManager::instance().isUndoingOrRedoing());
				throw CyclicReferenceError();
			}

			reffield._pointer.swap(inactiveTarget);

			// Remove the RefMaker from the old target's list of dependents if it has no
			// more references to it.
			if(inactiveTarget) {
				OVITO_ASSERT(inactiveTarget->dependents().contains(refmaker));
				if(!refmaker->hasReferenceTo(inactiveTarget.get())) {
					inactiveTarget->dependents().remove(refmaker);
				}
			}

			// Add the RefMaker to the list of dependents of the new target.
			if(reffield._pointer) {
				if(reffield._pointer->dependents().contains(refmaker) == false)
					reffield._pointer->dependents().push_back(refmaker);
			}

			// Inform derived classes.
			refmaker->referenceReplaced(*reffield.descriptor(), inactiveTarget.get(), reffield._pointer.get());

			// Send auto change message.
			reffield.generateTargetChangedEvent();
		}
	};

	if(UndoManager::instance().isRecording() && descriptor()->automaticUndo()) {
		SetReferenceOperation* op = new SetReferenceOperation(newTarget, *this);
		UndoManager::instance().push(op);
		op->swapReferences();
	}
	else {
		UndoSuspender noUndo;
		SetReferenceOperation(newTarget, *this).swapReferences();
	}
	OVITO_ASSERT(_pointer.get() == newTarget);
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

	class InsertReferenceOperation : public QUndoCommand
	{
	private:
	    OORef<RefTarget> target;
		VectorReferenceFieldBase& reffield;
		int index;
	public:
    	InsertReferenceOperation(RefTarget* _target, VectorReferenceFieldBase& _reffield, int _index)
			: target(_target), reffield(_reffield), index(_index) {}

		virtual void undo() override { removeReference(); }
		virtual void redo() override { addReference(); }

		/// Adds the target to the list reference field.
		int addReference() {
			OVITO_CHECK_POINTER(&reffield);
			OVITO_CHECK_OBJECT_POINTER(reffield.owner());
			OVITO_ASSERT(reffield.descriptor()->isVector());

			RefMaker* refmaker = reffield.owner();

			// Check for cyclic references.
			if(target && refmaker->isReferencedBy(target.get())) {
				OVITO_ASSERT(!UndoManager::instance().isUndoingOrRedoing());
				throw CyclicReferenceError();
			}

			// Add new reference to list field.
			if(index == -1) {
				index = reffield.pointers.size();
				reffield.pointers.push_back(target.get());
			}
			else {
				OVITO_ASSERT(index >= 0 && index <= reffield.pointers.size());
				reffield.pointers.insert(index, target.get());
			}
			if(target) {
				target.get()->incrementReferenceCount();
			}

			// Add the RefMaker to the list of dependents of the new target.
			if(target && target->dependents().contains(refmaker) == false)
				target->dependents().push_back(refmaker);

			// Inform derived classes.
			refmaker->referenceInserted(*reffield.descriptor(), target.get(), index);

			// Send auto change message.
			reffield.generateTargetChangedEvent();

			target = NULL;
			return index;
		}

		/// Removes the target from the list reference field.
		void removeReference() {
			OVITO_ASSERT(!target);
			OVITO_CHECK_POINTER(&reffield);
			OVITO_CHECK_OBJECT_POINTER(reffield.owner());
			OVITO_ASSERT(reffield.descriptor()->isVector());
			RefMaker* refmaker = reffield.owner();

			OVITO_ASSERT(index >= 0 && index < reffield.pointers.size());
			target = reffield.pointers.at(index);

			// Remove reference.
			reffield.pointers.remove(index);

			// Release old reference target if there are no more references to it.
			if(target) {
				target.get()->decrementReferenceCount();

				// Remove the refmaker from the old target's list of dependents.
				OVITO_CHECK_OBJECT_POINTER(target.get());
				OVITO_ASSERT(target->dependents().contains(refmaker));
				if(!refmaker->hasReferenceTo(target.get())) {
					target->dependents().remove(refmaker);
				}
			}

			// Inform derived classes.
			refmaker->referenceRemoved(*reffield.descriptor(), target.get(), index);

			// Send auto change message.
			reffield.generateTargetChangedEvent();
		}
	};

	if(UndoManager::instance().isRecording() && descriptor()->automaticUndo()) {
		InsertReferenceOperation* op = new InsertReferenceOperation(newTarget, *this, index);
		UndoManager::instance().push(op);
		return op->addReference();
	}
	else {
		UndoSuspender noUndo;
		return InsertReferenceOperation(newTarget, *this, index).addReference();
	}
}

/// Removes the element at index position i.
void VectorReferenceFieldBase::remove(int i)
{
	OVITO_ASSERT(i >=0 && i < size());

	class RemoveReferenceOperation : public QUndoCommand
	{
	private:
	    OORef<RefTarget> target;
		VectorReferenceFieldBase& reffield;
		int index;
	public:
    	RemoveReferenceOperation(VectorReferenceFieldBase& _reffield, int _index)
			: reffield(_reffield), index(_index) {}

		virtual void undo() override { addReference(); }
		virtual void redo() override { removeReference(); }

		/// Adds the target to the list reference field.
		int addReference() {
			OVITO_CHECK_POINTER(&reffield);
			OVITO_CHECK_OBJECT_POINTER(reffield.owner());
			OVITO_ASSERT(reffield.descriptor()->isVector());

			RefMaker* refmaker = reffield.owner();

			// Check for cyclic references.
			if(target && refmaker->isReferencedBy(target.get())) {
				OVITO_ASSERT(!UndoManager::instance().isUndoingOrRedoing());
				throw CyclicReferenceError();
			}

			// Add new reference to list field.
			if(index == -1) {
				index = reffield.pointers.size();
				reffield.pointers.push_back(target.get());
			}
			else {
				OVITO_ASSERT(index >= 0 && index <= reffield.pointers.size());
				reffield.pointers.insert(index, target.get());
			}
			if(target) {
				target.get()->incrementReferenceCount();
			}

			// Add the RefMaker to the list of dependents of the new target.
			if(target && target->dependents().contains(refmaker) == false)
				target->dependents().push_back(refmaker);

			// Inform derived classes.
			refmaker->referenceInserted(*reffield.descriptor(), target.get(), index);

			// Send auto change message.
			reffield.generateTargetChangedEvent();

			target = NULL;
			return index;
		}

		/// Removes the target from the list reference field.
		void removeReference() {
			OVITO_ASSERT(!target);
			OVITO_CHECK_POINTER(&reffield);
			OVITO_CHECK_OBJECT_POINTER(reffield.owner());
			OVITO_ASSERT(reffield.descriptor()->isVector());
			RefMaker* refmaker = reffield.owner();

			OVITO_ASSERT(index >= 0 && index < reffield.pointers.size());
			target = reffield.pointers.at(index);

			// Remove reference.
			reffield.pointers.remove(index);

			// Release old reference target if there are no more references to it.
			if(target) {
				target.get()->decrementReferenceCount();

				// Remove the refmaker from the old target's list of dependents.
				OVITO_CHECK_OBJECT_POINTER(target.get());
				OVITO_ASSERT(target->dependents().contains(refmaker));
				if(!refmaker->hasReferenceTo(target.get())) {
					target->dependents().remove(refmaker);
				}
			}

			// Inform derived classes.
			refmaker->referenceRemoved(*reffield.descriptor(), target.get(), index);

			// Send auto change message.
			reffield.generateTargetChangedEvent();
		}
	};

	if(UndoManager::instance().isRecording() && descriptor()->automaticUndo()) {
		RemoveReferenceOperation* op = new RemoveReferenceOperation(*this, i);
		UndoManager::instance().push(op);
		op->removeReference();
	}
	else {
		UndoSuspender noUndo;
		RemoveReferenceOperation(*this, i).removeReference();
	}
}


/// Clears all references at sets the vector size to zero.
void VectorReferenceFieldBase::clear()
{
	while(!pointers.empty())
		remove(pointers.size() - 1);
}

};
