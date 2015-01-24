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

#ifndef __OVITO_OPENGL_MESH_PRIMITIVE_H
#define __OVITO_OPENGL_MESH_PRIMITIVE_H

#include <core/Core.h>
#include <core/rendering/MeshPrimitive.h>
#include <core/utilities/mesh/TriMesh.h>
#include "OpenGLBuffer.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief Buffer object that stores a triangle mesh to be rendered in the viewports.
 */
class OVITO_CORE_EXPORT OpenGLMeshPrimitive : public MeshPrimitive, public std::enable_shared_from_this<OpenGLMeshPrimitive>
{
public:

	/// Constructor.
	OpenGLMeshPrimitive(ViewportSceneRenderer* renderer);

	/// Sets the mesh to be stored in this buffer object.
	virtual void setMesh(const TriMesh& mesh, const ColorA& meshColor) override;

	/// \brief Returns the number of triangle faces stored in the buffer.
	virtual int faceCount() override { return _vertexBuffer.elementCount(); }

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer) override;

private:

	/// Stores data of a single vertex passed to the OpenGL implementation.
	struct ColoredVertexWithNormal {
		Point_3<float> pos;
		Vector_3<float> normal;
		ColorAT<float> color;
	};

	/// The internal OpenGL vertex buffer that stores the vertex data.
	OpenGLBuffer<ColoredVertexWithNormal> _vertexBuffer;

	/// The GL context group under which the GL vertex buffer has been created.
	QOpenGLContextGroup* _contextGroup;

	/// The OpenGL shader program used to render the triangles.
	QOpenGLShaderProgram* _shader;

	/// The OpenGL shader program used to render the triangles in picking mode.
	QOpenGLShaderProgram* _pickingShader;

	/// Are we rendering a semi-transparent mesh?
	bool _hasAlpha;

	/// This is required to render translucent triangles in the correct order from back to front.
	std::vector<Point3> _triangleCoordinates;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_OPENGL_MESH_PRIMITIVE_H
