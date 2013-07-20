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
ViewportArrowGeometryBuffer::ViewportArrowGeometryBuffer(ViewportSceneRenderer* renderer, ArrowGeometryBuffer::Shape shape, ShadingMode shadingMode, RenderingQuality renderingQuality) :
	ArrowGeometryBuffer(shape, shadingMode, renderingQuality),
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_elementCount(-1), _cylinderSegments(16), _verticesPerElement(0),
	_mappedBuffer(nullptr)
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

	_raytracedCylinderShader = renderer->loadShaderProgram(
			"cylinder_raytraced",
			":/core/glsl/cylinder/cylinder_raytraced.vs",
			":/core/glsl/cylinder/cylinder_raytraced.fs");
}

/******************************************************************************
* Allocates a particle buffer with the given number of elements.
******************************************************************************/
void ViewportArrowGeometryBuffer::startSetElements(int elementCount)
{
	OVITO_ASSERT(_glGeometryBuffer.isCreated());
	OVITO_ASSERT(elementCount >= 0);
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_mappedBuffer == nullptr);

	if(!_glGeometryBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));

	_elementCount = elementCount;
	size_t bytesPerVertex = 0;
	if(shadingMode() == NormalShading) {
		int cylinderVertexCount = _cylinderSegments * 2 + 2;
		int discVertexCount = _cylinderSegments;
		int cylinderCount, discCount;
		bytesPerVertex = sizeof(ColoredVertexWithNormal);
		if(shape() == ArrowShape) {
			cylinderCount = 2;
			discCount = 2;
		}
		else {
			cylinderCount = 1;
			discCount = 2;
			if(renderingQuality() == HighQuality) {
				cylinderVertexCount = 14;
				discCount = discVertexCount = 0;
				bytesPerVertex = sizeof(ColoredVertexWithElementInfo);
			}
		}
		_verticesPerElement = cylinderCount * cylinderVertexCount + discCount * discVertexCount;

		// Prepare arrays to be passed to the glMultiDrawArrays() function.
		_stripPrimitiveVertexStarts.resize(_elementCount * cylinderCount);
		_stripPrimitiveVertexCounts.resize(_elementCount * cylinderCount);
		_fanPrimitiveVertexStarts.resize(_elementCount * discCount);
		_fanPrimitiveVertexCounts.resize(_elementCount * discCount);
		std::fill(_stripPrimitiveVertexCounts.begin(), _stripPrimitiveVertexCounts.end(), cylinderVertexCount);
		std::fill(_fanPrimitiveVertexCounts.begin(), _fanPrimitiveVertexCounts.end(), discVertexCount);
		auto ps_strip = _stripPrimitiveVertexStarts.begin();
		auto ps_fan = _fanPrimitiveVertexStarts.begin();
		GLint baseIndex = 0;
		for(GLint index = 0; index < _elementCount; index++) {
			for(int p = 0; p < cylinderCount; p++, baseIndex += cylinderVertexCount)
				*ps_strip++ = baseIndex;
			for(int p = 0; p < discCount; p++, baseIndex += discVertexCount)
				*ps_fan++ = baseIndex;
		}
	}
	else if(shadingMode() == FlatShading) {
		_verticesPerElement = 7;
		bytesPerVertex = sizeof(ColoredVertexWithVector);

		// Prepare arrays to be passed to the glMultiDrawArrays() function.
		_fanPrimitiveVertexStarts.resize(_elementCount);
		GLint startIndex = 0;
		for(auto& i : _fanPrimitiveVertexStarts) {
			i = startIndex;
			startIndex += _verticesPerElement;
		}
		_fanPrimitiveVertexCounts.resize(_elementCount);
		std::fill(_fanPrimitiveVertexCounts.begin(), _fanPrimitiveVertexCounts.end(), _verticesPerElement);
		_stripPrimitiveVertexStarts.clear();
		_stripPrimitiveVertexCounts.clear();
	}
	else OVITO_ASSERT(false);

	// Allocate vertex buffer memory.
	OVITO_ASSERT(_elementCount < std::numeric_limits<int>::max() / bytesPerVertex / _verticesPerElement);
	_glGeometryBuffer.allocate(_elementCount * _verticesPerElement * bytesPerVertex);
	if(_elementCount) {
		_mappedBuffer = _glGeometryBuffer.map(QOpenGLBuffer::WriteOnly);
		if(!_mappedBuffer)
			throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));
	}

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
* Sets the properties of a single element.
******************************************************************************/
void ViewportArrowGeometryBuffer::setElement(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width)
{
	OVITO_ASSERT(index >= 0 && index < _elementCount);
	OVITO_ASSERT(_mappedBuffer != nullptr);

	if(shape() == ArrowShape)
		createArrowElement(index, pos, dir, color, width);
	else
		createCylinderElement(index, pos, dir, color, width);
}

/******************************************************************************
* Creates the geometry for a single cylinder element.
******************************************************************************/
void ViewportArrowGeometryBuffer::createCylinderElement(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width)
{
	if(shadingMode() == NormalShading) {

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
		Point_3<float> v2 = v1 + dir;

		if(renderingQuality() != HighQuality) {
			ColoredVertexWithNormal* vertex = static_cast<ColoredVertexWithNormal*>(_mappedBuffer) + (index * _verticesPerElement);

			// Generate vertices for cylinder mantle.
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

			// Generate vertices for first cylinder cap.
			for(int i = 0; i < _cylinderSegments; i++) {
				Vector_3<float> n = _cosTable[i] * u + _sinTable[i] * v;
				Vector_3<float> d = n * width;
				vertex->pos = v1 + d;
				vertex->normal = Vector_3<float>(0,0,-1);
				vertex->color = c;
				vertex++;
			}

			// Generate vertices for second cylinder cap.
			for(int i = _cylinderSegments - 1; i >= 0; i--) {
				Vector_3<float> n = _cosTable[i] * u + _sinTable[i] * v;
				Vector_3<float> d = n * width;
				vertex->pos = v2 + d;
				vertex->normal = Vector_3<float>(0,0,1);
				vertex->color = c;
				vertex++;
			}
		}
		else {
			// Create bounding box geometry around cylinder for raytracing.
			ColoredVertexWithElementInfo* vertex = static_cast<ColoredVertexWithElementInfo*>(_mappedBuffer) + (index * _verticesPerElement);
			u *= width;
			v *= width;
			Point3 corners[8] = {
					v1 - u - v,
					v1 - u + v,
					v1 + u - v,
					v1 + u + v,
					v2 - u - v,
					v2 - u + v,
					v2 + u + v,
					v2 + u - v
			};
			const static size_t stripIndices[14] = { 3,2,6,7,4,2,0,3,1,6,5,4,1,0 };
			for(int i = 0; i < 14; i++, vertex++) {
				vertex->pos = corners[stripIndices[i]];
				vertex->base = v1;
				vertex->dir = dir;
				vertex->color = c;
				vertex->radius = width;
			}
		}
	}
	else if(shadingMode() == FlatShading) {

		Vector_3<float> t;
		float length = dir.length();
		if(length != 0)
			t = dir / length;
		else
			t.setZero();

		ColorAT<float> c = color;
		Point_3<float> base = pos;

		ColoredVertexWithVector* vertices = static_cast<ColoredVertexWithVector*>(_mappedBuffer) + (index * _verticesPerElement);
		OVITO_ASSERT_MSG(false, "ViewportArrowGeometryBuffer::createCylinderElement", "Function not implemented yet.");
	}
}

/******************************************************************************
* Creates the geometry for a single arrow element.
******************************************************************************/
void ViewportArrowGeometryBuffer::createArrowElement(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width)
{
	float arrowHeadRadius = width * 2.5;
	float arrowHeadLength = arrowHeadRadius * 1.8f;

	if(shadingMode() == NormalShading) {

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

		ColoredVertexWithNormal* vertex = static_cast<ColoredVertexWithNormal*>(_mappedBuffer) + (index * _verticesPerElement);

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

		Vector_3<float> t;
		float length = dir.length();
		if(length != 0)
			t = dir / length;
		else
			t.setZero();

		ColorAT<float> c = color;
		Point_3<float> base = pos;

		ColoredVertexWithVector* vertices = static_cast<ColoredVertexWithVector*>(_mappedBuffer) + (index * _verticesPerElement);
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
		for(int i = 0; i < _verticesPerElement; i++, ++vertices) {
			vertices->base = base;
			vertices->dir = t;
			vertices->color = c;
		}
	}
}


/******************************************************************************
* Finalizes the geometry buffer after all elements have been set.
******************************************************************************/
void ViewportArrowGeometryBuffer::endSetElements()
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_elementCount >= 0);
	OVITO_ASSERT(_mappedBuffer != nullptr || _elementCount == 0);

	if(_elementCount)
		_glGeometryBuffer.unmap();
	_glGeometryBuffer.release();
	_mappedBuffer = nullptr;
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportArrowGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);
	if(!vpRenderer) return false;
	return _glGeometryBuffer.isCreated()
			&& _elementCount >= 0
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
	OVITO_ASSERT(_elementCount >= 0);
	OVITO_ASSERT(_mappedBuffer == nullptr);

	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(_elementCount <= 0 || !vpRenderer || vpRenderer->isPicking())
		return;

	if(shadingMode() == NormalShading) {
		if(renderingQuality() == HighQuality && shape() == CylinderShape)
			renderRaytracedCylinders(vpRenderer, pickingBaseID);
		else
			renderShadedTriangles(vpRenderer, pickingBaseID);
	}
	else if(shadingMode() == FlatShading) {
		renderFlat(vpRenderer, pickingBaseID);
	}
}

/******************************************************************************
* Renders the arrows in shaded mode.
******************************************************************************/
void ViewportArrowGeometryBuffer::renderShadedTriangles(ViewportSceneRenderer* renderer, quint32 pickingBaseID)
{
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
		OVITO_ASSERT_MSG(false, "ViewportArrowGeometryBuffer::renderShadedTriangles", "glMultiDrawArrays() is not supported.");
	}

	_shadedShader->release();
}

/******************************************************************************
* Renders the cylinder elements in using a raytracing hardware shader.
******************************************************************************/
void ViewportArrowGeometryBuffer::renderRaytracedCylinders(ViewportSceneRenderer* renderer, quint32 pickingBaseID)
{
	glEnable(GL_CULL_FACE);

	if(!_raytracedCylinderShader->bind())
		throw Exception(tr("Failed to bind OpenGL shader."));

	_raytracedCylinderShader->setUniformValue("modelview_matrix",
			(QMatrix4x4)renderer->modelViewTM());
	_raytracedCylinderShader->setUniformValue("modelview_projection_matrix",
			(QMatrix4x4)(renderer->projParams().projectionMatrix * renderer->modelViewTM()));
	_raytracedCylinderShader->setUniformValue("projection_matrix", (QMatrix4x4)renderer->projParams().projectionMatrix);
	_raytracedCylinderShader->setUniformValue("inverse_projection_matrix", (QMatrix4x4)renderer->projParams().inverseProjectionMatrix);
	_raytracedCylinderShader->setUniformValue("is_perspective", renderer->projParams().isPerspective);

	GLint viewportCoords[4];
	glGetIntegerv(GL_VIEWPORT, viewportCoords);
	_raytracedCylinderShader->setUniformValue("viewport_origin", (float)viewportCoords[0], (float)viewportCoords[1]);
	_raytracedCylinderShader->setUniformValue("inverse_viewport_size", 2.0f / (float)viewportCoords[2], 2.0f / (float)viewportCoords[3]);

	_glGeometryBuffer.bind();
	_raytracedCylinderShader->setAttributeBuffer("vertex_pos", GL_FLOAT, offsetof(ColoredVertexWithElementInfo, pos), 3, sizeof(ColoredVertexWithElementInfo));
	_raytracedCylinderShader->enableAttributeArray("vertex_pos");
	_raytracedCylinderShader->setAttributeBuffer("cylinder_color", GL_FLOAT, offsetof(ColoredVertexWithElementInfo, color), 4, sizeof(ColoredVertexWithElementInfo));
	_raytracedCylinderShader->enableAttributeArray("cylinder_color");
	_raytracedCylinderShader->setAttributeBuffer("cylinder_base", GL_FLOAT, offsetof(ColoredVertexWithElementInfo, base), 3, sizeof(ColoredVertexWithElementInfo));
	_raytracedCylinderShader->enableAttributeArray("cylinder_base");
	_raytracedCylinderShader->setAttributeBuffer("cylinder_axis", GL_FLOAT, offsetof(ColoredVertexWithElementInfo, dir), 3, sizeof(ColoredVertexWithElementInfo));
	_raytracedCylinderShader->enableAttributeArray("cylinder_axis");
	_raytracedCylinderShader->setAttributeBuffer("cylinder_radius", GL_FLOAT, offsetof(ColoredVertexWithElementInfo, radius), 1, sizeof(ColoredVertexWithElementInfo));
	_raytracedCylinderShader->enableAttributeArray("cylinder_radius");
	_glGeometryBuffer.release();

	if(renderer->glfuncs30()) {
		OVITO_CHECK_OPENGL(renderer->glfuncs30()->glMultiDrawArrays(GL_TRIANGLE_STRIP, _stripPrimitiveVertexStarts.data(), _stripPrimitiveVertexCounts.data(), _stripPrimitiveVertexStarts.size()));
	}
	else if(renderer->glfuncs32()) {
		OVITO_CHECK_OPENGL(renderer->glfuncs32()->glMultiDrawArrays(GL_TRIANGLE_STRIP, _stripPrimitiveVertexStarts.data(), _stripPrimitiveVertexCounts.data(), _stripPrimitiveVertexStarts.size()));
	}
	else {
		OVITO_ASSERT_MSG(false, "ViewportArrowGeometryBuffer::renderRaytracedCylinders", "glMultiDrawArrays() is not supported.");
	}

	_raytracedCylinderShader->release();
}

/******************************************************************************
* Renders the arrows in flat mode.
******************************************************************************/
void ViewportArrowGeometryBuffer::renderFlat(ViewportSceneRenderer* renderer, quint32 pickingBaseID)
{
	if(!_flatShader->bind())
		throw Exception(tr("Failed to bind OpenGL shader."));

	_flatShader->setUniformValue("modelview_projection_matrix",
			(QMatrix4x4)(renderer->projParams().projectionMatrix * renderer->modelViewTM()));
	_flatShader->setUniformValue("is_perspective", renderer->projParams().isPerspective);
	AffineTransformation viewModelTM = renderer->modelViewTM().inverse();
	Vector3 eye_pos = viewModelTM.translation();
	_flatShader->setUniformValue("eye_pos", eye_pos.x(), eye_pos.y(), eye_pos.z());
	Vector3 viewDir = viewModelTM * Vector3(0,0,1);
	_flatShader->setUniformValue("parallel_view_dir", viewDir.x(), viewDir.y(), viewDir.z());

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
