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

/**
 * \file TriMeshFace.h
 * \brief Contains definition of Ovito::TriMeshFace class.
 */

#ifndef __OVITO_TRI_MESH_FACE_H
#define __OVITO_TRI_MESH_FACE_H

#include <core/Core.h>

namespace Ovito {

/// \brief The maximum number of smoothing groups in a mesh.
///
/// Each face in a TriMesh can be a member of one of the 32 possible smoothing groups.
/// Adjacent faces that belong to the same smoothing group are rendered with
/// interpolated normal vectors.
#define OVITO_MAX_NUM_SMOOTHING_GROUPS 		32

/**
 * \brief Represents a triangle in a TriMesh structure.
 */
class TriMeshFace
{
public:

	/// Bit-flags that can be assigned to a mesh face.
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

public:

	/// Default constructor, which sets all flags and the smoothing group to zero.
	TriMeshFace() : _smoothingGroups(0), _materialIndex(0), _flags(EDGES123) {}

	/************************************ Vertices *******************************/

	/// Sets the vertex indices of this face to new values.
	void setVertices(int a, int b, int c) {
		_vertices[0] = a; _vertices[1] = b; _vertices[2] = c;
	}

	/// Sets the vertex index of one vertex to a new value.
	///    which - 0, 1 or 2
	///    newIndex - The new index for the vertex.
	void setVertex(std::size_t which, int newIndex) {
		OVITO_ASSERT(which < 3);
		_vertices[which] = newIndex;
	}

	/// Returns the index into the Mesh vertices array of a face vertex.
	///    which - 0, 1 or 2
	/// Returns the index of the requested vertex.
	int vertex(std::size_t which) const {
		OVITO_ASSERT(which < 3);
		return _vertices[which];
	}

	/************************************ Edges *******************************/

	/// Sets the visibility of the three face edges.
	void setEdgeVisibility(bool e1, bool e2, bool e3) {
		if(e1) _flags |= EDGE1; else _flags &= ~EDGE1;
		if(e2) _flags |= EDGE2; else _flags &= ~EDGE2;
		if(e3) _flags |= EDGE3; else _flags &= ~EDGE3;
	}

	/// Sets the visibility of the three face edges all at once.
	void setEdgeVisibility(MeshFaceFlags edgeVisibility) {
		_flags = edgeVisibility | (_flags & ~EDGES123);
	}

	/// Returns true if the edge is visible.
	///    which - The index of the edge (0, 1 or 2)
	bool edgeVisible(std::size_t which) const {
		OVITO_ASSERT(which < 3);
		return (_flags & (1<<which));
	}

	/************************************ Material *******************************/

	/// Returns the material index assigned to this face.
	int materialIndex() const { return _materialIndex; }

	/// Sets the material index of this face.
	void setMaterialIndex(int index) { _materialIndex = index; }

	/// Sets the smoothing groups of this face.
	void setSmoothingGroups(quint32 smGroups) { _smoothingGroups = smGroups; }

	/// Returns the smoothing groups this face belongs to as a bit array.
	quint32 smoothingGroups() const { return _smoothingGroups; }

private:

	/// \brief The three vertices of the triangle face.
	///
	/// These values are indices into the vertex array of the mesh, starting at 0.
	std::array<int,3> _vertices;

	/// The bit flags.
	MeshFaceFlags _flags;

	/// Smoothing group bits. Specifies the smoothing groups this face belongs to.
	quint32 _smoothingGroups;

	/// The material index assigned to the face.
	int _materialIndex;

	friend class TriMesh;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TriMeshFace::MeshFaceFlags);

};	// End of namespace

Q_DECLARE_TYPEINFO(Ovito::TriMeshFace, Q_MOVABLE_TYPE);

#endif // __OVITO_TRI_MESH_FACE_H
