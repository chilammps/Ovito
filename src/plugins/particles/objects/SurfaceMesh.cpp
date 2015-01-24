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

#include <plugins/particles/Particles.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include "SurfaceMesh.h"
#include "SurfaceMeshDisplay.h"

namespace Ovito { namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, SurfaceMesh, DataObject);

/******************************************************************************
* Constructs an empty surface mesh object.
******************************************************************************/
SurfaceMesh::SurfaceMesh(DataSet* dataset) : DataObject(dataset), _isCompletelySolid(false)
{
	addDisplayObject(new SurfaceMeshDisplay(dataset));
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> SurfaceMesh::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<SurfaceMesh> clone = static_object_cast<SurfaceMesh>(DataObject::clone(deepCopy, cloneHelper));

	// Copy the internal mesh.
	clone->_mesh = this->_mesh;
	clone->_isCompletelySolid = this->_isCompletelySolid;

	return clone;
}

/******************************************************************************
* Fairs a closed triangle mesh.
******************************************************************************/
void SurfaceMesh::smoothMesh(HalfEdgeMesh& mesh, const SimulationCell& cell, int numIterations, FloatType k_PB, FloatType lambda)
{
	// This is the implementation of the mesh smoothing algorithm:
	//
	// Gabriel Taubin
	// A Signal Processing Approach To Fair Surface Design
	// In SIGGRAPH 95 Conference Proceedings, pages 351-358 (1995)

	FloatType mu = 1.0f / (k_PB - 1.0f/lambda);

	for(int iteration = 0; iteration < numIterations; iteration++) {
		smoothMeshIteration(mesh, lambda, cell);
		smoothMeshIteration(mesh, mu, cell);
	}
}

/******************************************************************************
* Performs one iteration of the smoothing algorithm.
******************************************************************************/
void SurfaceMesh::smoothMeshIteration(HalfEdgeMesh& mesh, FloatType prefactor, const SimulationCell& cell)
{
	const AffineTransformation absoluteToReduced = cell.matrix().inverse();
	const AffineTransformation reducedToAbsolute = cell.matrix();

	// Compute displacement for each vertex.
	std::vector<Vector3> displacements(mesh.vertexCount());
	parallelFor(mesh.vertexCount(), [&mesh, &displacements, prefactor, cell, absoluteToReduced](int index) {
		HalfEdgeMesh::Vertex* vertex = mesh.vertex(index);
		Vector3 d = Vector3::Zero();
		for(HalfEdgeMesh::Edge* edge = vertex->edges(); edge != nullptr; edge = edge->nextVertexEdge()) {
			d += cell.wrapVector(edge->vertex2()->pos() - vertex->pos());
		}
		if(vertex->edges() != nullptr)
			d *= (prefactor / vertex->numEdges());
		displacements[index] = d;
	});

	// Apply computed displacements.
	auto d = displacements.cbegin();
	for(HalfEdgeMesh::Vertex* vertex : mesh.vertices())
		vertex->pos() += *d++;
}

}	// End of namespace
}	// End of namespace
