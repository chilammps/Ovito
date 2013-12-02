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
 * \file ViewportTriMeshGeometryBuffer.h
 * \brief Contains the definition of the Ovito::ViewportTriMeshGeometryBuffer class.
 */

#ifndef __OVITO_VIEWPORT_TRIMESH_GEOMETRY_BUFFER_H
#define __OVITO_VIEWPORT_TRIMESH_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/TriMeshGeometryBuffer.h>
#include <core/scene/objects/geometry/TriMesh.h>

namespace Ovito {

/**
 * \brief Buffer object that stores a triangle mesh to be rendered in the viewports.
 */
class OVITO_CORE_EXPORT ViewportTriMeshGeometryBuffer : public TriMeshGeometryBuffer
{
public:

	/// Constructor.
	ViewportTriMeshGeometryBuffer(ViewportSceneRenderer* renderer);

	/// Sets the mesh to be stored in this buffer object.
	virtual void setMesh(const TriMesh& mesh, const ColorA& meshColor) override;

	/// \brief Returns the number of triangle faces stored in the buffer.
	virtual int faceCount() override { return _renderVertexCount / 3; }

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer, quint32 pickingBaseID) override;

protected:

	/// Makes vertex IDs available to the shader.
	void activateVertexIDs(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader);

	/// Disables vertex IDs.
	void deactivateVertexIDs(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader);

private:

	/// Stores data of a single vertex passed to the OpenGL implementation.
	struct ColoredVertexWithNormal {
		Point_3<float> pos;
		Vector_3<float> normal;
		ColorAT<float> color;
	};

	/// The internal OpenGL vertex buffer that stores the vertex data.
	QOpenGLBuffer _glVertexBuffer;

	/// The internal OpenGL vertex buffer that stores the vertex indices.
	QOpenGLBuffer _glIndexBuffer;

	/// The number of vertices stored in the OpenGL buffer.
	int _renderVertexCount;

	/// The GL context group under which the GL vertex buffer has been created.
	QOpenGLContextGroup* _contextGroup;

	/// The OpenGL shader program used to render the triangles.
	QOpenGLShaderProgram* _shader;

	/// The OpenGL shader program used to render the triangles in picking mode.
	QOpenGLShaderProgram* _pickingShader;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_VIEWPORT_TRIMESH_GEOMETRY_BUFFER_H
