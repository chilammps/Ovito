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

#ifndef __OVITO_PROPERTIES_EDITOR_H
#define __OVITO_PROPERTIES_EDITOR_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/gui/widgets/general/RolloutContainer.h>
#include "PropertiesPanel.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/**
 * \brief Base class for property editors for RefTarget derived objects.
 * 
 * The specific properties editor for a RefTarget derived object can be created
 * using the RefTarget::createPropertiesEditor() method.
 * 
 * Property editors live in a PropertiesPanel.
 */
class OVITO_CORE_EXPORT PropertiesEditor : public RefMaker
{
	
protected:

	/// \brief The constructor.
	PropertiesEditor();

public:

	/// \brief The virtual destructor.
	virtual ~PropertiesEditor() { clearAllReferences(); }

	/// \brief This will bind the editor to the given container.
	/// \param container The properties panel that's the host of the editor.
	/// \param mainWindow The main window that hosts the editor.
	/// \param rolloutParams Specifies how the editor's rollouts should be created.
	///
	/// This method is called by the PropertiesPanel class to initialize the editor and to create the UI.
	void initialize(PropertiesPanel* container, MainWindow* mainWindow, const RolloutInsertionParameters& rolloutParams);
	
	/// \brief Returns the rollout container widget this editor is placed in.
	PropertiesPanel* container() const { return _container; }

	/// \brief Returns the main window that hosts the editor.
	MainWindow* mainWindow() const { return _mainWindow; }

	/// \brief Creates a new rollout in the rollout container and returns
	///        the empty widget that can then be filled with UI controls.
	/// \param title The title of the rollout.
	/// \param rolloutParams Specifies how the rollout should be created.
	/// \param helpPage The help page or topic in the user manual that describes this rollout.
	///
	/// \note The rollout is automatically deleted when the editor is deleted.
	QWidget* createRollout(const QString& title, const RolloutInsertionParameters& rolloutParams, const char* helpPage = nullptr);
	
	/// \brief Returns the object currently being edited in this properties editor.
	/// \return The RefTarget derived object which is being edited in this editor.
	///
	/// \note This can be another object than the one used to create the editor via
	///       RefTarget::createPropertiesEditor(). Editors are re-usable and the obejct being edited
	///       can be set with setEditObject().
	///
	/// \sa setEditObject()
	RefTarget* editObject() const { return _editObject; }

	/// \brief Executes the passed functor and catches any exceptions thrown during its execution.
	/// If an exception is thrown by the functor, all changes done by the functor
	/// so far will be undone and an error message is shown to the user.
	template<typename Function>
	void undoableTransaction(const QString& operationLabel, Function&& func) {
		UndoableTransaction::handleExceptions(dataset()->undoStack(), operationLabel, std::forward<Function>(func));
	}

public Q_SLOTS:

	/// \brief Sets the object being edited in this editor.
	/// \param newObject The new object to load into the editor. This must be of the same class
	///                  as the previous object. 
	///
	/// This method generates a contentsReplaced() and a contentsChanged() signal.
	void setEditObject(RefTarget* newObject) {
		OVITO_ASSERT_MSG(!editObject() || !newObject || newObject->getOOType().isDerivedFrom(editObject()->getOOType()),
				"PropertiesEditor::setEditObject()", "This properties editor was not made for this object class.");
		_editObject = newObject;
	}

Q_SIGNALS:

	/// \brief This signal is emitted by the editor when a new edit object
	///        has been loaded into the editor via the setEditObject() method.
	/// \sa newEditObject The new object loaded into the editor.
    void contentsReplaced(Ovito::RefTarget* newEditObject);

	/// \brief This signal is emitted by the editor when the current edit object has generated a TargetChanged
	///        event or if a new object has been loaded into editor via the setEditObject() method.
	/// \sa editObject The object that has changed.
    void contentsChanged(Ovito::RefTarget* editObject);

protected:
	
	/// Creates the user interface controls for the editor.
	/// This must be implemented by sub-classes.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) = 0;
	
	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Is called when the value of a reference field of this RefMaker changes.
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) override;

private:
	
	/// The container widget the editor is shown in.
	PropertiesPanel* _container;

	/// The main window that hosts the editor.
	MainWindow* _mainWindow;

	/// The object being edited in this editor.
	ReferenceField<RefTarget> _editObject;
	
	/// The list of rollout widgets that have been created by editor.
	/// The cleanup handler is used to delete them when the editor is being deleted.
	QObjectCleanupHandler _rollouts;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_editObject);
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_PROPERTIES_EDITOR_H
