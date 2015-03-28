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

#ifndef __OVITO_TRI_MESH_H
#define __OVITO_TRI_MESH_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Mesh)

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
	Q_DECLARE_FLAGS(MeshFaceFlags, MeshFaceFlag);

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

/**
 * \brief Stores a triangular mesh.
 */
class OVITO_CORE_EXPORT TriMesh
{
public:

	/// \brief Constructs an empty mesh.
	TriMesh();

	/// \brief Resets the mesh to the empty state.
	void clear();

	/// \brief Returns the bounding box of the mesh.
	/// \return The bounding box of the mesh.
	///
	/// The bounding box is cached by the TriMesh object.
	/// Calling this method multiple times is cheap as long as the vertices of the mesh are not changed.
	const Box3& boundingBox() {
		if(_boundingBox.isEmpty())
			_boundingBox.addPoints(vertices().constData(), vertexCount());
		return _boundingBox;
	}

	/// \brief Returns the number of vertices in this mesh.
	int vertexCount() const { return _vertices.size(); }

	/// \brief Sets the number of vertices in this mesh.
	/// \param n The new number of vertices.
	///
	/// If \a n is larger than the old vertex count then new vertices are added to the mesh.
	/// These new vertices are not initialized by this method. One should use a method like setVertexPos()
	/// to assign a position to the new vertices.
	void setVertexCount(int n);

	/// \brief Allows direct access to the vertex position array of the mesh.
	/// \return A reference to the internal container that stores the vertex coordinates.
	/// \note When you change the vertex positions then you have to call invalidateVertices()
	/// to let the mesh know that it has to update its internal cache based on the new vertex coordinates.
	QVector<Point3>& vertices() { return _vertices; }

	/// \brief Allows direct read-access to the vertex position array of the mesh.
	/// \return A constant reference to the internal container that stores the vertex positions.
	const QVector<Point3>& vertices() const { return _vertices; }

	/// \brief Returns the coordinates of the vertex with the given index.
	/// \param index The index starting at 0 of the vertex whose position should be returned.
	/// \return The position of the given vertex.
	const Point3& vertex(int index) const {
		OVITO_ASSERT(index >= 0 && index < vertexCount());
		return _vertices[index];
	}

	/// \brief Returns a reference to the coordinates of the vertex with the given index.
	/// \param index The index starting at 0 of the vertex whose position should be returned.
	/// \return A reference to the coordinates of the given vertex. The reference can be used to alter the vertex position.
	/// \note If you change the vertex' position then you have to call invalidateVertices() to let the mesh know that it has
	/// to update its internal caches based on the new vertex position.
	Point3& vertex(int index) {
		OVITO_ASSERT(index >= 0 && index < vertexCount());
		return _vertices[index];
	}

	/// \brief Sets the coordinates of the vertex with the given index.
	/// \param index The index starting at 0 of the vertex whose position should be set.
	/// \param p The new position of the vertex.
	/// \note After you have finished changing the vertex positions you have to call invalidateVertices()
	/// to let the mesh know that it has to update its internal caches based on the new vertex position.
	void setVertex(int index, const Point3& p) {
		OVITO_ASSERT(index >= 0 && index < vertexCount());
		_vertices[index] = p;
	}

	/// \brief Adds a new vertex to the mesh.
	/// \param pos The coordinates of the new vertex.
	/// \return The index of the newly created vertex.
	int addVertex(const Point3& pos) {
		int index = vertexCount();
		setVertexCount(index + 1);
		_vertices[index] = pos;
		return index;
	}

	/// \brief Returns whether this mesh has colors associated with its vertices.
	bool hasVertexColors() const {
		return _hasVertexColors;
	}

	/// \brief Controls whether this mesh has colors associated with its vertices.
	void setHasVertexColors(bool enableColors) {
		_hasVertexColors = enableColors;
		_vertexColors.resize(enableColors ? _vertices.size() : 0);
	}

	/// \brief Allows direct access to the vertex color array of the mesh.
	/// \return A reference to the vector that stores all vertex colors.
	/// \note After you have finished changing the vertex colors,
	/// you have to call invalidateVertices() to let the mesh know that it has to update its internal
	/// caches based on the new vertex colors.
	QVector<ColorA>& vertexColors() {
		OVITO_ASSERT(_hasVertexColors);
		OVITO_ASSERT(_vertexColors.size() == _vertices.size());
		return _vertexColors;
	}

	/// \brief Allows direct read-access to the vertex color array of the mesh.
	/// \return A constant reference to the vector that stores all vertex colors.
	const QVector<ColorA>& vertexColors() const {
		OVITO_ASSERT(_hasVertexColors);
		OVITO_ASSERT(_vertexColors.size() == _vertices.size());
		return _vertexColors;
	}

	/// \brief Returns the color of the vertex with the given index.
	/// \param index The index starting at 0 of the vertex whose color should be returned.
	/// \return The color of the given vertex.
	const ColorA& vertexColor(int index) const {
		OVITO_ASSERT(index >= 0 && index < vertexCount());
		return vertexColors()[index];
	}

	/// \brief Returns a reference to the color of the vertex with the given index.
	/// \param index The index starting at 0 of the vertex whose color should be returned.
	/// \return A reference to the color of the given vertex. The reference can be used to alter the vertex color.
	/// \note After you have finished changing the vertex colors,
	/// you have to call invalidateVertices() to let the mesh know that it has
	/// to update its internal caches based on the new vertex colors.
	ColorA& vertexColor(int index) {
		OVITO_ASSERT(index >= 0 && index < vertexCount());
		return vertexColors()[index];
	}

	/// \brief Sets the color of the vertex with the given index.
	/// \param index The index starting at 0 of the vertex whose color should be set.
	/// \param p The new color of the vertex.
	/// \note After you have finished changing the vertex colors,
	/// you have to call invalidateVertices() to let the mesh know that it has
	/// to update its internal caches based on the new vertex colors.
	void setVertexColor(int index, const ColorA& c) {
		vertexColor(index) = c;
	}

	/// \brief Invalidates the parts of the internal mesh cache that depend on the vertex array.
	///
	/// This method must be called each time the vertices of the mesh have been modified.
	void invalidateVertices() {
		_boundingBox.setEmpty();
	}

	/// \brief Returns the number of faces (triangles) in this mesh.
	int faceCount() const { return _faces.size(); }

	/// \brief Sets the number of faces in this mesh.
	/// \param n The new number of faces.
	///
	/// If \a n is larger than the old face count then new faces are added to the mesh.
	/// These new faces are not initialized by this method. One has to use methods of the TriMeshFace class
	/// to assign vertices to the new faces.
	void setFaceCount(int n);

	/// \brief Allows direct access to the face array of the mesh.
	/// \return A reference to the vector that stores all mesh faces.
	/// \note If you change the faces in the returned array then
	/// you have to call invalidateFaces() to let the mesh know that it has
	/// to update its internal caches based on the new face definitions.
	QVector<TriMeshFace>& faces() { return _faces; }

	/// \brief Allows direct read-access to the face array of the mesh.
	/// \return A const reference to the vector that stores all mesh faces.
	const QVector<TriMeshFace>& faces() const { return _faces; }

	/// \brief Returns the face with the given index.
	/// \param index The index starting at 0 of the face who should be returned.
	const TriMeshFace& face(int index) const {
		OVITO_ASSERT(index >= 0 && index < faceCount());
		return _faces[index];
	}

	/// \brief Returns a reference to the face with the given index.
	/// \param index The index starting at 0 of the face who should be returned.
	/// \return A reference to the requested face. This reference can be used to change the
	/// the face.
	/// \note If you change the returned face then
	/// you have to call invalidateFaces() to let the mesh know that it has
	/// to update its internal caches based on the new face definition.
	TriMeshFace& face(int index) {
		OVITO_ASSERT(index >= 0 && index < faceCount());
		return _faces[index];
	}

	/// \brief Adds a new triangle face and returns a reference to it.
	/// \return A reference to the new face. The new face has to be initialized
	/// after it has been created.
	///
	/// Increases the number of faces by one.
	TriMeshFace& addFace();

	/// \brief Invalidates the parts of the internal mesh cache that depend on the face array.
	///
	/// This must be called each time the faces of the mesh have been modified.
	void invalidateFaces() {}

	/// \brief Returns whether this mesh has colors associated with its faces.
	bool hasFaceColors() const {
		return _hasFaceColors;
	}

	/// \brief Controls whether this mesh has colors associated with its faces.
	void setHasFaceColors(bool enableColors) {
		_hasFaceColors = enableColors;
		_faceColors.resize(enableColors ? _faces.size() : 0);
	}

	/// \brief Allows direct access to the face color array of the mesh.
	/// \return A reference to the vector that stores all face colors.
	/// \note After you have finished changing the face colors,
	/// you have to call invalidateFaces() to let the mesh know that it has to update its internal
	/// caches based on the new colors.
	QVector<ColorA>& faceColors() {
		OVITO_ASSERT(_hasFaceColors);
		OVITO_ASSERT(_faceColors.size() == _faces.size());
		return _faceColors;
	}

	/// \brief Allows direct read-access to the face color array of the mesh.
	/// \return A constant reference to the vector that stores all face colors.
	const QVector<ColorA>& faceColors() const {
		OVITO_ASSERT(_hasFaceColors);
		OVITO_ASSERT(_faceColors.size() == _faces.size());
		return _faceColors;
	}

	/// \brief Returns the color of the face with the given index.
	/// \param index The index starting at 0 of the face whose color should be returned.
	/// \return The color of the given face.
	const ColorA& faceColor(int index) const {
		OVITO_ASSERT(index >= 0 && index < faceCount());
		return faceColors()[index];
	}

	/// \brief Returns a reference to the color of the face with the given index.
	/// \param index The index starting at 0 of the face whose color should be returned.
	/// \return A reference to the color of the given face. The reference can be used to alter the face color.
	/// \note After you have finished changing the face colors,
	/// you have to call invalidateFaces() to let the mesh know that it has
	/// to update its internal caches based on the new colors.
	ColorA& faceColor(int index) {
		OVITO_ASSERT(index >= 0 && index < faceCount());
		return faceColors()[index];
	}

	/// \brief Sets the color of the face with the given index.
	/// \param index The index starting at 0 of the face whose color should be set.
	/// \param p The new color of the face.
	/// \note After you have finished changing the face colors,
	/// you have to call invalidateFaces() to let the mesh know that it has
	/// to update its internal caches based on the new colors.
	void setFaceColor(int index, const ColorA& c) {
		vertexColor(index) = c;
	}

	/************************************* Ray intersection *************************************/

	/// \brief Performs a ray intersection calculation.
	/// \param ray The ray to test for intersection with the object.
	/// \param t If an intersection has been found, the method stores the distance of the intersection in this output parameter.
	/// \param normal If an intersection has been found, the method stores the surface normal at the point of intersection in this output parameter.
	/// \param faceIndex If an intersection has been found, the method stores the index of the face intersected by the ray in this output parameter.
	/// \param backfaceCull Controls whether backfacing faces are tested too.
	/// \return True if the closest intersection has been found. False if no intersection has been found.
	bool intersectRay(const Ray3& ray, FloatType& t, Vector3& normal, int& faceIndex, bool backfaceCull) const;

	/*********************************** Persistence ***********************************/

	/// \brief Saves the mesh to the given stream.
    /// \param stream The destination stream.
	void saveToStream(SaveStream& stream);

	/// \brief Loads the mesh from the given stream.
    /// \param stream The source stream.
	void loadFromStream(LoadStream& stream);

	/// \brief Exports the triangle mesh to a VTK file.
	void saveToVTK(CompressedTextWriter& stream);

private:

	/// The cached bounding box of the mesh computed from the vertices.
	Box3 _boundingBox;

	/// Array of vertex coordinates.
	QVector<Point3> _vertices;

	/// Indicates that per-vertex colors are stored in this mesh.
	bool _hasVertexColors;

	/// Array of vertex colors.
	QVector<ColorA> _vertexColors;

	/// Indicates that per-face colors are stored in this mesh.
	bool _hasFaceColors;

	/// Array of face colors.
	QVector<ColorA> _faceColors;

	/// Array of mesh faces.
	QVector<TriMeshFace> _faces;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_TYPEINFO(Ovito::TriMeshFace, Q_MOVABLE_TYPE);

#endif // __OVITO_TRI_MESH_H
