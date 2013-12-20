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
#include "ViewportLineGeometryBuffer.h"
#include "ViewportSceneRenderer.h"

namespace Ovito {

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportLineGeometryBuffer::ViewportLineGeometryBuffer(ViewportSceneRenderer* renderer) :
	_contextGroup(QOpenGLContextGroup::currentContextGroup())
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	// Initialize OpenGL shaders.
	_shader = renderer->loadShaderProgram("line", ":/core/glsl/lines/line.vs", ":/core/glsl/lines/line.fs");
	_pickingShader = renderer->loadShaderProgram("line.picking", ":/core/glsl/lines/picking/line.vs", ":/core/glsl/lines/picking/line.fs");
	_thickLineShader = renderer->loadShaderProgram("thick_line", ":/core/glsl/lines/thick_line.vs", ":/core/glsl/lines/line.fs");
}

/******************************************************************************
* Allocates a vertex buffer with the given number of vertices.
******************************************************************************/
void ViewportLineGeometryBuffer::setVertexCount(int vertexCount, FloatType lineWidth)
{
	OVITO_ASSERT(vertexCount >= 0);
	OVITO_ASSERT(vertexCount < std::numeric_limits<int>::max() / sizeof(ColorA));
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(lineWidth >= 0);

	_lineWidth = lineWidth;

	if(lineWidth == 1) {
		_positionsBuffer.create(QOpenGLBuffer::StaticDraw, vertexCount);
		_colorsBuffer.create(QOpenGLBuffer::StaticDraw, vertexCount);
	}
	else {
		OVITO_STATIC_ASSERT(sizeof(Vector3) == sizeof(GLfloat)*3);
		_positionsBuffer.create(QOpenGLBuffer::StaticDraw, vertexCount, 2);
		_colorsBuffer.create(QOpenGLBuffer::StaticDraw, vertexCount, 2);
		_vectorsBuffer.create(QOpenGLBuffer::StaticDraw, vertexCount, 2);
	}
}

/******************************************************************************
* Sets the coordinates of the vertices.
******************************************************************************/
void ViewportLineGeometryBuffer::setVertexPositions(const Point3* coordinates)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	_positionsBuffer.fill(coordinates);

	if(_lineWidth != 1) {
		Vector3* vectors = _vectorsBuffer.map(QOpenGLBuffer::WriteOnly);
		Vector3* vectors_end = vectors + _vectorsBuffer.elementCount() * _vectorsBuffer.verticesPerElement();
		for(; vectors != vectors_end; vectors += 4, coordinates += 2) {
			vectors[3] = vectors[0] = coordinates[1] - coordinates[0];
			vectors[1] = vectors[2] = -vectors[0];
		}
		_vectorsBuffer.unmap();
	}
}

/******************************************************************************
* Sets the colors of the vertices.
******************************************************************************/
void ViewportLineGeometryBuffer::setVertexColors(const ColorA* colors)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	_colorsBuffer.fill(colors);
}

/******************************************************************************
* Sets the color of all vertices to the given value.
******************************************************************************/
void ViewportLineGeometryBuffer::setVertexColor(const ColorA color)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	_colorsBuffer.fillConstant(color);
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportLineGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = qobject_cast<ViewportSceneRenderer*>(renderer);
	if(!vpRenderer) return false;
	return _positionsBuffer.isCreated() && (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void ViewportLineGeometryBuffer::render(SceneRenderer* renderer)
{
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_STATIC_ASSERT(sizeof(FloatType) == 4);
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(vertexCount() <= 0 || !vpRenderer)
		return;

	QOpenGLShaderProgram* shader;
	if(_lineWidth == 1) {
		if(!renderer->isPicking())
			shader = _shader;
		else
			shader = _pickingShader;
	}
	else {
		if(!renderer->isPicking())
			shader = _thickLineShader;
		else
			return;
	}

	if(!shader->bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL shader."));

	OVITO_CHECK_OPENGL(shader->setUniformValue("modelview_projection_matrix",
			(QMatrix4x4)(vpRenderer->projParams().projectionMatrix * vpRenderer->modelViewTM())));

	_positionsBuffer.bindPositions(vpRenderer, shader);
	if(!renderer->isPicking()) {
		_colorsBuffer.bindColors(vpRenderer, shader, 4);
	}
	else {
		shader->setUniformValue("pickingBaseID", (GLint)vpRenderer->registerSubObjectIDs(vertexCount() / 2));
		vpRenderer->activateVertexIDs(shader, _positionsBuffer.elementCount() * _positionsBuffer.verticesPerElement());
	}

	if(_lineWidth != 1) {
		shader->setUniformValue("is_perspective", renderer->projParams().isPerspective);
		shader->setUniformValue("line_width", _lineWidth);
		AffineTransformation viewModelTM = vpRenderer->modelViewTM().inverse();
		Vector3 eye_pos = viewModelTM.translation();
		shader->setUniformValue("eye_pos", eye_pos.x(), eye_pos.y(), eye_pos.z());
		Vector3 viewDir = viewModelTM * Vector3(0,0,1);
		shader->setUniformValue("parallel_view_dir", viewDir.x(), viewDir.y(), viewDir.z());
		_vectorsBuffer.bind(vpRenderer, shader, "vector", GL_FLOAT, 0, 3);
		OVITO_CHECK_OPENGL(glDrawArrays(GL_QUADS, 0, _positionsBuffer.elementCount() * _positionsBuffer.verticesPerElement()));
	}
	else {
		OVITO_CHECK_OPENGL(glDrawArrays(GL_LINES, 0, _positionsBuffer.elementCount() * _positionsBuffer.verticesPerElement()));
	}

	_positionsBuffer.detachPositions(vpRenderer, shader);
	if(!renderer->isPicking()) {
		_colorsBuffer.detachColors(vpRenderer, shader);
	}
	else {
		vpRenderer->deactivateVertexIDs(shader);
	}
	if(_lineWidth != 1) {
		_vectorsBuffer.detach(vpRenderer, shader, "vector");
	}
	shader->release();

	OVITO_CHECK_OPENGL();
}

};
