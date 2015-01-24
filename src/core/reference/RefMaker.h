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

#ifndef __OVITO_REFMAKER_H
#define __OVITO_REFMAKER_H

#include <core/Core.h>
#include "ReferenceEvent.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

class SingleReferenceFieldBase;		// defined in PropertyFieldDescriptor.h
class VectorReferenceFieldBase;		// defined in PropertyFieldDescriptor.h

/**
 * \brief Exception that is thrown when trying to create a cyclic reference.
 *
 * This exception object is thrown by the RefMaker class when
 * a cyclic reference would be created by setting a reference field to
 * a new target.
 */
class OVITO_CORE_EXPORT CyclicReferenceError : public Exception
{
public:

	/// \brief Default constructor.
	CyclicReferenceError() : Exception(QStringLiteral("Cyclic reference error")) {}
};

/**
 * \brief Base class for all objects that hold references to other objects (reference targets).
 */
class OVITO_CORE_EXPORT RefMaker : public OvitoObject
{
protected:

	/// \brief Constructor.
	RefMaker(DataSet* dataset = nullptr) : _dataset(dataset) {}

	/////////////////////////////// Reference field events ///////////////////////////////////

	/// \brief Is called when a RefTarget referenced by this object has generated an event.
	/// \param source A direct reference target of this RefMaker specifying the source of the message.
	///               Note that this may not be the sender or generator of the notification
	///               event. The sender is returned by ReferenceEvent::sender().
	/// \param event The notification event received by this RefMaker.
	/// \return \c true if the event should be recursively passed on to dependents of this object;
	///         \c false if the event should not be sent to other dependents.
	///
	/// \note When this method is overridden in sub-classes then the base implementation of this method
	///       should always be called to allow the base class to handle message from its specific
	///       reference targets.
	///
	/// The default implementation of this method does nothing and returns \c true if the event's
	/// shouldPropagate() method returns true.
	///
	/// \sa RefTarget::notifyDependents()
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) {
		return event->shouldPropagate();
	}

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
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) {}

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
	/// \sa VectorReferenceField::push_back()
	/// \sa referenceRemoved()
	virtual void referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex) {}

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
	/// \sa VectorReferenceField::remove()
	/// \sa referenceInserted()
	virtual void referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex) {}

	/// \brief Processes a notification event from a RefTarget referenced by this RefMaker.
	/// \param source Specifies the RefTarget referenced by this RefMaker that delivered the event.
	/// \param event The notification event.
	/// \return If \c true then the event is passed on to the dependents of this object.
	virtual bool handleReferenceEvent(RefTarget* source, ReferenceEvent* event);

	/// \brief Is called when the value of a property of this object has changed.
	/// \param field Specifies the property field of this RefMaker that has changed.
	///              This is always a non-animatable PropertyField.
	///
	/// This method can by overridden by derived classes that want to be informed when
	/// any of their property fields change.
	///
	/// \note When this method is overridden in sub-classes then the base implementation of this method
	///       should always be called from the new implementation to allow the base classes to handle
	///       messages for their specific property fields.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) {}

	/// \brief Stops observing a RefTarget object.
	/// \param target All references hold by the RefMaker to the this target are cleared.
	///
	/// All reference fields containing a pointer to \a target will be reset to \c NULL.
	/// If \a target is referenced in a VectorReferenceField then the item is
	/// removed from the vector field.
	///
	/// \undoable
	void clearReferencesTo(RefTarget* target);

	/// \brief Replaces all references of this RefMaker to some RefTarget with new ones.
	/// \param oldTarget Specifies which references should be replaced.
	/// \param newTarget Specifies the new target that should replace the old one.
	/// \undoable
	void replaceReferencesTo(RefTarget* oldTarget, RefTarget* newTarget);

	/// \brief Clears a reference field.
	/// \param field Specifies the reference field of this RefMaker to be cleared.
	///
	/// If the reference field specified by \a field is a single reference field then it is set to the value \c NULL.
	/// If it is a VectorReferenceField then all references are removed.
	///
	/// \undoable
	void clearReferenceField(const PropertyFieldDescriptor& field);

	/// \brief Clears all references held by this RefMarker.
	///
	/// All single reference fields are set to \c NULL and all vector reference
	/// fields are cleared.
	///
	/// \undoable
	void clearAllReferences();

	/// \brief Saves the class' contents to an output stream.
	/// \param stream The destination stream.
	///
	/// Derived classes can overwrite this virtual method to store their specific data
	/// in the output stream. The derived class \b must always call the base implementation of the saveToStream() method
	/// before it writes its own data to the stream.
	///
	/// The RefMaker implementation of this virtual method saves all
	/// references and referenced RefTarget object of this RefMaker to the output stream.
	///
	/// \sa loadFromStream()
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// \brief Loads the class' contents from an input stream.
	/// \param stream The source stream.
	/// \throw Exception when a parsing error has occurred.
	///
	/// Derived classes can overwrite this virtual method to read their specific data
	/// from the input stream. The derived class \b must always call the loadFromStream() method
	/// of the base class before it reads its own data from the stream.
	///
	/// The RefMaker implementation of this method restores all
	/// reference fields and loads the referenced objects that had been serialized to the data stream.
	///
	/// \sa saveToStream()
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// \brief Allows the object to parse the serialized contents of a property field in a custom way.
	/// \param stream The source stream.
	/// \param serializedField The property field to be parsed.
	/// \throw Exception when a parsing error has occurred.
	/// \return \c true if the function has parsed the field; \c false if the default parsing routine should be invoked.
	///
	/// Overriding this method is useful if a property field has been replaced by another. To maintain file
	/// compatibility, the object can parse the value of the old property field from the file and store it in the new field.
	///
	/// The default implementation returns \c false.
	virtual bool loadPropertyFieldFromStream(ObjectLoadStream& stream, const ObjectLoadStream::SerializedPropertyField& serializedField) { return false; }

public:

	/// \brief Returns true if this object is an instance of a RefTarget derived class.
	virtual bool isRefTarget() const { return false; }

	/////////////////////////// Runtime property field access ///////////////////////////////

	/// \brief Returns the value stored in a non-animatable property field of this RefMaker object.
	/// \param field The descriptor of a property field defined by this RefMaker derived class.
	/// \return The current value of the property field.
	/// \sa OvitoObjectType::firstPropertyField()
	/// \sa OvitoObjectType::findPropertyField()
	QVariant getPropertyFieldValue(const PropertyFieldDescriptor& field) const;

	/// \brief Sets the value stored in a non-animatable property field of this RefMaker object.
	/// \param field The descriptor of a property field defined by this RefMaker derived class.
	/// \param newValue The value to be assigned to the property. The QVariant data type must match the property data type.
	/// \sa OvitoObjectType::firstPropertyField()
	/// \sa OvitoObjectType::findPropertyField()
	void setPropertyFieldValue(const PropertyFieldDescriptor& field, const QVariant& newValue);

	/// \brief Loads the user-defined default values of this object's parameter fields from the
	///        application's settings store.
	///
	/// This function should be called immediately after creation of the object instance.
	/// It loads the default value for every property field for which the user has set
	/// a default value. This is usually the case for property fields that have the
	/// PROPERTY_FIELD_MEMORIZE flag set.
	///
	/// This function is recursive, i.e., it also loads default parameter values for
	/// referenced objects (when the PROPERTY_FIELD_MEMORIZE flag is set for this RefMaker's reference field).
	virtual void loadUserDefaults();

	/////////////////////////// Runtime reference field access //////////////////////////////

	/// \brief Looks up a reference field.
	/// \param field The descriptor of a reference field defined in this RefMaker derived class.
	/// \return The field object for this RefMaker instance and the specified field.
	/// \sa getVectorReferenceField()
	/// \sa OvitoObjectType::firstPropertyField()
	/// \sa OvitoObjectType::findPropertyField()
	const SingleReferenceFieldBase& getReferenceField(const PropertyFieldDescriptor& field) const;

	/// \brief Looks up a vector reference field.
	/// \param field The descriptor of a vector reference field defined in this RefMaker derived class.
	/// \return The field object for this RefMaker instance and the specified vector field.
	/// \sa getReferenceField()
	/// \sa OvitoObjectType::firstPropertyField()
	/// \sa OvitoObjectType::findPropertyField()
	const VectorReferenceFieldBase& getVectorReferenceField(const PropertyFieldDescriptor& field) const;

	////////////////////////////// Dependencies //////////////////////////////////

	/// \brief Checks whether this RefMaker has any (direct) references to a RefTarget.
	/// \param target Specifies the reference target.
	/// \return \c true if this RefMaker object has at least one direct reference to the given
	///         RefTarget \a target; \c false otherwise.
	///
	/// \sa isReferencedBy()
	bool hasReferenceTo(RefTarget* target) const;

	/// \brief Checks whether this object is directly or indirectly referenced by the given RefMaker.
	/// \param obj The RefMaker that might hold a reference to \c this object.
	///
	/// The RefMaker base implementation always returns \a false since this class is not a RefTarget and can therefore
	/// not be referenced. RefTarget overrides this method with a more meaningful implementation.
	virtual bool isReferencedBy(const RefMaker* obj) const { return false; }

	/// \brief Returns a list of all targets this RefMaker depends on (directly as well as indirectly).
	/// \return A list of all RefTargets that are directly or indirectly referenced by this RefMaker.
	/// \note The returned list is gathered recursively.
	QSet<RefTarget*> getAllDependencies() const;

	///////////////////////////// DataSet access ///////////////////////////////

	/// \brief Returns the dataset this object belongs to.
	DataSet* dataset() const {
		OVITO_ASSERT_MSG(_dataset != nullptr, "RefMaker::dataset()", "Tried to access non-existing parent dataset of RefMaker.");
		return _dataset;
	}

	/// \brief Changes the dataset this object belongs to.
	void setDataset(DataSet* dataset) { _dataset = dataset; }

protected:

	/// \brief This method is called after the reference counter of this object has reached zero
	///        and before the object is being deleted.
	virtual void aboutToBeDeleted() override;

private:

	/// \brief Recursive gathering function used by getAllDependencies().
	static void walkNode(QSet<RefTarget*>& nodes, const RefMaker* node);

	/// The dataset this object belongs to.
	DataSet* _dataset;

	friend class RefTarget;
	friend class SingleReferenceFieldBase;
	friend class VectorReferenceFieldBase;
	friend class PropertyFieldBase;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#include "NativePropertyFieldDescriptor.h"

#endif // __OVITO_REFMAKER_H
