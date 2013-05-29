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
 * \file RefMaker.h
 * \brief Contains the definition of the Ovito::RefMaker class.
 */

#ifndef __OVITO_REFMAKER_H
#define __OVITO_REFMAKER_H

#include <core/Core.h>
#include "ReferenceEvent.h"

namespace Ovito {

class RefTarget;					// defined in RefTarget.h
class PropertyFieldDescriptor;		// defined in PropertyFieldDescriptor.h
class SingleReferenceFieldBase;		// defined in PropertyFieldDescriptor.h
class VectorReferenceFieldBase;		// defined in PropertyFieldDescriptor.h

/**
 * \brief Exception that is thrown when trying to create a cyclic reference.
 *
 * This exception object is thrown by the RefMaker class when
 * a cyclic reference would be created by setting a reference field to
 * a new target.
 */
class CyclicReferenceError : public Exception
{
public:

	/// \brief Default constructor.
	CyclicReferenceError() : Exception("Cyclic reference error") {}
};

/**
 * \brief Base class for all objects that hold references to other objects (reference targets).
 */
class RefMaker : public QObject
{
	Q_OBJECT

protected:

	/// \brief The default constructor.
	/// Subclasses should initialize their reference fields in the constructor using
	/// the \c INIT_PROPERTY_FIELD macro.
	RefMaker() : OvitoObject() {}

	///////////////////////////// Reference field event handlers ///////////////////////////////////

	/// \brief Handles a notification message from a RefTarget referenced by this RefMaker.
	/// \param source Specifies the RefTarget referenced by this RefMaker that delivered the message.
	/// \param msg The notification message.
	/// \return If \c true then the message is passed on to all dependents of this object.
	///
	/// Normaly you don't have to call or override this methods.
	/// Override onRefTargetMessage() to process notification messages sent by referenced objects.
	///
	/// \sa onRefTargetMessage()
	virtual bool processTargetNotification(RefTarget* source, RefTargetMessage* msg);

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

	/// \brief Replaces all references of this RefMaker to some RefTarget with new ones.
	/// \param oldTarget Specifies which references should be replaced.
	/// \param newTarget Specifies the new target that should replace the old one.
	/// \note This is the same method as above but using a smart pointer for the parameter \a newTarget.
	/// \undoable
	template<class T>
	void replaceReferencesTo(RefTarget* oldTarget, const intrusive_ptr<T>& newTarget) {
		replaceReferencesTo(oldTarget, newTarget.get());
	}

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

public:

	/////////////////////////// Runtime property field access ///////////////////////////////

	/// \brief Returns the value stored in a non-animatable property field of this RefMaker object.
	/// \param field The descriptor of a property field defined by this RefMaker derived class.
	/// \return The current value of the property field.
	/// \sa PluginClassDescriptor::firstPropertyField()
	/// \sa PluginClassDescriptor::findPropertyField()
	QVariant getPropertyFieldValue(const PropertyFieldDescriptor& field) const;

	/// \brief Sets the value stored in a non-animatable property field of this RefMaker object.
	/// \param field The descriptor of a property field defined by this RefMaker derived class.
	/// \param newValue The value to be assigned to the property. The QVariant data type must match the property data type.
	/// \sa PluginClassDescriptor::firstPropertyField()
	/// \sa PluginClassDescriptor::findPropertyField()
	void setPropertyFieldValue(const PropertyFieldDescriptor& field, const QVariant& newValue);

	/////////////////////////// Runtime reference field access //////////////////////////////

	/// \brief Looks up a reference field.
	/// \param field The descriptor of a reference field defined in this RefMaker derived class.
	/// \return The field object for this RefMaker instance and the specified field.
	/// \sa getVectorReferenceField()
	/// \sa PluginClassDescriptor::firstPropertyField()
	/// \sa PluginClassDescriptor::findPropertyField()
	const SingleReferenceFieldBase& getReferenceField(const PropertyFieldDescriptor& field) const;

	/// \brief Looks up a vector reference field.
	/// \param field The descriptor of a vector reference field defined in this RefMaker derived class.
	/// \return The field object for this RefMaker instance and the specified vector field.
	/// \sa getReferenceField()
	/// \sa PluginClassDescriptor::firstPropertyField()
	/// \sa PluginClassDescriptor::findPropertyField()
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

	///////////////////////////// from PluginClass ///////////////////////////////

	/// \brief Deletes this object.
	///
	/// This implementation releases all references held by this RefMaker before deleting the object.
	virtual void autoDeleteObject() override;

private:

	/// \brief Recursive gathering function used by getAllDependencies().
	static void walkNode(QSet<RefTarget*>& nodes, const RefMaker* node);

private:

	friend class RefTarget;
	friend class SingleReferenceFieldBase;
	friend class VectorReferenceFieldBase;
	friend class PropertyFieldBase;

	OVITO_OBJECT
};

};

#endif // __OVITO_REFMAKER_H
