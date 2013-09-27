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
ViewportParticleGeometryBuffer::ViewportParticleGeometryBuffer(ViewportSceneRenderer* renderer, ShadingMode shadingMode, RenderingQuality renderingQuality) :
	ParticleGeometryBuffer(shadingMode, renderingQuality),
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_particleCount(-1),
	_billboardTexture(0)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	// Initialize OpenGL shaders.
	_flatImposterShader = renderer->loadShaderProgram(
			"particle_flat_sphere",
			":/core/glsl/particles/sprites/imposter_without_depth.vs",
			":/core/glsl/particles/sprites/flat.fs");
	_shadedImposterShaderWithoutDepth = renderer->loadShaderProgram(
			"particle_textured_sprite_sphere_without_depth",
			":/core/glsl/particles/sprites/imposter_without_depth.vs",
			":/core/glsl/particles/sprites/imposter_without_depth.fs");
	_shadedImposterShaderWithDepth = renderer->loadShaderProgram(
			"particle_textured_sprite_sphere_with_depth",
			":/core/glsl/particles/sprites/imposter_with_depth.vs",
			":/core/glsl/particles/sprites/imposter_with_depth.fs");
	_imposterPickingShaderWithoutDepth = renderer->loadShaderProgram(
			"particle_textured_sprite_sphere_without_depth_picking",
			":/core/glsl/particles/picking/sprites/imposter_without_depth.vs",
			":/core/glsl/particles/picking/sprites/imposter_without_depth.fs");
	_imposterPickingShaderWithDepth = renderer->loadShaderProgram(
			"particle_textured_sprite_sphere_with_depth_picking",
			":/core/glsl/particles/picking/sprites/imposter_with_depth.vs",
			":/core/glsl/particles/picking/sprites/imposter_with_depth.fs");
	if(QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Geometry)) {
		_raytracedSphereShader = renderer->loadShaderProgram(
				"particle_raytraced_sphere",
				":/core/glsl/particles/raytraced/sphere.vs",
				":/core/glsl/particles/raytraced/sphere.fs",
				":/core/glsl/particles/raytraced/sphere.gs");
		_raytracedPickingSphereShader = renderer->loadShaderProgram(
				"particle_raytraced_sphere_picking",
				":/core/glsl/particles/picking/raytraced/sphere.vs",
				":/core/glsl/particles/picking/raytraced/sphere.fs",
				":/core/glsl/particles/picking/raytraced/sphere.gs");
	}
	else {
		_raytracedSphereShader = nullptr;
		_raytracedPickingSphereShader = nullptr;
	}

	if(!_glPositionsBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glPositionsBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glRadiiBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glRadiiBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glColorsBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glColorsBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);

	initializeBillboardTexture(renderer);
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

	// Reset index buffer.
	if(_glIndexBuffer.isCreated())
		_glIndexBuffer.destroy();
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
	if(_particleCount) {
		FloatType* bufferData = static_cast<FloatType*>(_glRadiiBuffer.map(QOpenGLBuffer::WriteOnly));
		if(!bufferData)
			throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));
		std::fill(bufferData, bufferData + _particleCount, radius);
		_glRadiiBuffer.unmap();
	}
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
	if(_particleCount) {
		Color* bufferData = static_cast<Color*>(_glColorsBuffer.map(QOpenGLBuffer::WriteOnly));
		if(!bufferData)
			throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));
		std::fill(bufferData, bufferData + _particleCount, color);
		_glColorsBuffer.unmap();
	}
	_glColorsBuffer.release();
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportParticleGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);
	if(!vpRenderer) return false;
	return _glPositionsBuffer.isCreated()
			&& _particleCount >= 0
			&& (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void ViewportParticleGeometryBuffer::render(SceneRenderer* renderer, quint32 pickingBaseID)
{
	OVITO_CHECK_OPENGL();
	OVITO_ASSERT(_glPositionsBuffer.isCreated());
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_ASSERT(_particleCount >= 0);
	OVITO_STATIC_ASSERT(sizeof(FloatType) == 4);
	OVITO_STATIC_ASSERT(sizeof(Color) == 12);
	OVITO_STATIC_ASSERT(sizeof(Point3) == 12);

	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(_particleCount <= 0 || !vpRenderer)
		return;

	if(renderingQuality() < HighQuality || shadingMode() == FlatShading || _raytracedSphereShader == nullptr)
		renderPointSprites(vpRenderer, pickingBaseID);
	else
		renderRaytracedSpheres(vpRenderer, pickingBaseID);
}

/******************************************************************************
* Renders the particles using OpenGL point sprites.
******************************************************************************/
void ViewportParticleGeometryBuffer::renderPointSprites(ViewportSceneRenderer* renderer, quint32 pickingBaseID)
{
	// Load billboard texture.
	if(shadingMode() != FlatShading && !renderer->isPicking())
		activateBillboardTexture();

	// Let the vertex shader compute the point size.
	OVITO_CHECK_OPENGL(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));

	// Enable point sprites when using compatibility OpenGL. In the core profile, they are already enabled by default.
	if(renderer->glformat().profile() != QSurfaceFormat::CoreProfile) {
		OVITO_CHECK_OPENGL(glEnable(GL_TEXTURE_2D));
		OVITO_CHECK_OPENGL(glEnable(GL_POINT_SPRITE));

		// Specify point sprite texture coordinate replacement mode.
		glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	}

	// This is how our point sprite's size will be modified by its
	// distance from the viewer
	GLint viewportCoords[4];
	glGetIntegerv(GL_VIEWPORT, viewportCoords);
	float param = renderer->projParams().projectionMatrix(1,1) * viewportCoords[3];
	float distanceAttenuation[3];
	if(renderer->projParams().isPerspective) {
		distanceAttenuation[0] = 0;
		distanceAttenuation[1] = 0;
		distanceAttenuation[2] = 1.0f / (param * param);
	}
	else {
		distanceAttenuation[0] = 1.0f / param;
		distanceAttenuation[1] = 0;
		distanceAttenuation[2] = 0;
	}
	OVITO_CHECK_OPENGL(glPointSize(1));

	if(renderer->glfuncs20()) {
		OVITO_CHECK_OPENGL(renderer->glfuncs20()->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, distanceAttenuation));
#ifdef Q_OS_MACX
		if(renderer->glcontext()->surface()->surfaceClass() == QSurface::Offscreen)
			OVITO_CHECK_OPENGL(renderer->glfuncs20()->glPointParameterf(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT));
#endif
	}
	else if(renderer->glfuncs30()) {
		OVITO_CHECK_OPENGL(renderer->glfuncs30()->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, distanceAttenuation));
#ifdef Q_OS_MACX
		if(renderer->glcontext()->surface()->surfaceClass() == QSurface::Offscreen)
			OVITO_CHECK_OPENGL(renderer->glfuncs30()->glPointParameterf(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT));
#endif
	}
	else if(renderer->glfuncs32()) {
		OVITO_CHECK_OPENGL(renderer->glfuncs32()->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, distanceAttenuation));
#ifdef Q_OS_MACX
		if(renderer->glcontext()->surface()->surfaceClass() == QSurface::Offscreen)
			OVITO_CHECK_OPENGL(renderer->glfuncs32()->glPointParameterf(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT));
#endif
	}
	else {
		OVITO_ASSERT(false);
	}

	// Activate OpenGL shader program.
	QOpenGLShaderProgram* shader;
	if(!renderer->isPicking()) {
		shader = (shadingMode() == FlatShading) ? _flatImposterShader :
				(renderingQuality() == LowQuality ? _shadedImposterShaderWithoutDepth : _shadedImposterShaderWithDepth);
	}
	else {
		shader = (shadingMode() == FlatShading || renderingQuality() == LowQuality) ?
				_imposterPickingShaderWithoutDepth : _imposterPickingShaderWithDepth;
	}
	OVITO_CHECK_POINTER(shader);
	if(!shader->bind())
		throw Exception(tr("Failed to bind OpenGL shader program."));

	shader->setUniformValue("basePointSize", param);
	shader->setUniformValue("projection_matrix", (QMatrix4x4)renderer->projParams().projectionMatrix);
	shader->setUniformValue("modelview_matrix", (QMatrix4x4)renderer->modelViewTM());
	if(renderer->isPicking())
		OVITO_CHECK_OPENGL(shader->setUniformValue("pickingBaseID", (GLint)pickingBaseID));

	if(!_glPositionsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	if(renderer->glformat().majorVersion() >= 3) {
		shader->setAttributeBuffer("particle_pos", GL_FLOAT, 0, 3);
		shader->enableAttributeArray("particle_pos");
	}
	else {
		OVITO_CHECK_OPENGL(glEnableClientState(GL_VERTEX_ARRAY));
		OVITO_CHECK_OPENGL(glVertexPointer(3, GL_FLOAT, 0, 0));
	}
	_glPositionsBuffer.release();

	if(!renderer->isPicking()) {
		if(!_glColorsBuffer.bind())
			throw Exception(tr("Failed to bind OpenGL vertex buffer."));
		if(renderer->glformat().majorVersion() >= 3) {
			shader->setAttributeBuffer("particle_color", GL_FLOAT, 0, 3);
			shader->enableAttributeArray("particle_color");
		}
		else {
			OVITO_CHECK_OPENGL(glEnableClientState(GL_COLOR_ARRAY));
			OVITO_CHECK_OPENGL(glColorPointer(3, GL_FLOAT, 0, 0));
		}
		_glColorsBuffer.release();
	}
	else {
		if(renderer->glformat().majorVersion() >= 3) {
			// Create and fill vertex index buffer.
			if(!_glIndexBuffer.isCreated()) {
				if(!_glIndexBuffer.create())
					throw Exception(tr("Failed to create OpenGL vertex buffer."));
				_glIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
				if(!_glIndexBuffer.bind())
					throw Exception(tr("Failed to bind OpenGL vertex buffer."));
				_glIndexBuffer.allocate(_particleCount * sizeof(GLint));
				OVITO_ASSERT(_particleCount > 0);
				GLint* bufferData = static_cast<GLint*>(_glIndexBuffer.map(QOpenGLBuffer::WriteOnly));
				if(!bufferData)
					throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));
				for(GLint index = 0; index < _particleCount; index++)
					bufferData[index] = index;
				_glIndexBuffer.unmap();
			}
			else {
				if(!_glIndexBuffer.bind())
					throw Exception(tr("Failed to bind OpenGL vertex buffer."));
			}
			shader->setAttributeBuffer("vertexID", GL_INT, 0, 1);
			shader->enableAttributeArray("vertexID");
			_glIndexBuffer.release();
		}
	}

	if(!_glRadiiBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	shader->setAttributeBuffer("particle_radius", GL_FLOAT, 0, 1);
	shader->enableAttributeArray("particle_radius");
	_glRadiiBuffer.release();

	OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _particleCount));

	OVITO_CHECK_OPENGL(glDisable(GL_VERTEX_PROGRAM_POINT_SIZE));
	if(renderer->glformat().majorVersion() < 3) {
		OVITO_CHECK_OPENGL(glDisableClientState(GL_VERTEX_ARRAY));
		OVITO_CHECK_OPENGL(glDisableClientState(GL_COLOR_ARRAY));
	}
	else {
		shader->disableAttributeArray("particle_pos");
		if(!renderer->isPicking())
			shader->disableAttributeArray("particle_color");
	}
	shader->disableAttributeArray("particle_radius");
	shader->release();

	// Disable point sprites.
	if(renderer->glformat().profile() != QSurfaceFormat::CoreProfile) {
		OVITO_CHECK_OPENGL(glDisable(GL_POINT_SPRITE));
		OVITO_CHECK_OPENGL(glDisable(GL_TEXTURE_2D));
	}
}

/******************************************************************************
* Renders the particles using raytracing implemented in an OpenGL fragment shader.
******************************************************************************/
void ViewportParticleGeometryBuffer::renderRaytracedSpheres(ViewportSceneRenderer* renderer, quint32 pickingBaseID)
{
	OVITO_CHECK_POINTER(_raytracedSphereShader && _raytracedPickingSphereShader);
	QOpenGLShaderProgram* shader = !renderer->isPicking() ? _raytracedSphereShader : _raytracedPickingSphereShader;

	if(!shader->bind())
		throw Exception(tr("Failed to bind OpenGL shader program."));

	glEnable(GL_CULL_FACE);

	// Set up look-up table for cube vertices.
	static const GLfloat cubeVerts[8][3] = {
		{-1, -1, -1},
		{-1,  1, -1},
		{ 1, -1, -1},
		{ 1,  1, -1},
		{-1, -1,  1},
		{-1,  1,  1},
		{ 1,  1,  1},
		{ 1, -1,  1}
	};
	shader->setUniformValueArray("cubeVerts", &cubeVerts[0][0], 8, 3);

	// Set up look-up table for cube triangle strip.
	static const GLint stripIndices[14] = { 3,2,6,7,4,2,0,3,1,6,5,4,1,0 };
	shader->setUniformValueArray("stripIndices", &stripIndices[0], 14);

	shader->setUniformValue("projection_matrix", (QMatrix4x4)renderer->projParams().projectionMatrix);
	shader->setUniformValue("inverse_projection_matrix", (QMatrix4x4)renderer->projParams().inverseProjectionMatrix);
	shader->setUniformValue("modelview_matrix", (QMatrix4x4)renderer->modelViewTM());
	shader->setUniformValue("is_perspective", renderer->projParams().isPerspective);
	if(renderer->isPicking())
		OVITO_CHECK_OPENGL(shader->setUniformValue("pickingBaseID", (GLint)pickingBaseID));

	GLint viewportCoords[4];
	glGetIntegerv(GL_VIEWPORT, viewportCoords);
	shader->setUniformValue("viewport_origin", (float)viewportCoords[0], (float)viewportCoords[1]);
	shader->setUniformValue("inverse_viewport_size", 2.0f / (float)viewportCoords[2], 2.0f / (float)viewportCoords[3]);

	if(!_glPositionsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	shader->setAttributeBuffer("particle_pos", GL_FLOAT, 0, 3);
	shader->enableAttributeArray("particle_pos");
	_glPositionsBuffer.release();

	if(!renderer->isPicking()) {
		if(!_glColorsBuffer.bind())
			throw Exception(tr("Failed to bind OpenGL vertex buffer."));
		shader->setAttributeBuffer("particle_color", GL_FLOAT, 0, 3);
		shader->enableAttributeArray("particle_color");
		_glColorsBuffer.release();
	}

	if(!_glRadiiBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	shader->setAttributeBuffer("particle_radius", GL_FLOAT, 0, 1);
	shader->enableAttributeArray("particle_radius");
	_glRadiiBuffer.release();

	OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _particleCount));

	shader->disableAttributeArray("particle_pos");
	if(!renderer->isPicking())
		shader->disableAttributeArray("particle_color");
	shader->disableAttributeArray("particle_radius");
	shader->release();
}

/******************************************************************************
* Creates the textures used for billboard rendering of particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::initializeBillboardTexture(ViewportSceneRenderer* renderer)
{
	static std::vector<std::array<GLubyte,4>> textureImages[BILLBOARD_TEXTURE_LEVELS];
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

					textureImages[mipmapLevel][pixelOffset][2] = 255;
					textureImages[mipmapLevel][pixelOffset][3] = 255;

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

	renderer->glfuncs()->glActiveTexture(GL_TEXTURE0);

	// Transfer pixel data to OpenGL texture.
	OVITO_CHECK_OPENGL(glBindTexture(GL_TEXTURE_2D, _billboardTexture));
	for(int mipmapLevel = 0; mipmapLevel < BILLBOARD_TEXTURE_LEVELS; mipmapLevel++) {
		int resolution = (1 << (BILLBOARD_TEXTURE_LEVELS - mipmapLevel - 1));

		OVITO_CHECK_OPENGL(glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGBA,
				resolution, resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImages[mipmapLevel].data()));
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
	OVITO_ASSERT(_billboardTexture != 0);

	glBindTexture(GL_TEXTURE_2D, _billboardTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	OVITO_ASSERT(BILLBOARD_TEXTURE_LEVELS >= 3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, BILLBOARD_TEXTURE_LEVELS - 3);
}

};
