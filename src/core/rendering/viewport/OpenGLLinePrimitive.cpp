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
#include "OpenGLLinePrimitive.h"
#include "ViewportSceneRenderer.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Constructor.
******************************************************************************/
OpenGLLinePrimitive::OpenGLLinePrimitive(ViewportSceneRenderer* renderer) :
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_indicesBuffer(QOpenGLBuffer::IndexBuffer)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	// Initialize OpenGL shaders.
	_shader = renderer->loadShaderProgram("line", ":/core/glsl/lines/line.vs", ":/core/glsl/lines/line.fs");
	_pickingShader = renderer->loadShaderProgram("line.picking", ":/core/glsl/lines/picking/line.vs", ":/core/glsl/lines/picking/line.fs");
	_thickLineShader = renderer->loadShaderProgram("thick_line", ":/core/glsl/lines/thick_line.vs", ":/core/glsl/lines/line.fs");
	_thickLinePickingShader = renderer->loadShaderProgram("thick_line.picking", ":/core/glsl/lines/picking/thick_line.vs", ":/core/glsl/lines/picking/line.fs");
	
	// Use VBO to store glDrawElements() indices only on a real core profile implementation.
	_useIndexVBO = (renderer->glformat().profile() == QSurfaceFormat::CoreProfile);
}

/******************************************************************************
* Allocates a vertex buffer with the given number of vertices.
******************************************************************************/
void OpenGLLinePrimitive::setVertexCount(int vertexCount, FloatType lineWidth)
{
	OVITO_ASSERT(vertexCount >= 0);
	OVITO_ASSERT((vertexCount & 1) == 0);
	OVITO_ASSERT(vertexCount < std::numeric_limits<int>::max() / sizeof(ColorA));
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(lineWidth >= 0);

	_lineWidth = lineWidth;

	if(lineWidth == 1) {
		_positionsBuffer.create(QOpenGLBuffer::StaticDraw, vertexCount);
		_colorsBuffer.create(QOpenGLBuffer::StaticDraw, vertexCount);
	}
	else {
		_positionsBuffer.create(QOpenGLBuffer::StaticDraw, vertexCount, 2);
		_colorsBuffer.create(QOpenGLBuffer::StaticDraw, vertexCount, 2);
		_vectorsBuffer.create(QOpenGLBuffer::StaticDraw, vertexCount, 2);
		GLuint* indices;
		if(_useIndexVBO) {
			_indicesBuffer.create(QOpenGLBuffer::StaticDraw, vertexCount * 6 / 2);
			indices = _indicesBuffer.map(QOpenGLBuffer::WriteOnly);
		}
		else {
			_indicesBufferClient.resize(vertexCount * 6 / 2);
			indices = _indicesBufferClient.data();
		}
		for(int i = 0; i < vertexCount; i += 2, indices += 6) {
			indices[0] = i * 2;
			indices[1] = i * 2 + 1;
			indices[2] = i * 2 + 2;
			indices[3] = i * 2;
			indices[4] = i * 2 + 2;
			indices[5] = i * 2 + 3;
		}
		if(_useIndexVBO) _indicesBuffer.unmap();
	}
}

/******************************************************************************
* Sets the coordinates of the vertices.
******************************************************************************/
void OpenGLLinePrimitive::setVertexPositions(const Point3* coordinates)
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
void OpenGLLinePrimitive::setVertexColors(const ColorA* colors)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	_colorsBuffer.fill(colors);
}

/******************************************************************************
* Sets the color of all vertices to the given value.
******************************************************************************/
void OpenGLLinePrimitive::setLineColor(const ColorA color)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	_colorsBuffer.fillConstant(color);
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool OpenGLLinePrimitive::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = qobject_cast<ViewportSceneRenderer*>(renderer);
	if(!vpRenderer) return false;
	return _positionsBuffer.isCreated() && (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void OpenGLLinePrimitive::render(SceneRenderer* renderer)
{
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_STATIC_ASSERT(sizeof(FloatType) == 4);
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(vertexCount() <= 0 || !vpRenderer)
		return;

	vpRenderer->rebindVAO();

	if(_lineWidth == 1)
		renderLines(vpRenderer);
	else
		renderThickLines(vpRenderer);
}

/******************************************************************************
* Renders the lines using GL_LINES mode.
******************************************************************************/
void OpenGLLinePrimitive::renderLines(ViewportSceneRenderer* renderer)
{
	QOpenGLShaderProgram* shader;
	if(!renderer->isPicking())
		shader = _shader;
	else
		shader = _pickingShader;

	if(!shader->bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL shader."));

	OVITO_CHECK_OPENGL(shader->setUniformValue("modelview_projection_matrix",
			(QMatrix4x4)(renderer->projParams().projectionMatrix * renderer->modelViewTM())));

	_positionsBuffer.bindPositions(renderer, shader);
	if(!renderer->isPicking()) {
		_colorsBuffer.bindColors(renderer, shader, 4);
	}
	else {
		shader->setUniformValue("pickingBaseID", (GLint)renderer->registerSubObjectIDs(vertexCount() / 2));
		renderer->activateVertexIDs(shader, _positionsBuffer.elementCount() * _positionsBuffer.verticesPerElement());
	}

	OVITO_CHECK_OPENGL(glDrawArrays(GL_LINES, 0, _positionsBuffer.elementCount() * _positionsBuffer.verticesPerElement()));

	_positionsBuffer.detachPositions(renderer, shader);
	if(!renderer->isPicking()) {
		_colorsBuffer.detachColors(renderer, shader);
	}
	else {
		renderer->deactivateVertexIDs(shader);
	}
	shader->release();

	OVITO_REPORT_OPENGL_ERRORS();
}

/******************************************************************************
* Renders the lines using polygons.
******************************************************************************/
void OpenGLLinePrimitive::renderThickLines(ViewportSceneRenderer* renderer)
{
	QOpenGLShaderProgram* shader;
	if(!renderer->isPicking())
		shader = _thickLineShader;
	else
		shader = _thickLinePickingShader;

	if(!shader->bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL shader."));

	OVITO_CHECK_OPENGL(shader->setUniformValue("modelview_matrix", (QMatrix4x4)renderer->modelViewTM()));
	OVITO_CHECK_OPENGL(shader->setUniformValue("projection_matrix", (QMatrix4x4)renderer->projParams().projectionMatrix));

	_positionsBuffer.bindPositions(renderer, shader);
	if(!renderer->isPicking()) {
		_colorsBuffer.bindColors(renderer, shader, 4);
	}
	else {
		shader->setUniformValue("pickingBaseID", (GLint)renderer->registerSubObjectIDs(vertexCount() / 2));
		renderer->activateVertexIDs(shader, _positionsBuffer.elementCount() * _positionsBuffer.verticesPerElement());
	}

	GLint viewportCoords[4];
	glGetIntegerv(GL_VIEWPORT, viewportCoords);
	FloatType param = renderer->projParams().projectionMatrix(1,1) * viewportCoords[3];
	shader->setUniformValue("line_width", 0.5f * _lineWidth / param);
	shader->setUniformValue("is_perspective", renderer->projParams().isPerspective);
	_vectorsBuffer.bind(renderer, shader, "vector", GL_FLOAT, 0, 3, sizeof(Vector3));

	if(_useIndexVBO) {
		_indicesBuffer.oglBuffer().bind();
		OVITO_CHECK_OPENGL(glDrawElements(GL_TRIANGLES, _indicesBuffer.elementCount(), GL_UNSIGNED_INT, nullptr));
		_indicesBuffer.oglBuffer().release();
	}
	else {
		OVITO_CHECK_OPENGL(glDrawElements(GL_TRIANGLES, _indicesBufferClient.size(), GL_UNSIGNED_INT, _indicesBufferClient.data()));
	}

	_positionsBuffer.detachPositions(renderer, shader);
	if(!renderer->isPicking()) {
		_colorsBuffer.detachColors(renderer, shader);
	}
	else {
		renderer->deactivateVertexIDs(shader);
	}
	_vectorsBuffer.detach(renderer, shader, "vector");
	shader->release();

	OVITO_REPORT_OPENGL_ERRORS();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
