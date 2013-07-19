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
	_arrowCount(-1), _cylinderSegments(16), _verticesPerArrow(0),
	_mappedVerticesShaded(nullptr), _mappedVerticesFlat(nullptr)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	if(!_glGeometryBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glGeometryBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// Initialize OpenGL shaders.

	_shadedShader = renderer->loadShaderProgram(
			"arrow_shaded",
			":/core/glsl/arrows/shaded.vs",
			":/core/glsl/arrows/shaded.fs");

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
	OVITO_ASSERT(_mappedVerticesShaded == nullptr);
	OVITO_ASSERT(_mappedVerticesFlat == nullptr);

	if(!_glGeometryBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));

	_arrowCount = arrowCount;
	if(shadingMode() == NormalShading) {
		int cylinderCount = _cylinderSegments * 2 + 2;
		int discCount = _cylinderSegments;
		_verticesPerArrow = 2 * cylinderCount + 2 * discCount;
		OVITO_ASSERT(arrowCount < std::numeric_limits<int>::max() / sizeof(ColoredVertexWithNormal) / _verticesPerArrow);
		_glGeometryBuffer.allocate(_arrowCount * _verticesPerArrow * sizeof(ColoredVertexWithNormal));
		if(_arrowCount)
			_mappedVerticesShaded = static_cast<ColoredVertexWithNormal*>(_glGeometryBuffer.map(QOpenGLBuffer::WriteOnly));

		// Prepare arrays to be passed to the glMultiDrawArrays() function.
		_stripPrimitiveVertexStarts.resize(_arrowCount * 2);
		_stripPrimitiveVertexCounts.resize(_arrowCount * 2);
		_fanPrimitiveVertexStarts.resize(_arrowCount * 2);
		_fanPrimitiveVertexCounts.resize(_arrowCount * 2);
		auto ps_strip = _stripPrimitiveVertexStarts.begin();
		auto pc_strip = _stripPrimitiveVertexCounts.begin();
		auto ps_fan = _fanPrimitiveVertexStarts.begin();
		auto pc_fan = _fanPrimitiveVertexCounts.begin();
		GLint baseIndex = 0;
		for(GLint index = 0; index < _arrowCount; index++, baseIndex += _verticesPerArrow) {
			*ps_strip++ = baseIndex;
			*ps_strip++ = baseIndex + cylinderCount;
			*ps_fan++ = baseIndex + cylinderCount * 2;
			*ps_fan++ = baseIndex + cylinderCount * 2 + discCount;
			*pc_strip++ = cylinderCount;
			*pc_strip++ = cylinderCount;
			*pc_fan++ = discCount;
			*pc_fan++ = discCount;
		}
	}
	else if(shadingMode() == FlatShading) {
		_verticesPerArrow = 7;
		OVITO_ASSERT(arrowCount < std::numeric_limits<int>::max() / sizeof(ColoredVertexWithVector) / _verticesPerArrow);
		_glGeometryBuffer.allocate(_arrowCount * _verticesPerArrow * sizeof(ColoredVertexWithVector));
		if(_arrowCount)
			_mappedVerticesFlat = static_cast<ColoredVertexWithVector*>(_glGeometryBuffer.map(QOpenGLBuffer::WriteOnly));

		// Prepare arrays to be passed to the glMultiDrawArrays() function.
		_fanPrimitiveVertexStarts.resize(_arrowCount);
		GLint startIndex = 0;
		for(auto& i : _fanPrimitiveVertexStarts) {
			i = startIndex;
			startIndex += _verticesPerArrow;
		}
		_fanPrimitiveVertexCounts.resize(_arrowCount);
		std::fill(_fanPrimitiveVertexCounts.begin(), _fanPrimitiveVertexCounts.end(), _verticesPerArrow);
		_stripPrimitiveVertexStarts.clear();
		_stripPrimitiveVertexCounts.clear();
	}
	else OVITO_ASSERT(false);

	if(!_mappedVerticesShaded && !_mappedVerticesFlat && _arrowCount)
		throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));

	// Precompute cosine and sine functions.
	if(shadingMode() == NormalShading) {
		_cosTable.resize(_cylinderSegments+1);
		_sinTable.resize(_cylinderSegments+1);
		for(int i = 0; i <= _cylinderSegments; i++) {
			float angle = (FLOATTYPE_PI * 2 / _cylinderSegments) * i;
			_cosTable[i] = std::cos(angle);
			_sinTable[i] = std::sin(angle);
		}
	}
}

/******************************************************************************
* Sets the properties of a single arrow.
******************************************************************************/
void ViewportArrowGeometryBuffer::setArrow(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width)
{
	OVITO_ASSERT(index >= 0 && index < _arrowCount);

	float arrowHeadRadius = width * 2.5;
	float arrowHeadLength = arrowHeadRadius * 1.8f;

	if(shadingMode() == NormalShading) {
		OVITO_ASSERT(_mappedVerticesShaded != nullptr);

		// Build local coordinate system.
		Vector_3<float> t, u, v;
		float length = dir.length();
		if(length != 0) {
			t = dir / length;
			if(dir.y() != 0 || dir.x() != 0)
				u = Vector_3<float>(dir.y(), -dir.x(), 0);
			else
				u = Vector_3<float>(-dir.z(), 0, dir.x());
			u.normalize();
			v = u.cross(t);
		}
		else {
			t.setZero();
			u.setZero();
			v.setZero();
		}

		ColorAT<float> c = color;
		Point_3<float> v1 = pos;
		Point_3<float> v2;
		Point_3<float> v3 = v1 + dir;
		float r;
		if(length > arrowHeadLength) {
			v2 = v1 + t * (length - arrowHeadLength);
			r = arrowHeadRadius;
		}
		else {
			v2 = v1;
			r = arrowHeadRadius * length / arrowHeadLength;
		}

		ColoredVertexWithNormal* vertex = _mappedVerticesShaded + (index * _verticesPerArrow);

		// Generate vertices for cylinder.
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

		// Generate vertices for head cone.
		for(int i = 0; i <= _cylinderSegments; i++) {
			Vector_3<float> n = _cosTable[i] * u + _sinTable[i] * v;
			Vector_3<float> d = n * r;
			vertex->pos = v2 + d;
			vertex->normal = n;
			vertex->color = c;
			vertex++;
			vertex->pos = v3;
			vertex->normal = n;
			vertex->color = c;
			vertex++;
		}

		// Generate vertices for cylinder cap.
		for(int i = 0; i < _cylinderSegments; i++) {
			Vector_3<float> n = _cosTable[i] * u + _sinTable[i] * v;
			Vector_3<float> d = n * width;
			vertex->pos = v1 + d;
			vertex->normal = Vector_3<float>(0,0,-1);
			vertex->color = c;
			vertex++;
		}

		// Generate vertices for cone cap.
		for(int i = 0; i < _cylinderSegments; i++) {
			Vector_3<float> n = _cosTable[i] * u + _sinTable[i] * v;
			Vector_3<float> d = n * r;
			vertex->pos = v2 + d;
			vertex->normal = Vector_3<float>(0,0,-1);
			vertex->color = c;
			vertex++;
		}
	}
	else if(shadingMode() == FlatShading) {
		OVITO_ASSERT(_mappedVerticesFlat != nullptr);

		Vector_3<float> t;
		float length = dir.length();
		if(length != 0)
			t = dir / length;
		else
			t.setZero();

		ColorAT<float> c = color;
		Point_3<float> base = pos;

		ColoredVertexWithVector* vertices = _mappedVerticesFlat + (index * _verticesPerArrow);
		if(length > arrowHeadLength) {
			vertices[0].pos = Point_3<float>(length, 0, 0);
			vertices[1].pos = Point_3<float>(length - arrowHeadLength, arrowHeadRadius, 0);
			vertices[2].pos = Point_3<float>(length - arrowHeadLength, width, 0);
			vertices[3].pos = Point_3<float>(0, width, 0);
			vertices[4].pos = Point_3<float>(0, -width, 0);
			vertices[5].pos = Point_3<float>(length - arrowHeadLength, -width, 0);
			vertices[6].pos = Point_3<float>(length - arrowHeadLength, -arrowHeadRadius, 0);
		}
		else {
			float r = arrowHeadRadius * length / arrowHeadLength;
			vertices[0].pos = Point_3<float>(length, 0, 0);
			vertices[1].pos = Point_3<float>(0, r, 0);
			vertices[2].pos = Point_3<float>::Origin();
			vertices[3].pos = Point_3<float>::Origin();
			vertices[4].pos = Point_3<float>::Origin();
			vertices[5].pos = Point_3<float>::Origin();
			vertices[6].pos = Point_3<float>(0, -r, 0);
		}
		for(int i = 0; i < _verticesPerArrow; i++, ++vertices) {
			vertices->base = base;
			vertices->dir = t;
			vertices->color = c;
		}
	}
}

/******************************************************************************
* Finalizes the geometry buffer after all arrows have been set.
******************************************************************************/
void ViewportArrowGeometryBuffer::endSetArrows()
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_arrowCount >= 0);
	OVITO_ASSERT(_mappedVerticesShaded != nullptr || _mappedVerticesFlat != nullptr || _arrowCount == 0);

	if(_arrowCount)
		_glGeometryBuffer.unmap();
	_glGeometryBuffer.release();
	_mappedVerticesShaded = nullptr;
	_mappedVerticesFlat = nullptr;
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

	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(_arrowCount <= 0 || !vpRenderer || vpRenderer->isPicking())
		return;

	if(shadingMode() == NormalShading)
		renderShaded(vpRenderer, pickingBaseID);
	else if(shadingMode() == FlatShading)
		renderFlat(vpRenderer, pickingBaseID);
}

/******************************************************************************
* Renders the arrows in shaded mode.
******************************************************************************/
void ViewportArrowGeometryBuffer::renderShaded(ViewportSceneRenderer* renderer, quint32 pickingBaseID)
{
	OVITO_ASSERT(_mappedVerticesShaded == nullptr);
	glEnable(GL_CULL_FACE);

	if(!_shadedShader->bind())
		throw Exception(tr("Failed to bind OpenGL shader."));

	_shadedShader->setUniformValue("modelview_projection_matrix",
			(QMatrix4x4)(renderer->projParams().projectionMatrix * renderer->modelViewTM()));

	_shadedShader->setUniformValue("normal_matrix", (QMatrix3x3)(renderer->modelViewTM().linear().inverse().transposed()));

	_glGeometryBuffer.bind();
	_shadedShader->setAttributeBuffer("vertex_pos", GL_FLOAT, offsetof(ColoredVertexWithNormal, pos), 3, sizeof(ColoredVertexWithNormal));
	_shadedShader->enableAttributeArray("vertex_pos");
	_shadedShader->setAttributeBuffer("vertex_normal", GL_FLOAT, offsetof(ColoredVertexWithNormal, normal), 3, sizeof(ColoredVertexWithNormal));
	_shadedShader->enableAttributeArray("vertex_normal");
	_shadedShader->setAttributeBuffer("vertex_color", GL_FLOAT, offsetof(ColoredVertexWithNormal, color), 4, sizeof(ColoredVertexWithNormal));
	_shadedShader->enableAttributeArray("vertex_color");
	_glGeometryBuffer.release();

	if(renderer->glfuncs30()) {
		OVITO_CHECK_OPENGL(renderer->glfuncs30()->glMultiDrawArrays(GL_TRIANGLE_STRIP, _stripPrimitiveVertexStarts.data(), _stripPrimitiveVertexCounts.data(), _stripPrimitiveVertexStarts.size()));
		OVITO_CHECK_OPENGL(renderer->glfuncs30()->glMultiDrawArrays(GL_TRIANGLE_FAN, _fanPrimitiveVertexStarts.data(), _fanPrimitiveVertexCounts.data(), _fanPrimitiveVertexStarts.size()));
	}
	else if(renderer->glfuncs32()) {
		OVITO_CHECK_OPENGL(renderer->glfuncs32()->glMultiDrawArrays(GL_TRIANGLE_STRIP, _stripPrimitiveVertexStarts.data(), _stripPrimitiveVertexCounts.data(), _stripPrimitiveVertexStarts.size()));
		OVITO_CHECK_OPENGL(renderer->glfuncs32()->glMultiDrawArrays(GL_TRIANGLE_FAN, _fanPrimitiveVertexStarts.data(), _fanPrimitiveVertexCounts.data(), _fanPrimitiveVertexStarts.size()));
	}
	else {
		OVITO_ASSERT_MSG(false, "ViewportArrowGeometryBuffer::renderShaded", "glMultiDrawArrays() is not supported.");
	}

	_shadedShader->release();
}

/******************************************************************************
* Renders the arrows in flat mode.
******************************************************************************/
void ViewportArrowGeometryBuffer::renderFlat(ViewportSceneRenderer* renderer, quint32 pickingBaseID)
{
	OVITO_ASSERT(_mappedVerticesFlat == nullptr);
	if(!_flatShader->bind())
		throw Exception(tr("Failed to bind OpenGL shader."));

	_flatShader->setUniformValue("modelview_projection_matrix",
			(QMatrix4x4)(renderer->projParams().projectionMatrix * renderer->modelViewTM()));

	Vector3 viewDir = renderer->modelViewTM().inverse() * Vector3(0,0,1);
	_flatShader->setUniformValue("view_dir", viewDir.x(), viewDir.y(), viewDir.z());

	_glGeometryBuffer.bind();
	_flatShader->setAttributeBuffer("vertex_pos", GL_FLOAT, offsetof(ColoredVertexWithVector, pos), 3, sizeof(ColoredVertexWithVector));
	_flatShader->enableAttributeArray("vertex_pos");
	_flatShader->setAttributeBuffer("vector_base", GL_FLOAT, offsetof(ColoredVertexWithVector, base), 3, sizeof(ColoredVertexWithVector));
	_flatShader->enableAttributeArray("vector_base");
	_flatShader->setAttributeBuffer("vector_dir", GL_FLOAT, offsetof(ColoredVertexWithVector, dir), 3, sizeof(ColoredVertexWithVector));
	_flatShader->enableAttributeArray("vector_dir");
	_flatShader->setAttributeBuffer("vertex_color", GL_FLOAT, offsetof(ColoredVertexWithVector, color), 4, sizeof(ColoredVertexWithVector));
	_flatShader->enableAttributeArray("vertex_color");
	_glGeometryBuffer.release();

	if(renderer->glfuncs30()) {
		OVITO_CHECK_OPENGL(renderer->glfuncs30()->glMultiDrawArrays(GL_TRIANGLE_FAN, _fanPrimitiveVertexStarts.data(), _fanPrimitiveVertexCounts.data(), _fanPrimitiveVertexStarts.size()));
	}
	else if(renderer->glfuncs32()) {
		OVITO_CHECK_OPENGL(renderer->glfuncs32()->glMultiDrawArrays(GL_TRIANGLE_FAN, _fanPrimitiveVertexStarts.data(), _fanPrimitiveVertexCounts.data(), _fanPrimitiveVertexStarts.size()));
	}
	else {
		OVITO_ASSERT_MSG(false, "ViewportArrowGeometryBuffer::renderFlat", "glMultiDrawArrays() is not supported.");
	}

	_flatShader->release();
}

};
