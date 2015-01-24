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

#ifndef __OVITO_SURFACE_MESH_H
#define __OVITO_SURFACE_MESH_H

#include <plugins/particles/Particles.h>
#include <core/scene/objects/DataObject.h>
#include <core/utilities/mesh/HalfEdgeMesh.h>
#include <plugins/particles/data/SimulationCell.h>

namespace Ovito { namespace Particles {

/**
 * \brief A closed triangle mesh representing a surface.
 */
class OVITO_PARTICLES_EXPORT SurfaceMesh : public DataObject
{
public:

	/// \brief Constructor that creates an empty SurfaceMesh object.
	Q_INVOKABLE SurfaceMesh(DataSet* dataset);

	/// Returns the title of this object.
	virtual QString objectTitle() override { return tr("Surface mesh"); }

	/// \brief Returns whether this object, when returned as an editable sub-object by another object,
	///        should be displayed in the modification stack.
	///
	/// Return false because this object cannot be edited.
	virtual bool isSubObjectEditable() const override { return false; }

	/// Returns a const-reference to the mesh encapsulated by this data object.
	const HalfEdgeMesh& mesh() const { return _mesh; }

	/// Returns a reference to the mesh encapsulated by this data object.
	/// The reference can be used to modify the mesh. However, each time the mesh has been modified,
	/// notifyDependents(ReferenceEvent::TargetChanged) must be called to increment
	/// the data object's revision number.
	HalfEdgeMesh& mesh() { return _mesh; }

	/// Indicates whether the entire simulation cell is part of the solid region.
	bool isCompletelySolid() const { return _isCompletelySolid; }

	/// Sets whether the entire simulation cell is part of the solid region.
	void setCompletelySolid(bool flag) { _isCompletelySolid = flag; }

	/// \brief Clears the triangle mesh by deleting all vertices and faces.
	void clearMesh() {
		mesh().clear();
		notifyDependents(ReferenceEvent::TargetChanged);
	}

	/// Fairs the triangle mesh stored in this object.
	void smoothMesh(const SimulationCell& cell, int numIterations, FloatType k_PB = 0.1f, FloatType lambda = 0.5f) {
		smoothMesh(mesh(), cell, numIterations, k_PB, lambda);
		notifyDependents(ReferenceEvent::TargetChanged);
	}

	/// Fairs a triangle mesh.
	static void smoothMesh(HalfEdgeMesh& mesh, const SimulationCell& cell, int numIterations, FloatType k_PB = 0.1f, FloatType lambda = 0.5f);

protected:

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

	/// Performs one iteration of the smoothing algorithm.
	static void smoothMeshIteration(HalfEdgeMesh& mesh, FloatType prefactor, const SimulationCell& cell);

private:

	/// The internal triangle mesh.
	HalfEdgeMesh _mesh;

	/// Indicates that the entire simulation cell is part of the solid region.
	bool _isCompletelySolid;

	Q_OBJECT
	OVITO_OBJECT
};

}	// End of namespace
}	// End of namespace

#endif // __OVITO_SURFACE_MESH_H
