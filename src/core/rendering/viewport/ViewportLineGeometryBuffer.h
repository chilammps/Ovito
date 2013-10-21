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
 * \file ViewportLineGeometryBuffer.h
 * \brief Contains the definition of the Ovito::ViewportLineGeometryBuffer class.
 */

#ifndef __OVITO_VIEWPORT_LINE_GEOMETRY_BUFFER_H
#define __OVITO_VIEWPORT_LINE_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/LineGeometryBuffer.h>

namespace Ovito {

class ViewportSceneRenderer;

/**
 * \brief Buffer object that stores line geometry to be rendered in the viewports.
 */
class OVITO_CORE_EXPORT ViewportLineGeometryBuffer : public LineGeometryBuffer
{
public:

	/// Constructor.
	ViewportLineGeometryBuffer(ViewportSceneRenderer* renderer);

	/// \brief Allocates a geometry buffer with the given number of vertices.
	virtual void setSize(int vertexCount) override;

	/// \brief Returns the number of vertices stored in the buffer.
	virtual int vertexCount() const override { return _vertexCount; }

	/// \brief Sets the coordinates of the vertices.
	virtual void setVertexPositions(const Point3* coordinates) override;

	/// \brief Sets the colors of the vertices.
	virtual void setVertexColors(const ColorA* colors) override;

	/// \brief Sets the color of all vertices to the given value.
	virtual void setVertexColor(const ColorA color) override;

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer) override;

private:

	/// The internal OpenGL vertex buffer that stores the vertex positions.
	QOpenGLBuffer _glPositionsBuffer;

	/// The internal OpenGL vertex buffer that stores the vertex colors.
	QOpenGLBuffer _glColorsBuffer;

	/// The GL context group under which the GL vertex buffer has been created.
	QOpenGLContextGroup* _contextGroup;

	/// The OpenGL shader program used to render the lines.
	QOpenGLShaderProgram* _shader;

	/// The number of vertices stored in the buffer.
	int _vertexCount;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_VIEWPORT_LINE_GEOMETRY_BUFFER_H
