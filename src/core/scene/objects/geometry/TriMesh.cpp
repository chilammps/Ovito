///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2008) Alexander Stukowski
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

#include <mesh/Mesh.h>
#include "TriMesh.h"

namespace Mesh {

/******************************************************************************
* Default constructor. Creates an empty mesh.
******************************************************************************/
TriMesh::TriMesh() : cacheState(NONE_CACHED), renderVertices(NULL), _hasVertexColors(false)
{
}

/******************************************************************************
* Copy constructor.
******************************************************************************/
TriMesh::TriMesh(const TriMesh& mesh) : cacheState(NONE_CACHED), renderVertices(NULL), _hasVertexColors(false)
{
	*this = mesh;
}

/******************************************************************************
* Destructor.
******************************************************************************/
TriMesh::~TriMesh()
{
	delete[] renderVertices;
}

/******************************************************************************
* Clears all vertices and faces.
******************************************************************************/
void TriMesh::clearMesh()
{
	_vertices.clear();
	_faces.clear();
	renderEdges.clear();
	faceGroups.clear();
    delete[] renderVertices;
	renderVertices = NULL;
	cacheState = NONE_CACHED;
	_hasVertexColors = false;
}

/******************************************************************************
* Sets the number of vertices in this mesh.
******************************************************************************/
void TriMesh::setVertexCount(int n)
{
	_vertices.resize(n);
	if(_hasVertexColors)
		_vertexColors.resize(n);
	invalidateVertices();
}

/******************************************************************************
* Sets the number of faces in this mesh.
******************************************************************************/
void TriMesh::setFaceCount(int n)
{
	_faces.resize(n);
	invalidateFaces();
}

/******************************************************************************
* Adds a new triangle face and returns a reference to it.
* The new face is NOT initialized by this method.
******************************************************************************/
TriMeshFace& TriMesh::addFace()
{
	setFaceCount(faceCount()+1);
	return _faces.back();
}

/******************************************************************************
* Builds a list of all visible edges of the mesh.
* This list is used for fast wireframe rendering of the mesh.
******************************************************************************/
void TriMesh::buildRenderEdges()
{
	renderEdges.clear();

#define DIRECT_INDICES 4
	// Stores the indices of the adjacend edges for each vertex.
	struct VertexEdgeEntry {
		int count;	// The number of adjacend edges.
		union {
			int v[DIRECT_INDICES];	// This is used for vertices with few adjacend edges.
			struct {
				int allocated;
				int* v;			// This is used for vertices with many adjacend edges.
			} array;
		};
	};
	VertexEdgeEntry* elist = new VertexEdgeEntry[vertexCount()];
	memset(elist, 0, sizeof(VertexEdgeEntry) * vertexCount());

	for(int f = 0; f < faceCount(); f++) {
		for(int v = 0; v < 3; v++) {

			// Skip hidden edges.
			TriMeshFace& face = _faces[f];
            if(!face.edgeVisibility(v)) continue;

            int ev1 = face.vertex(v);
			int ev2 = face.vertex((v==2)?0:(v+1));
			if(ev2 < ev1) swap(ev1, ev2);

			// Check if the edge is in the list of adjacend edges of the current vertex.
			VertexEdgeEntry* entry = &elist[ev1];
			bool skip = false;
			for(int i=0; i<entry->count; i++) {
				if(entry->count <= DIRECT_INDICES) {
					OVITO_ASSERT_MSG(entry->v[i] < vertexCount(), "TriMesh::buildRenderEdges()", "A face vertex index is out of range.");
					if(entry->v[i] == ev2) { skip = true; break; }
				}
				else {
					OVITO_ASSERT_MSG(entry->array.v[i] < vertexCount(), "TriMesh::buildRenderEdges()", "A face vertex index is out of range.");
					if(entry->array.v[i] == ev2) { skip = true; break; }
				}
			}
			if(skip) continue;

			// Update the list of adjacend edges.
			if(entry->count < DIRECT_INDICES) {
				entry->v[entry->count++] = ev2;
			}
			else {
				if(entry->count == DIRECT_INDICES) {
					int allocated = DIRECT_INDICES*2;
					int* array = new int[allocated];
					memcpy(array, entry->v, DIRECT_INDICES * sizeof(int));
					entry->array.allocated = allocated;
					entry->array.v = array;
				}
				else if(entry->count == entry->array.allocated) {
					entry->array.allocated *= 2;
					int* array = new int[entry->array.allocated];
					memcpy(array, entry->array.v, entry->count*sizeof(int));
					delete[] entry->array.v;
					entry->array.v = array;
				}
				OVITO_ASSERT(entry->count < entry->array.allocated);
                entry->array.v[entry->count++] = ev2;
			}

			// Create new render egde.
			RenderEdge edge;
			edge.v[0] = ev1;
			edge.v[1] = ev2;
			renderEdges.push_back(edge);
		}
	}

	// Cleanup temporary arrays.
    for(int i=0; i<vertexCount(); i++)
		if(elist[i].count > DIRECT_INDICES)
			delete[] elist[i].array.v;
	delete[] elist;

	cacheState |= RENDER_EDGES_CACHED;
}

/******************************************************************************
* Computes the vertex and face normals for all vertices and faces in the mesh.
* This list is used for fast shaded rendering of the mesh.
******************************************************************************/
void TriMesh::buildRenderVertices()
{
	quint32 allMask = 0;

	// Allocate render vertex array.
	delete[] renderVertices;
	renderVertices = new RenderVertex[faceCount() * 3];

	// Reset face groups.
	faceGroups.clear();

	// Initialize array and compute face normals.
	QVector<TriMeshFace>::iterator face = faces().begin();
	const QVector<TriMeshFace>::iterator faceend = faces().end();
	RenderVertex* vert = renderVertices;
	for(; face != faceend; ++face) {
		// Compute face normal.
		Vector3 d1 = vertex(face->v[1]) - vertex(face->v[0]);
		Vector3 d2 = vertex(face->v[2]) - vertex(face->v[0]);
		Vector3 normal = CrossProduct(d1, d2);
		if(normal != NULL_VECTOR) {
			face->normal = Normalize(normal);
			allMask |= face->smoothingGroup();
		}
		else face->normal = Vector3(0,0,1);

		// Initialize render vertices for this face.
		for(size_t v = 0; v < 3; v++, vert++) {
			vert->normal = NULL_VECTOR;
			vert->pos = vertex(face->v[v]);
			if(hasVertexColors() == false) {
				vert->color[0] = 0.8f;
				vert->color[1] = 0.8f;
				vert->color[2] = 0.8f;
				vert->color[3] = 1.0f;
			}
			else {
				const ColorA& c = vertexColor(face->v[v]);
				vert->color[0] = float(c.r);
				vert->color[1] = float(c.g);
				vert->color[2] = float(c.b);
				vert->color[3] = float(c.a);
			}
			vert->uv = ORIGIN;
		}

		// Allocate material group.
		faceGroups.resize(max(faceGroups.size(), face->materialIndex()+1));
	}

	int* vertexNormalCounts = NULL;
	if(allMask != 0) {
		Vector_3<float>* groupVertexNormals = new Vector_3<float>[vertexCount()];
		int* groupNormalCounts = new int[vertexCount()];
        vertexNormalCounts = new int[faceCount() * 3];
		memset(vertexNormalCounts, 0, sizeof(int) * faceCount() * 3);

		for(int group=0; group<MAX_NUM_SMOOTHING_GROUPS; group++) {
			quint32 groupMask = ((quint32)1)<<group;
            if((allMask & groupMask) == 0) continue;

			// Reset temporary array.
			memset(groupVertexNormals, 0, sizeof(Vector_3<float>) * vertexCount());
			memset(groupNormalCounts, 0, sizeof(int) * vertexCount());

			// Compute vertex normals for current smoothing group.
			QVector<TriMeshFace>::const_iterator face = faces().begin();
			const QVector<TriMeshFace>::const_iterator faceend = faces().end();
			for(; face != faceend; ++face) {
				// Skip faces with wrong smoothing group.
				if((face->smoothingGroup() & groupMask) == 0) continue;

				// Add face normal to vertex normals.
				for(int fv=0; fv<3; fv++) {
					int vindex = face->vertex(fv);
					groupVertexNormals[vindex].X += (float)face->normal.X;
					groupVertexNormals[vindex].Y += (float)face->normal.Y;
					groupVertexNormals[vindex].Z += (float)face->normal.Z;
					groupNormalCounts[vindex] ++;
				}
			}

			// Add temporary vertex normals to face vertex normals.
			face = faces().begin();
			RenderVertex* rv = renderVertices;
			int* vnc = vertexNormalCounts;
			for(; face != faceend; ++face, rv += 3, vnc += 3) {
				// Skip faces with wrong smoothing group.
				if((face->smoothingGroup() & groupMask) == 0) continue;

				for(int fv=0; fv<3; fv++) {
					int vindex = face->vertex(fv);
					rv[fv].normal += groupVertexNormals[vindex];
					vnc[fv] += groupNormalCounts[vindex];
				}
			}
		}
		delete[] groupVertexNormals;
		delete[] groupNormalCounts;
	}

	// Normalize vertex normals.
	int* vnCounts = vertexNormalCounts;
	vert = renderVertices;

	for(face = faces().begin(); face != faces().end(); ++face) {
		// Insert into right material group.
        MaterialGroup& materialGroup = faceGroups[face->materialIndex()];
		for(int fv=0; fv<3; fv++, vnCounts++, vert++) {
			materialGroup.push_back(vert - renderVertices);
			if(vertexNormalCounts == NULL || *vnCounts == 0) {
				vert->normal.X = (float)face->normal.X;
				vert->normal.Y = (float)face->normal.Y;
				vert->normal.Z = (float)face->normal.Z;
			}
			else if(*vnCounts > 1) {
				vert->normal = NormalizeSafely(vert->normal);
			}
		}
	}
    delete[] vertexNormalCounts;

	cacheState |= RENDER_VERTICES_CACHED;
}

/******************************************************************************
* Makes a complete copy of a mesh.
******************************************************************************/
TriMesh& TriMesh::operator=(const TriMesh& mesh)
{
	_vertices = mesh._vertices;
	_faces = mesh._faces;
	_vertexColors = mesh._vertexColors;
	_hasVertexColors = mesh._hasVertexColors;
	bbox = mesh.bbox;
    cacheState = NONE_CACHED;
	renderEdges.clear();
	return *this;
}

/******************************************************************************
* Saves the mesh to the given stream.
******************************************************************************/
void TriMesh::saveToStream(SaveStream& stream)
{
	stream.beginChunk(0x02);

	// Save vertices.
	stream << _vertices;

	// Save vertex colors.
	stream << _hasVertexColors;
	stream << _vertexColors;

	// Save faces.
	stream << faceCount();
	for(QVector<TriMeshFace>::const_iterator face = faces().begin(); face != faces().end(); ++face) {
		stream.writeEnum(face->flags);
		stream << face->v[0];
		stream << face->v[1];
		stream << face->v[2];
		stream << face->smoothingGroup();
		stream << face->materialIndex();
		stream << face->normal;
	}

	stream.endChunk();
}

/******************************************************************************
* Loads the mesh from the given stream.
******************************************************************************/
void TriMesh::loadFromStream(LoadStream& stream)
{
	stream.expectChunk(0x02);

	// Reset mesh.
	clearMesh();

	// Load vertices.
	stream >> _vertices;

	// Load vertex colors.
	stream >> _hasVertexColors;
	stream >> _vertexColors;
	OVITO_ASSERT(_vertexColors.size() == _vertices.size() || !_hasVertexColors);

	// Load faces.
	int nFaces;
	stream >> nFaces;
	_faces.resize(nFaces);
	for(QVector<TriMeshFace>::iterator face = faces().begin(); face != faces().end(); ++face) {
		stream.readEnum(face->flags);
		stream >> face->v[0];
		stream >> face->v[1];
		stream >> face->v[2];
		stream >> face->smGroup >> face->matIndex;
		stream >> face->normal;
	}

	stream.closeChunk();
}

/******************************************************************************
* Performs a ray intersection calculation.
******************************************************************************/
bool TriMesh::intersectRay(const Ray3& ray, FloatType& t, Vector3& normal, int& faceIndex, bool backfaceCull) const
{
	QVector<TriMeshFace>::const_iterator face = faces().constBegin();
	QVector<TriMeshFace>::const_iterator faceend = faces().constEnd();
	FloatType bestT = FLOATTYPE_MAX;
	for(; face != faceend; ++face) {

		Point3 v0 = vertex(face->vertex(0));
		Vector3 e1 = vertex(face->vertex(1)) - v0;
		Vector3 e2 = vertex(face->vertex(2)) - v0;

		Vector3 h = CrossProduct(ray.dir, e2);
		FloatType a = DotProduct(e1, h);

		if(fabs(a) < 1e-5)
			continue;

		FloatType f = 1/a;
		Vector3 s = ray.base - v0;
		FloatType u = f * (DotProduct(s, h));

		if(u < 0.0 || u > 1.0)
			continue;

		Vector3 q = CrossProduct(s, e1);
		FloatType v = f * DotProduct(ray.dir, q);

		if(v < 0.0 || u + v > 1.0)
			continue;

		FloatType tt = f * DotProduct(e2, q);

		if(tt < FLOATTYPE_EPSILON)
			continue;

		if(tt >= bestT)
			continue;

		// Compute face normal.
		Vector3 faceNormal = CrossProduct(e1, e2);
		if(faceNormal == NULL_VECTOR) continue;

		// Do backface culling.
		if(backfaceCull && DotProduct(faceNormal, ray.dir) >= 0)
			continue;

		bestT = tt;
		normal = faceNormal;
		faceIndex = (face - faces().constBegin());
	}

	if(bestT != FLOATTYPE_MAX) {
		t = bestT;
		return true;
	}

	return false;
}


};	// End of namespace Mesh
