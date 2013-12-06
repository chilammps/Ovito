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
#include <core/scene/objects/geometry/TriMeshObject.h>

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, TriMeshObject, SceneObject)

/******************************************************************************
* Default constructor.
******************************************************************************/
TriMeshObject::TriMeshObject(DataSet* dataset) : SceneObject(dataset)
{
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void TriMeshObject::saveToStream(ObjectSaveStream& stream)
{
	SceneObject::saveToStream(stream);
	stream.beginChunk(0x01);
	_mesh.saveToStream(stream);
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void TriMeshObject::loadFromStream(ObjectLoadStream& stream)
{
	SceneObject::loadFromStream(stream);
	stream.expectChunk(0x01);
	_mesh.loadFromStream(stream);
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> TriMeshObject::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<TriMeshObject> clone = static_object_cast<TriMeshObject>(SceneObject::clone(deepCopy, cloneHelper));

	// Shallow copy the internal mesh.
	clone->_mesh = this->_mesh;

	return clone;
}

};
