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

#ifndef __OVITO_HALF_EDGE_MESH_H
#define __OVITO_HALF_EDGE_MESH_H

#include <core/Core.h>
#include <base/utilities/MemoryPool.h>

namespace Ovito {

class TriMesh;		// defined in TriMesh.h

/**
 * Stores a polygonal mesh using a half-edge data structure.
 */
class OVITO_CORE_EXPORT HalfEdgeMesh
{
public:

	class Vertex;
	class Face;

	/// A single half-edge.
	class Edge
	{
	public:

		/// Returns the vertex this half-edge is coming from.
		Vertex* vertex1() const { return _prevFaceEdge->_vertex2; }

		/// Returns the vertex this half-edge is pointing to.
		Vertex* vertex2() const { return _vertex2; }

		/// Returns a pointer to the face that is adjacent to this half-edge.
		Face* face() const { return _face; }

		/// Returns the next half-edge in the linked-list of half-edges that
		/// leave the same vertex as this edge.
		Edge* nextVertexEdge() const { return _nextVertexEdge; }

		/// Returns the next half-edge in the linked-list of half-edges adjacent to the
		/// same face as this edge.
		Edge* nextFaceEdge() const { return _nextFaceEdge; }

		/// Returns the previous half-edge in the linked-list of half-edges adjacent to the
		/// same face as this edge.
		Edge* prevFaceEdge() const { return _prevFaceEdge; }

		/// Returns a pointer to this edge's opposite half-edge.
		Edge* oppositeEdge() const { return _oppositeEdge; }

		/// Links two opposite half-edges.
		void linkToOppositeEdge(Edge* oppositeEdge) {
			OVITO_ASSERT(_oppositeEdge == nullptr);
			OVITO_ASSERT(oppositeEdge->_oppositeEdge == nullptr);
			OVITO_ASSERT(vertex1() == oppositeEdge->vertex2());
			OVITO_ASSERT(vertex2() == oppositeEdge->vertex1());
			_oppositeEdge = oppositeEdge;
			oppositeEdge->_oppositeEdge = this;
		}

	protected:

		/// Constructor.
		Edge(Vertex* vertex2, Face* face) : _oppositeEdge(nullptr), _vertex2(vertex2), _face(face) {}

		/// The opposite half-edge.
		Edge* _oppositeEdge;

		/// The vertex this half-edge is pointing to.
		Vertex* _vertex2;

		/// The face adjacent to this half-edge.
		Face* _face;

		/// The next half-edge in the linked-list of half-edges of the source vertex.
		Edge* _nextVertexEdge;

		/// The next half-edge in the linked-list of half-edges adjacent to the face.
		Edge* _nextFaceEdge;

		/// The previous half-edge in the linked-list of half-edges adjacent to the face.
		Edge* _prevFaceEdge;

		friend HalfEdgeMesh;
	};

	/// A vertex of a mesh.
	class Vertex
	{
	public:

		/// Returns the head of the vertex' linked-list of outgoing half-edges.
		Edge* edges() const { return _edges; }

		/// Returns the coordinates of the vertex.
		const Point3& pos() const { return _pos; }

		/// Returns the coordinates of the vertex.
		Point3& pos() { return _pos; }

		/// Sets the coordinates of the vertex.
		void setPos(const Point3& p) { _pos = p; }

		/// Returns the index of the vertex in the list of vertices of the mesh.
		int index() const { return _index; }

		/// Returns the number of faces (as well as half-edges) adjacent to this vertex.
		int numEdges() const { return _numEdges; }

	protected:

		/// Constructor.
		Vertex(const Point3& pos, int index = -1) : _pos(pos), _edges(nullptr), _numEdges(0),  _index(index) {}

		/// Adds an adjacent half-edge to this vertex.
		void addEdge(Edge* edge) {
			edge->_nextVertexEdge = _edges;
			_edges = edge;
			_numEdges++;
		}

		/// The coordinates of the vertex.
		Point3 _pos;

		/// The number of faces (as well as half-edges) adjacent to this vertex.
		int _numEdges;

		/// The head of the linked-list of outgoing half-edges.
		Edge* _edges;

		/// The index of the vertex in the list of vertices of the mesh.
		int _index;

		friend HalfEdgeMesh;
	};

	/// A polygonal face of the mesh.
	class Face
	{
	public:

		/// Returns a pointer to the head of the linked-list of half-edges that are adjacent to this face.
		Edge* edges() const { return _edges; }

		/// Returns the index of the face in the list of face of the mesh.
		int index() const { return _index; }

		/// Returns the bit flags assigned to this face.
		unsigned int flags() const { return _flags; }

		/// Tests if a flag is set for this face.
		bool testFlag(unsigned int flag) const { return (_flags & flag); }

		/// Sets a bit flag for this face.
		void setFlag(unsigned int flag) { _flags |= flag; }

		/// Clears a bit flag of this face.
		void clearFlag(unsigned int flag) { _flags &= ~flag; }

		/// Computes the number of edges (as well as vertices) of this face.
		int edgeCount() const {
			int c = 0;
			OVITO_CHECK_POINTER(edges());
			Edge* e = edges();
			do {
				c++;
				e = e->nextFaceEdge();
			}
			while(e != edges());
			return c;
		}

	protected:

		/// Constructor.
		Face(int index = -1) : _edges(nullptr), _index(index), _flags(0) {}

		/// The head of the linked-list of half-edges that are adjacent to this face.
		Edge* _edges;

		/// The index of the face in the list of faces of the mesh.
		int _index;

		/// The bit-wise flags assigned to this face.
		unsigned int _flags;

		friend HalfEdgeMesh;
	};


public:

	/// Default constructor.
	HalfEdgeMesh() {}

	/// Copy constructor.
	HalfEdgeMesh(const HalfEdgeMesh& other) {
		*this = other;
	}

	/// Removes all faces, edges, and vertices from this mesh.
	void clear();

	/// Returns the list of vertices in the mesh.
	const std::vector<Vertex*>& vertices() const { return _vertices; }

	/// Returns the list of faces in the mesh.
	const std::vector<Face*>& faces() const { return _faces; }

	/// Returns the number of vertices in this mesh.
	int vertexCount() const { return _vertices.size(); }

	/// Returns the number of faces in this mesh.
	int faceCount() const { return _faces.size(); }

	/// Returns a pointer to the vertex with the given index.
	Vertex* vertex(int index) const {
		OVITO_ASSERT(index >= 0 && index < _vertices.size());
		return _vertices[index];
	}

	/// Returns a pointer to the face with the given index.
	Face* face(int index) const {
		OVITO_ASSERT(index >= 0 && index < _faces.size());
		return _faces[index];
	}

	/// Reserves memory for the given total number of vertices.
	void reserveVertices(int vertexCount) { _vertices.reserve(vertexCount); }

	/// Reserves memory for the given total number of faces.
	void reserveFaces(int faceCount) { _faces.reserve(faceCount); }

	/// Adds a new vertex to the mesh.
	Vertex* createVertex(const Point3& pos);

	/// Creates a new face defined by the given vertices.
	/// Half-edges connecting the vertices are created by this method too.
	Face* createFace(std::initializer_list<Vertex*> vertices) {
		return createFace(vertices.begin(), vertices.end());
	}

	/// Creates a new face defined by the given range of vertices.
	/// Half-edges connecting the vertices are created by this method too.
	template<typename VertexPointerIterator>
	Face* createFace(VertexPointerIterator* begin, VertexPointerIterator* end) {
		OVITO_ASSERT(std::distance(begin, end) >= 2);
		Face* face = _facePool.construct(faceCount());
		_faces.push_back(face);

		Vertex* v2 = *begin;
		auto v1 = end;
		Edge* edge;
		for(;;) {
			if(v1 == begin) break;
			--v1;
			edge = _edgePool.construct(v2, face);
			(*v1)->addEdge(edge);
			edge->_nextFaceEdge = face->_edges;
			if(face->_edges)
				face->_edges->_prevFaceEdge = edge;
			face->_edges = edge;
			v2 = *v1;
		}
		// Link last edge to first edge of face and vice versa.
		edge->_prevFaceEdge = (*(end - 1))->_edges;
		OVITO_ASSERT(edge->_prevFaceEdge->_nextFaceEdge == nullptr);
		edge->_prevFaceEdge->_nextFaceEdge = edge;
		return face;
	}

	/// Tries to wire each half-edge of the mesh with its opposite (reverse) half-edge.
	void connectOppositeHalfedges();

	/// Copy operator.
	HalfEdgeMesh& operator=(const HalfEdgeMesh& other);

	/// Swaps the contents of this mesh with another mesh.
	void swap(HalfEdgeMesh& other);

	/// Converts this half-edge mesh to a triangle mesh.
	void convertToTriMesh(TriMesh& output) const;

	/// Clears the given flag for all faces.
	void clearFaceFlag(unsigned int flag) const {
		for(Face* face : faces())
			face->clearFlag(flag);
	}

private:

	/// This derived class is needed to make the protected Vertex constructor accessible.
	class InternalVertex : public Vertex {
	public:
		InternalVertex(const Point3& pos, int index = -1) : Vertex(pos, index) {}
	};

	/// This derived class is needed to make the protected Edge constructor accessible.
	class InternalEdge : public Edge {
	public:
		InternalEdge(Vertex* vertex2, Face* face) : Edge(vertex2, face) {}
	};

	/// This derived class is needed to make the protected Face constructor accessible.
	class InternalFace : public Face {
	public:
		InternalFace(int index = -1) : Face(index) {}
	};

private:

	/// The vertices of the mesh.
	std::vector<Vertex*> _vertices;
	MemoryPool<InternalVertex> _vertexPool;

	/// The edges of the mesh.
	MemoryPool<InternalEdge> _edgePool;

	/// The faces of the mesh.
	std::vector<Face*> _faces;
	MemoryPool<InternalFace> _facePool;
};

}; // End of namespace

#endif // __OVITO_HALF_EDGE_MESH_H
