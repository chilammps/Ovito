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
#include <core/reference/RefMaker.h>
#include "UndoStack.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Undo)

/******************************************************************************
* Increments the suspend count of the undo stack associated with the given
* object.
******************************************************************************/
UndoSuspender::UndoSuspender(RefMaker* object)
{
	OVITO_CHECK_OBJECT_POINTER(object);
	if(object->dataset()) {
		_suspendCount = &object->dataset()->undoStack()._suspendCount;
		++(*_suspendCount);
	}
	else _suspendCount = nullptr;
}

/******************************************************************************
* Initializes the undo manager.
******************************************************************************/
UndoStack::UndoStack() : _suspendCount(0), _index(-1), _cleanIndex(-1),
		_isUndoing(false), _isRedoing(false), _undoLimit(20)
{
}

/******************************************************************************
* Registers a single undoable operation.
* This object will be put onto the undo stack.
******************************************************************************/
void UndoStack::push(UndoableOperation* operation)
{
	OVITO_CHECK_POINTER(operation);
	OVITO_ASSERT_MSG(isUndoingOrRedoing() == false, "UndoStack::push()", "Cannot record an operation while undoing or redoing another operation.");
	OVITO_ASSERT_MSG(!isSuspended(), "UndoStack::push()", "Not in recording state.");

	UndoSuspender noUndo(*this);

	// Discard previously undone operations.
	_operations.resize(index() + 1);
	if(cleanIndex() > index()) _cleanIndex = -1;

	if(_compoundStack.empty()) {
		_operations.emplace_back(operation);
		_index++;
		OVITO_ASSERT(index() == count() - 1);
		limitUndoStack();
		indexChanged(index());
		cleanChanged(false);
		canUndoChanged(true);
		undoTextChanged(operation->displayName());
		canRedoChanged(false);
		redoTextChanged(QString());
	}
	else {
		_compoundStack.back()->addOperation(operation);
	}
}

/******************************************************************************
* Opens a new compound operation and assigns it the given display name.
******************************************************************************/
void UndoStack::beginCompoundOperation(const QString& displayName)
{
	OVITO_ASSERT_MSG(isUndoingOrRedoing() == false, "UndoStack::beginCompoundOperation()", "Cannot record an operation while undoing or redoing another operation.");
	_compoundStack.emplace_back(new CompoundOperation(displayName));
}

/******************************************************************************
* Closes the current compound operation.
******************************************************************************/
void UndoStack::endCompoundOperation(bool commit)
{
	OVITO_ASSERT_MSG(isUndoingOrRedoing() == false, "UndoStack::endCompoundOperation()", "Cannot record an operation while undoing or redoing another operation.");
	OVITO_ASSERT_MSG(!_compoundStack.empty(), "UndoStack::endCompoundOperation()", "Missing call to beginCompoundOperation().");

	if(!commit) {
		UndoSuspender noUndo(*this);

		// Undo operations in current compound operation first.
		resetCurrentCompoundOperation();
		// Then discard compound operation.
		_compoundStack.pop_back();
	}
	else {

		// Take current compound operation from the macro stack.
		std::unique_ptr<CompoundOperation> cop = std::move(_compoundStack.back());
		_compoundStack.pop_back();

		// Check if the operation should be kept.
		if(_suspendCount > 0 || !cop->isSignificant()) {
			// Discard operation.
			UndoSuspender noUndo(*this);
			cop.reset();
			return;
		}

		// Put new operation on stack.
		push(cop.release());
	}
}

/******************************************************************************
* Undoes all actions of the current compound operation.
******************************************************************************/
void UndoStack::resetCurrentCompoundOperation()
{
	OVITO_ASSERT_MSG(isUndoingOrRedoing() == false, "UndoStack::resetCurrentCompoundOperation()", "Cannot reset operation while undoing or redoing another operation.");
	OVITO_ASSERT_MSG(!_compoundStack.empty(), "UndoStack::resetCurrentCompoundOperation()", "Missing call to beginCompoundOperation().");

	CompoundOperation* cop = _compoundStack.back().get();
	// Undo operations.
	UndoSuspender noUndo(*this);
	_isUndoing = true;
	try {
		cop->undo();
		cop->clear();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
	_isUndoing = false;
}

/******************************************************************************
* Shrinks the undo stack to maximum number of undo steps.
******************************************************************************/
void UndoStack::limitUndoStack()
{
	if(_undoLimit < 0) return;
	int n = count() - _undoLimit;
	if(n > 0) {
		if(index() >= n) {
			UndoSuspender noUndo(*this);
			_operations.erase(_operations.begin(), _operations.begin() + n);
			_index -= n;
			indexChanged(index());
		}
	}
}

/******************************************************************************
* Resets the undo system. The undo stack will be cleared.
******************************************************************************/
void UndoStack::clear()
{
	_operations.clear();
	_compoundStack.clear();
	_index = -1;
	_cleanIndex = -1;
	indexChanged(index());
	cleanChanged(isClean());
	canUndoChanged(false);
	canRedoChanged(false);
	undoTextChanged(QString());
	redoTextChanged(QString());
}

/******************************************************************************
* Marks the stack as clean and emits cleanChanged() if the stack was not already clean.
******************************************************************************/
void UndoStack::setClean()
{
	if(!isClean()) {
		_cleanIndex = index();
		cleanChanged(cleanIndex());
	}
}

/******************************************************************************
* Marks the stack as dirty and emits cleanChanged() if the stack was not already dirty.
******************************************************************************/
void UndoStack::setDirty()
{
	bool signal = isClean();
	_cleanIndex = -2;
	if(signal) cleanChanged(false);
}

/******************************************************************************
* Undoes the last operation in the undo stack.
******************************************************************************/
void UndoStack::undo()
{
	OVITO_ASSERT(isRecording() == false);
	OVITO_ASSERT(isUndoingOrRedoing() == false);
	OVITO_ASSERT_MSG(_compoundStack.empty(), "UndoStack::undo()", "Cannot undo last operation while a compound operation is open.");
	if(canUndo() == false) return;

	UndoableOperation* curOp = _operations[index()].get();
	OVITO_CHECK_POINTER(curOp);
	_isUndoing = true;
	suspend();
	try {
		curOp->undo();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
	_isUndoing = false;
	resume();
	_index--;
	indexChanged(index());
	cleanChanged(isClean());
	canUndoChanged(canUndo());
	undoTextChanged(undoText());
	canRedoChanged(canRedo());
	redoTextChanged(redoText());
}

/******************************************************************************
* Redoes the last undone operation in the undo stack.
******************************************************************************/
void UndoStack::redo()
{
	OVITO_ASSERT(isRecording() == false);
	OVITO_ASSERT(isUndoingOrRedoing() == false);
	OVITO_ASSERT_MSG(_compoundStack.empty(), "UndoStack::redo()", "Cannot redo operation while a compound operation is open.");
	if(canRedo() == false) return;

	UndoableOperation* nextOp = _operations[index() + 1].get();
	OVITO_CHECK_POINTER(nextOp);
	_isRedoing = true;
	suspend();
	try {
		nextOp->redo();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
	_isRedoing = false;
	resume();
	_index++;
	indexChanged(index());
	cleanChanged(isClean());
	canUndoChanged(canUndo());
	undoTextChanged(undoText());
	canRedoChanged(canRedo());
	redoTextChanged(redoText());
}

/******************************************************************************
* Undo the compound edit operation that was made.
******************************************************************************/
void UndoStack::CompoundOperation::undo()
{
	for(int i = (int)_subOperations.size() - 1; i >= 0; --i) {
		OVITO_CHECK_POINTER(_subOperations[i]);
		_subOperations[i]->undo();
	}
}

/******************************************************************************
* Re-apply the compound change, assuming that it has been undone.
******************************************************************************/
void UndoStack::CompoundOperation::redo()
{
	for(size_t i = 0; i < _subOperations.size(); i++) {
		OVITO_CHECK_POINTER(_subOperations[i]);
		_subOperations[i]->redo();
	}
}

/******************************************************************************
* Is called to undo an operation.
******************************************************************************/
void TargetChangedUndoOperation::undo()
{
	_target->notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Is called to redo an operation.
******************************************************************************/
void TargetChangedRedoOperation::redo()
{
	_target->notifyDependents(ReferenceEvent::TargetChanged);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
