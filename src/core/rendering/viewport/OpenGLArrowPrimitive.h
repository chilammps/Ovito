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

	struct ColoredVertexWithNormal {
		Point_3<float> pos;
		Vector_3<float> normal;
		ColorAT<float> color;
	};

	struct ColoredVertexWithVector {
		Point_3<float> pos;
		Point_3<float> base;
		Vector_3<float> dir;
		ColorAT<float> color;
	};

	struct ColoredVertexWithElementInfo {
		Point_3<float> pos;
		Point_3<float> base;
		Vector_3<float> dir;
		ColorAT<float> color;
		float radius;
	};

	/// \brief Creates the geometry for a single cylinder element.
	void createCylinderElement(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width);

	/// \brief Creates the geometry for a single arrow element.
	void createArrowElement(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width);

	/// \brief Renders the elements in shaded mode.
	void renderShadedTriangles(ViewportSceneRenderer* renderer);

	/// \brief Renders the cylinder elements in using a raytracing hardware shader.
	void renderRaytracedCylinders(ViewportSceneRenderer* renderer);

	/// \brief Renders the arrows in flat mode.
	void renderFlat(ViewportSceneRenderer* renderer);

private:

	/// The internal OpenGL vertex buffers that store the vertices and colors.
	std::vector<QOpenGLBuffer> _glGeometryBuffers;

	/// The GL context group under which the GL vertex buffers have been created.
	QPointer<QOpenGLContextGroup> _contextGroup;

	/// The number of elements stored in the buffer.
	int _elementCount;

	/// The number of cylinder segments to generate.
	int _cylinderSegments;

	/// The number of mesh vertices generated per element.
	int _verticesPerElement;

	/// Pointer to the memory-mapped VBO buffer.
	void* _mappedBuffer;

	/// The index of the VBO buffer currently mapped to memory.
	int _mappedBufferIndex;

	/// The maximum size (in bytes) of a single VBO buffer.
	int _maxVBOSize;

	/// The maximum number of render elements per VBO buffer.
	int _maxVBOElements;

	// The OpenGL shader programs that are used to render the arrows.
	QOpenGLShaderProgram* _flatShader;
	QOpenGLShaderProgram* _shadedShader;
	QOpenGLShaderProgram* _raytracedCylinderShader;
	QOpenGLShaderProgram* _flatPickingShader;
	QOpenGLShaderProgram* _shadedPickingShader;
	QOpenGLShaderProgram* _raytracedCylinderPickingShader;

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
