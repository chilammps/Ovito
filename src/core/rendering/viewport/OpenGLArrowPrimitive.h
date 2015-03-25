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

#ifndef __OVITO_VIEWPORT_ARROW_GEOMETRY_BUFFER_H
#define __OVITO_VIEWPORT_ARROW_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/ArrowPrimitive.h>
#include "OpenGLBuffer.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief Buffer object that stores a set of arrows to be rendered in the viewports.
 */
class OVITO_CORE_EXPORT OpenGLArrowPrimitive : public ArrowPrimitive
{
public:

	/// Constructor.
	OpenGLArrowPrimitive(ViewportSceneRenderer* renderer, ArrowPrimitive::Shape shape, ShadingMode shadingMode, RenderingQuality renderingQuality);

	/// \brief Allocates a geometry buffer with the given number of elements.
	virtual void startSetElements(int elementCount) override;

	/// \brief Returns the number of elements stored in the buffer.
	virtual int elementCount() const override { return _elementCount; }

	/// \brief Sets the properties of a single line element.
	virtual void setElement(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width) override;

	/// \brief Finalizes the geometry buffer after all elements have been set.
	virtual void endSetElements() override;

	/// \brief Changes the shading mode for elements.
	virtual bool setShadingMode(ShadingMode mode) override { return (mode == shadingMode()); }

	/// \brief Changes the rendering quality of elements.
	virtual bool setRenderingQuality(RenderingQuality level) { return (renderingQuality() == level); }

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer) override;

private:

	/// \brief Creates the geometry for a single cylinder element.
	void createCylinderElement(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width);

	/// \brief Creates the geometry for a single arrow element.
	void createArrowElement(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width);

	/// Renders the geometry as triangle mesh with normals.
	void renderWithNormals(ViewportSceneRenderer* renderer);

	/// Renders the geometry as with extra information passed to the vertex shader.
	void renderWithElementInfo(ViewportSceneRenderer* renderer);

private:

	/// Per-vertex data stored in VBOs when rendering triangle geometry.
	struct VertexWithNormal {
		Point_3<float> pos;
		Vector_3<float> normal;
		ColorAT<float> color;
	};

	/// Per-vertex data stored in VBOs when rendering raytraced cylinders.
	struct VertexWithElementInfo {
		Point_3<float> pos;
		Point_3<float> base;
		Vector_3<float> dir;
		ColorAT<float> color;
		float radius;
	};

	/// The GL context group under which the GL vertex buffers have been created.
	QPointer<QOpenGLContextGroup> _contextGroup;

	/// The number of elements stored in the buffer.
	int _elementCount;

	/// The number of cylinder segments to generate.
	int _cylinderSegments;

	/// The number of mesh vertices generated per element.
	int _verticesPerElement;

	/// The OpenGL vertex buffer objects that store the vertices with normal vectors for polygon rendering.
	std::vector<OpenGLBuffer<VertexWithNormal>> _verticesWithNormals;

	/// The OpenGL vertex buffer objects that store the vertices with full element info for raytraced shader rendering.
	std::vector<OpenGLBuffer<VertexWithElementInfo>> _verticesWithElementInfo;

	/// The index of the VBO chunk currently mapped to memory.
	int _mappedChunkIndex;

	/// Pointer to the memory-mapped VBO buffer.
	VertexWithNormal* _mappedVerticesWithNormals;

	/// Pointer to the memory-mapped VBO buffer.
	VertexWithElementInfo* _mappedVerticesWithElementInfo;

	/// The maximum size (in bytes) of a single VBO buffer.
	int _maxVBOSize;

	/// The maximum number of render elements per VBO buffer.
	int _chunkSize;

	/// Indicates that an OpenGL geometry shader is being used.
	bool _usingGeometryShader;

	/// The OpenGL shader program that is used for rendering.
	QOpenGLShaderProgram* _shader;

	/// The OpenGL shader program that is used for picking primitives.
	QOpenGLShaderProgram* _pickingShader;

	/// Lookup table for fast cylinder geometry generation.
	std::vector<float> _cosTable;

	/// Lookup table for fast cylinder geometry generation.
	std::vector<float> _sinTable;

	/// Primitive start indices passed to glMultiDrawArrays() using GL_TRIANGLE_STRIP primitives.
	std::vector<GLint> _stripPrimitiveVertexStarts;

	/// Primitive vertex counts passed to glMultiDrawArrays() using GL_TRIANGLE_STRIP primitives.
	std::vector<GLsizei> _stripPrimitiveVertexCounts;

	/// Primitive start indices passed to glMultiDrawArrays() using GL_TRIANGLE_FAN primitives.
	std::vector<GLint> _fanPrimitiveVertexStarts;

	/// Primitive vertex counts passed to glMultiDrawArrays() using GL_TRIANGLE_FAN primitives.
	std::vector<GLsizei> _fanPrimitiveVertexCounts;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_VIEWPORT_ARROW_GEOMETRY_BUFFER_H
