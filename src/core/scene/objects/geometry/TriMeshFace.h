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

/**
 * \file TriMeshFace.h
 * \brief Contains definition of Mesh::TriMeshFace class.
 */

#ifndef __OVITO_TRI_MESH_FACE_H
#define __OVITO_TRI_MESH_FACE_H

#include <mesh/Mesh.h>

namespace Mesh {

/// \brief The maximum number of smoothing groups in a mesh.
///
/// Each face in a TriMesh can be a member of one of the 32 smoothing groups.
/// Adjacent faces that belong to the same smoothing group are rendered with
/// an interpolated normal vector.
///
/// \sa TriMeshFace::setSmootingGroup()
#define MAX_NUM_SMOOTHING_GROUPS 32

/******************************************************************************
*
******************************************************************************/
/**
 * \brief Represents a triangle in a TriMesh structure.
 *
 * \author Alexander Stukowski
 * \sa TriMesh
 */
struct TriMeshFace
{
	/************************************ Fields *********************************/

	/// \brief Indices of the three triangle vertices.
	///
	/// These values are indices into the vertex array of the mesh, starting at 0.
	/// \sa TriMesh::vertexPositions()
	int v[3];

	/// \brief The face normal vector.
	Vector3 normal;

	/// Flags that can be assigned to a mesh face.
	enum MeshFaceFlag {
		NONE = 0,		//< No flags
		EDGE1 = (1<<0),	//< First edge visible
		EDGE2 = (1<<1),	//< Second edge visible
		EDGE3 = (1<<2),	//< Third edge visible
        EDGES12 = EDGE1 | EDGE2,	//< First and second edge visible
		EDGES23 = EDGE2 | EDGE3,	//< Second and third edge visible
		EDGES13 = EDGE1 | EDGE3,	//< First and third edge visible
		EDGES123 = EDGE1 | EDGE2 | EDGE3,	//< All edges Visible
	};
	Q_DECLARE_FLAGS(MeshFaceFlags, MeshFaceFlag)

	/// Smoothing group bits for the face.
	quint32 smGroup;

	/// The material index assigned to the face.
	int matIndex;

	/// The bit flags.
	MeshFaceFlags flags;

	/// Default constructor that sets all flags and the smoothing group to zero.
	TriMeshFace() : smGroup(0), matIndex(0), flags(EDGES123) {}

	/************************************ Vertices *******************************/

	/// Sets the vertex indices of this face to new values.
	void setVertices(int a, int b, int c) {
		v[0] = a; v[1] = b; v[2] = c;
	}

	/// Sets the vertex index of one vertex to a new value.
	///    which - 0, 1 or 2
	///    newIndex - The new index for the vertex.
	void setVertex(int which, int newIndex) {
		OVITO_ASSERT(which >= 0 && which < 3);
		v[which] = newIndex;
	}

	/// Returns the index into the Mesh vertices array of a face vertex.
	///    which - 0, 1 or 2
	/// Returns the index of the requested vertex.
	int vertex(int which) const {
		OVITO_ASSERT(which >= 0 && which < 3);
		return v[which];
	}

	/************************************ Edges *******************************/

	/// Sets the visibility of the three face edges.
	void setEdgeVisibility(bool e1, bool e2, bool e3) {
		if(e1) flags |= EDGE1; else flags &= ~EDGE1;
		if(e2) flags |= EDGE2; else flags &= ~EDGE2;
		if(e3) flags |= EDGE3; else flags &= ~EDGE3;
	}

	/// Sets the visibility of the three face edges all at once.
	void setEdgeVisibility(MeshFaceFlags edgeVisibility) {
		flags = edgeVisibility | (flags & ~EDGES123);
	}

	/// Returns true if the edge is visible.
	///    which - The index of the edge (0, 1 or 2)
	bool edgeVisibility(int which) const {
		OVITO_ASSERT(which >= 0 && which < 3);
		return (flags & (1<<which)) != 0;
	}

	/************************************ Material *******************************/

	/// Returns the material index assigned to this face.
	int materialIndex() const { return matIndex; }

	/// Sets the material index of this face.
	void setMaterialIndex(int index) { this->matIndex = index; }

	/// Sets the smooting group of this face.
	void setSmoothingGroup(quint32 smGroup) { this->smGroup = smGroup; }

	/// Returns the smooting group of this face.
	quint32 smoothingGroup() const { return smGroup; }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(TriMeshFace::MeshFaceFlags)

};	// End of namespace Mesh

Q_DECLARE_TYPEINFO(Mesh::TriMeshFace, Q_MOVABLE_TYPE);

#endif // __OVITO_TRI_MESH_FACE_H
