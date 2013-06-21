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
#include "ViewportParticleGeometryBuffer.h"
#include "ViewportSceneRenderer.h"

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, ViewportParticleGeometryBuffer, ParticleGeometryBuffer);

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportParticleGeometryBuffer::ViewportParticleGeometryBuffer(ViewportSceneRenderer* renderer) :
	_renderer(renderer),
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_particleCount(-1)
{
	if(!_glPositionsBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glPositionsBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glRadiiBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glRadiiBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glColorsBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glColorsBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
}

/******************************************************************************
* Allocates a particle buffer with the given number of particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::setSize(int particleCount)
{
	OVITO_ASSERT(_glPositionsBuffer.isCreated());
	OVITO_ASSERT(_glRadiiBuffer.isCreated());
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(particleCount >= 0);
	OVITO_ASSERT(particleCount < std::numeric_limits<int>::max() / sizeof(Point3));
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);

	_particleCount = particleCount;
}

/******************************************************************************
* Sets the coordinates of the particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticlePositions(const Point3* coordinates)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glPositionsBuffer.isCreated());
	OVITO_ASSERT(_particleCount >= 0);

	if(!_glPositionsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glPositionsBuffer.allocate(coordinates, _particleCount * sizeof(Point3));
	_glPositionsBuffer.release();
}

/******************************************************************************
* Sets the radii of the particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleRadii(const FloatType* radii)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glRadiiBuffer.isCreated());
	OVITO_ASSERT(_particleCount >= 0);

	if(!_glRadiiBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glRadiiBuffer.allocate(radii, _particleCount * sizeof(FloatType));
	_glRadiiBuffer.release();
}

/******************************************************************************
* Sets the radius of all particles to the given value.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleRadius(FloatType radius)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glRadiiBuffer.isCreated());
	OVITO_ASSERT(_particleCount >= 0);

	if(!_glRadiiBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glRadiiBuffer.allocate(_particleCount * sizeof(FloatType));
	FloatType* bufferData = static_cast<FloatType*>(_glRadiiBuffer.map(QOpenGLBuffer::WriteOnly));
	if(!bufferData)
		throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));
	std::fill(bufferData, bufferData + _particleCount, radius);
	_glRadiiBuffer.unmap();
	_glRadiiBuffer.release();
}

/******************************************************************************
* Sets the colors of the particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleColors(const Color* colors)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(_particleCount >= 0);

	if(!_glColorsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glColorsBuffer.allocate(colors, _particleCount * sizeof(Color));
	_glColorsBuffer.release();
}

/******************************************************************************
* Sets the color of all particles to the given value.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleColor(const Color color)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(_particleCount >= 0);

	if(!_glColorsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glColorsBuffer.allocate(_particleCount * sizeof(Color));
	Color* bufferData = static_cast<Color*>(_glColorsBuffer.map(QOpenGLBuffer::WriteOnly));
	if(!bufferData)
		throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));
	std::fill(bufferData, bufferData + _particleCount, color);
	_glColorsBuffer.unmap();
	_glColorsBuffer.release();
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportParticleGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = qobject_cast<ViewportSceneRenderer*>(renderer);
	if(!vpRenderer) return false;
	return _glPositionsBuffer.isCreated()
			&& _particleCount >= 0
			&& (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void ViewportParticleGeometryBuffer::render()
{
	OVITO_ASSERT(_glPositionsBuffer.isCreated());
	OVITO_STATIC_ASSERT(sizeof(FloatType) == 4);
	OVITO_STATIC_ASSERT(sizeof(Color) == 12);
	OVITO_STATIC_ASSERT(sizeof(Point3) == 12);
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_ASSERT(_particleCount >= 0);

	// Use point sprites.
	OVITO_CHECK_OPENGL(glEnable(GL_POINT_SPRITE));

	// This is how our point sprite's size will be modified by its
	// distance from the viewer
	float param = renderer()->projParams().projectionMatrix(1,1) * renderer()->viewport()->size().height();
	if(renderer()->projParams().isPerspective) {
		float quadratic[] = { 0, 0, 100.0f / (param * param) };
		renderer()->glfuncs()->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, quadratic);
		OVITO_CHECK_OPENGL(glPointSize(10.0));
	}
	else {
		float constant[] = { 1, 0, 0 };
		renderer()->glfuncs()->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, constant);
		OVITO_CHECK_OPENGL(glPointSize(param));
	}
	// No fading of small points.
	OVITO_CHECK_OPENGL(renderer()->glfuncs()->glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 0.0f));
	OVITO_CHECK_OPENGL(renderer()->glfuncs()->glPointParameterf(GL_POINT_SIZE_MIN, 0.01f));

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	if(!_glPositionsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	OVITO_CHECK_OPENGL(glVertexPointer(3, GL_FLOAT, sizeof(Point3), NULL));
	_glPositionsBuffer.release();

	if(!_glColorsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	OVITO_CHECK_OPENGL(glColorPointer(3, GL_FLOAT, sizeof(Color), NULL));
	_glColorsBuffer.release();

	OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _particleCount));

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_POINT_SPRITE);
}

};
