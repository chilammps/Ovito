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

#include <core/Core.h>
#include "ViewportArrowGeometryBuffer.h"
#include "ViewportSceneRenderer.h"

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, ViewportArrowGeometryBuffer, ArrowGeometryBuffer);

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportArrowGeometryBuffer::ViewportArrowGeometryBuffer(ViewportSceneRenderer* renderer, ShadingMode shadingMode, RenderingQuality renderingQuality) :
	ArrowGeometryBuffer(shadingMode, renderingQuality),
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_arrowCount(-1), _cylinderSegments(8), _verticesPerArrow(0), _mappedVertices(nullptr)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	if(!_glGeometryBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glGeometryBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// Initialize OpenGL shaders.
	_flatShader = renderer->loadShaderProgram(
			"arrow_flat",
			":/core/glsl/arrows/flat.vs",
			":/core/glsl/arrows/flat.fs");
}

/******************************************************************************
* Allocates a particle buffer with the given number of arrows.
******************************************************************************/
void ViewportArrowGeometryBuffer::startSetArrows(int arrowCount)
{
	OVITO_ASSERT(_glGeometryBuffer.isCreated());
	OVITO_ASSERT(arrowCount >= 0);
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_mappedVertices == nullptr);

	_arrowCount = arrowCount;
	_verticesPerArrow = _cylinderSegments * 2 + 2;
	OVITO_ASSERT(arrowCount < std::numeric_limits<int>::max() / sizeof(ColoredVertexWithNormal) / _verticesPerArrow);

	if(!_glGeometryBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glGeometryBuffer.allocate(_arrowCount * _verticesPerArrow * sizeof(ColoredVertexWithNormal));
	if(_arrowCount) {
		_mappedVertices = static_cast<ColoredVertexWithNormal*>(_glGeometryBuffer.map(QOpenGLBuffer::WriteOnly));
		if(!_mappedVertices)
			throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));
	}

	// Prepare arrays to be passed to the glMultiDrawArrays() function.
	_primitiveVertexStarts.resize(_arrowCount);
	GLint startIndex = 0;
	for(auto& i : _primitiveVertexStarts) {
		i = startIndex;
		startIndex += _verticesPerArrow;
	}
	_primitiveVertexCounts.resize(_arrowCount);
	std::fill(_primitiveVertexCounts.begin(), _primitiveVertexCounts.end(), _verticesPerArrow);

	// Precompute cosine and sine functions.
	_cosTable.resize(_cylinderSegments+1);
	_sinTable.resize(_cylinderSegments+1);
	for(int i = 0; i <= _cylinderSegments; i++) {
		float angle = (FLOATTYPE_PI * 2 / _cylinderSegments) * i;
		_cosTable[i] = std::cos(angle);
		_sinTable[i] = std::sin(angle);
	}
}

/******************************************************************************
* Sets the properties of a single arrow.
******************************************************************************/
void ViewportArrowGeometryBuffer::setArrow(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width)
{
	OVITO_ASSERT(_mappedVertices != nullptr);
	OVITO_ASSERT(index >= 0 && index < _arrowCount);

	Vector_3<float> t, u, v;

	if(dir != Vector3::Zero()) {
		t = dir.normalized();
		u = Vector_3<float>(dir.y(), -dir.x(), 0);
		if(u == Vector_3<float>::Zero())
			u = Vector_3<float>(-dir.z(), 0, dir.x());
		u.normalize();
		v = u.cross(t);
	}
	else {
		t.setZero();
		u.setZero();
		v.setZero();
	}

	Point_3<float> v1 = pos;
	Point_3<float> v2 = v1 + dir;
	ColorAT<float> c = color;

	ColoredVertexWithNormal* vertex = _mappedVertices + (index * _verticesPerArrow);
	for(int i = 0; i <= _cylinderSegments; i++) {
		Vector_3<float> n = _cosTable[i] * u + _sinTable[i] * v;
		Vector_3<float> d = n * width;
		vertex->pos = v1 + d;
		vertex->normal = n;
		vertex->color = c;
		vertex++;
		vertex->pos = v2 + d;
		vertex->normal = n;
		vertex->color = c;
		vertex++;
	}
}

/******************************************************************************
* Finalizes the geometry buffer after all arrows have been set.
******************************************************************************/
void ViewportArrowGeometryBuffer::endSetArrows()
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_arrowCount >= 0);
	OVITO_ASSERT(_mappedVertices != nullptr || _arrowCount == 0);

	if(_arrowCount)
		_glGeometryBuffer.unmap();
	_glGeometryBuffer.release();
	_mappedVertices = nullptr;
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportArrowGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);
	if(!vpRenderer) return false;
	return _glGeometryBuffer.isCreated()
			&& _arrowCount >= 0
			&& (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void ViewportArrowGeometryBuffer::render(SceneRenderer* renderer, quint32 pickingBaseID)
{
	OVITO_CHECK_OPENGL();
	OVITO_ASSERT(_glGeometryBuffer.isCreated());
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_ASSERT(_arrowCount >= 0);
	OVITO_ASSERT(_mappedVertices== nullptr);

	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(_arrowCount <= 0 || !vpRenderer)
		return;

	glEnable(GL_CULL_FACE);

	if(!_flatShader->bind())
		throw Exception(tr("Failed to bind OpenGL shader."));

	_flatShader->setUniformValue("modelview_projection_matrix",
			(QMatrix4x4)(vpRenderer->projParams().projectionMatrix * vpRenderer->modelViewTM()));

	_glGeometryBuffer.bind();
	_flatShader->setAttributeBuffer("vertex_pos", GL_FLOAT, offsetof(ColoredVertexWithNormal, pos), 3, sizeof(ColoredVertexWithNormal));
	_flatShader->enableAttributeArray("vertex_pos");
	_flatShader->setAttributeBuffer("vertex_color", GL_FLOAT, offsetof(ColoredVertexWithNormal, color), 4, sizeof(ColoredVertexWithNormal));
	_flatShader->enableAttributeArray("vertex_color");
	_glGeometryBuffer.release();

	OVITO_CHECK_OPENGL(glMultiDrawArrays(GL_TRIANGLE_STRIP, _primitiveVertexStarts.data(), _primitiveVertexCounts.data(), _arrowCount));

	_flatShader->release();
}

};
