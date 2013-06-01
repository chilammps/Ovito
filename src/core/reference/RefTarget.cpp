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
#include <core/reference/RefTarget.h>
#include <core/reference/CloneHelper.h>
#include <core/gui/undo/UndoManager.h>
#if 0
#include <core/gui/properties/PropertiesEditor.h>
#include <core/gui/ApplicationManager.h>
#include <core/gui/mainwnd/MainFrame.h>
#include <core/gui/panels/CommandPanel.h>
#include <core/plugins/Plugin.h>
#endif

namespace Ovito {

// Gives the class run-time type information.
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(RefTarget, RefMaker);

/******************************************************************************
* The virtual destructor.
******************************************************************************/
RefTarget::~RefTarget()
{
	// Make sure there are no more dependents left.
	// This should be the case if this RefTarget has been deleted using autoDeleteObject().
	OVITO_ASSERT_MSG(dependents().empty(), "RefTarget destructor", "Object has not been deleted via autoDeleteObject() method.");
}

/******************************************************************************
* Deletes this target object.
******************************************************************************/
void RefTarget::autoDeleteObject()
{
	// This will remove all references to this target object.
	notifyDependents(ReferenceEvent::TargetDeleted);

	// This will remove all reference held by the object itself.
	RefMaker::autoDeleteObject();
}

/******************************************************************************
* Notifies all registered dependents by sending out a message.
******************************************************************************/
void RefTarget::notifyDependents(ReferenceEvent& event)
{
	OVITO_CHECK_OBJECT_POINTER(this);
	OVITO_ASSERT_MSG(event.sender() == this, "RefTarget::notifyDependents()", "The notifying object is not the sender given in the event object.");

	// Be careful here: The list of dependents can change at any time while broadcasting
	// the message.
	for(int i = dependents().size() - 1; i >= 0; --i) {
		if(i >= dependents().size()) continue;
		OVITO_CHECK_OBJECT_POINTER(this);
		OVITO_CHECK_OBJECT_POINTER(dependents()[i]);
		dependents()[i]->handleReferenceEvent(this, &event);
	}

#ifdef OVITO_DEBUG
	if(event.type() == ReferenceEvent::TargetDeleted && !dependents().empty()) {
		qDebug() << "Object being deleted:" << this;
		for(int i = 0; i < dependents().size(); i++) {
			qDebug() << "  Dependent" << i << ":" << dependents()[i];
		}
		OVITO_ASSERT_MSG(false, "RefTarget deletion", "RefTarget has generated a TargetDeleted event but it still has dependents.");
	}
#endif
}

/******************************************************************************
* Handles a change notification message from a RefTarget.
* This implementation calls the onRefTargetMessage method
* and passes the message on to dependents of this RefTarget.
******************************************************************************/
bool RefTarget::handleReferenceEvent(RefTarget* source, ReferenceEvent* event)
{
	OVITO_CHECK_OBJECT_POINTER(this);

	// Let this object process the message.
	if(!RefMaker::handleReferenceEvent(source, event))
		return false;

	// Pass message on to dependents of this RefTarget.
	for(int i = dependents().size() - 1; i >=0 ; --i) {
		OVITO_ASSERT(i < dependents().size());
		OVITO_CHECK_OBJECT_POINTER(dependents()[i]);
		dependents()[i]->handleReferenceEvent(this, event);
		OVITO_CHECK_OBJECT_POINTER(this);
	}

	return true;
}


/******************************************************************************
* Checks if this object is directly or indirectly referenced by the given RefMaker.
******************************************************************************/
bool RefTarget::isReferencedBy(const RefMaker* obj) const
{
	for(RefMaker* m : dependents()) {
		OVITO_CHECK_OBJECT_POINTER(m);
		if(m == obj) return true;
		if(m->isReferencedBy(obj)) return true;
	}
	return false;
}

/******************************************************************************
* Creates a copy of this RefTarget object.
* If deepCopy is true, then all objects referenced by this RefTarget should be copied too.
* This copying should be done via the passed CloneHelper instance.
* Classes that override this method MUST call the base class' version of this method
* to create an instance. The base implementation of RefTarget::clone() will create an
* instance of the derived class which can safely be cast.
******************************************************************************/
OORef<RefTarget> RefTarget::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Create a new instance of the object's class.
	OORef<RefTarget> clone = static_object_cast<RefTarget>(getOOType().createInstance());
	if(!clone || !clone->getOOType().isDerivedFrom(getOOType()))
		throw Exception(tr("Failed to create clone instance of class %1.").arg(getOOType().name()));

	// Clone properties and referenced objects.
	for(const OvitoObjectType* clazz = &getOOType(); clazz; clazz = clazz->superClass()) {
		for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field; field = field->next()) {
			if(field->isReferenceField()) {
				if(field->isVector() == false) {
					OVITO_ASSERT(field->singleStorageAccessFunc != NULL);
					const SingleReferenceFieldBase& sourceField = field->singleStorageAccessFunc(this);
					// Clone reference target.
					OORef<RefTarget> clonedReference;
					if(field->flags().testFlag(PROPERTY_FIELD_NEVER_CLONE_TARGET))
						clonedReference = (RefTarget*)sourceField;
					else if(field->flags().testFlag(PROPERTY_FIELD_ALWAYS_CLONE))
						clonedReference = cloneHelper.cloneObject((RefTarget*)sourceField, deepCopy);
					else if(field->flags().testFlag(PROPERTY_FIELD_ALWAYS_DEEP_COPY))
						clonedReference = cloneHelper.cloneObject((RefTarget*)sourceField, true);
					else
						clonedReference = cloneHelper.copyReference((RefTarget*)sourceField, deepCopy);
					// Store in reference field of destination object.
					field->singleStorageAccessFunc(clone.get()).setValue(clonedReference.get());
				}
				else {
					OVITO_ASSERT(field->vectorStorageAccessFunc != NULL);
					// Clone all reference targets in the source vector.
					const VectorReferenceFieldBase& sourceField = field->vectorStorageAccessFunc(this);
					VectorReferenceFieldBase& destField = field->vectorStorageAccessFunc(clone.get());
					for(int i=0; i<sourceField.size(); i++) {
						OORef<RefTarget> clonedReference;
						// Clone reference target.
						if(field->flags().testFlag(PROPERTY_FIELD_NEVER_CLONE_TARGET))
							clonedReference = (RefTarget*)sourceField[i];
						else if(field->flags().testFlag(PROPERTY_FIELD_ALWAYS_CLONE))
							clonedReference = cloneHelper.cloneObject((RefTarget*)sourceField[i], deepCopy);
						else if(field->flags().testFlag(PROPERTY_FIELD_ALWAYS_DEEP_COPY))
							clonedReference = cloneHelper.cloneObject((RefTarget*)sourceField[i], true);
						else
							clonedReference = cloneHelper.copyReference((RefTarget*)sourceField[i], deepCopy);
						// Store in reference field of destination object.
						destField.insertInternal(clonedReference.get());
					}
				}
			}
			else {
				// Just copy primitive value for property fields.
				clone->setPropertyFieldValue(*field, getPropertyFieldValue(*field));
			}
		}
	}

	return clone;
}

#if 0
/******************************************************************************
* Creates a PropertiesEditor for this object.
******************************************************************************/
PropertiesEditor::SmartPtr RefTarget::createPropertiesEditor()
{
	try {
		// Look in the meta data of this RefTarget derived class and its super classes for the right editor class.
		PluginClassDescriptor* clazz = pluginClassDescriptor();
		do {
			QDomElement propertiesEditorElement = clazz->getMetaData("Properties-Editor");
			if(propertiesEditorElement.isElement()) {
				PluginClassDescriptor* editorClass = clazz->plugin()->getRequiredClass(propertiesEditorElement);
				if(!editorClass->isKindOf(PLUGINCLASSINFO(PropertiesEditor)))
					throw Exception(tr("The class %1 specified in the manifest of class %2 is not derived from the PropertiesEditor base class.").arg(editorClass->name(), clazz->name()));
				return dynamic_object_cast<PropertiesEditor>(editorClass->createInstance());
			}
			clazz = clazz->baseClass();
		}
		while(clazz != NULL);
	}
	catch(Exception& ex) {
		ex.prependGeneralMessage(tr("Could no create editor component for the %1 object.").arg(schematicTitle()));
		ex.showError();
	}
	return NULL;
}

/******************************************************************************
* Returns whether this object is currently being edited in the main command
* panel of the application's main window.
******************************************************************************/
bool RefTarget::isBeingEdited() const
{
	if(!APPLICATION_MANAGER.guiMode()) return false;
	return MAIN_FRAME->commandPanel()->editObject() == this;
}
#endif

};
