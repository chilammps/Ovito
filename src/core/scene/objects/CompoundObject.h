///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_COMPOUND_OBJECT_H
#define __OVITO_COMPOUND_OBJECT_H

#include <core/Core.h>
#include <core/scene/objects/DataObject.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

/**
 * \brief A DataObject that stores a collection of other \ref DataObject "DataObjects".
 */
class OVITO_CORE_EXPORT CompoundObject : public DataObject
{
public:

	/// Constructs an empty compound data object.
	Q_INVOKABLE CompoundObject(DataSet* dataset);

	/// Asks the object for the result of the modification pipeline at the given time.
	virtual PipelineFlowState evaluate(TimePoint time) override;

	/// \brief Returns the list of imported data objects.
	const QVector<DataObject*>& dataObjects() const { return _dataObjects; }

	/// \brief Inserts a new object into the list of data objects held by this container object.
	void addDataObject(DataObject* obj) {
		if(!_dataObjects.contains(obj)) {
			obj->setSaveWithScene(saveWithScene());
			_dataObjects.push_back(obj);
		}
	}

	/// \brief Looks for an object of the given type in the list of data objects and returns it.
	template<class T>
	T* findDataObject() const {
		for(DataObject* obj : dataObjects()) {
			T* castObj = dynamic_object_cast<T>(obj);
			if(castObj) return castObj;
		}
		return nullptr;
	}

	/// \brief Removes all data objects owned by this CompoundObject that are not
	///        listed in the given set of active objects.
	void removeInactiveObjects(const QSet<DataObject*>& activeObjects) {
		for(int index = _dataObjects.size() - 1; index >= 0; index--)
			if(!activeObjects.contains(_dataObjects[index]))
				_dataObjects.remove(index);
	}

	/// \brief Controls whether the imported data is saved along with the scene.
	/// \param on \c true if data should be stored in the scene file; \c false if the data resides only in the external file.
	/// \undoable
	virtual void setSaveWithScene(bool on) override {
		DataObject::setSaveWithScene(on);
		// Propagate flag to sub-objects.
		for(DataObject* obj : dataObjects())
			obj->setSaveWithScene(on);
	}

	/// Returns the attributes set or loaded by the file importer which are fed into the modification pipeline
	/// along with the data objects.
	const QVariantMap& attributes() const { return _attributes; }

	/// Sets the attributes that will be fed into the modification pipeline
	/// along with the data objects.
	void setAttributes(const QVariantMap& attributes) { _attributes = attributes; }

	/// Resets the attributes that will be fed into the modification pipeline
	/// along with the data objects.
	void clearAttributes() { _attributes.clear(); }

	/// Returns the number of sub-objects that should be displayed in the modifier stack.
	virtual int editableSubObjectCount() override;

	/// Returns a sub-object that should be listed in the modifier stack.
	virtual RefTarget* editableSubObject(int index) override;

protected:

	/// Is called when a RefTarget has been added to a VectorReferenceField of this RefMaker.
	virtual void referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex) override;

	/// Is called when a RefTarget has been added to a VectorReferenceField of this RefMaker.
	virtual void referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex) override;

private:

	/// Stores the data objects of the compound.
	VectorReferenceField<DataObject> _dataObjects;

	/// Attributes set or loaded by the file importer which will be fed into the modification pipeline
	/// along with the data objects.
	QVariantMap _attributes;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_dataObjects);
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_COMPOUND_OBJECT_H
