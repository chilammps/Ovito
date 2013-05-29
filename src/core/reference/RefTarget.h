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
	/// \sa RefTarget::getDependents()
	class DependentsList : private QVarLengthArray<RefMaker*, 4> {
	private:
		typedef QVarLengthArray<RefMaker*, 4> BaseClass;
	public:
		typedef RefMaker** const_iterator;
		/// Default constructor.
		DependentsList() : QVarLengthArray<RefMaker*, 4>() {}
		/// Returns the number of dependents.
		inline int size() const { return BaseClass::size(); }
		/// Returns whether there are no dependents.
		inline bool empty() const { return BaseClass::isEmpty(); }
		/// Returns a dependent.
		inline RefMaker* operator[](int index) const { OVITO_ASSERT(index < size()); return BaseClass::operator[](index); }
		/// Checks whether an object is in the list.
		inline bool contains(RefMaker* o) const {
			const const_iterator enditer = end();
			for(const_iterator iter = begin(); iter != enditer; ++iter)
				if(*iter == o) return true;
			return false;
		}
		/// Returns an iterator to the first element.
		inline const_iterator begin() const { return const_cast<const_iterator>(BaseClass::data()); }
		/// Returns an iterator to the last element.
		inline const_iterator end() const { return begin() + size(); }
		/// Adds a dependent to the list.
		inline void push_back(RefMaker* o) { OVITO_ASSERT(!contains(o)); BaseClass::append(o); }
		/// Removes a dependent from the list.
		inline void remove(RefMaker* o) {
			RefMaker** iter = BaseClass::data();
			RefMaker* const* enditer = iter + size();
			OVITO_ASSERT(iter != enditer);
			// Find object's index.
			for(; *iter != o; ++iter) OVITO_ASSERT(iter != enditer);
			// Shift following elements by one.
			for(RefMaker** iter2 = iter+1; iter2 != enditer; ) {
				*iter = *iter2;
				iter = iter2++;
			}
			// Reduce array size by one.
			BaseClass::resize(size()-1);
		}
	};

protected:

	/// The default constructor.
	RefTarget() : RefMaker() {}

	/// \brief The virtual destructor.
	virtual ~RefTarget();

	/////////////////////// Overridable reference field event handlers ///////////////////////////////////

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
	/// The RefTarget implementation of this virtual method generates a REFERENCE_FIELD_CHANGED notification message
	///
	/// \sa ReferenceFieldMessage
	virtual void onRefTargetReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) {
		notifyDependents(ReferenceFieldMessage(this, REFERENCE_FIELD_CHANGED, field, oldTarget, newTarget));
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
	/// The RefTarget implementation of this virtual method generates a REFERENCE_FIELD_ADDED notification message
	///
	/// \sa ReferenceFieldMessage
	virtual void onRefTargetInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex) {
		notifyDependents(ReferenceFieldMessage(this, REFERENCE_FIELD_ADDED, field, NULL, newTarget, listIndex));
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
	/// The RefTarget implementation of this virtual method generates a REFERENCE_FIELD_REMOVED notification message
	///
	/// \sa ReferenceFieldMessage
	virtual void onRefTargetRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex) {
		notifyDependents(ReferenceFieldMessage(this, REFERENCE_FIELD_REMOVED, field, oldTarget, NULL, listIndex));
	}

	/// \brief Handles a notification message from a RefTarget referenced by this RefMaker.
	/// \param source Specifies the RefTarget referenced by this RefMaker that delivered the message.
	/// \param msg The notification message.
	/// \return If \c true then the message is passed on to all dependents of this object.
	virtual bool processTargetNotification(RefTarget* source, RefTargetMessage* msg);

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

	//////////////////////////////// Notification messages ////////////////////////////////////

	/// \brief Notifies all registered dependents that this RefTarget has changes in some way.
	/// \param msg The notification message to be sent to all dependents of this RefTarget.
	///
	/// \sa RefMaker::onRefTargetMessage()
	void notifyDependents(const RefTargetMessage& msg);

	/// \brief Sends a default RefTargetMessage with the given type.
	/// \param messageType The message type identifier passed to the RefTargetMessage constructor.
	///
	/// \sa RefMaker::onRefTargetMessage()
	void notifyDependents(int messageType) {
		notifyDependents(RefTargetMessage(this, messageType));
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
	/// First sends a REFTARGET_DELETED notification message.
	virtual void autoDeleteObject() override;

	/////////////////////////////// Editor interface /////////////////////////////

#if 0
	/// \brief Creates a PropertiesEditor for this object.
	/// \return The new editor component that allows the user to edit the properties of this RefTarget object.
	///         It will be automatically destroyed by the system when the editor is closed.
	///         Returns NULL if no editor component exists for this RefTarget.
	///
	/// This method should be implemented by derived classes that want to provide a graphical user interface
	/// that allows the user to edit the object properties. It is not necessary to override this method if the
	/// properties editor class for this RefTarget class is already specified in the plugin" manifest.
	virtual intrusive_ptr<PropertiesEditor> createPropertiesEditor();

	/// \brief Returns whether this object is currently being edited in the main command panel of the
	///        application's main window.
	/// \return \c true if there is an editor open in the main command panel for this RefTarget; \c false otherwise.
	bool isBeingEdited() const;

	/// \brief Returns the title of this object.
	/// \return A string that is used as label or title for this object in the user interface.
	///
	/// The default implementation returns PluginClassDescriptor::schematicTitle() for
	/// using the class of this object instance. Sub-classes can override this method to
	/// return a title that depends on the internal state of the object for instance.
	virtual QString schematicTitle();
#endif

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

