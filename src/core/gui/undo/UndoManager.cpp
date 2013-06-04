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
#include <core/gui/undo/UndoManager.h>
#include <core/gui/actions/ActionManager.h>

namespace Ovito {

/// The singleton instance of the class.
QScopedPointer<UndoManager> UndoManager::_instance;

/******************************************************************************
* Initializes the undo manager.
******************************************************************************/
UndoManager::UndoManager() : _suspendCount(0), _index(-1), _cleanIndex(-1),
		_isUndoing(false), _isRedoing(false), _undoLimit(20)
{
}

/******************************************************************************
* Registers a single undoable operation.
* This object will be put onto the undo stack.
******************************************************************************/
void UndoManager::push(UndoableOperation* operation)
{
	OVITO_CHECK_POINTER(operation);
	OVITO_ASSERT_MSG(isUndoingOrRedoing() == false, "UndoManager::push()", "Cannot record an operation while undoing or redoing another operation.");
	OVITO_ASSERT_MSG(isRecording() == false, "UndoManager::push()", "Not in recording state.");
	if(_suspendCount > 0 || _compoundStack.empty()) {
		delete operation;
		return;
	}
	_compoundStack.back()->addOperation(operation);
}

/******************************************************************************
* Opens a new compound operation and assigns it the given display name.
******************************************************************************/
CompoundOperation* UndoManager::beginCompoundOperation(const QString& displayName)
{
	OVITO_ASSERT_MSG(isUndoingOrRedoing() == false, "UndoManager::beginCompoundOperation()", "Cannot record an operation while undoing or redoing another operation.");
	CompoundOperation* cop = new CompoundOperation(displayName);
	_compoundStack.push_back(cop);
	return cop;
}

/******************************************************************************
* Closes the current compound operation.
******************************************************************************/
void UndoManager::endCompoundOperation()
{
	OVITO_ASSERT_MSG(isUndoingOrRedoing() == false, "UndoManager::endCompoundOperation()", "Cannot record an operation while undoing or redoing another operation.");
	OVITO_ASSERT_MSG(!_compoundStack.empty(), "UndoManager::endCompoundOperation()", "Missing call to beginCompoundOperation().");

	// Take current compound operation from the stack.
	CompoundOperation* cop = currentCompoundOperation();
	if(!cop)
		throw Exception("Invalid operation.");
	_compoundStack.pop_back();

	// Check if the operation should be kept.
	if(_suspendCount > 0 || !cop->isSignificant()) {
		delete cop;	// Discard operation.
		return;
	}

	// Nest compound operations.
	CompoundOperation* parentOp = currentCompoundOperation();
	if(parentOp) {
		parentOp->addOperation(cop);
		return;
	}

	// Discard already undone operations.
	for(int i = index() + 1; i < count(); i++)
		delete _operations[i];
	_operations.resize(index() + 1);
	if(cleanIndex() > index()) _cleanIndex = -1;

	// Insert new operation.
	_operations.push_back(cop);
	_index++;
	OVITO_ASSERT(index() == count() - 1);
	limitUndoStack();
	indexChanged(index());
	cleanChanged(false);
	canUndoChanged(true);
	undoTextChanged(cop->displayName());
	canRedoChanged(false);
	redoTextChanged(QString());
}

/******************************************************************************
* Shrinks the undo stack to maximum number of undo steps.
******************************************************************************/
void UndoManager::limitUndoStack()
{
	if(_undoLimit < 0) return;
	int n = count() - _undoLimit;
	if(n > 0) {
		if(index() >= n) {
			for(int i = 0; i < n; i++)
				delete _operations[i];
			_operations.remove(0, n);
			_index -= n;
			indexChanged(index());
		}
	}
}

/******************************************************************************
* Resets the undo system. The undo stack will be cleared.
******************************************************************************/
void UndoManager::clear()
{
	for(UndoableOperation* op : _operations)
		delete op;
	_operations.clear();
	for(CompoundOperation* op : _compoundStack)
		delete op;
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
void UndoManager::setClean()
{
	if(!isClean()) {
		_cleanIndex = index();
		cleanChanged(cleanIndex());
	}
}

/******************************************************************************
* Marks the stack as dirty and emits cleanChanged() if the stack was not already dirty.
******************************************************************************/
void UndoManager::setDirty()
{
	bool signal = isClean();
	_cleanIndex = -2;
	if(signal) cleanChanged(false);
}

/******************************************************************************
* Undoes the last operation in the undo stack.
******************************************************************************/
void UndoManager::undo()
{
	OVITO_ASSERT_MSG(_compoundStack.empty(), "UndoManager::undo()", "Cannot undo last operation while a compound operation is open.");
	if(canUndo() == false) return;

	UndoSuspender noUndoRecording;

	UndoableOperation* curOp = _operations[index()];
	OVITO_CHECK_POINTER(curOp);
	_isUndoing = true;
	try {
		curOp->undo();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
	_isUndoing = false;
	_index--;
	OVITO_ASSERT(index() >= 0);
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
void UndoManager::redo()
{
	OVITO_ASSERT_MSG(_compoundStack.empty(), "UndoManager::redo()", "Cannot redo operation while a compound operation is open.");
	if(canRedo() == false) return;

	UndoSuspender noUndoRecording;

	UndoableOperation* nextOp = _operations[index() + 1];
	OVITO_CHECK_POINTER(nextOp);
	_isRedoing = true;
	try {
		nextOp->redo();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
	_isRedoing = false;
	_index++;
	indexChanged(index());
	cleanChanged(isClean());
	canUndoChanged(canUndo());
	undoTextChanged(undoText());
	canRedoChanged(canRedo());
	redoTextChanged(redoText());
}

class UndoAction : public QAction
{
	Q_OBJECT
public:
	UndoAction(QObject* parent, const QString& prefix) : QAction(parent), _prefix(prefix) {}
public Q_SLOTS:
	void setPrefixedText(const QString& text) { setText(tr("%1 %2").arg(_prefix).arg(text)); }
private:
	QString _prefix;
};

// Required for Automoc
#include "UndoManager.moc"

/******************************************************************************
* Creates an undo QAction object with the given parent.
******************************************************************************/
QAction* UndoManager::createUndoAction(QObject* parent)
{
    UndoAction* action = new UndoAction(parent, tr("Undo"));
    action->setEnabled(canUndo());
    action->setPrefixedText(undoText());
    connect(this, SIGNAL(canUndoChanged(bool)), action, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(undoTextChanged(QString)), action, SLOT(setPrefixedText(QString)));
    connect(action, SIGNAL(triggered()), this, SLOT(undo()));
    return action;
}

/******************************************************************************
* Creates a redo QAction object with the given parent.
******************************************************************************/
QAction* UndoManager::createRedoAction(QObject* parent)
{
    UndoAction* action = new UndoAction(parent, tr("Redo"));
    action->setEnabled(canRedo());
    action->setPrefixedText(redoText());
    connect(this, SIGNAL(canRedoChanged(bool)), action, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(redoTextChanged(QString)), action, SLOT(setPrefixedText(QString)));
    connect(action, SIGNAL(triggered()), this, SLOT(redo()));
    return action;
}

/******************************************************************************
* Undo the compound edit operation that was made.
******************************************************************************/
void CompoundOperation::undo()
{
	UndoSuspender noUndoRecording;
	for(int i = _subOperations.size() - 1; i >= 0; --i) {
		OVITO_CHECK_POINTER(_subOperations[i]);
		_subOperations[i]->undo();
	}
}

/******************************************************************************
* Re-apply the compound change, assuming that it has been undone.
******************************************************************************/
void CompoundOperation::redo()
{
	UndoSuspender noUndoRecording;
	for(int i = 0; i < _subOperations.size(); i++) {
		OVITO_CHECK_POINTER(_subOperations[i]);
		_subOperations[i]->redo();
	}
}

};
