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
 * \file RefTarget.h
 * \brief Contains the definition of the Ovito::RefTarget class.
 */

#ifndef __OVITO_REFTARGET_H
#define __OVITO_REFTARGET_H

#include <core/Core.h>
#include "RefMaker.h"

namespace Ovito {

class PropertiesEditor;		// defined in PropertiesEditor.h
class PropertiesPanel;		// defined in PropertiesPanel.h
class CloneHelper;			// defined in CloneHelper.h

/**
 * \brief Base class for objects that are referenced by RefMaker objects.
 */
class RefTarget : public RefMaker
{
	Q_OBJECT
	OVITO_OBJECT

public:

	/// \brief This light-weight array class holds the list of dependents of a RefTarget.
	class DependentsList : public QVarLengthArray<RefMaker*, 4> {
	private:
		typedef QVarLengthArray<RefMaker*, 4> BaseClass;
	public:
		/// Checks whether an object is in the list.
		inline bool contains(RefMaker* o) const {
			for(auto iter : *this)
				if(iter == o) return true;
			return false;
		}
		/// Adds a dependent to the list.
		inline void push_back(RefMaker* o) {
			OVITO_ASSERT(!contains(o));
			BaseClass::append(o);
		}
		/// Removes a dependent from the list.
		inline void remove(RefMaker* o) {
			for(auto iter = begin(); ; ++iter) {
				OVITO_ASSERT(iter != end());
				if(*iter == o) {
					erase(iter);
					return;
				}
			}
			OVITO_ASSERT(false);
		}
	};

protected:

	/// The default constructor.
	RefTarget() : RefMaker() {}

	/// \brief The virtual destructor.
	virtual ~RefTarget();

	//////////////////////////// Reference event handling ////////////////////////////////

	/// \brief Is called when the value of a reference field of this RefMaker changes.
	/// \param field Specifies the reference field of this RefMaker that has been changed.
	///              This is always a single reference ReferenceField.
	/// \param oldTarget The old target that was referenced by the ReferenceField. This can be \c NULL.
	/// \param newTarget The new target that is now referenced by the ReferenceField. This can be \c NULL.
	///
	/// This method can by overridden by derived classes that want to be informed when
	/// any of their reference fields are changed.
	///
	/// \note When this method is overridden in sub-classes then the base implementation of this method
	///       should always be called from the new implementation to allow the base classes to handle
	///       messages for their specific reference fields.
	///
	/// The RefTarget implementation of this virtual method generates a ReferenceEvent::ReferenceChanged notification event
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) override {
		ReferenceFieldEvent event(ReferenceEvent::ReferenceChanged, this, field, oldTarget, newTarget);
		notifyDependents(event);
	}

	/// \brief Is called when a RefTarget has been added to a VectorReferenceField of this RefMaker.
	/// \param field Specifies the reference field of this RefMaker to which a new entry has been added.
	///              This is always a VectorReferenceField.
	/// \param newTarget The new target added to the list of referenced objects.
	/// \param listIndex The index into the VectorReferenceField at which the new entry has been inserted.
	///
	/// This method can by overridden by derived classes that want to be informed when
	/// a reference has been added to one of its vector reference fields.
	///
	/// \note When this method is overridden in sub-classes then the base implementation of this method
	///       should always be called from the new implementation to allow the base classes to handle
	///       messages for their specific reference fields.
	///
	/// The RefTarget implementation of this virtual method generates a ReferenceEvent::ReferenceAdded notification event
	virtual void referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex) override {
		ReferenceFieldEvent event(ReferenceEvent::ReferenceAdded, this, field, NULL, newTarget, listIndex);
		notifyDependents(event);
	}

	/// \brief Is called when a RefTarget has been removed from a VectorReferenceField of this RefMaker.
	/// \param field Specifies the reference field of this RefMaker from which an entry has been removed.
	///              This is always a VectorReferenceField.
	/// \param oldTarget The old target that was reference before it has been removed from the vector reference field.
	/// \param listIndex The index into the VectorReferenceField at which the old entry was stored.
	///
	/// This method can by overridden by derived classes that want to be informed when
	/// a reference has been removed from one of its vector reference fields.
	///
	/// \note When this method is overridden in sub-classes then the base implementation of this method
	///       should always be called from the new implementation to allow the base classes to handle
	///       messages for their specific reference fields.
	///
	/// The RefTarget implementation of this virtual method generates a ReferenceEvent::ReferenceRemoved notification event
	virtual void referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex) override {
		ReferenceFieldEvent event(ReferenceEvent::ReferenceRemoved, this, field, oldTarget, NULL, listIndex);
		notifyDependents(event);
	}

	/// \brief Handles a notification event from a RefTarget referenced by this object.
	/// \param source Specifies the RefTarget that delivered the event.
	/// \param event The notification event.
	/// \return If \c true then the message is passed on to all dependents of this object.
	virtual bool handleReferenceEvent(RefTarget* source, ReferenceEvent* event) override;

	//////////////////////////////// Object cloning //////////////////////////////////////

	/// \brief Creates a copy of this RefTarget object.
	/// \param deepCopy If \c true, then all objects referenced by this RefTarget should also be copied.
	///                 If \c false, then the new object clone should just take over the references of the orginal object
	///                 and no copying of sub-objects takes place.
	/// \param cloneHelper Copying of sub-objects should be done using the passed CloneHelper instance.
	///                    It makes sure that only one copy per object is made through the whole object graph.
	///
	/// The default implementation of this method takes care of instance creation. It creates
	/// a new instance of the original object class.
	///
	/// Sub-classes should override this method and must always call the base class' version of this method
	/// to create the new instance. The returned smart pointer can safely be cast to the class type of the orginal
	/// object.
	///
	/// Every sub-class that has reference fields or other internal data fields should override this
	/// method and copy or transfer the members to the new clone.
	///
	/// \sa CloneHelper::cloneObject()
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper);

public:

	//////////////////////////////// Notification events ////////////////////////////////////

	/// \brief Sends an event to all dependents of this RefTarget.
	/// \param event The notification event to be sent to all dependents of this RefTarget.
	virtual void notifyDependents(ReferenceEvent& event);

	/// \brief Sends an event to all dependents of this RefTarget.
	/// \param eventType The event type passed to the ReferenceEvent constructor.
	inline void notifyDependents(ReferenceEvent::Type eventType) {
		ReferenceEvent event(eventType, this);
		notifyDependents(event);
	}

	////////////////////////////////// Dependency graph ///////////////////////////////////////

	/// \brief Checks whether this RefTarget is directly or indirectly referenced by a RefMaker.
	/// \param obj The RefMaker that might hold a reference to \c this object.
	/// \return \c true if \a obj has a direct or indirect reference to this RefTarget;
	///         \c false if \a obj does not depend on this RefTarget.
	virtual bool isReferencedBy(const RefMaker* obj) const override;

	/// \brief Returns the list of RefMaker objects that hold at least one direct reference
	///        to this RefTarget.
	/// \return A list of reference makers that directly depend on this RefTarget and receive
	///         notification messages from it.
	const DependentsList& dependents() const { return _dependents; }

	///////////////////////////// from PluginClass ///////////////////////////////

	/// \brief Deletes this object.
	///
	/// This function is automatically called on a reference target when it has no more dependents.
	/// First generates a ReferenceEvent::TargetDeleted notification event.
	virtual void autoDeleteObject() override;

	/////////////////////////////// Editor interface /////////////////////////////

	/// \brief Creates a PropertiesEditor for this object.
	/// \return The new editor component that allows the user to edit the properties of this RefTarget object.
	///         It will be automatically destroyed by the system when the editor is closed.
	///         Returns NULL if no editor component exists for this RefTarget.
	///
	/// This method should be implemented by derived classes that want to provide a graphical user interface
	/// to let the user edit the object's properties.
	virtual OORef<PropertiesEditor> createPropertiesEditor();

	/// \brief Determines whether this object is currently being edited in an PropertiesEditor.
	/// \return \c true if there is an active editor for this RefTarget; \c false otherwise.
	bool isBeingEdited() const;

	/// \brief Returns the title of this object.
	/// \return A string that is used as label or title for this object in the user interface.
	///
	/// The default implementation returns OvitoObjectType::objectTitle() for
	/// using the class of this object instance. Sub-classes can override this method to
	/// return a title that depends on the internal state of the object for instance.
	virtual QString objectTitle();

private:

	/// \brief Returns a modifiable list of RefMaker objects that depend on this RefTarget.
	DependentsList& dependents() { return _dependents; }

	/// The list of reference fields that hold a reference to this target.
	DependentsList _dependents;

	friend class RefMaker;
	friend class CloneHelper;
	friend class SingleReferenceFieldBase;
	friend class VectorReferenceFieldBase;
};

};

// Also include the CloneHelper header because it is always needed in conjunction with the RefTarget class.
#include "CloneHelper.h"

#endif // __OVITO_REFTARGET_H

