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
 * \file PipelineFlowState.h 
 * \brief Contains the definition of the Ovito::PipelineFlowState class.
 */
 
#ifndef __OVITO_PIPELINE_FLOW_STATE_H
#define __OVITO_PIPELINE_FLOW_STATE_H

#include <core/Core.h>
#include <core/animation/TimeInterval.h>
#include <core/reference/RefMaker.h>
#include <core/utilities/ObjectStatus.h>

namespace Ovito {

class SceneObject;		// defined in SceneObject.h

/**
 * \brief This object flows down the geometry pipeline of an ObjectNode.
 */
class OVITO_CORE_EXPORT PipelineFlowState
{
public:

	/// \brief Default constructor that creates an empty state object.
	PipelineFlowState() : _stateValidity(TimeInterval::empty()) {}

	/// \brief Constructor that creates a state object and initializes it with a SceneObject.
	/// \param sceneObject The object represents the current state of a geometry pipeline evaluation.
	/// \param validityInterval Specifies the time interval during which the returned object is valid.
	///                         For times outside this interval the geometry pipeline has to be re-evaluated.
	PipelineFlowState(SceneObject* sceneObject, const TimeInterval& validityInterval) : _stateValidity(validityInterval) {
		addObject(sceneObject);
	}

	/// \brief Constructor that creates a state object and initializes it with a list of SceneObjects.
	/// \param status A status object that describes the outcome of the pipeline evaluation.
	/// \param sceneObjects The objects that represents the current state of a geometry pipeline evaluation.
	/// \param validityInterval Specifies the time interval during which the returned objects are valid.
	PipelineFlowState(const ObjectStatus& status, const QVector<SceneObject*>& sceneObjects, const TimeInterval& validityInterval) :
		_status(status), _stateValidity(validityInterval)
	{
		for(const auto& obj : sceneObjects)
			addObject(obj);
	}

	/// \brief Discards the contents of this state object.
	void clear() {
		_objects.clear();
		_revisionNumbers.clear();
		_stateValidity.setEmpty();
		_status = ObjectStatus();
	}

	/// \brief Adds an additional scene object to this state.
	void addObject(SceneObject* obj);

	/// \brief Replaces a scene object with a new one.
	void replaceObject(SceneObject* oldObj, const OORef<SceneObject>& newObj);

	/// \brief Removes a scene object from this state.
	void removeObject(SceneObject* sceneObj) {
		replaceObject(sceneObj, nullptr);
	}

	/// \brief Returns the list of scene objects stored in this flow state.
	const QVector<OORef<SceneObject>>& objects() const { return _objects; }

	/// \brief Returns the number of objects stored in this container.
	int count() const { return _objects.size(); }

	/// \brief Finds an object of the given type in the list of scene objects stored in this flow state.
	template<class ObjectType>
	ObjectType* findObject() const {
		for(const auto& o : _objects) {
			if(ObjectType* obj = dynamic_object_cast<ObjectType>(o.get()))
				return obj;
		}
		return nullptr;
	}

	/// \brief Tries to convert one of the to scene objects stored in this flow state to the given object type.
	OORef<SceneObject> convertObject(const OvitoObjectType& objectClass, TimePoint time) const;

	/// \brief Tries to convert one of the to scene objects stored in this flow state to the given object type.
	template<class ObjectType>
	OORef<ObjectType> convertObject(TimePoint time) const {
		return static_object_cast<ObjectType>(convertObject(ObjectType::OOType, time));
	}

	/// \brief Gets the validity interval for this pipeline state.
	/// \return The time interval during which the returned object is valid.
	///         For times outside this interval the geometry pipeline has to be re-evaluated.
	const TimeInterval& stateValidity() const { return _stateValidity; }
	
	/// \brief Specifies the validity interval for this pipeline state.
	/// \param newInterval The time interval during which the object set by setResult() is valid.
	/// \sa intersectStateValidity()
	void setStateValidity(const TimeInterval& newInterval) { _stateValidity = newInterval; }

	/// \brief Reduces the validity interval of this pipeline state to include only the given time interval.
	/// \param intersectionInterval The current validity interval is reduced to include only this time interval.
	/// \sa setStateValidity()
	void intersectStateValidity(const TimeInterval& intersectionInterval) { _stateValidity.intersect(intersectionInterval); }

	/// \brief Returns true if this state object has no valid contents.
	bool isEmpty() const { return _objects.empty(); }

	/// \brief Updates the stored revision number for a scene object.
	void updateRevisionNumber(SceneObject* obj);

	/// \brief Updates the stored revision numbers for all scene objects.
	void updateRevisionNumbers();

	/// \brief Returns the revision of the scene object at the given index.
	int revisionNumber(int index) const { return _revisionNumbers[index]; }

	/// Returns the status of the pipeline evaluation.
	const ObjectStatus& status() const { return _status; }

	/// Changes the stored status record.
	void setStatus(const ObjectStatus& status) { _status = status; }

private:

	/// Contains the objects that flow up the geometry pipeline
	/// and are modified by modifiers.
	QVector<OORef<SceneObject>> _objects;

	/// Each scene object is associated
	/// with a revision number, which is used to detect changes to the object.
	QVector<int> _revisionNumbers;

	/// Contains the validity interval for this pipeline flow state.
	TimeInterval _stateValidity;

	/// The status structure.
	ObjectStatus _status;
};

};

#include <core/scene/objects/SceneObject.h>

#endif // __OVITO_PIPELINE_FLOW_STATE_H
