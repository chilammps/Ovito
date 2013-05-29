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
#include <core/data/units/ParameterUnit.h>

namespace Ovito {

/******************************************************************************
* Sends the REFTARGET_CHANGED message to the dependents of the field owner.
******************************************************************************/
void PropertyFieldBase::sendChangeNotification(int messageType)
{
	// Send auto change message.
	RefTarget* thisTarget = dynamic_object_cast<RefTarget>(owner());
	if(thisTarget && descriptor()->sendChangeMessageEnabled())
		thisTarget->notifyDependents(messageType);
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
	if(pointer == newTarget) return;	// Nothing has changed.

    // Check object type
	if(newTarget && !newTarget->getOOType()->isDerivedFrom(descriptor()->targetClass())) {
		OVITO_ASSERT_MSG(false, "SingleReferenceFieldBase::SetValue", "Tried to create a reference to an incompatible object for this reference field.");
		throw Exception(QString("Cannot set a reference field of type %1 to an incompatible object of type %2.").arg(descriptor()->targetClass()->name(), newTarget->getOOType()->name()));
	}

	class SetReferenceOperation : public UndoableOperation
	{
	private:
	    intrusive_ptr<RefTarget> inactiveTarget;
		SingleReferenceFieldBase& reffield;
	public:
    	SetReferenceOperation(RefTarget* _oldTarget, SingleReferenceFieldBase& _reffield)
			: inactiveTarget(_oldTarget), reffield(_reffield) {}

		virtual void undo() { SwapReferences(); }
		virtual void redo() { SwapReferences(); }
		virtual QString displayName() const { return "Set reference"; }

		/// Replaces the target stored in a reference field.
		void SwapReferences() {
			CHECK_POINTER(&reffield);
			CHECK_OBJECT_POINTER(reffield.owner());
			OVITO_ASSERT(!reffield.descriptor()->isVector());

			RefMaker* refmaker = reffield.owner();

			// Check for cyclic references.
			if(inactiveTarget && refmaker->isReferencedBy(inactiveTarget.get())) {
				OVITO_ASSERT(!UNDO_MANAGER.isUndoingOrRedoing());
				throw CyclicReferenceError();
			}

			reffield.pointer.swap(inactiveTarget);

			// Remove the RefMaker from the old target's list of dependents if it has no
			// more references to it.
			if(inactiveTarget) {
				//MsgLogger() << "Removing reference from" << refmaker->pluginClassDescriptor()->name() << "to" << inactiveTarget->pluginClassDescriptor()->name() << endl;
				OVITO_ASSERT(inactiveTarget->getDependents().contains(refmaker));
				if(!refmaker->hasReferenceTo(inactiveTarget.get())) {
					inactiveTarget->dependentsList().remove(refmaker);
				}
			}

			// Add the RefMaker to the list of dependents of the new target.
			if(reffield.pointer) {
				//MsgLogger() << "Creating reference from" << refmaker->pluginClassDescriptor()->name() << "to" << reffield.pointer->pluginClassDescriptor()->name() << endl;
				if(reffield.pointer->dependentsList().contains(refmaker) == false)
					reffield.pointer->dependentsList().push_back(refmaker);
			}

			// Inform derived classes.
			refmaker->onRefTargetReplaced(*reffield.descriptor(), inactiveTarget.get(), reffield.pointer.get());

			// Send auto change message.
			reffield.sendChangeNotification();
		}
	};

	if(UndoManager::instance().isRecording() && descriptor()->automaticUndo()) {
		SetReferenceOperation* op = new SetReferenceOperation(newTarget, *this);
		UndoManager::instance().addOperation(op);
		op->swapReferences();
	}
	else {
		UndoSuspender noUndo;
		SetReferenceOperation(newTarget, *this).SwapReferences();
	}
	OVITO_ASSERT(pointer.get() == newTarget);
}

/******************************************************************************
* Adds a reference target to the internal list.
* Creates an undo record so the insertion can be undone at a later time.
******************************************************************************/
int VectorReferenceFieldBase::insertInternal(RefTarget* newTarget, int index)
{
    // Check object type
	if(newTarget && !newTarget->pluginClassDescriptor()->isKindOf(descriptor()->targetClass())) {
		OVITO_ASSERT_MSG(false, "VectorReferenceFieldBase::Insert", "Cannot add incompatible object to this vector reference field.");
		throw Exception(QString("Cannot add an object to a reference field of type %1 that has the incompatible type %2.").arg(descriptor()->targetClass()->name(), newTarget->pluginClassDescriptor()->name()));
	}

	class InsertReferenceOperation : public UndoableOperation
	{
	private:
	    intrusive_ptr<RefTarget> target;
		VectorReferenceFieldBase& reffield;
		int index;
	public:
    	InsertReferenceOperation(RefTarget* _target, VectorReferenceFieldBase& _reffield, int _index)
			: target(_target), reffield(_reffield), index(_index) {}

		virtual void undo() {
			RemoveReference();
		}
		virtual void redo() {
			AddReference();
		}
		virtual QString displayName() const { return "Insert reference"; }

		/// Adds the target to the list reference field.
		int AddReference() {
			CHECK_POINTER(&reffield);
			CHECK_OBJECT_POINTER(reffield.owner());
			OVITO_ASSERT(reffield.descriptor()->isVector());

			RefMaker* refmaker = reffield.owner();

			// Check for cyclic references.
			if(target && refmaker->isReferencedBy(target.get())) {
				OVITO_ASSERT(!UNDO_MANAGER.isUndoingOrRedoing());
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
				intrusive_ptr_add_ref(target.get());
				//MsgLogger() << "Creating reference from" << refmaker->pluginClassDescriptor()->name() << "to" << target->pluginClassDescriptor()->name() << endl;
			}

			// Add the RefMaker to the list of dependents of the new target.
			if(target && target->getDependents().contains(refmaker) == false)
				target->dependentsList().push_back(refmaker);

			// Inform derived classes.
			refmaker->onRefTargetInserted(*reffield.descriptor(), target.get(), index);

			// Send auto change message.
			reffield.sendChangeNotification();

			target = NULL;
			return index;
		}

		/// Removes the target from the list reference field.
		void RemoveReference() {
			OVITO_ASSERT(!target);
			CHECK_POINTER(&reffield);
			CHECK_OBJECT_POINTER(reffield.owner());
			OVITO_ASSERT(reffield.descriptor()->isVector());
			RefMaker* refmaker = reffield.owner();

			OVITO_ASSERT(index >= 0 && index < reffield.pointers.size());
			target = reffield.pointers.at(index);

			// Remove reference.
			reffield.pointers.remove(index);

			// Release old reference target if there are no more references to it.
			if(target) {
				//MsgLogger() << "Removing reference from" << refmaker->pluginClassDescriptor()->name() << "to" << target->pluginClassDescriptor()->name() << endl;

				intrusive_ptr_release(target.get());

				// Remove the refmaker from the old target's list of dependents.
				CHECK_OBJECT_POINTER(target.get());
				OVITO_ASSERT(target->getDependents().contains(refmaker));
				if(!refmaker->hasReferenceTo(target.get())) {
					target->dependentsList().remove(refmaker);
				}
			}

			// Inform derived classes.
			refmaker->onRefTargetRemoved(*reffield.descriptor(), target.get(), index);

			// Send auto change message.
			reffield.sendChangeNotification();
		}
	};

	if(UNDO_MANAGER.isRecording() && descriptor()->automaticUndo()) {
		InsertReferenceOperation* op = new InsertReferenceOperation(newTarget, *this, index);
		UNDO_MANAGER.addOperation(op);
		return op->AddReference();
	}
	else {
		UndoSuspender noUndo;
		return InsertReferenceOperation(newTarget, *this, index).AddReference();
	}
}

/// Removes the element at index position i.
void VectorReferenceFieldBase::remove(int i)
{
	OVITO_ASSERT(i >=0 && i < size());

	class RemoveReferenceOperation : public UndoableOperation
	{
	private:
	    intrusive_ptr<RefTarget> target;
		VectorReferenceFieldBase& reffield;
		int index;
	public:
    	RemoveReferenceOperation(VectorReferenceFieldBase& _reffield, int _index)
			: reffield(_reffield), index(_index) {}

		virtual void undo() {
			AddReference();
		}
		virtual void redo() {
			RemoveReference();
		}
		virtual QString displayName() const { return "Remove reference"; }

		/// Adds the target to the list reference field.
		int AddReference() {
			CHECK_POINTER(&reffield);
			CHECK_OBJECT_POINTER(reffield.owner());
			OVITO_ASSERT(reffield.descriptor()->isVector());

			RefMaker* refmaker = reffield.owner();

			// Check for cyclic references.
			if(target && refmaker->isReferencedBy(target.get())) {
				OVITO_ASSERT(!UNDO_MANAGER.isUndoingOrRedoing());
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
				intrusive_ptr_add_ref(target.get());
				//MsgLogger() << "Creating reference from" << refmaker->pluginClassDescriptor()->name() << "to" << target->pluginClassDescriptor()->name() << endl;
			}

			// Add the RefMaker to the list of dependents of the new target.
			if(target && target->getDependents().contains(refmaker) == false)
				target->dependentsList().push_back(refmaker);

			// Inform derived classes.
			refmaker->onRefTargetInserted(*reffield.descriptor(), target.get(), index);

			// Send auto change message.
			reffield.sendChangeNotification();

			target = NULL;
			return index;
		}

		/// Removes the target from the list reference field.
		void RemoveReference() {
			OVITO_ASSERT(!target);
			CHECK_POINTER(&reffield);
			CHECK_OBJECT_POINTER(reffield.owner());
			OVITO_ASSERT(reffield.descriptor()->isVector());
			RefMaker* refmaker = reffield.owner();

			OVITO_ASSERT(index >= 0 && index < reffield.pointers.size());
			target = reffield.pointers.at(index);

			// Remove reference.
			reffield.pointers.remove(index);

			// Release old reference target if there are no more references to it.
			if(target) {
				//MsgLogger() << "Removing reference from" << refmaker->pluginClassDescriptor()->name() << "to" << target->pluginClassDescriptor()->name() << endl;

				intrusive_ptr_release(target.get());

				// Remove the refmaker from the old target's list of dependents.
				CHECK_OBJECT_POINTER(target.get());
				OVITO_ASSERT(target->getDependents().contains(refmaker));
				if(!refmaker->hasReferenceTo(target.get())) {
					target->dependentsList().remove(refmaker);
				}
			}

			// Inform derived classes.
			refmaker->onRefTargetRemoved(*reffield.descriptor(), target.get(), index);

			// Send auto change message.
			reffield.sendChangeNotification();
		}
	};

	if(UNDO_MANAGER.isRecording() && descriptor()->automaticUndo()) {
		RemoveReferenceOperation* op = new RemoveReferenceOperation(*this, i);
		UNDO_MANAGER.addOperation(op);
		op->RemoveReference();
	}
	else {
		UndoSuspender noUndo;
		RemoveReferenceOperation(*this, i).RemoveReference();
	}
}


/// Clears all references at sets the vector size to zero.
void VectorReferenceFieldBase::clear()
{
	while(!pointers.empty())
		remove(pointers.size() - 1);
}

/******************************************************************************
* Return the human readable and localized name of the parameter field.
* This information is parsed from the plugin manifest file.
******************************************************************************/
QString PropertyFieldDescriptor::displayName() const
{
	if(_displayName.isEmpty())
		return identifier();
	else
		return _displayName;
}

/******************************************************************************
* If this reference field contains a reference to a controller than
* this method returns the unit that is associated with the controller.
* This method is used by the NumericalParameterUI class.
******************************************************************************/
ParameterUnit* PropertyFieldDescriptor::parameterUnit() const
{
	if(_parameterUnitClassDescriptor != NULL) {
		return UNITS_MANAGER.getUnit(_parameterUnitClassDescriptor);
	}
	return NULL;
}

};
