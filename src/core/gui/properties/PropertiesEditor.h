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
 * \file PropertiesEditor.h 
 * \brief Contains the definition of the Ovito::PropertiesEditor class.
 */

#ifndef __OVITO_PROPERTIES_EDITOR_H
#define __OVITO_PROPERTIES_EDITOR_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/gui/widgets/general/RolloutContainer.h>
#include "PropertiesPanel.h"

namespace Ovito {

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

	/// \brief The default constructor.
	PropertiesEditor();

public:

	/// \brief The virtual destructor.
	virtual ~PropertiesEditor() { clearAllReferences(); }

	/// \brief This will bind the editor to the given container.
	/// \param container The properties panel that's the host of the editor.
	/// \param rolloutParams Specifies how the editor's rollouts should be created.
	///
	/// This method is called by the PropertiesPanel class to initialize the editor and to create the GUI controls. 
	void initialize(PropertiesPanel* container, const RolloutInsertionParameters& rolloutParams) {		
		OVITO_CHECK_POINTER(container);
		OVITO_ASSERT_MSG(_container == NULL, "PropertiesEditor::initialize()", "Editor can only be initialized once.");
		_container = container;
		createUI(rolloutParams);
	}
	
	/// \brief Returns the rollout container widget this editor is placed in.
	/// \return The host of the editor.
	PropertiesPanel* container() const { return _container; }

	/// \brief Creates a new rollout in the rollout container and returns
	///        the empty widget that can then be filled with UI controls.
	/// \param title The title of the rollout.
	/// \param rolloutParams Specifies how the rollout should be created.
	///
	/// \note The rollout is automatically deleted when the editor is deleted.
	QWidget* createRollout(const QString& title, const RolloutInsertionParameters& rolloutParams);
	
	/// \brief Returns the object currently being edited in this properties editor.
	/// \return The RefTarget derived object which is being edited in this editor.
	///
	/// \note This can be another object than the one used to create the editor via
	///       RefTarget::createPropertiesEditor(). Editors are re-usable and the obejct being edited
	///       can be set with setEditObject().
	///
	/// \sa setEditObject()
	RefTarget* editObject() const { return _editObject; }

public Q_SLOTS:

	/// \brief Sets the object being edited in this editor.
	/// \param newObject The new object to load into the editor. This must be of the same class
	///                  as the previous object. 
	/// This will update the UI controls to reflect the new contents. Derived editor classes
	/// must override this method and update the values shown in the UI controls. 
	/// New implementations of this method must call this base implementation.
	///
	/// This method generates a contentsReplaced() and a contentsChanged() signal.
	///
	/// \sa editObject()
	virtual void setEditObject(RefTarget* newObject) {
		OVITO_ASSERT_MSG(!editObject() || !newObject || newObject->getOOType().isDerivedFrom(editObject()->getOOType()),
				"PropertiesEditor::setEditObject()", "This properties editor was not made for this object class.");
		_editObject = newObject;
		contentsReplaced(newObject);
		contentsChanged(newObject);
	}

Q_SIGNALS:

	/// \brief This signal is emitted by the editor when a new edit object
	///        has been loaded into the editor via the setEditObject() method.
	/// \sa newEditObject The new object loaded into the editor.
	void contentsReplaced(RefTarget* newEditObject);

	/// \brief This signal is emitted by the editor when the current edit object has generated a TargetChanged
	///        event or if a new object has been loaded into editor via the setEditObject() method.
	/// \sa editObject The object that has changed.
	void contentsChanged(RefTarget* editObject);

protected:
	
	/// Creates the user interface controls for the editor.
	/// This must be implemented by sub-classes.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) = 0;
	
	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private:
	
	/// The container widget the editor is shown in.
	PropertiesPanel* _container;

	/// The object being edited in this editor.
	ReferenceField<RefTarget> _editObject;
	
	/// The list of rollout widgets that have been created by editor.
	/// The cleanup handler is used to delete them when the editor is being deleted.
	QObjectCleanupHandler _rollouts;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_editObject);
};

};

#endif // __OVITO_PROPERTIES_EDITOR_H
