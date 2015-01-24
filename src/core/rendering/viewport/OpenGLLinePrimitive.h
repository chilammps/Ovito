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

#ifndef __OVITO_OPENGL_LINE_PRIMITIVE_H
#define __OVITO_OPENGL_LINE_PRIMITIVE_H

#include <core/Core.h>
#include <core/rendering/LinePrimitive.h>
#include "OpenGLBuffer.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief This class is responsible for rendering line primitives using OpenGL.
 */
class OVITO_CORE_EXPORT OpenGLLinePrimitive : public LinePrimitive
{
public:

	/// Constructor.
	OpenGLLinePrimitive(ViewportSceneRenderer* renderer);

	/// \brief Allocates a geometry buffer with the given number of vertices.
	virtual void setVertexCount(int vertexCount, FloatType lineWidth) override;

	/// \brief Returns the number of vertices stored in the buffer.
	virtual int vertexCount() const override { return _positionsBuffer.elementCount(); }

	/// \brief Sets the coordinates of the vertices.
	virtual void setVertexPositions(const Point3* coordinates) override;

	/// \brief Sets the colors of the vertices.
	virtual void setVertexColors(const ColorA* colors) override;

	/// \brief Sets the color of all vertices to the given value.
	virtual void setLineColor(const ColorA color) override;

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer) override;

protected:

	/// \brief Renders the lines using GL_LINES mode.
	void renderLines(ViewportSceneRenderer* renderer);

	/// \brief Renders the lines using polygons.
	void renderThickLines(ViewportSceneRenderer* renderer);

private:

	/// The internal OpenGL vertex buffer that stores the vertex positions.
	OpenGLBuffer<Point3> _positionsBuffer;

	/// The internal OpenGL vertex buffer that stores the vertex colors.
	OpenGLBuffer<ColorA> _colorsBuffer;

	/// The internal OpenGL vertex buffer that stores the line segment vectors.
	OpenGLBuffer<Vector3> _vectorsBuffer;

	/// The internal OpenGL vertex buffer that stores the indices for a call to glDrawElements().
	OpenGLBuffer<GLuint> _indicesBuffer;

	/// The client-side buffer that stores the indices for a call to glDrawElements().
	std::vector<GLuint> _indicesBufferClient;
	
	/// The GL context group under which the GL vertex buffer has been created.
	QOpenGLContextGroup* _contextGroup;

	/// The OpenGL shader program used to render the lines.
	QOpenGLShaderProgram* _shader;

	/// The OpenGL shader program used to render the lines in picking mode.
	QOpenGLShaderProgram* _pickingShader;

	/// The OpenGL shader program used to render thick lines.
	QOpenGLShaderProgram* _thickLineShader;

	/// The OpenGL shader program used to render thick lines in picking mode.
	QOpenGLShaderProgram* _thickLinePickingShader;

	/// The width of lines in screen space.
	FloatType _lineWidth;
	
	/// Indicates that an index VBO is used.
	bool _useIndexVBO;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_OPENGL_LINE_PRIMITIVE_H
