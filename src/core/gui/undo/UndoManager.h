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
 * \file UndoManager.h 
 * \brief Contains the definition of the Ovito::UndoManager class.
 */

#ifndef __OVITO_UNDO_MANAGER_H
#define __OVITO_UNDO_MANAGER_H

#include <core/Core.h>

namespace Ovito {

/**
 * \brief This class records a change to a Qt property to a QObject derived class.
 * 
 * This QUndoCommand class can be used to record
 * a change to a Qt property of an object. The property must defined through the
 * standard Qt mechanism using the \c Q_PROPERTY macro.
 */
class SimplePropertyChangeOperation : public QUndoCommand
{
public:

	/// \brief Constructor.
	/// \param obj The object whose property is being changed.
	/// \param propName The identifier of the property that is changed. This is the identifier
	///                 name given to the property in the \c Q_PROPERTY macro.  
	/// \note This class does not make a copy of the property name parameter.
	///       So the caller should only pass constant string literals to this constructor.
	SimplePropertyChangeOperation(OvitoObject* obj, const char* propName, const QString& text = QString()) :
		QUndoCommand(text), object(obj), propertyName(propName)
	{
		if(text.isEmpty())
			setText(QString("Change %1").arg(propertyName));

		// Make a copy of the current property value.
		oldValue = object->property(propertyName);
		OVITO_ASSERT_MSG(oldValue.isValid(), "SimplePropertyChangeOperation", "The object does not have a property with the given name.");  
	}

	///	\brief Provides a localized, human readable description of this operation.
	virtual QString displayName() const;

	/// \brief Restores the old property value.
	virtual void undo() override {
		// Swap old value and current property value.
		QVariant temp = object->property(propertyName);
		object->setProperty(propertyName, oldValue);
		oldValue = temp;
	}

	/// \brief Re-apply the change, assuming that it has been undone. 
	virtual void redo() override { undo(); }
	
private:

	/// The object whose property has been changed.
	OORef<OvitoObject> object;
	
	/// The name of the property that has been changed.
	const char* propertyName;
	
	/// The old value of the property.
	QVariant oldValue;
};

/**
 * \brief Stores and manages the undo stack.
 * 
 * The UndoManager records all user operations. Operations can be undone or reversed
 * one by one.
 */
class UndoManager : public QUndoStack
{
	Q_OBJECT
	
public:

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the ActionManager singleton class.
	inline static UndoManager& instance() {
		if(!_instance) _instance.reset(new UndoManager());
		return *_instance.data();
	}

	/// \brief Begins composition of a macro command with the given text description.
	void beginMacro(const QString& text) {
		OVITO_ASSERT_MSG(!_isRecording, "UndoManager::beginMacro", "Undo recording is already active. endMacro() needs to be called first.");
		OVITO_ASSERT_MSG(_suspendCount == 0, "UndoManager::beginMacro", "Undo recording is suspended. Cannot start a new macro at this time.");
		QUndoStack::beginMacro(text);
		setActive(true);
	}

	/// \brief Ends composition of a macro command.
	void endMacro() {
		OVITO_ASSERT_MSG(_isRecording, "UndoManager::endMacro", "Undo recording is not active. beginMacro() needs to be called first.");
		OVITO_ASSERT_MSG(_suspendCount == 0, "UndoManager::endMacro", "Undo recording was suspended since the call to beginMacro().");
		QUndoStack::endMacro();
		setActive(false);
	}

	/// \brief Returns whether the manager is currently recording undoable operations.
	/// \return \c true if the UndoManager currently records any changes made to the scene on its stack.
	///         \c false if changes to the scene are ignored by the UndoManager.
	///
	/// The recording state can be controlled via the suspend() and resume() methods.
	/// Or it can be temporarily suspended using the UndoSuspender helper class.
	bool isRecording() const { return _suspendCount == 0 && _isRecording; }

	/// \brief Records a single operation.
	/// \param operation An instance of a QUndoCommand derived class that encapsulates
	///                  the operation. The UndoManager becomes the owner of  
	///                  this object and is responsible for its deletion.
	void push(QUndoCommand* operation);
	
	/// \brief Suspends the recording of undoable operations.
	///
	/// Recording of operations is suspended by this method until a call to resume().
	/// If suspend() is called multiple times then resume() must be called the same number of
	/// times until recording is enabled again.
	///
	/// It is recommended to use the UndoSuspender helper class to suspend recording because
	/// this is more exception save than the suspend()/resume() combination.
	void suspend() { _suspendCount++; }

	/// \brief Resumes the recording of undoable operations.
	/// 
	/// This re-enables recording of undoable operations after it has 
	/// been suspended by a call to suspend().
	void resume() { 
		OVITO_ASSERT_MSG(_suspendCount > 0, "UndoManager::resume()", "resume() has been called more often than suspend().");
		_suspendCount--;
	}

	/// \brief Indicates whether the undo manager is currently undoing a recorded operation.
	/// \return \c true if the UndoManager is currently restoring the application state before a user
	///         operation. This is usually due to a call to undo();
	///         \c false otherwise.
	bool isUndoing() const { return _isUndoing; }

	/// \brief Indicates whether the undo manager is currently redoing a breviously undone operation.
	/// \return \c true if the UndoManager is currently replaying a recorded operation. 
	///         This is usually due to a call to redo();
	///         \c false otherwise.
	bool isRedoing() const { return _isRedoing; }

	/// \brief Indicates whether the undo manager is currently undoing or redoing a recorded operation.
	/// \return \c true if currently an operation from the undo stack is being undone or redone, i.e.
	///         isUndoing() or isRedoing() returns \c true; \c false otherwise.
	bool isUndoingOrRedoing() const { return isUndoing() || isRedoing(); }
	
public Q_SLOTS:

	/// \brief Resets the undo stack.
	void reset() { OVITO_ASSERT(!_isRecording); clear(); }

	/// \brief Undoes the last operation in the undo stack.
	void undo();

	/// \brief Re-does the last undone operation in the undo stack.
	void redo();

private:

	/// A call to suspend() increases this value by one.
	/// A call to resume() decreases it.
	int _suspendCount;

	/// Indicates that the undo manager is currently recording operations.
	bool _isRecording;

	/// Indicates if we are currently undoing an operation.
	bool _isUndoing;

	/// Indicates if we are currently redoing an operation.
	bool _isRedoing;

private:
    
	/// Private constructor.
	/// This is a singleton class; no public instances are allowed.
	UndoManager();

	/// The singleton instance of this class.
	static QScopedPointer<UndoManager> _instance;
};

/**
 * \brief A small helper object that suspends recording of undoable operations while it
 *        exists. It can be used to make your code exception-safe.
 * 
 * The constructor of this class calls UndoManager::suspend() and
 * the destructor calls UndoManager::resume().
 * 
 * Create an instance of this class on the stack to suspend recording of operations
 * during the lifetime of the class instance.
 */
struct UndoSuspender {
	UndoSuspender() { UndoManager::instance().suspend(); }
	~UndoSuspender() { UndoManager::instance().resume(); }
};

};

#endif // __OVITO_UNDO_MANAGER_H
