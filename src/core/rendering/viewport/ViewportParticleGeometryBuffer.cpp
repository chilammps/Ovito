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

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportParticleGeometryBuffer::ViewportParticleGeometryBuffer(ViewportSceneRenderer* renderer, ShadingMode shadingMode, RenderingQuality renderingQuality, ParticleShape shape) :
	ParticleGeometryBuffer(shadingMode, renderingQuality, shape),
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_particleCount(-1),
	_billboardTexture(0),
	_verticesPerParticle(1)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	// Initialize OpenGL shaders.
	_flatImposterShader = renderer->loadShaderProgram(
			"particle_flat_sphere",
			":/core/glsl/particles/pointsprites/sphere/without_depth.vs",
			":/core/glsl/particles/pointsprites/sphere/flat_shading.fs");
	_shadedImposterShaderWithoutDepth = renderer->loadShaderProgram(
			"particle_textured_sprite_sphere_without_depth",
			":/core/glsl/particles/pointsprites/sphere/without_depth.vs",
			":/core/glsl/particles/pointsprites/sphere/without_depth.fs");
	_shadedImposterShaderWithDepth = renderer->loadShaderProgram(
			"particle_textured_sprite_sphere_with_depth",
			":/core/glsl/particles/pointsprites/sphere/with_depth.vs",
			":/core/glsl/particles/pointsprites/sphere/with_depth.fs");
	_imposterPickingShaderWithoutDepth = renderer->loadShaderProgram(
			"particle_textured_sprite_sphere_without_depth_picking",
			":/core/glsl/particles/pointsprites/sphere/picking/without_depth.vs",
			":/core/glsl/particles/pointsprites/sphere/picking/flat_shading.fs");
	_imposterPickingShaderWithDepth = renderer->loadShaderProgram(
			"particle_textured_sprite_sphere_with_depth_picking",
			":/core/glsl/particles/pointsprites/sphere/picking/with_depth.vs",
			":/core/glsl/particles/pointsprites/sphere/picking/with_depth.fs");
	_flatSquareImposterShader = renderer->loadShaderProgram(
			"particle_flat_sprite_square",
			":/core/glsl/particles/pointsprites/sphere/without_depth.vs",
			":/core/glsl/particles/pointsprites/square/flat_shading.fs");
	_imposterSquarePickingShaderWithoutDepth = renderer->loadShaderProgram(
			"particle_flat_sprite_square_without_depth_picking",
			":/core/glsl/particles/pointsprites/sphere/picking/without_depth.vs",
			":/core/glsl/particles/pointsprites/square/picking/flat_shading.fs");
	_cubeTristripShader = renderer->loadShaderProgram(
			"particle_cube_tristrip",
			":/core/glsl/particles/geometry/cube/cube_tristrip.vs",
			":/core/glsl/particles/geometry/cube/cube.fs");
	_cubeTristripPickingShader = renderer->loadShaderProgram(
			"particle_cube_tristrip_picking",
			":/core/glsl/particles/geometry/cube/picking/cube_tristrip.vs",
			":/core/glsl/particles/geometry/cube/picking/cube.fs");
	_raytracedSphereTristripShader = renderer->loadShaderProgram(
			"particle_sphere_tristrip",
			":/core/glsl/particles/geometry/sphere/sphere_tristrip.vs",
			":/core/glsl/particles/geometry/sphere/sphere.fs");
	_raytracedSphereTristripPickingShader = renderer->loadShaderProgram(
			"particle_sphere_tristrip_picking",
			":/core/glsl/particles/geometry/sphere/picking/sphere_tristrip.vs",
			":/core/glsl/particles/geometry/sphere/picking/sphere.fs");
	if(QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Geometry)) {
		_raytracedSphereShader = renderer->loadShaderProgram(
				"particle_raytraced_sphere",
				":/core/glsl/particles/geometry/sphere/sphere.vs",
				":/core/glsl/particles/geometry/sphere/sphere.fs",
				":/core/glsl/particles/geometry/sphere/sphere.gs");
		_raytracedPickingSphereShader = renderer->loadShaderProgram(
				"particle_raytraced_sphere_picking",
				":/core/glsl/particles/geometry/sphere/picking/sphere.vs",
				":/core/glsl/particles/geometry/sphere/picking/sphere.fs",
				":/core/glsl/particles/geometry/sphere/picking/sphere.gs");
		_cubeShader = renderer->loadShaderProgram(
				"particle_cube",
				":/core/glsl/particles/geometry/cube/cube.vs",
				":/core/glsl/particles/geometry/cube/cube.fs",
				":/core/glsl/particles/geometry/cube/cube.gs");
		_cubePickingShader = renderer->loadShaderProgram(
				"particle_cube_picking",
				":/core/glsl/particles/geometry/cube/picking/cube.vs",
				":/core/glsl/particles/geometry/cube/picking/cube.fs",
				":/core/glsl/particles/geometry/cube/picking/cube.gs");
	}
	else {
		_raytracedSphereShader = nullptr;
		_raytracedPickingSphereShader = nullptr;
		_cubeShader = nullptr;
		_cubePickingShader = nullptr;
		OVITO_ASSERT(!hasGeometryShaders());
	}
	
	if(!_glPositionsBuffer.create())
		throw Exception(QStringLiteral("Failed to create OpenGL vertex buffer."));
	_glPositionsBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glRadiiBuffer.create())
		throw Exception(QStringLiteral("Failed to create OpenGL vertex buffer."));
	_glRadiiBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glColorsBuffer.create())
		throw Exception(QStringLiteral("Failed to create OpenGL vertex buffer."));
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
	OVITO_ASSERT(particleCount >= 0);
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);

	_particleCount = particleCount;

	// Determine the required number of vertices per particles.
	if(shadingMode() == FlatShading) {
		_verticesPerParticle = 1;
	}
	else {
		if(particleShape() == SphericalShape && renderingQuality() < HighQuality)
			_verticesPerParticle = 1;
		else {
			if(hasGeometryShaders())
				_verticesPerParticle = 1;
			else
				_verticesPerParticle = 14;
		}
	}

	OVITO_ASSERT(particleCount < std::numeric_limits<int>::max() / sizeof(Point3) / _verticesPerParticle);
}

/******************************************************************************
* Helper method that fills a vertex data with the given data.
******************************************************************************/
template<typename T>
void fillOpenGLBuffer(QOpenGLBuffer& buffer, const T* data, int particleCount, int verticesPerParticle)
{
	OVITO_ASSERT(buffer.isCreated());
	OVITO_ASSERT(particleCount >= 0);

	if(!buffer.bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL vertex buffer."));
	if(verticesPerParticle == 1) {
		buffer.allocate(data, particleCount * sizeof(T));
	}
	else {
		buffer.allocate(particleCount * verticesPerParticle * sizeof(T));
		if(particleCount) {
			T* bufferData = static_cast<T*>(buffer.map(QOpenGLBuffer::WriteOnly));
			OVITO_CHECK_POINTER(bufferData);
			if(!bufferData)
				throw Exception(QStringLiteral("Failed to map OpenGL vertex buffer to memory."));
			for(; particleCount != 0; particleCount--, ++data) {
				for(int i = 0; i < verticesPerParticle; i++, ++bufferData) {
					*bufferData = *data;
				}
			}
			buffer.unmap();
		}
	}
	buffer.release();
	OVITO_CHECK_OPENGL();
}

/******************************************************************************
* Helper method that fills a vertex data with the give constant value.
******************************************************************************/
template<typename T>
void fillOpenGLBuffer(QOpenGLBuffer& buffer, T value, int particleCount, int verticesPerParticle)
{
	OVITO_ASSERT(buffer.isCreated());
	OVITO_ASSERT(particleCount >= 0);
	OVITO_ASSERT(verticesPerParticle >= 1);

	if(!buffer.bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL vertex buffer."));
	buffer.allocate(particleCount * verticesPerParticle * sizeof(T));
	if(particleCount) {
		T* bufferData = static_cast<T*>(buffer.map(QOpenGLBuffer::WriteOnly));
		OVITO_CHECK_POINTER(bufferData);
		if(!bufferData)
			throw Exception(QStringLiteral("Failed to map OpenGL vertex buffer to memory."));
		std::fill(bufferData, bufferData + particleCount * verticesPerParticle, value);
		buffer.unmap();
	}
	buffer.release();
	OVITO_CHECK_OPENGL();
}

/******************************************************************************
* Sets the coordinates of the particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticlePositions(const Point3* coordinates)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	fillOpenGLBuffer(_glPositionsBuffer, coordinates, _particleCount, _verticesPerParticle);
}

/******************************************************************************
* Sets the radii of the particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleRadii(const FloatType* radii)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	fillOpenGLBuffer(_glRadiiBuffer, radii, _particleCount, _verticesPerParticle);
}

/******************************************************************************
* Sets the radius of all particles to the given value.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleRadius(FloatType radius)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	fillOpenGLBuffer(_glRadiiBuffer, radius, _particleCount, _verticesPerParticle);
}

/******************************************************************************
* Sets the colors of the particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleColors(const Color* colors)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	fillOpenGLBuffer(_glColorsBuffer, colors, _particleCount, _verticesPerParticle);
}

/******************************************************************************
* Sets the color of all particles to the given value.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleColor(const Color color)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	fillOpenGLBuffer(_glColorsBuffer, color, _particleCount, _verticesPerParticle);
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
void ViewportParticleGeometryBuffer::render(SceneRenderer* renderer)
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

	if(shadingMode() == FlatShading) {
		renderPointSprites(vpRenderer);
	}
	else {
		if(particleShape() == SphericalShape && renderingQuality() < HighQuality)
			renderPointSprites(vpRenderer);
		else
			renderCubes(vpRenderer);
	}
}

/******************************************************************************
* Renders the particles using OpenGL point sprites.
******************************************************************************/
void ViewportParticleGeometryBuffer::renderPointSprites(ViewportSceneRenderer* renderer)
{
	OVITO_ASSERT(_verticesPerParticle == 1);

	// Let the vertex shader compute the point size.
	OVITO_CHECK_OPENGL(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));

	// Enable point sprites when using the compatibility OpenGL profile.
	// In the core profile, they are already enabled by default.
	if(renderer->isCoreProfile() == false) {
		OVITO_CHECK_OPENGL(glEnable(GL_POINT_SPRITE));

		// Specify point sprite texture coordinate replacement mode.
		glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	}

	if(particleShape() == SphericalShape)
		activateBillboardTexture(renderer);

	// Pick the right OpenGL shader program.
	QOpenGLShaderProgram* shader;
	if(!renderer->isPicking()) {
		if(shadingMode() == FlatShading || particleShape() != SphericalShape) {
			if(particleShape() == SphericalShape)
				shader = _flatImposterShader;
			else
				shader = _flatSquareImposterShader;
		}
		else {
			if(renderingQuality() == LowQuality)
				shader = _shadedImposterShaderWithoutDepth;
			else
				shader = _shadedImposterShaderWithDepth;
		}
	}
	else {
		if(shadingMode() == FlatShading || particleShape() != SphericalShape) {
			if(particleShape() == SphericalShape)
				shader = _imposterPickingShaderWithoutDepth;
			else
				shader = _imposterSquarePickingShaderWithoutDepth;
		}
		else {
			if(renderingQuality() == LowQuality)
				shader = _imposterPickingShaderWithoutDepth;
			else
				shader = _imposterPickingShaderWithDepth;
		}
	}
	OVITO_CHECK_POINTER(shader);
	if(!shader->bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL shader program."));

	// This is how our point sprite's size will be modified based on the distance from the viewer.
	GLint viewportCoords[4];
	glGetIntegerv(GL_VIEWPORT, viewportCoords);
	float param = renderer->projParams().projectionMatrix(1,1) * viewportCoords[3];

	if(renderer->isCoreProfile() == false) {
		// This is a fallback if GL_VERTEX_PROGRAM_POINT_SIZE is not supported.
		std::array<float,3> distanceAttenuation;
		if(renderer->projParams().isPerspective)
			distanceAttenuation = std::array<float,3>{{ 0.0f, 0.0f, 1.0f / (param * param) }};
		else
			distanceAttenuation = std::array<float,3>{{ 1.0f / param, 0.0f, 0.0f }};
		OVITO_CHECK_OPENGL(glPointSize(1));
		OVITO_CHECK_OPENGL(renderer->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, distanceAttenuation.data()));
	}

	shader->setUniformValue("basePointSize", param);
	shader->setUniformValue("projection_matrix", (QMatrix4x4)renderer->projParams().projectionMatrix);
	shader->setUniformValue("modelview_matrix", (QMatrix4x4)renderer->modelViewTM());

	bindParticlePositionBuffer(renderer, shader);
	bindParticleColorBuffer(renderer, shader);
	bindParticleRadiusBuffer(renderer, shader);

	OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _particleCount));

	OVITO_CHECK_OPENGL(glDisable(GL_VERTEX_PROGRAM_POINT_SIZE));
	detachParticlePositionBuffer(renderer, shader);
	detachParticleColorBuffer(renderer, shader);
	detachParticleRadiusBuffer(renderer, shader);
	shader->release();

	// Disable point sprites.
	if(renderer->isCoreProfile() == false)
		OVITO_CHECK_OPENGL(glDisable(GL_POINT_SPRITE));

	if(particleShape() == SphericalShape)
		deactivateBillboardTexture(renderer);
}

/******************************************************************************
* Renders a cube for each particle using triangle strips.
******************************************************************************/
void ViewportParticleGeometryBuffer::renderCubes(ViewportSceneRenderer* renderer)
{
	// Pick the right OpenGL shader program.
	QOpenGLShaderProgram* shader;
	if(hasGeometryShaders()) {
		OVITO_ASSERT(_verticesPerParticle == 1);
		if(!renderer->isPicking()) {
			if(particleShape() == SphericalShape)
				shader = _raytracedSphereShader;
			else
				shader = _cubeShader;
		}
		else {
			if(particleShape() == SphericalShape)
				shader = _raytracedPickingSphereShader;
			else
				shader = _cubePickingShader;
		}
	}
	else {
		OVITO_ASSERT(_verticesPerParticle == 14);
		if(!renderer->isPicking()) {
			if(particleShape() == SphericalShape)
				shader = _raytracedSphereTristripShader;
			else
				shader = _cubeTristripShader;
		}
		else {
			if(particleShape() == SphericalShape)
				shader = _raytracedSphereTristripPickingShader;
			else
				shader = _cubeTristripPickingShader;
		}
	}
	if(!shader->bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL shader program."));

	// Need to render only the front facing sides of the cubes.
	glEnable(GL_CULL_FACE);

	// This is to draw the cube with a single triangle strip.
	// The cube vertices:
	static const GLfloat cubeVerts[14][3] = {
		{ 1,  1,  1},
		{ 1, -1,  1},
		{ 1,  1, -1},
		{ 1, -1, -1},
		{-1, -1, -1},
		{ 1, -1,  1},
		{-1, -1,  1},
		{ 1,  1,  1},
		{-1,  1,  1},
		{ 1,  1, -1},
		{-1,  1, -1},
		{-1, -1, -1},
		{-1,  1,  1},
		{-1, -1,  1},
	};
	shader->setUniformValueArray("cubeVerts", &cubeVerts[0][0], 14, 3);

	// Set up look-up table for triangle strips.
	if(particleShape() != SphericalShape && !renderer->isPicking()) {
		// The normal vectors for the cube triangle strip.
		static const GLfloat normals[14][3] = {
			{ 1,  0,  0},
			{ 1,  0,  0},
			{ 1,  0,  0},
			{ 1,  0,  0},
			{ 0,  0, -1},
			{ 0, -1,  0},
			{ 0, -1,  0},
			{ 0,  0,  1},
			{ 0,  0,  1},
			{ 0,  1,  0},
			{ 0,  1,  0},
			{ 0,  0, -1},
			{-1,  0,  0},
			{-1,  0,  0}
		};
		shader->setUniformValueArray("normals", &normals[0][0], 14, 3);
		shader->setUniformValue("normal_matrix", (QMatrix3x3)(renderer->modelViewTM().linear().inverse().transposed()));
	}

	shader->setUniformValue("projection_matrix", (QMatrix4x4)renderer->projParams().projectionMatrix);
	shader->setUniformValue("inverse_projection_matrix", (QMatrix4x4)renderer->projParams().inverseProjectionMatrix);
	shader->setUniformValue("modelview_matrix", (QMatrix4x4)renderer->modelViewTM());
	shader->setUniformValue("is_perspective", renderer->projParams().isPerspective);

	GLint viewportCoords[4];
	glGetIntegerv(GL_VIEWPORT, viewportCoords);
	shader->setUniformValue("viewport_origin", (float)viewportCoords[0], (float)viewportCoords[1]);
	shader->setUniformValue("inverse_viewport_size", 2.0f / (float)viewportCoords[2], 2.0f / (float)viewportCoords[3]);

	bindParticlePositionBuffer(renderer, shader);
	bindParticleColorBuffer(renderer, shader);
	bindParticleRadiusBuffer(renderer, shader);

	if(hasGeometryShaders()) {
		OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _particleCount));
	}
	else {
		// Prepare arrays required for glMultiDrawArrays().
		if(_primitiveStartIndices.size() != _particleCount) {
			_primitiveStartIndices.resize(_particleCount);
			_primitiveVertexCounts.resize(_particleCount);
			GLint index = 0;
			for(GLint& s : _primitiveStartIndices) {
				s = index;
				index += _verticesPerParticle;
			}
			std::fill(_primitiveVertexCounts.begin(), _primitiveVertexCounts.end(), _verticesPerParticle);
		}
		OVITO_ASSERT(_verticesPerParticle == 14);

		renderer->activateVertexIDs(shader, _verticesPerParticle * _particleCount);

		OVITO_CHECK_OPENGL(renderer->glMultiDrawArrays(GL_TRIANGLE_STRIP,
				_primitiveStartIndices.data(),
				_primitiveVertexCounts.data(),
				_primitiveStartIndices.size()));

		renderer->deactivateVertexIDs(shader);
	}

	detachParticlePositionBuffer(renderer, shader);
	detachParticleColorBuffer(renderer, shader);
	detachParticleRadiusBuffer(renderer, shader);

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

	renderer->glfuncs()->glActiveTexture(GL_TEXTURE0);

	// Create OpenGL texture.
	glGenTextures(1, &_billboardTexture);

	// Make sure texture gets deleted again when this object is destroyed.
	attachOpenGLResources();

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
	glDeleteTextures(1, &_billboardTexture);
	_billboardTexture = 0;
}

/******************************************************************************
* Activates a texture for billboard rendering of spherical particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::activateBillboardTexture(ViewportSceneRenderer* renderer)
{
	// Active texture only if needed.
	if(shadingMode() == FlatShading) return;
	if(renderer->isPicking()) return;
	if(particleShape() != SphericalShape) return;

	// Enable texture mapping when using compatibility OpenGL.
	// In the core profile, this is already enabled by default.
	if(renderer->isCoreProfile() == false)
		OVITO_CHECK_OPENGL(glEnable(GL_TEXTURE_2D));

	OVITO_ASSERT(_billboardTexture != 0);

	OVITO_CHECK_OPENGL(renderer->glfuncs()->glActiveTexture(GL_TEXTURE0));
	OVITO_CHECK_OPENGL(glBindTexture(GL_TEXTURE_2D, _billboardTexture));

	OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
	OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	OVITO_ASSERT(BILLBOARD_TEXTURE_LEVELS >= 3);
	OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, BILLBOARD_TEXTURE_LEVELS - 3));
}

/******************************************************************************
* Deactivates the texture used for billboard rendering of spherical particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::deactivateBillboardTexture(ViewportSceneRenderer* renderer)
{
	// Disable texture mapping again when not using core profile.
	if(renderer->isCoreProfile() == false)
		OVITO_CHECK_OPENGL(glDisable(GL_TEXTURE_2D));
}

/******************************************************************************
* Binds the vertex buffer containing the particle positions to the corresponding
* shader input attribute.
******************************************************************************/
void ViewportParticleGeometryBuffer::bindParticlePositionBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader)
{
	OVITO_ASSERT(_glPositionsBuffer.isCreated());

	if(!_glPositionsBuffer.bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL vertex positions buffer."));

	if(renderer->glformat().majorVersion() >= 3) {
		shader->enableAttributeArray("particle_pos");
		shader->setAttributeBuffer("particle_pos", GL_FLOAT, 0, 3);
	}
	else {
		// Older OpenGL implementations cannot take vertex coordinates
		// through a custom shader attribute.
		OVITO_CHECK_OPENGL(glEnableClientState(GL_VERTEX_ARRAY));
		OVITO_CHECK_OPENGL(glVertexPointer(3, GL_FLOAT, 0, 0));
	}
	_glPositionsBuffer.release();
}

/******************************************************************************
* Detaches the vertex buffer containing the particle positions from the corresponding
* shader input attribute.
******************************************************************************/
void ViewportParticleGeometryBuffer::detachParticlePositionBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader)
{
	if(renderer->glformat().majorVersion() >= 3)
		shader->disableAttributeArray("particle_pos");
	else
		OVITO_CHECK_OPENGL(glDisableClientState(GL_VERTEX_ARRAY));
}

/******************************************************************************
* Binds the vertex buffer containing the particle colors to the corresponding
* shader input attribute.
******************************************************************************/
void ViewportParticleGeometryBuffer::bindParticleColorBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader)
{
	if(!renderer->isPicking()) {
		OVITO_ASSERT(_glColorsBuffer.isCreated());
		if(!_glColorsBuffer.bind())
			throw Exception(QStringLiteral("Failed to bind OpenGL vertex color buffer."));
		if(renderer->glformat().majorVersion() >= 3) {
			shader->enableAttributeArray("particle_color");
			shader->setAttributeBuffer("particle_color", GL_FLOAT, 0, 3);
		}
		else {
			OVITO_CHECK_OPENGL(glEnableClientState(GL_COLOR_ARRAY));
			OVITO_CHECK_OPENGL(glColorPointer(3, GL_FLOAT, 0, 0));
		}
		_glColorsBuffer.release();
	}
	else {

		OVITO_CHECK_OPENGL(shader->setUniformValue("pickingBaseID", (GLint)renderer->registerSubObjectIDs(_particleCount)));

		// In picking mode, the OpenGL vertex shader needs particle IDs to compute the particle colors.
		renderer->activateVertexIDs(shader, _particleCount * _verticesPerParticle);
	}
}

/******************************************************************************
* Detaches the vertex buffer containing the particle colors from the corresponding
* shader input attribute.
******************************************************************************/
void ViewportParticleGeometryBuffer::detachParticleColorBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader)
{
	if(!renderer->isPicking()) {
		if(renderer->glformat().majorVersion() >= 3)
			shader->disableAttributeArray("particle_color");
		else
			OVITO_CHECK_OPENGL(glDisableClientState(GL_COLOR_ARRAY));
	}
	else {
		renderer->deactivateVertexIDs(shader);
	}
}

/******************************************************************************
* Binds the vertex buffer containing the particle radii to the corresponding
* shader input attribute.
******************************************************************************/
void ViewportParticleGeometryBuffer::bindParticleRadiusBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader)
{
	OVITO_ASSERT(_glRadiiBuffer.isCreated());

	if(!_glRadiiBuffer.bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL vertex radius buffer."));
	shader->enableAttributeArray("particle_radius");
	shader->setAttributeBuffer("particle_radius", GL_FLOAT, 0, 1);
	_glRadiiBuffer.release();
}

/******************************************************************************
* Detaches the vertex buffer containing the particle radii from the corresponding
* shader input attribute.
******************************************************************************/
void ViewportParticleGeometryBuffer::detachParticleRadiusBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader)
{
	shader->disableAttributeArray("particle_radius");
}

};
