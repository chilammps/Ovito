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
UndoManager::UndoManager() : _isRecording(false), _suspendCount(0), _isUndoing(false), _isRedoing(false)
{
}

/******************************************************************************
* Registers a single undoable operation.
* This object will be put onto the undo stack.
******************************************************************************/
void UndoManager::push(QUndoCommand* operation)
{
	OVITO_ASSERT_MSG(isRecording(), "UndoManager::push", "Undo recording is not active.");
	OVITO_ASSERT_MSG(isUndoingOrRedoing() == false, "UndoManager::addOperation()", "Cannot record an operation while undoing or redoing another operation.");
	QUndoStack::push(operation);
}

/******************************************************************************
* Undoes the last operation in the undo stack.
******************************************************************************/
void UndoManager::undo()
{
	UndoSuspender noUndoRecording;
	_isUndoing = true;
	try {
		QUndoStack::undo();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
	catch(...) {}
	_isUndoing = false;
}

/******************************************************************************
* Re-does the last undone operation in the undo stack.
******************************************************************************/
void UndoManager::redo()
{
	UndoSuspender noUndoRecording;
	_isRedoing = true;
	try {
		QUndoStack::redo();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
	catch(...) {}
	_isRedoing = false;
}

};
