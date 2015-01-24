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

#ifndef __OVITO_TRIMESH_OBJECT_H
#define __OVITO_TRIMESH_OBJECT_H

#include <core/Core.h>
#include <core/scene/objects/DataObject.h>
#include <core/utilities/mesh/TriMesh.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene) OVITO_BEGIN_INLINE_NAMESPACE(StdObj)

/**
 * \brief A data object type that consist of a triangle mesh.
 */
class OVITO_CORE_EXPORT TriMeshObject : public DataObject
{
public:

	/// Constructor that creates an object with an empty triangle mesh.
	Q_INVOKABLE TriMeshObject(DataSet* dataset);

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Triangle mesh"); }

	/// \brief Returns whether this object, when returned as an editable sub-object by another object,
	///        should be displayed in the modification stack.
	///
	/// Return false because this object cannot be edited.
	virtual bool isSubObjectEditable() const override { return false; }

	/// Returns a const-reference to the triangle mesh encapsulated by this data object.
	const TriMesh& mesh() const { return _mesh; }

	/// Returns a reference to the triangle mesh encapsulated by this data object.
	/// The reference can be used to modify the mesh. However, each time the mesh has been modified,
	/// this->notifyDependents(ReferenceEvent::TargetChanged) must be called to increment
	/// the data object's revision number.
	TriMesh& mesh() { return _mesh; }

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

private:

	/// The triangle mesh encapsulated by this data object.
	TriMesh _mesh;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_TRIMESH_OBJECT_H
