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
 * \brief Abstract base class for records of undoable operations.
 *
 * All atomic operations or functions that modify the scene in same way
 * should register an UndoableOperation with the UndoManager using UndoManager::push().
 *
 * For each specific operation a sub-class of UndoableOperation should be defined that
 * allows the UndoManager to undo or to re-do the operation at a later time.
 *
 * Multiple atomic operations can be combined into a CompoundOperation. They can then be undone
 * or redone at once.
 */
class OVITO_CORE_EXPORT UndoableOperation
{
public:

	/// \brief A virtual destructor.
	///
	/// The default implementation does nothing.
	virtual ~UndoableOperation() {}

	/// \brief Provides a localized, human readable description of this operation.
	/// \return A localized string that describes the operation. It is shown in the
	///         edit menu of the application.
	///
	/// The default implementation returns a default string, but it should be overridden
	/// by all sub-classes.
	virtual QString displayName() const { return "Undoable operation"; }

	/// \brief Undoes the operation encapsulated by this object.
	///
	/// This method is called by the UndoManager to undo the operation.
	virtual void undo() = 0;

	/// \brief Re-apply the change, assuming that it had been undone before.
	///
	/// This method is called by the UndoManager to re-do the operation.
	virtual void redo() = 0;

	/// \brief Indicates whether this is a compound operation.
	/// \return \c true if this is an instance of the CompoundOperation class; \c false otherwise.
	virtual bool isCompoundOperation() const { return false; }
};

/**
 * \brief This class is used to combine multiple UndoableOperation objects into one.
 */
class OVITO_CORE_EXPORT CompoundOperation : public UndoableOperation
{
public:

	/// \brief Creates an empty compound operation with the given display name.
	/// \param name The localized and human-readable display name for this compound operation.
	///             It will be returned by the displayName() method.
	///             It can later be changed using the setDisplayName() method.
	/// \sa displayName()
	CompoundOperation(const QString& name) : _displayName(name) {}

	/// \brief Deletes all sub-operations of this compound operation.
	virtual ~CompoundOperation() {
		for(UndoableOperation* op : _subOperations)
			delete op;
	}

	/// \brief Provides a localized, human readable description of this operation.
	/// \return A localized string that describes the operation. It is shown in the
	///         edit menu of the application.
	virtual QString displayName() const override { return _displayName; }

	/// \brief Sets this operation's display name to a new string.
	/// \param newName The localized and human-readable display name for this compound operation.
	/// \sa displayName()
	void setDisplayName(const QString& newName) { _displayName = newName; }

	/// Undo the edit operation that was made.
	virtual void undo() override;

	/// Re-apply the change, assuming that it has been undone.
	virtual void redo() override;

	/// \brief Adds a sub-record to this compound operation.
	/// \param operation An instance of a UndoableOperation derived class that encapsulates
	///                  the operation. The CompoundOperation becomes the owner of
	///                  this object and is responsible for its deletion.
	void addOperation(UndoableOperation* operation) { _subOperations.push_back(operation); }

	/// \brief Indicates whether this UndoableOperation is significant or can be ignored.
	/// \return \c true if the CompoundOperation contains at least one sub-operation; \c false it is empty.
	bool isSignificant() const { return _subOperations.empty() == false; }

	/// \brief Removes all sub-records from this compound after optionally undoing all sub-operation.
	/// \param undo Controls whether all sub-operations should undone in reverse order before they are removed from
	///             the CompoundOperation.
	///
	/// This method calls UndoableOperation::undo() on all sub-operations contained in this compound.
	void clear(bool undo = true) {
		if(undo)
			this->undo();
		for(UndoableOperation* op : _subOperations)
			delete op;
		_subOperations.clear();
	}

	/// \brief Indicates whether this is a compound operation.
	/// \return Always \c true.
	virtual bool isCompoundOperation() const override { return true; }

private:

	/// List of contained operations.
	QVector<UndoableOperation*> _subOperations;

	/// Stores the display name of this compound passed to the constructor.
	QString _displayName;
};

/**
 * \brief This class records a change to a Qt property to a QObject derived class.
 * 
 * This UndoableOperation can be used to record
 * a change to a Qt property of an object. The property must defined through the
 * standard Qt mechanism using the \c Q_PROPERTY macro.
 */
class OVITO_CORE_EXPORT SimplePropertyChangeOperation : public UndoableOperation
{
public:

	/// \brief Constructor.
	/// \param obj The object whose property is being changed.
	/// \param propName The identifier of the property that is changed. This is the identifier
	///                 name given to the property in the \c Q_PROPERTY macro.  
	/// \note This class does not make a copy of the property name parameter.
	///       So the caller should only pass constant string literals to this constructor.
	SimplePropertyChangeOperation(OvitoObject* obj, const char* propName) :
		_object(obj), _propertyName(propName)
	{
		// Make a copy of the current property value.
		_oldValue = _object->property(_propertyName);
		OVITO_ASSERT_MSG(_oldValue.isValid(), "SimplePropertyChangeOperation", "The object does not have a property with the given name.");
	}

	/// \brief Restores the old property value.
	virtual void undo() override {
		// Swap old value and current property value.
		QVariant temp = _object->property(_propertyName);
		_object->setProperty(_propertyName, _oldValue);
		_oldValue = temp;
	}

	/// \brief Re-apply the change, assuming that it has been undone. 
	virtual void redo() override { undo(); }
	
private:

	/// The object whose property has been changed.
	OORef<OvitoObject> _object;
	
	/// The name of the property that has been changed.
	const char* _propertyName;
	
	/// The old value of the property.
	QVariant _oldValue;
};

/**
 * \brief Stores and manages the undo stack.
 * 
 * The UndoManager records all user operations. Operations can be undone or reversed
 * one by one.
 */
class OVITO_CORE_EXPORT UndoManager : public QObject
{
	Q_OBJECT
	
public:

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the ActionManager singleton class.
	inline static UndoManager& instance() {
		OVITO_ASSERT_MSG(_instance != nullptr, "UndoManager::instance", "Singleton object is not initialized yet.");
		return *_instance;
	}

	/// \brief Begins composition of a macro command with the given text description.
	/// \param text A human-readable name that is shown in the edit menu and that describes
	///             the operation.
	/// \return A pointer to the newly created compound operation entry that has been put on top
	///         of the undo stack. It can be used to restore the old application state when the current
	///         operation has been aborted.
	///
	/// The current compound operation on top of the stack can also be retrieved via
	/// currentCompoundOperation().
	///
	/// \note Each call to beginCompoundOperation() must be followed by a call to
	///       endCompoundOperation() to commit the operation. Multiple compound operations
	///       can be nested by multiple calls to beginCompoundOperation() followed by the same
	///       number of calls to endCompoundOperation().
	CompoundOperation* beginCompoundOperation(const QString& text);

	/// \brief Ends composition of a macro command.
	void endCompoundOperation();

	/// \brief Gets the current compound record on the stack that is being
	///        filled with undoable operation records.
	/// \return The last compound operation opened using beginCompoundOperation() or \c NULL
	///         If the there is currently no compound operation open.
	CompoundOperation* currentCompoundOperation() const {
        if(_compoundStack.empty()) return nullptr;
		return _compoundStack.back();
	}

	/// \brief Returns whether the manager is currently recording undoable operations.
	/// \return \c true if the UndoManager currently records any changes made to the scene on its stack.
	///         \c false if changes to the scene are ignored by the UndoManager.
	///
	/// The recording state can be controlled via the suspend() and resume() methods.
	/// Or it can be temporarily suspended using the UndoSuspender helper class.
	bool isRecording() const { return _suspendCount == 0 && _compoundStack.empty() == false; }

	/// \brief Records a single operation.
	/// \param operation An instance of a UndoableOperation derived class that encapsulates
	///                  the operation. The UndoManager becomes the owner of  
	///                  this object and is responsible for its deletion.
	void push(UndoableOperation* operation);
	
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

	/// \brief Indicates whether the undo manager is currently redoing a previously undone operation.
	/// \return \c true if the UndoManager is currently replaying a recorded operation. 
	///         This is usually due to a call to redo();
	///         \c false otherwise.
	bool isRedoing() const { return _isRedoing; }

	/// \brief Indicates whether the undo manager is currently undoing or redoing a recorded operation.
	/// \return \c true if currently an operation from the undo stack is being undone or redone, i.e.
	///         isUndoing() or isRedoing() returns \c true; \c false otherwise.
	bool isUndoingOrRedoing() const { return isUndoing() || isRedoing(); }
	
	/// \brief Returns true if there is an operation available for undo; otherwise returns false.
	bool canUndo() const { return index() >= 0; }

	/// \brief Returns true if there is an operation available for redo; otherwise returns false.
	bool canRedo() const { return index() < count() - 1; }

	/// \brief Returns the text of the command which will be undone in the next call to undo().
	QString undoText() const { return canUndo() ? _operations[index()]->displayName() : QString(); }

	/// \brief Returns the text of the command which will be redone in the next call to redo().
	QString redoText() const { return canRedo() ? _operations[index() + 1]->displayName() : QString(); }

	/// \brief Returns the index of the current operation.
	///
	/// This is the operation that will be undone on the next call to undo().
	/// It is not always the top-most operation on the stack, since a number of operations may have been undone.
	int index() const { return _index; }

	/// \brief Returns the number of operations on the stack. Compound operations are counted as one operation.
	int count() const { return _operations.size(); }

	/// \brief If the stack is in the clean state, returns true; otherwise returns false.
	bool isClean() const { return index() == cleanIndex(); }

	/// \brief Returns the clean index.
	int cleanIndex() const { return _cleanIndex; }

	/// \brief Gets the maximum number of undo steps to hold in memory.
	/// \return The maximum number of steps the UndoManager maintains on its stack.
	///         A negative value means infinite number of undo steps.
	///
	/// If the maximum number of undo steps is reached then the oldest operation at the bottom of the
	/// stack are removed.
	int undoLimit() const { return _undoLimit; }

	/// \brief Sets the maximum number of undo steps to hold in memory.
	/// \param steps The maximum height of the undo stack.
	///              A negative value means infinite number of undo steps.
	void setUndoLimit(int steps) { _undoLimit = steps; limitUndoStack(); }

	/// \brief Shrinks the undo stack to maximum number of undo steps.
	///
	/// If the current stack is higher then the maximum number of steps then the oldest entries
	/// from the bottom of the stack are removed.
	void limitUndoStack();

	/// Creates an undo QAction object with the given parent. Triggering this action will cause a call to undo().
	QAction* createUndoAction(QObject* parent);

	/// Creates a redo QAction object with the given parent. Triggering this action will cause a call to redo().
	QAction* createRedoAction(QObject* parent);

public Q_SLOTS:

	/// \brief Resets the undo stack.
	void clear();

	/// \brief Undoes the last operation in the undo stack.
	void undo();

	/// \brief Re-does the last undone operation in the undo stack.
	void redo();

	/// \brief Marks the stack as clean and emits cleanChanged() if the stack was not already clean.
	void setClean();

	/// \brief Marks the stack as dirty and emits cleanChanged() if the stack was not already dirty.
	void setDirty();

Q_SIGNALS:

	/// This signal is emitted whenever the value of canUndo() changes.
	void canUndoChanged(bool canUndo);

	/// This signal is emitted whenever the value of canRedo() changes.
	void canRedoChanged(bool canRedo);

	/// This signal is emitted whenever the value of undoText() changes.
	void undoTextChanged(const QString& undoText);

	/// This signal is emitted whenever the value of redoText() changes.
	void redoTextChanged(const QString& redoText);

	/// This signal is emitted whenever an operation modifies the state of the document.
	void indexChanged(int index);

	/// This signal is emitted whenever the stack enters or leaves the clean state.
	void cleanChanged(bool clean);

private:

	/// The stack with records of undoable operations.
    QVector<UndoableOperation*> _operations;

	/// A call to suspend() increases this value by one.
	/// A call to resume() decreases it.
	int _suspendCount;

	/// Current position in the undo stack. This is where
	/// new undoable edits will be inserted.
	int _index;

	/// The state which has been marked as clean.
	int _cleanIndex;

	/// The stack of open compound records.
	QVector<CompoundOperation*> _compoundStack;

	/// Maximum number of records in the undo stack.
	int _undoLimit;

	/// Indicates if we are currently undoing an operation.
	bool _isUndoing;

	/// Indicates if we are currently redoing an operation.
	bool _isRedoing;

private:
    
	/// Private constructor.
	/// This is a singleton class; no public instances are allowed.
	UndoManager();

	/// Create the singleton instance of this class.
	static void initialize() { _instance = new UndoManager(); }

	/// Deletes the singleton instance of this class.
	static void shutdown() { delete _instance; _instance = nullptr; }

	/// The singleton instance of this class.
	static UndoManager* _instance;

	friend class Application;
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

/**
 * Helper class that begins a new compound operation.
 * Unless the operation committed, the destructor of this class will undo all operations.
 */
class UndoableTransaction
{
public:

	/// Constructor that calls UndoManager::beginCompoundOperation().
	UndoableTransaction(const QString& displayName) : _committed(false) {
		UndoManager::instance().beginCompoundOperation(displayName);
	}

	/// Destructor that undoes all recorded operations unless commit() was called.
	~UndoableTransaction() {
		if(!_committed) {
			UndoManager::instance().currentCompoundOperation()->clear();
			UndoManager::instance().endCompoundOperation();
		}
	}

	/// Commits all recorded operations by calling UndoManager::endCompoundOperation().
	void commit() {
		OVITO_ASSERT(!_committed);
		_committed = true;
		UndoManager::instance().endCompoundOperation();
	}

	/// Executes the passed functor and catches any exceptions during its execution.
	/// If an exception is thrown by the functor, all changes done by the functor
	/// so far will be undone, the error message is shown to the user, and this function returns false.
	/// If no exception is thrown, the operations are committed and this function returns true.
	template<typename Function>
	static bool handleExceptions(const QString& operationLabel, Function func) {
		try {
			UndoableTransaction transaction(operationLabel);
			func();
			transaction.commit();
			return true;
		}
		catch(const Exception& ex) {
			ex.showError();
			return false;
		}
	}

private:

	bool _committed;
};

};

#endif // __OVITO_UNDO_MANAGER_H
