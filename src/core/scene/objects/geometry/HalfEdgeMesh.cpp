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
#include "HalfEdgeMesh.h"
#include "TriMesh.h"

namespace Ovito {

/******************************************************************************
* Removes all faces, edges, and vertices from this mesh.
******************************************************************************/
void HalfEdgeMesh::clear()
{
	_vertices.clear();
	_faces.clear();
	_vertexPool.clear();
	_edgePool.clear();
	_facePool.clear();
}

/******************************************************************************
* Adds a new vertex to the mesh.
******************************************************************************/
HalfEdgeMesh::Vertex* HalfEdgeMesh::createVertex(const Point3& pos)
{
	Vertex* vert = _vertexPool.construct(pos, vertexCount());
	_vertices.push_back(vert);
	return vert;
}

/******************************************************************************
* Tries to wire each half-edge of the mesh with its opposite (reverse) half-edge.
******************************************************************************/
void HalfEdgeMesh::connectOppositeHalfedges()
{
	for(Vertex* v1 : vertices()) {
		for(Edge* edge = v1->edges(); edge != nullptr; edge = edge->nextVertexEdge()) {
			if(edge->oppositeEdge() != nullptr) {
				OVITO_ASSERT(edge->oppositeEdge()->oppositeEdge() == edge);
				continue;		// Edge is already linked to its opposite edge.
			}

			// Search in the edge list of the second vertex for an half-edge that goes back to the first vertex.
			for(Edge* oppositeEdge = edge->vertex2()->edges(); oppositeEdge != nullptr; oppositeEdge = oppositeEdge->nextVertexEdge()) {
				if(oppositeEdge->oppositeEdge() != nullptr) continue;
				if(oppositeEdge->vertex2() == v1) {

					// Link the two half-edges.
					edge->linkToOppositeEdge(oppositeEdge);
					break;
				}
			}
		}
	}
}

/******************************************************************************
* Copy operator.
******************************************************************************/
HalfEdgeMesh& HalfEdgeMesh::operator=(const HalfEdgeMesh& other)
{
	clear();

	// Copy vertices.
	reserveVertices(other.vertexCount());
	for(Vertex* v : other.vertices()) {
		int index = createVertex(v->pos())->index();
		OVITO_ASSERT(index == v->index());
	}

	// Copy faces and half-edges.
	reserveFaces(other.faceCount());
	for(Face* face_o : other.faces()) {
		Face* face_c = _facePool.construct(faceCount());
		OVITO_ASSERT(face_c->index() == face_o->index());
		_faces.push_back(face_c);

		if(!face_o->edges()) continue;
		Edge* edge_o = face_o->edges()->prevFaceEdge();
		Edge* lastEdge = nullptr;
		do {
			Vertex* v1 = vertex(edge_o->vertex1()->index());
			Vertex* v2 = vertex(edge_o->vertex2()->index());
			Edge* edge_c = _edgePool.construct(v2, face_c);
			v1->addEdge(edge_c);
			if(!lastEdge) lastEdge = edge_c;
			edge_c->_nextFaceEdge = face_c->_edges;
			if(face_c->_edges)
				face_c->_edges->_prevFaceEdge = edge_c;
			face_c->_edges = edge_c;
			edge_o = edge_o->prevFaceEdge();
		}
		while(edge_o != face_o->edges()->prevFaceEdge());

		// Link last edge to first edge of face and vice versa.
		lastEdge->_nextFaceEdge = face_c->edges();
		face_c->edges()->_prevFaceEdge = lastEdge;
	}

	// Link opposite half-edges.
	auto face_o = other.faces().cbegin();
	auto face_c = faces().cbegin();
	for(; face_c != faces().cend(); ++face_c, ++face_o) {
		Edge* edge_o = (*face_o)->edges();
		Edge* edge_c = (*face_c)->edges();
		if(!edge_o) continue;
		do {
			if(edge_o->oppositeEdge() != nullptr && edge_c->oppositeEdge() == nullptr) {
				Face* oppositeFace = face(edge_o->oppositeEdge()->face()->index());
				HalfEdgeMesh::Edge* oppositeEdge = oppositeFace->edges();
				do {
					OVITO_CHECK_POINTER(oppositeEdge);
					if(oppositeEdge->vertex1() == edge_c->vertex2() && oppositeEdge->vertex2() == edge_c->vertex1()) {
						edge_c->linkToOppositeEdge(oppositeEdge);
						break;
					}
					oppositeEdge = oppositeEdge->nextFaceEdge();
				}
				while(oppositeEdge != oppositeFace->edges());
				OVITO_ASSERT(edge_c->oppositeEdge());
			}
			edge_o = edge_o->nextFaceEdge();
			edge_c = edge_c->nextFaceEdge();
		}
		while(edge_o != (*face_o)->edges());
	}

	return *this;
}

/******************************************************************************
* Swaps the contents of this mesh with another mesh.
******************************************************************************/
void HalfEdgeMesh::swap(HalfEdgeMesh& other)
{
	_vertices.swap(other._vertices);
	_faces.swap(other._faces);
	_vertexPool.swap(other._vertexPool);
	_edgePool.swap(other._edgePool);
	_facePool.swap(other._facePool);
}

/******************************************************************************
* Converts this half-edge mesh to a triangle mesh.
******************************************************************************/
void HalfEdgeMesh::convertToTriMesh(TriMesh& output) const
{
	output.clear();

	// Transfer vertices.
	output.setVertexCount(vertexCount());
	auto vout = output.vertices().begin();
	for(Vertex* v : vertices()) {
		OVITO_ASSERT(v->index() == (vout - output.vertices().begin()));
		*vout++ = v->pos();
	}

	// Count number of output triangles.
	int triangleCount = 0;
	for(Face* face : faces())
		triangleCount += std::max(face->edgeCount() - 2, 0);

#if 0
	// Validate mesh.
	for(Face* face : faces()) {
		Edge* edge = face->edges();
		do {
			OVITO_ASSERT(edge->vertex1() != edge->vertex2());
			OVITO_ASSERT(edge->oppositeEdge() && edge->oppositeEdge()->oppositeEdge() == edge);
			OVITO_ASSERT(edge->oppositeEdge()->vertex1() == edge->vertex2());
			OVITO_ASSERT(edge->oppositeEdge()->vertex2() == edge->vertex1());
			OVITO_ASSERT(edge->nextFaceEdge()->vertex1() == edge->vertex2());
			OVITO_ASSERT(edge->prevFaceEdge()->vertex2() == edge->vertex1());
			OVITO_ASSERT(edge->nextFaceEdge()->prevFaceEdge() == edge);
			OVITO_ASSERT(edge->nextFaceEdge() != edge->oppositeEdge());
			OVITO_ASSERT(edge->prevFaceEdge() != edge->oppositeEdge());
			edge = edge->nextFaceEdge();
		}
		while(edge != face->edges());
	}
#endif

	// Transfer faces.
	output.setFaceCount(triangleCount);
	auto fout = output.faces().begin();
	for(Face* face : faces()) {
		int baseVertex = face->edges()->vertex2()->index();
		Edge* edge = face->edges()->nextFaceEdge()->nextFaceEdge();
		while(edge != face->edges()) {
			fout->setVertices(baseVertex, edge->vertex1()->index(), edge->vertex2()->index());
			++fout;
			edge = edge->nextFaceEdge();
		}
	}
	OVITO_ASSERT(fout == output.faces().end());

	output.invalidateVertices();
	output.invalidateFaces();
}

}; // End of namespace
