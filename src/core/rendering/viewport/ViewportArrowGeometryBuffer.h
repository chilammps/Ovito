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
 * \file ViewportArrowGeometryBuffer.h
 * \brief Contains the definition of the Ovito::ViewportArrowGeometryBuffer class.
 */

#ifndef __OVITO_VIEWPORT_ARROW_GEOMETRY_BUFFER_H
#define __OVITO_VIEWPORT_ARROW_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/ArrowGeometryBuffer.h>
#include <core/utilities/opengl/SharedOpenGLResource.h>

namespace Ovito {

class ViewportSceneRenderer;

/**
 * \brief Buffer object that stores a set of arrows to be rendered in the viewports.
 */
class ViewportArrowGeometryBuffer : public ArrowGeometryBuffer
{
public:

	/// Constructor.
	ViewportArrowGeometryBuffer(ViewportSceneRenderer* renderer, ShadingMode shadingMode, RenderingQuality renderingQuality);

	/// \brief Allocates a geometry buffer with the given number of arrows.
	virtual void startSetArrows(int arrowCount) override;

	/// \brief Returns the number of arrows stored in the buffer.
	virtual int arrowCount() const override { return _arrowCount; }

	/// \brief Sets the properties of a single arrow.
	virtual void setArrow(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width) override;

	/// \brief Finalizes the geometry buffer after all arrows have been set.
	virtual void endSetArrows() override;

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer, quint32 pickingBaseID = 0) override;

private:

	struct ColoredVertexWithNormal {
		Point_3<float> pos;
		Vector_3<float> normal;
		ColorAT<float> color;
	};

private:

	/// The internal OpenGL vertex buffer that stores the vertices and colors.
	QOpenGLBuffer _glGeometryBuffer;

	/// The GL context group under which the GL vertex buffers have been created.
	QPointer<QOpenGLContextGroup> _contextGroup;

	/// The number of arrows stored in the buffer.
	int _arrowCount;

	/// The number of cylinder segments to generate.
	int _cylinderSegments;

	/// The number of mesh vertices generated per arrow.
	int _verticesPerArrow;

	// The OpenGL shader programs that are used to render the arrows.
	QPointer<QOpenGLShaderProgram> _flatShader;

	/// Pointer to the memory-mapped geometry buffer.
	ColoredVertexWithNormal* _mappedVertices;

	/// Lookup table for fast cylinder geometry generation.
	std::vector<float> _cosTable;

	/// Lookup table for fast cylinder geometry generation.
	std::vector<float> _sinTable;

	/// Primitive start indices passed to glMultiDrawArrays().
	std::vector<GLint> _primitiveVertexStarts;

	/// Primitive vertex counts passed to glMultiDrawArrays().
	std::vector<GLsizei> _primitiveVertexCounts;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_VIEWPORT_ARROW_GEOMETRY_BUFFER_H
