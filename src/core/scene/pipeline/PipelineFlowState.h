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

#ifndef __OVITO_PIPELINE_FLOW_STATE_H
#define __OVITO_PIPELINE_FLOW_STATE_H

#include <core/Core.h>
#include <core/animation/TimeInterval.h>
#include <core/reference/RefMaker.h>
#include <core/scene/objects/VersionedObjectReference.h>
#include "PipelineStatus.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

/**
 * \brief This object flows down the geometry pipeline of an ObjectNode.
 */
class OVITO_CORE_EXPORT PipelineFlowState
{
public:

	/// \brief Default constructor that creates an empty state object.
	PipelineFlowState() : _stateValidity(TimeInterval::empty()) {}

	/// \brief Constructor that creates a state object and initializes it with a DataObject.
	/// \param dataObject The object represents the current state of a geometry pipeline evaluation.
	/// \param validityInterval Specifies the time interval during which the returned object is valid.
	///                         For times outside this interval the geometry pipeline has to be re-evaluated.
	PipelineFlowState(DataObject* dataObject, const TimeInterval& validityInterval) : _stateValidity(validityInterval) {
		addObject(dataObject);
	}

	/// \brief Constructor that creates a state object and initializes it with a list of data objects.
	/// \param status A status object that describes the outcome of the pipeline evaluation.
	/// \param dataObjects The objects that represents the current state of a geometry pipeline evaluation.
	/// \param validityInterval Specifies the time interval during which the returned objects are valid.
	PipelineFlowState(const PipelineStatus& status, const QVector<DataObject*>& dataObjects, const TimeInterval& validityInterval, const QVariantMap& attributes = QVariantMap()) :
		_status(status), _stateValidity(validityInterval), _attributes(attributes)
	{
		_objects.reserve(dataObjects.size());
		for(const auto& obj : dataObjects)
			addObject(obj);
	}

	/// \brief Discards the contents of this state object.
	void clear() {
		_objects.clear();
		_stateValidity.setEmpty();
		_status = PipelineStatus();
		_attributes.clear();
	}

	/// \brief Returns true if the given object is part of this pipeline flow state.
	/// \note The method ignores the revision number of the object.
	bool contains(DataObject* obj) const;

	/// \brief Adds an additional data object to this state.
	void addObject(DataObject* obj);

	/// \brief Replaces a data object with a new one.
	void replaceObject(DataObject* oldObj, DataObject* newObj);

	/// \brief Removes a data object from this state.
	void removeObject(DataObject* dataObj) {
		replaceObject(dataObj, nullptr);
	}

	/// \brief Returns the list of data objects stored in this flow state.
	const QVector<VersionedOORef<DataObject>>& objects() const { return _objects; }

	/// \brief Finds an object of the given type in the list of data objects stored in this flow state.
	template<class ObjectType>
	ObjectType* findObject() const {
		for(DataObject* o : _objects) {
			if(ObjectType* obj = dynamic_object_cast<ObjectType>(o))
				return obj;
		}
		return nullptr;
	}

	/// \brief Tries to convert one of the to data objects stored in this flow state to the given object type.
	OORef<DataObject> convertObject(const OvitoObjectType& objectClass, TimePoint time) const;

	/// \brief Tries to convert one of the to data objects stored in this flow state to the given object type.
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

	/// \brief Updates the stored revision numbers for all data objects.
	void updateRevisionNumbers();

	/// Returns the status of the pipeline evaluation.
	const PipelineStatus& status() const { return _status; }

	/// Sets the stored status.
	void setStatus(const PipelineStatus& status) { _status = status; }

	/// Returns the auxiliary attributes associated with the state.
	const QVariantMap& attributes() const { return _attributes; }

	/// Returns a modifiable reference to the auxiliary attributes associated with this state.
	QVariantMap& attributes() { return _attributes; }

private:

	/// The data that has been output by the modification pipeline.
	/// This is a list of data objects and associated revision numbers
	/// to easily detect changes.
	QVector<VersionedOORef<DataObject>> _objects;

	/// Contains the validity interval for this pipeline flow state.
	TimeInterval _stateValidity;

	/// The status of the pipeline evaluation.
	PipelineStatus _status;

	/// Extra attributes associated with the pipeline flow state.
	QVariantMap _attributes;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#include <core/scene/objects/DataObject.h>

#endif // __OVITO_PIPELINE_FLOW_STATE_H
