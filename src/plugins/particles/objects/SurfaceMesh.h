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
#include <core/scene/objects/DataObjectWithSharedStorage.h>
#include <core/utilities/mesh/HalfEdgeMesh.h>
#include <plugins/particles/data/SimulationCell.h>

namespace Ovito { namespace Particles {

/**
 * \brief A closed triangle mesh representing a surface.
 */
class OVITO_PARTICLES_EXPORT SurfaceMesh : public DataObjectWithSharedStorage<HalfEdgeMesh>
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

	/// Indicates whether the entire simulation cell is part of the solid region.
	bool isCompletelySolid() const { return _isCompletelySolid; }

	/// Sets whether the entire simulation cell is part of the solid region.
	void setCompletelySolid(bool flag) { _isCompletelySolid = flag; }

	/// Fairs the triangle mesh stored in this object.
	void smoothMesh(const SimulationCell& cell, int numIterations, FloatType k_PB = 0.1f, FloatType lambda = 0.5f) {
		smoothMesh(*modifiableStorage(), cell, numIterations, k_PB, lambda);
		changed();
	}

	/// Fairs a triangle mesh.
	static void smoothMesh(HalfEdgeMesh& mesh, const SimulationCell& cell, int numIterations, FloatType k_PB = 0.1f, FloatType lambda = 0.5f);

protected:

	/// Performs one iteration of the smoothing algorithm.
	static void smoothMeshIteration(HalfEdgeMesh& mesh, FloatType prefactor, const SimulationCell& cell);

private:

	/// Indicates that the entire simulation cell is part of the solid region.
	PropertyField<bool> _isCompletelySolid;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_isCompletelySolid);
};

}	// End of namespace
}	// End of namespace

#endif // __OVITO_SURFACE_MESH_H
