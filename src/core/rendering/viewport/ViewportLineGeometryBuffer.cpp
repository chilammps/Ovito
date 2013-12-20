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
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_vertexCount(-1)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	if(!_glPositionsBuffer.create())
		throw Exception(QStringLiteral("Failed to create OpenGL vertex buffer."));
	_glPositionsBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glColorsBuffer.create())
		throw Exception(QStringLiteral("Failed to create OpenGL vertex buffer."));
	_glColorsBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// Initialize OpenGL shaders.
	_shader = renderer->loadShaderProgram("line", ":/core/glsl/lines/line.vs", ":/core/glsl/lines/line.fs");
	_pickingShader = renderer->loadShaderProgram("line.picking", ":/core/glsl/lines/picking/line.vs", ":/core/glsl/lines/picking/line.fs");
}

/******************************************************************************
* Allocates a vertex buffer with the given number of vertices.
******************************************************************************/
void ViewportLineGeometryBuffer::setSize(int vertexCount)
{
	OVITO_ASSERT(_glPositionsBuffer.isCreated());
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(vertexCount >= 0);
	OVITO_ASSERT(vertexCount < std::numeric_limits<int>::max() / sizeof(ColorA));
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);

	_vertexCount = vertexCount;
}

/******************************************************************************
* Sets the coordinates of the vertices.
******************************************************************************/
void ViewportLineGeometryBuffer::setVertexPositions(const Point3* coordinates)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glPositionsBuffer.isCreated());
	OVITO_ASSERT(_vertexCount >= 0);

	if(!_glPositionsBuffer.bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL vertex buffer."));
	OVITO_CHECK_OPENGL(_glPositionsBuffer.allocate(coordinates, _vertexCount * sizeof(Point3)));
	_glPositionsBuffer.release();
}

/******************************************************************************
* Sets the colors of the vertices.
******************************************************************************/
void ViewportLineGeometryBuffer::setVertexColors(const ColorA* colors)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(_vertexCount >= 0);

	if(!_glColorsBuffer.bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL vertex buffer."));
	OVITO_CHECK_OPENGL(_glColorsBuffer.allocate(colors, _vertexCount * sizeof(ColorA)));
	_glColorsBuffer.release();
}

/******************************************************************************
* Sets the color of all vertices to the given value.
******************************************************************************/
void ViewportLineGeometryBuffer::setVertexColor(const ColorA color)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(_vertexCount >= 0);

	if(!_glColorsBuffer.bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL vertex buffer."));
	OVITO_CHECK_OPENGL(_glColorsBuffer.allocate(_vertexCount * sizeof(ColorA)));
	if(_vertexCount) {
		ColorA* bufferData = static_cast<ColorA*>(_glColorsBuffer.map(QOpenGLBuffer::WriteOnly));
		if(!bufferData)
			throw Exception(QStringLiteral("Failed to map OpenGL vertex buffer to memory."));
		std::fill(bufferData, bufferData + _vertexCount, color);
		_glColorsBuffer.unmap();
	}
	_glColorsBuffer.release();
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportLineGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = qobject_cast<ViewportSceneRenderer*>(renderer);
	if(!vpRenderer) return false;
	return _glPositionsBuffer.isCreated()
			&& _vertexCount >= 0
			&& (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void ViewportLineGeometryBuffer::render(SceneRenderer* renderer)
{
	OVITO_ASSERT(_glPositionsBuffer.isCreated());
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_ASSERT(_vertexCount >= 0);
	OVITO_STATIC_ASSERT(sizeof(FloatType) == 4);
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(_vertexCount <= 0 || !vpRenderer)
		return;

	QOpenGLShaderProgram* shader;
	if(!renderer->isPicking())
		shader = _shader;
	else
		shader = _pickingShader;

	if(!shader->bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL shader."));

	OVITO_CHECK_OPENGL(shader->setUniformValue("modelview_projection_matrix",
			(QMatrix4x4)(vpRenderer->projParams().projectionMatrix * vpRenderer->modelViewTM())));

	if(renderer->isPicking())
		shader->setUniformValue("pickingBaseID", (GLint)vpRenderer->registerSubObjectIDs(vertexCount() / 2));

	OVITO_CHECK_OPENGL(_glPositionsBuffer.bind());
	if(vpRenderer->glformat().majorVersion() >= 3) {
		OVITO_CHECK_OPENGL(shader->enableAttributeArray("vertex_pos"));
		OVITO_CHECK_OPENGL(shader->setAttributeBuffer("vertex_pos", GL_FLOAT, 0, 3));
	}
	else {
		OVITO_CHECK_OPENGL(glEnableClientState(GL_VERTEX_ARRAY));
		OVITO_CHECK_OPENGL(glVertexPointer(3, GL_FLOAT, 0, 0));
	}
	_glPositionsBuffer.release();

	if(!renderer->isPicking()) {
		OVITO_CHECK_OPENGL(_glColorsBuffer.bind());
		if(vpRenderer->glformat().majorVersion() >= 3) {
			OVITO_CHECK_OPENGL(shader->enableAttributeArray("vertex_color"));
			OVITO_CHECK_OPENGL(shader->setAttributeBuffer("vertex_color", GL_FLOAT, 0, 4));
		}
		else {
			OVITO_CHECK_OPENGL(glEnableClientState(GL_COLOR_ARRAY));
			OVITO_CHECK_OPENGL(glColorPointer(4, GL_FLOAT, 0, 0));
		}
		_glColorsBuffer.release();
	}
	else {
		vpRenderer->activateVertexIDs(shader, _vertexCount);
	}

	OVITO_CHECK_OPENGL(glDrawArrays(GL_LINES, 0, _vertexCount));

	if(!renderer->isPicking()) {
		if(vpRenderer->glformat().majorVersion() >= 3) {
			shader->disableAttributeArray("vertex_pos");
			shader->disableAttributeArray("vertex_color");
		}
		else {
			OVITO_CHECK_OPENGL(glDisableClientState(GL_VERTEX_ARRAY));
			OVITO_CHECK_OPENGL(glDisableClientState(GL_COLOR_ARRAY));
		}
	}
	else {
		if(vpRenderer->glformat().majorVersion() >= 3) {
			shader->disableAttributeArray("vertex_pos");
		}
		else {
			OVITO_CHECK_OPENGL(glDisableClientState(GL_VERTEX_ARRAY));
		}
		vpRenderer->deactivateVertexIDs(shader);
	}
	shader->release();

	OVITO_CHECK_OPENGL();
}

};
