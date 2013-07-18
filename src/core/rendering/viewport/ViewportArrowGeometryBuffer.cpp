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
	_arrowCount(-1)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	if(!_glVertexBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glColorsBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glColorsBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);

	// Initialize OpenGL shaders.
	_flatImposterShader = renderer->loadShaderProgram(
			"particle_flat_sphere",
			":/core/glsl/particles/sprites/imposter_without_depth.vs",
			":/core/glsl/particles/sprites/flat.fs");
}

/******************************************************************************
* Allocates a particle buffer with the given number of arrows.
******************************************************************************/
void ViewportArrowGeometryBuffer::setSize(int arrowCount)
{
	OVITO_ASSERT(_glVertexBuffer.isCreated());
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(arrowCount >= 0);
	OVITO_ASSERT(arrowCount < std::numeric_limits<int>::max() / sizeof(Point3) / 2);
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);

	_arrowCount = arrowCount;
}

/******************************************************************************
* Sets the start and end coordinates of the arrows.
******************************************************************************/
void ViewportArrowGeometryBuffer::setArrows(const Point3* coordinates)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glVertexBuffer.isCreated());
	OVITO_ASSERT(_arrowCount >= 0);

	if(!_glVertexBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glVertexBuffer.allocate(coordinates, _arrowCount * sizeof(Point3));
	_glVertexBuffer.release();
}

/******************************************************************************
* Sets the colors of the arrows.
******************************************************************************/
void ViewportArrowGeometryBuffer::setArrowColors(const Color* colors)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(_arrowCount >= 0);

	if(!_glColorsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glColorsBuffer.allocate(colors, _arrowCount * sizeof(Color));
	_glColorsBuffer.release();
}

/******************************************************************************
* Sets the color of all arrows to the given value.
******************************************************************************/
void ViewportArrowGeometryBuffer::setArrowColor(const Color color)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(_arrowCount >= 0);

	if(!_glColorsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glColorsBuffer.allocate(_arrowCount * sizeof(Color));
	if(_arrowCount) {
		Color* bufferData = static_cast<Color*>(_glColorsBuffer.map(QOpenGLBuffer::WriteOnly));
		if(!bufferData)
			throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));
		std::fill(bufferData, bufferData + _arrowCount, color);
		_glColorsBuffer.unmap();
	}
	_glColorsBuffer.release();
}

/******************************************************************************
* Sets the width of all arrows to the given value.
******************************************************************************/
void ViewportArrowGeometryBuffer::setArrowWidth(FloatType width)
{

}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportArrowGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);
	if(!vpRenderer) return false;
	return _glVertexBuffer.isCreated()
			&& _arrowCount >= 0
			&& (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void ViewportArrowGeometryBuffer::render(SceneRenderer* renderer, quint32 pickingBaseID)
{
	OVITO_CHECK_OPENGL();
	OVITO_ASSERT(_glVertexBuffer.isCreated());
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_ASSERT(_arrowCount >= 0);
	OVITO_STATIC_ASSERT(sizeof(FloatType) == 4);
	OVITO_STATIC_ASSERT(sizeof(Color) == 12);
	OVITO_STATIC_ASSERT(sizeof(Point3) == 12);

	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(_arrowCount <= 0 || !vpRenderer)
		return;
}

};
