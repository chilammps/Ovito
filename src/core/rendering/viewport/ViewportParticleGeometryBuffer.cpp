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

/// The maximum resolution of the texture used for billboard rendering of particles. Specified as a power of two.
#define BILLBOARD_TEXTURE_LEVELS 	8

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, ViewportParticleGeometryBuffer, ParticleGeometryBuffer);

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportParticleGeometryBuffer::ViewportParticleGeometryBuffer(ViewportSceneRenderer* renderer) :
	_renderer(renderer),
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_particleCount(-1),
	_billboardTexture(0)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	if(!_glPositionsBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glPositionsBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glRadiiBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glRadiiBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glColorsBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glColorsBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);

	initializeBillboardTexture();

	// Initialize OpenGL shaders.
	_flatImposterShader = renderer->loadShaderProgram("particle_flat_sphere", ":/core/glsl/particle_sprite_sphere_without_depth.vertex.glsl", ":/core/glsl/particle_flat.fragment.glsl");
	_shadedImposterShaderWithoutDepth = renderer->loadShaderProgram("particle_textured_sprite_sphere_without_depth", ":/core/glsl/particle_sprite_sphere_without_depth.vertex.glsl", ":/core/glsl/particle_sprite_sphere_without_depth.fragment.glsl");
	_shadedImposterShaderWithDepth = renderer->loadShaderProgram("particle_textured_sprite_sphere_with_depth", ":/core/glsl/particle_sprite_sphere_with_depth.vertex.glsl", ":/core/glsl/particle_sprite_sphere_with_depth.fragment.glsl");
	if(QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Geometry))
		_raytracedSphereShader = renderer->loadShaderProgram("particle_raytraced_sphere", ":/core/glsl/particle_raytraced_sphere.vertex.glsl", ":/core/glsl/particle_raytraced_sphere.fragment.glsl", ":/core/glsl/particle_raytraced_sphere.geometry.glsl");
	else
		_raytracedSphereShader = nullptr;
}

/******************************************************************************
* Destructor.
******************************************************************************/
ViewportParticleGeometryBuffer::~ViewportParticleGeometryBuffer()
{
	destroyOpenGLResources();
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
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_ASSERT(_particleCount >= 0);
	OVITO_STATIC_ASSERT(sizeof(FloatType) == 4);
	OVITO_STATIC_ASSERT(sizeof(Color) == 12);
	OVITO_STATIC_ASSERT(sizeof(Point3) == 12);

	if(_particleCount <= 0)
		return;

	if(renderingQuality() < HighQuality || shadingMode() == FlatShading || _raytracedSphereShader == nullptr)
		renderPointSprites();
	else
		renderRaytracedSpheres();
}

/******************************************************************************
* Renders the particles using OpenGL point sprites.
******************************************************************************/
void ViewportParticleGeometryBuffer::renderPointSprites()
{
	// Load billboard texture.
	if(shadingMode() != FlatShading)
		activateBillboardTexture();

	// Enable point sprites iwhen Using OpenGL 3.0/3.1. For new versions, they are already enabled by default.
	if(renderer()->glformat().profile() != QSurfaceFormat::CoreProfile)
		OVITO_CHECK_OPENGL(glEnable(GL_POINT_SPRITE));

	// This is how our point sprite's size will be modified by its
	// distance from the viewer
	float param = renderer()->projParams().projectionMatrix(1,1) * renderer()->viewport()->size().height();
	float distanceAttenuation[] = { 0, 0, 0 };
	if(renderer()->projParams().isPerspective) {
		distanceAttenuation[2] = 100.0f / (param * param);
		OVITO_CHECK_OPENGL(glPointSize(10.0));
	}
	else {
		distanceAttenuation[0] = 1;
		OVITO_CHECK_OPENGL(glPointSize(param));
	}
	if(renderer()->glfuncs30()) {
		OVITO_CHECK_OPENGL(renderer()->glfuncs30()->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, distanceAttenuation));
		OVITO_CHECK_OPENGL(renderer()->glfuncs30()->glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 0.0f));
		OVITO_CHECK_OPENGL(renderer()->glfuncs30()->glPointParameterf(GL_POINT_SIZE_MIN, 0.01f));
	}
	else if(renderer()->glfuncs32()) {
		OVITO_CHECK_OPENGL(renderer()->glfuncs32()->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, distanceAttenuation));
		OVITO_CHECK_OPENGL(renderer()->glfuncs32()->glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 0.0f));
		OVITO_CHECK_OPENGL(renderer()->glfuncs32()->glPointParameterf(GL_POINT_SIZE_MIN, 0.01f));
	}

	// Activate OpenGL shader program.
	QOpenGLShaderProgram* shader =
			(shadingMode() == FlatShading) ? _flatImposterShader :
			(renderingQuality() == LowQuality ? _shadedImposterShaderWithoutDepth : _shadedImposterShaderWithDepth);
	OVITO_CHECK_POINTER(shader);
	if(!shader->bind())
		throw Exception(tr("Failed to bind OpenGL shader program."));

	// Let the vertex shader compute the point size.
	OVITO_CHECK_OPENGL(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
	shader->setUniformValue("basePointSize", param);
	shader->setUniformValue("projection_matrix", (QMatrix4x4)renderer()->projParams().projectionMatrix);
	shader->setUniformValue("modelview_matrix", (QMatrix4x4)renderer()->modelViewTM());

	if(!_glPositionsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	shader->setAttributeBuffer("particle_pos", GL_FLOAT, 0, 3);
	shader->enableAttributeArray("particle_pos");
	_glPositionsBuffer.release();

	if(!_glColorsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	shader->setAttributeBuffer("particle_color", GL_FLOAT, 0, 3);
	shader->enableAttributeArray("particle_color");
	_glColorsBuffer.release();

	if(!_glRadiiBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	shader->setAttributeBuffer("particle_radius", GL_FLOAT, 0, 1);
	shader->enableAttributeArray("particle_radius");
	_glRadiiBuffer.release();

	OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _particleCount));

	shader->disableAttributeArray("particle_pos");
	shader->disableAttributeArray("particle_color");
	shader->disableAttributeArray("particle_radius");
	shader->release();
	OVITO_CHECK_OPENGL(glDisable(GL_VERTEX_PROGRAM_POINT_SIZE));
}

/******************************************************************************
* Renders the particles using raytracing implemented in an OpenGL fragment shader.
******************************************************************************/
void ViewportParticleGeometryBuffer::renderRaytracedSpheres()
{
	OVITO_CHECK_POINTER(_raytracedSphereShader);
	QOpenGLShaderProgram* shader = _raytracedSphereShader;

	if(!shader->bind())
		throw Exception(tr("Failed to bind OpenGL shader program."));

	shader->setUniformValue("projection_matrix", (QMatrix4x4)renderer()->projParams().projectionMatrix);
	shader->setUniformValue("inverse_projection_matrix", (QMatrix4x4)renderer()->projParams().inverseProjectionMatrix);
	shader->setUniformValue("modelview_matrix", (QMatrix4x4)renderer()->modelViewTM());
	shader->setUniformValue("is_perspective", renderer()->projParams().isPerspective);

	GLint viewportCoords[4];
	glGetIntegerv(GL_VIEWPORT, viewportCoords);
	shader->setUniformValue("viewport_origin", (float)viewportCoords[0], (float)viewportCoords[1]);
	shader->setUniformValue("inverse_viewport_size", 2.0f / (float)viewportCoords[2], 2.0f / (float)viewportCoords[3]);

	if(!_glPositionsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	shader->setAttributeBuffer("particle_pos", GL_FLOAT, 0, 3);
	shader->enableAttributeArray("particle_pos");
	_glPositionsBuffer.release();

	if(!_glColorsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	shader->setAttributeBuffer("particle_color", GL_FLOAT, 0, 3);
	shader->enableAttributeArray("particle_color");
	_glColorsBuffer.release();

	if(!_glRadiiBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	shader->setAttributeBuffer("particle_radius", GL_FLOAT, 0, 1);
	shader->enableAttributeArray("particle_radius");
	_glRadiiBuffer.release();

	OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _particleCount));

	shader->disableAttributeArray("particle_pos");
	shader->disableAttributeArray("particle_color");
	shader->disableAttributeArray("particle_radius");
	shader->release();
}

/******************************************************************************
* Creates the textures used for billboard rendering of particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::initializeBillboardTexture()
{
	static std::vector<std::array<GLubyte,2>> textureImages[BILLBOARD_TEXTURE_LEVELS];
	static bool generatedImages = false;

	if(generatedImages == false) {
		generatedImages = true;
		for(int mipmapLevel = 0; mipmapLevel < BILLBOARD_TEXTURE_LEVELS; mipmapLevel++) {
			int resolution = (1 << (BILLBOARD_TEXTURE_LEVELS - mipmapLevel - 1));
			textureImages[mipmapLevel].resize(resolution*resolution);
			size_t pixelOffset = 0;
			for(int y = 0; y < resolution; y++) {
				for(int x = 0; x < resolution; x++, pixelOffset++) {
					Vector2 r((FloatType(x - resolution/2) + 0.5) / (resolution/2), (FloatType(y - resolution/2) + 0.5) / (resolution/2));
					FloatType r2 = r.squaredLength();
					FloatType r2_clamped = std::min(r2, FloatType(1));
					FloatType diffuse_brightness = sqrt(1 - r2_clamped) * 0.6 + 0.4;

					textureImages[mipmapLevel][pixelOffset][0] =
							(GLubyte)(std::min(diffuse_brightness, (FloatType)1.0) * 255.0);

					if(r2 < 1.0) {
						// Store specular brightness in alpha channel of texture.
						Vector2 sr = r + Vector2(0.6883, 0.982);
						FloatType specular = std::max(FloatType(1) - sr.squaredLength(), FloatType(0));
						specular *= specular;
						specular *= specular * (1 - r2_clamped*r2_clamped);
						textureImages[mipmapLevel][pixelOffset][1] =
								(GLubyte)(std::min(specular, FloatType(1)) * 255.0);
					}
					else {
						// Set transparent pixel.
						textureImages[mipmapLevel][pixelOffset][1] = 0;
					}
				}
			}
		}
	}

	// Create OpenGL texture.
	glGenTextures(1, &_billboardTexture);

	// Make sure texture gets deleted again when this object is destroyed.
	attachOpenGLResources();

	// Transfer pixel data to OpenGL texture.
	OVITO_CHECK_OPENGL(glBindTexture(GL_TEXTURE_2D, _billboardTexture));
	for(int mipmapLevel = 0; mipmapLevel < BILLBOARD_TEXTURE_LEVELS; mipmapLevel++) {
		int resolution = (1 << (BILLBOARD_TEXTURE_LEVELS - mipmapLevel - 1));
		OVITO_CHECK_OPENGL(glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RG,
				resolution, resolution, 0, GL_RG, GL_UNSIGNED_BYTE, textureImages[mipmapLevel].data()));
	}
}

/******************************************************************************
* This method that takes care of freeing the shared OpenGL resources owned
* by this class.
******************************************************************************/
void ViewportParticleGeometryBuffer::freeOpenGLResources()
{
	OVITO_CHECK_OPENGL(glDeleteTextures(1, &_billboardTexture));
}

/******************************************************************************
* Activates a texture for billboard rendering of particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::activateBillboardTexture()
{
	renderer()->glfuncs()->glActiveTexture(GL_TEXTURE0);

	OVITO_ASSERT(_billboardTexture != 0);

	glBindTexture(GL_TEXTURE_2D, _billboardTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	OVITO_ASSERT(BILLBOARD_TEXTURE_LEVELS >= 3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, BILLBOARD_TEXTURE_LEVELS - 3);
}

};
