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

#ifndef __OVITO_DEFAULT_TRIMESH_GEOMETRY_BUFFER_H
#define __OVITO_DEFAULT_TRIMESH_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/MeshPrimitive.h>
#include <core/utilities/mesh/TriMesh.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/**
 * \brief Buffer object that stores triangle mesh geometry to be rendered by a non-interactive renderer.
 */
class OVITO_CORE_EXPORT DefaultMeshPrimitive : public MeshPrimitive
{
public:

	/// Constructor.
	DefaultMeshPrimitive() {}

	/// Sets the mesh to be stored in this buffer object.
	virtual void setMesh(const TriMesh& mesh, const ColorA& meshColor) override {
		// Create a shallow copy of the mesh and store it in this buffer object.
		_mesh = mesh;
		_meshColor = meshColor;
	}

	/// \brief Returns the number of triangle faces stored in the buffer.
	virtual int faceCount() override { return _mesh.faceCount(); }

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer) override;

	/// Returns a internal triangle mesh.
	const TriMesh& mesh() const { return _mesh; }

	/// Returns the rendering color to be used if the mesh doesn't have per-vertex colors.
	const ColorA& meshColor() const { return _meshColor; }

private:

	/// The mesh storing the geometry.
	TriMesh _mesh;

	/// The rendering color to be used if the mesh doesn't have per-vertex colors.
	ColorA _meshColor;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_DEFAULT_TRIMESH_GEOMETRY_BUFFER_H
