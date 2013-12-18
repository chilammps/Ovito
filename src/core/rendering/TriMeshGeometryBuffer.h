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
 * \file TriMeshGeometryBuffer.h
 * \brief Contains the definition of the Ovito::TriMeshGeometryBuffer class.
 */

#ifndef __OVITO_TRIMESH_GEOMETRY_BUFFER_H
#define __OVITO_TRIMESH_GEOMETRY_BUFFER_H

#include <core/Core.h>

namespace Ovito {

/**
 * \brief Abstract base class for scene render buffers that store triangle meshes.
 */
class OVITO_CORE_EXPORT TriMeshGeometryBuffer
{
public:

	/// \brief Virtual base constructor.
	virtual ~TriMeshGeometryBuffer() {}

	/// Sets the mesh to be stored in this buffer object.
	virtual void setMesh(const TriMesh& mesh, const ColorA& meshColor) = 0;

	/// \brief Returns the number of triangle faces stored in the buffer.
	virtual int faceCount() = 0;

	/// \brief Returns true if the buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) = 0;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer) = 0;
};

};

#endif // __OVITO_TRIMESH_GEOMETRY_BUFFER_H
