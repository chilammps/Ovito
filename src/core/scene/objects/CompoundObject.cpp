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

#include <core/Core.h>
#include "CompoundObject.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, CompoundObject, DataObject);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(CompoundObject, _dataObjects, "SceneObjects", DataObject, PROPERTY_FIELD_ALWAYS_DEEP_COPY);
SET_PROPERTY_FIELD_LABEL(CompoundObject, _dataObjects, "Objects");

/******************************************************************************
* Constructs an empty compound data object.
******************************************************************************/
CompoundObject::CompoundObject(DataSet* dataset) : DataObject(dataset)
{
	INIT_PROPERTY_FIELD(CompoundObject::_dataObjects);
}

/******************************************************************************
* Asks the object for the result of the geometry pipeline at the given time.
******************************************************************************/
PipelineFlowState CompoundObject::evaluate(TimePoint time)
{
	return PipelineFlowState(PipelineStatus::Success, dataObjects(), TimeInterval::infinite(), attributes());
}

/******************************************************************************
* Returns the number of sub-objects that should be displayed in the modifier stack.
******************************************************************************/
int CompoundObject::editableSubObjectCount()
{
	return dataObjects().size();
}

/******************************************************************************
* Returns a sub-object that should be listed in the modifier stack.
******************************************************************************/
RefTarget* CompoundObject::editableSubObject(int index)
{
	return dataObjects()[index];
}

/******************************************************************************
* Is called when a RefTarget has been added to a VectorReferenceField of this RefMaker.
******************************************************************************/
void CompoundObject::referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex)
{
	if(field == PROPERTY_FIELD(CompoundObject::_dataObjects))
		notifyDependents(ReferenceEvent::SubobjectListChanged);

	DataObject::referenceInserted(field, newTarget, listIndex);
}

/******************************************************************************
* Is called when a RefTarget has been added to a VectorReferenceField of this RefMaker.
******************************************************************************/
void CompoundObject::referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex)
{
	if(field == PROPERTY_FIELD(CompoundObject::_dataObjects))
		notifyDependents(ReferenceEvent::SubobjectListChanged);

	DataObject::referenceRemoved(field, newTarget, listIndex);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
