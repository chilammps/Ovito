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

IMPLEMENT_OVITO_OBJECT(Core, ViewportLineGeometryBuffer, LineGeometryBuffer);

/******************************************************************************
* Allocates a geometry buffer with the given number of vertices.
******************************************************************************/
void ViewportLineGeometryBuffer::beginCreate(int vertexCount)
{
	OVITO_ASSERT(!_glbuffer.isCreated() && !_vertexBuffer);
	OVITO_ASSERT(vertexCount >= 0);

	if(!_glbuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));

	if(!_glbuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));

	_glbuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
	_glbuffer.allocate(vertexCount * sizeof(Vertex));

	_vertexBuffer = static_cast<Vertex*>(_glbuffer.map(QOpenGLBuffer::WriteOnly));
	if(!_vertexBuffer)
		throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));

	_vertexCount = vertexCount;
}

/******************************************************************************
* This finalizes the buffer after it has has been filled with data.
******************************************************************************/
void ViewportLineGeometryBuffer::endCreate()
{
	OVITO_ASSERT(_vertexBuffer != nullptr);

	if(!_glbuffer.unmap())
		throw Exception(tr("Failed to unmap OpenGL vertex buffer from memory."));

	_glbuffer.release();
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportLineGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = qobject_cast<ViewportSceneRenderer*>(renderer);
	if(!vpRenderer) return false;
	return _glbuffer.isCreated() && (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void ViewportLineGeometryBuffer::render()
{
	OVITO_ASSERT(_glbuffer.isCreated());
	OVITO_STATIC_ASSERT(sizeof(FloatType) == 4);
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());

	if(!_glbuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	OVITO_CHECK_OPENGL(glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (const char*)offsetof(Vertex, position)));
	OVITO_CHECK_OPENGL(glColorPointer(4, GL_FLOAT, sizeof(Vertex), (const char*)offsetof(Vertex, color)));
	OVITO_CHECK_OPENGL(glDrawArrays(GL_LINES, 0, _vertexCount));
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	_glbuffer.release();
}

};
