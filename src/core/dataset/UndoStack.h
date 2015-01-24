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

#ifndef __OVITO_UNDO_STACK_H
#define __OVITO_UNDO_STACK_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Undo)

/**
 * \brief Abstract base class for records of undoable operations.
 *
 * All atomic operations or functions that modify the scene in same way
 * should register an UndoableOperation with the UndoStack using UndoStack::push().
 *
 * For each specific operation a sub-class of UndoableOperation should be defined that
 * allows the UndoStack to undo or to re-do the operation at a later time.
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
	virtual QString displayName() const { return QStringLiteral("Undoable operation"); }

	/// \brief Undoes the operation encapsulated by this object.
	///
	/// This method is called by the UndoStack to undo the operation.
	virtual void undo() = 0;

	/// \brief Re-apply the change, assuming that it had been undone before.
	///
	/// This method is called by the UndoStack to re-do the operation.
	/// The default implementation calls undo(). That means, undo() must be implemented such
	/// that it works both ways.
	virtual void redo() { undo(); }
};

/**
 * \brief This helper class records a change to an object's property.
 *
 * It stores the old value of the property, which will be restored on a call to undo().
 *
 * The user of this class has to specify the property getter and setter methods as
 * template parameters.
 */
template<typename ValueType, typename ObjectType, typename GetterFunction, typename SetterFunction>
class SimpleValueChangeOperation : public UndoableOperation
{
public:

	/// \brief Constructor.
	SimpleValueChangeOperation(ObjectType* obj, GetterFunction getterFunc, SetterFunction setterFunc) :
		_obj(obj),
		_oldValue((obj->*getterFunc)()),
		_getterFunc(getterFunc),
		_setterFunc(setterFunc) {}

	/// \brief Restores the old property value.
	virtual void undo() override {
		// Swap old value and current property value.
		ValueType temp = (_obj.get()->*_getterFunc)();
		(_obj.get()->*_setterFunc)(_oldValue);
		_oldValue = temp;
	}

private:

	/// The value getter function.
	GetterFunction _getterFunc;

	/// The value setter function.
	SetterFunction _setterFunc;

	/// The old value of the property.
	ValueType _oldValue;

	/// The object whose property was changed.
	OORef<ObjectType> _obj;
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
	
private:

	/// The object whose property has been changed.
	OORef<OvitoObject> _object;
	
	/// The name of the property that has been changed.
	const char* _propertyName;
	
	/// The old value of the property.
	QVariant _oldValue;
};

/**
 * \brief This undo record simply generates a TargetChanged event for a RefTarget whenever an operation is undone.
 */
class OVITO_CORE_EXPORT TargetChangedUndoOperation : public UndoableOperation
{
public:

	/// \brief Constructor.
	/// \param target The object that is being changed.
	TargetChangedUndoOperation(RefTarget* target) : _target(target) {}

	virtual void undo() override;
	virtual void redo() override {}

private:

	/// The object that has been changed.
	OORef<RefTarget> _target;
};

/**
 * \brief This undo record simply generates a TargetChanged event for a RefTarget whenever an operation is redone.
 */
class OVITO_CORE_EXPORT TargetChangedRedoOperation : public UndoableOperation
{
public:

	/// \brief Constructor.
	/// \param target The object that is being changed.
	TargetChangedRedoOperation(RefTarget* target) : _target(target) {}

	virtual void undo() override {}
	virtual void redo() override;

private:

	/// The object that has been changed.
	OORef<RefTarget> _target;
};

/**
 * \brief Stores and manages the undo stack.
 * 
 * The UndoStack records all user operations. Operations can be undone or reversed
 * one by one.
 */
class OVITO_CORE_EXPORT UndoStack : public QObject
{
	Q_OBJECT
	
public:

	/// Constructor.
	UndoStack();

	/// \brief Begins composition of a macro command with the given text description.
	/// \param text A human-readable name that is shown in the edit menu to describe
	///             the operation.
	///
	/// \note Each call to beginCompoundOperation() must be followed by a call to
	///       endCompoundOperation() to commit the operation. Multiple compound operations
	///       can be nested by multiple calls to beginCompoundOperation() followed by the same
	///       number of calls to endCompoundOperation().
	void beginCompoundOperation(const QString& text);

	/// \brief Ends composition of a macro command.
	/// \param commit If true, the macro operation is put on the undo stack. If false,
	///        all actions of the macro operation are undone, and nothing is put on the undo stack.
	void endCompoundOperation(bool commit = true);

	/// \brief Undoes all actions of the current compound operation.
	void resetCurrentCompoundOperation();

	/// \brief Returns whether the manager is currently recording undoable operations.
	/// \return \c true if the UndoStack currently records any changes made to the scene on its stack.
	///         \c false if changes to the scene are ignored by the UndoStack.
	///
	/// The recording state can be controlled via the suspend() and resume() methods.
	/// Or it can be temporarily suspended using the UndoSuspender helper class.
	bool isRecording() const { return !isSuspended() && _compoundStack.empty() == false; }

	/// \brief Records a single operation.
	/// \param operation An instance of a UndoableOperation derived class that encapsulates
	///                  the operation. The UndoStack becomes the owner of
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

	/// \brief Returns true if the recording of operations is currently suspended.
	bool isSuspended() const { return _suspendCount != 0; }

	/// \brief Resumes the recording of undoable operations.
	/// 
	/// This re-enables recording of undoable operations after it has 
	/// been suspended by a call to suspend().
	void resume() { 
		OVITO_ASSERT_MSG(_suspendCount > 0, "UndoStack::resume()", "resume() has been called more often than suspend().");
		_suspendCount--;
	}

	/// \brief Indicates whether the undo stack is currently undoing a recorded operation.
	/// \return \c true if the UndoStack is currently restoring the application state before a user
	///         operation. This is usually due to a call to undo();
	///         \c false otherwise.
	bool isUndoing() const { return _isUndoing; }

	/// \brief Indicates whether the undo stack is currently redoing a previously undone operation.
	/// \return \c true if the UndoStack is currently replaying a recorded operation.
	///         This is usually due to a call to redo();
	///         \c false otherwise.
	bool isRedoing() const { return _isRedoing; }

	/// \brief Indicates whether the undo stack is currently undoing or redoing a recorded operation.
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
	int count() const { return (int)_operations.size(); }

	/// \brief If the stack is in the clean state, returns true; otherwise returns false.
	bool isClean() const { return index() == cleanIndex(); }

	/// \brief Returns the clean index.
	int cleanIndex() const { return _cleanIndex; }

	/// \brief Gets the maximum number of undo steps to hold in memory.
	/// \return The maximum number of steps the UndoStack maintains.
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

	/// Registers an undo record for changing a property of an object.
	///
	/// The setter method for a property of an object should call this function
	/// to create an undo record that allows to restore the old property value.
	/// Note that the function must be called by the setter method before the new
	/// property value is stored, because this method will query the old property
	/// value by calling the getter method.
	template<typename ValueType, class ObjectType, typename GetterFunction, typename SetterFunction>
	void undoablePropertyChange(ObjectType* obj, GetterFunction getterFunc, SetterFunction setterFunc) {
		if(isRecording()) {
			push(new SimpleValueChangeOperation<ValueType, ObjectType,
					GetterFunction, SetterFunction>(obj, getterFunc, setterFunc));
		}
	}

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


	/**
	 * \brief This class is used to combine multiple UndoableOperation objects into one.
	 */
	class CompoundOperation : public UndoableOperation
	{
	public:

		/// \brief Creates an empty compound operation with the given display name.
		/// \param name The localized and human-readable display name for this compound operation.
		CompoundOperation(const QString& name) : _displayName(name) {}

		/// \brief Deletes all sub-operations of this compound operation.
		virtual ~CompoundOperation() {}

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
		void addOperation(UndoableOperation* operation) { _subOperations.emplace_back(operation); }

		/// \brief Indicates whether this UndoableOperation is significant or can be ignored.
		/// \return \c true if the CompoundOperation contains at least one sub-operation; \c false it is empty.
		bool isSignificant() const { return _subOperations.empty() == false; }

		/// \brief Removes all sub-operations from this compound operation.
		void clear() { _subOperations.clear(); }

	private:

		/// List of contained operations.
		std::vector<std::unique_ptr<UndoableOperation>> _subOperations;

		/// Stores the display name of this compound passed to the constructor.
		QString _displayName;
	};

private:

	/// The stack with records of undoable operations.
	std::deque<std::unique_ptr<UndoableOperation>> _operations;

	/// A call to suspend() increases this value by one.
	/// A call to resume() decreases it.
	int _suspendCount;

	/// Current position in the undo stack. This is where
	/// new undoable edits will be inserted.
	int _index;

	/// The state which has been marked as clean.
	int _cleanIndex;

	/// The stack of open compound records.
	std::vector<std::unique_ptr<CompoundOperation>> _compoundStack;

	/// Maximum number of records in the undo stack.
	int _undoLimit;

	/// Indicates if we are currently undoing an operation.
	bool _isUndoing;

	/// Indicates if we are currently redoing an operation.
	bool _isRedoing;

	friend class UndoSuspender;
};

/**
 * \brief A small helper object that suspends recording of undoable operations while it
 *        exists.
 * 
 * The constructor of this class calls UndoStack::suspend() and
 * the destructor calls UndoStack::resume().
 * 
 * Use this to make your code exception-safe.
 * Create an instance of this class on the stack to suspend recording of operations
 * during the lifetime of the class instance.
 */
class OVITO_CORE_EXPORT UndoSuspender {
public:
	UndoSuspender(UndoStack& undoStack) : _suspendCount(&undoStack._suspendCount) { ++(*_suspendCount); }
	UndoSuspender(RefMaker* object);
	~UndoSuspender() {
		if(_suspendCount) {
			OVITO_ASSERT_MSG((*_suspendCount) > 0, "UndoStack::resume()", "resume() has been called more often than suspend().");
			--(*_suspendCount);
		}
	}
private:
	int* _suspendCount;
};

/**
 * Helper class that begins a new compound operation.
 * Unless the operation committed, the destructor of this class will undo all operations.
 */
class OVITO_CORE_EXPORT UndoableTransaction
{
public:

	/// Constructor that calls UndoStack::beginCompoundOperation().
	UndoableTransaction(UndoStack& undoStack, const QString& displayName) : _undoStack(undoStack), _committed(false) {
		if(!_undoStack.isSuspended())
			_undoStack.beginCompoundOperation(displayName);
	}

	/// Destructor that undoes all recorded operations unless commit() was called.
	~UndoableTransaction() {
		if(!_committed && !_undoStack.isSuspended()) {
			_undoStack.endCompoundOperation(false);
		}
	}

	/// Commits all recorded operations by calling UndoStack::endCompoundOperation().
	void commit() {
		OVITO_ASSERT(!_committed);
		_committed = true;
		if(!_undoStack.isSuspended())
			_undoStack.endCompoundOperation();
	}

	/// Executes the passed functor and catches any exceptions thrown during its execution.
	/// If an exception is thrown by the functor, all changes done by the functor
	/// so far will be undone, the error message is shown to the user, and this function returns false.
	/// If no exception is thrown, the operations are committed and this function returns true.
	template<typename Function>
	static bool handleExceptions(UndoStack& undoStack, const QString& operationLabel, Function&& func) {
		try {
			UndoableTransaction transaction(undoStack, operationLabel);
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

	UndoStack& _undoStack;
	bool _committed;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_UNDO_STACK_H
