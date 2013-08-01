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
 * \file SceneObject.h
 * \brief Contains the definition of the Ovito::SceneObject class.
 */

#ifndef __OVITO_SCENE_OBJECT_H
#define __OVITO_SCENE_OBJECT_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/animation/TimeInterval.h>
#include <core/scene/pipeline/PipelineFlowState.h>
#include <core/scene/display/DisplayObject.h>
#include <core/utilities/concurrent/Future.h>

namespace Ovito {

class DataSet;				// defined in DataSet.h

/**
 * \brief Abstract base class for all objects in the scene.

 * A single SceneObject can be referenced by multiple ObjectNode instances.
 */
class OVITO_CORE_EXPORT SceneObject : public RefTarget
{
protected:

	/// \brief Default constructor.
	SceneObject();

public:

	/// \brief Asks the object for its validity interval at the given time.
	/// \param time The animation time at which the validity interval should be computed.
	/// \return The maximum time interval that contains \a time and during which the object is valid.
	///
	/// When computing the validity interval of the object, an implementation of this method
	/// should take validity intervals of all sub-objects and sub-controller into account.
	///
	/// The default implementation return TimeInterval::forever().
	virtual TimeInterval objectValidity(TimePoint time) { return TimeInterval::forever(); }

	/// \brief This asks the object whether it supports the conversion to another object type.
	/// \param objectClass The destination type. This must be a SceneObject derived class.
	/// \return \c true if this object can be converted to the requested type given by \a objectClass or any sub-class thereof.
	///         \c false if the conversion is not possible.
	///
	/// The default implementation returns \c true if the class \a objectClass is the source object type or any derived type.
	/// This is the trivial case: It requires no real conversion at all.
	///
	/// Sub-classes should override this method to allow the conversion to a MeshObject, for example.
	/// When overriding, the base implementation of this method should always be called.
	virtual bool canConvertTo(const OvitoObjectType& objectClass) {
		// Can always convert to itself.
		return this->getOOType().isDerivedFrom(objectClass);
	}

	/// \brief Lets the object convert itself to another object type.
	/// \param objectClass The destination type. This must be a SceneObject derived class.
	/// \param time The time at which to convert the object.
	/// \return The newly created object or \c NULL if no conversion is possible.
	///
	/// Whether the object can be converted to the desired destination type can be checked in advance using
	/// the canConvertTo() method.
	///
	/// Sub-classes should override this method to allow the conversion to a MeshObject for example.
	/// When overriding, the base implementation of this method should always be called.
	virtual OORef<SceneObject> convertTo(const OvitoObjectType& objectClass, TimePoint time) {
		// Trivial conversion.
		if(this->getOOType().isDerivedFrom(objectClass))
			return this;
		else
			return nullptr;
	}

	/// \brief Lets the object convert itself to another object type.
	/// \param time The time at which to convert the object.
	///
	/// This is a wrapper of the function above using C++ templates.
	/// It just casts the conversion result to the given class.
	template<class T>
	OORef<T> convertTo(TimePoint time) {
		return static_object_cast<T>(convertTo(T::OOType, time));
	}

	/// \brief Asks the object for the result of the geometry pipeline at the given time.
	/// \param time The animation time at which the geometry pipeline is being evaluated.
	/// \return The pipeline flow state generated by this object.
	///
	/// The default implementation just returns the scene object itself as the evaluation result.
	virtual PipelineFlowState evaluate(TimePoint time) {
		return PipelineFlowState(this, objectValidity(time));
	}

	/// \brief Returns a structure that describes the current status of the object.
	///
	/// The default implementation of this method returns an empty status object
	/// that indicates success (ObjectStatus::StatusType::Success).
	///
	/// An object should generate a ReferenceEvent::StatusChanged event when its status has changed.
	virtual ObjectStatus status() const { return ObjectStatus(); }

	/// \brief Returns the attached display object that is responsible for rendering this
	///        scene object.
	DisplayObject* displayObject() const { return _displayObject.get(); }

	/// \brief Attaches a display object to this scene object that will be responsible for rendering the
	///        scene object.
	void setDisplayObject(DisplayObject* displayObj) { _displayObject = displayObj; }

	/// \brief Returns whether the internal data is saved along with the scene.
	/// \return \c true if the data is stored in the scene file; \c false if the data can be restored from an external file or recomputed.
	bool saveWithScene() const { return _saveWithScene; }

	/// \brief Sets whether the per-particle data is saved along with the scene.
	/// \param on \c true if the data should be stored in the scene file; \c false if the per-particle data can be restored from an external file.
	/// \undoable
	void setSaveWithScene(bool on) { _saveWithScene = on; }

	/// \brief Returns the number of input objects that are referenced by this scene object.
	/// \return The number of input objects that this object relies on.
	///
	/// The default implementation of this method returns 0.
	virtual int inputObjectCount() { return 0; }

	/// \brief Returns an input object of this scene object.
	/// \param index The index of the input object. This must be between 0 and inputObjectCount()-1.
	/// \return The requested input object. Can be \c NULL.
	virtual SceneObject* inputObject(int index) {
		OVITO_ASSERT_MSG(false, "SceneObject::inputObject", "This type of scene object has no input objects.");
		return nullptr;
	}

	/// \brief Returns the current value of the revision counter of this scene object.
	/// This counter is increment every time the object changes.
	unsigned int revisionNumber() const { return _revisionNumber; }

	/// \brief Sends an event to all dependents of this RefTarget.
	/// \param event The notification event to be sent to all dependents of this RefTarget.
	virtual void notifyDependents(ReferenceEvent& event) override;

	/// \brief Sends an event to all dependents of this RefTarget.
	/// \param eventType The event type passed to the ReferenceEvent constructor.
	inline void notifyDependents(ReferenceEvent::Type eventType) {
		RefTarget::notifyDependents(eventType);
	}

protected:

	/// Handles reference events sent by reference targets of this object.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

private:

	/// The revision counter of this scene object.
	/// The counter is increment every time the object changes.
	unsigned int _revisionNumber;

	/// The attached display object that is responsible for rendering this scene object.
	OORef<DisplayObject> _displayObject;

	/// Controls whether the internal data is saved along with the scene.
	/// If false, only metadata will be saved in a scene file while the contents get restored
	/// from an external data source or get recomputed.
	PropertyField<bool> _saveWithScene;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_saveWithScene);
};

};

#endif // __OVITO_SCENE_OBJECT_H
