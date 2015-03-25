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
#include "OpenGLParticlePrimitive.h"
#include "ViewportSceneRenderer.h"

/// The maximum resolution of the texture used for billboard rendering of particles. Specified as a power of two.
#define BILLBOARD_TEXTURE_LEVELS 	8

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Constructor.
******************************************************************************/
OpenGLParticlePrimitive::OpenGLParticlePrimitive(ViewportSceneRenderer* renderer, ShadingMode shadingMode,
		RenderingQuality renderingQuality, ParticleShape shape, bool translucentParticles) :
	ParticlePrimitive(shadingMode, renderingQuality, shape, translucentParticles),
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_shader(nullptr), _pickingShader(nullptr),
	_usingGeometryShader(renderer->useGeometryShaders()),
	_maxVBOSize(4*1024*1024), _particleCount(-1)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	// Determine rendering technique to use.
	if(shadingMode == FlatShading) {
		if(renderer->usePointSprites())
			_renderingTechnique = POINT_SPRITES;
		else
			_renderingTechnique = IMPOSTER_QUADS;
	}
	else {
		if(shape == SphericalShape && renderingQuality < HighQuality) {
			if(renderer->usePointSprites())
				_renderingTechnique = POINT_SPRITES;
			else
				_renderingTechnique = IMPOSTER_QUADS;
		}
		else {
			_renderingTechnique = CUBE_GEOMETRY;
		}
	}

	// Load the right OpenGL shaders.
	if(_renderingTechnique == POINT_SPRITES) {
		if(shadingMode == FlatShading) {
			if(shape == SphericalShape) {
				_shader = renderer->loadShaderProgram("particle_pointsprite_spherical_flat",
						":/core/glsl/particles/pointsprites/sphere/without_depth.vs",
						":/core/glsl/particles/pointsprites/sphere/flat_shading.fs");
				_pickingShader = renderer->loadShaderProgram("particle_pointsprite_spherical_nodepth_picking",
						":/core/glsl/particles/pointsprites/sphere/picking/without_depth.vs",
						":/core/glsl/particles/pointsprites/sphere/picking/flat_shading.fs");
			}
			else if(shape == SquareShape) {
				_shader = renderer->loadShaderProgram("particle_pointsprite_square_flat",
						":/core/glsl/particles/pointsprites/sphere/without_depth.vs",
						":/core/glsl/particles/pointsprites/square/flat_shading.fs");
				_pickingShader = renderer->loadShaderProgram("particle_pointsprite_square_flat_picking",
						":/core/glsl/particles/pointsprites/sphere/picking/without_depth.vs",
						":/core/glsl/particles/pointsprites/square/picking/flat_shading.fs");
			}
		}
		else if(shadingMode == NormalShading) {
			if(shape == SphericalShape) {
				if(renderingQuality == LowQuality) {
					_shader = renderer->loadShaderProgram("particle_pointsprite_spherical_shaded_nodepth",
							":/core/glsl/particles/pointsprites/sphere/without_depth.vs",
							":/core/glsl/particles/pointsprites/sphere/without_depth.fs");
					_pickingShader = renderer->loadShaderProgram("particle_pointsprite_spherical_nodepth_picking",
							":/core/glsl/particles/pointsprites/sphere/picking/without_depth.vs",
							":/core/glsl/particles/pointsprites/sphere/picking/flat_shading.fs");
				}
				else if(renderingQuality == MediumQuality) {
					_shader = renderer->loadShaderProgram("particle_pointsprite_spherical_shaded_depth",
							":/core/glsl/particles/pointsprites/sphere/with_depth.vs",
							":/core/glsl/particles/pointsprites/sphere/with_depth.fs");
					_pickingShader = renderer->loadShaderProgram("particle_pointsprite_spherical_shaded_depth_picking",
							":/core/glsl/particles/pointsprites/sphere/picking/with_depth.vs",
							":/core/glsl/particles/pointsprites/sphere/picking/with_depth.fs");
				}
			}
		}
	}
	else if(_renderingTechnique == IMPOSTER_QUADS) {
		if(shadingMode == FlatShading) {
			if(shape == SphericalShape) {
				if(_usingGeometryShader) {
					_shader = renderer->loadShaderProgram("particle_geomshader_imposter_spherical_flat",
							":/core/glsl/particles/imposter/sphere/without_depth.vs",
							":/core/glsl/particles/imposter/sphere/flat_shading.fs",
							":/core/glsl/particles/imposter/sphere/without_depth.gs");
					_pickingShader = renderer->loadShaderProgram("particle_geomshader_imposter_spherical_nodepth_picking",
							":/core/glsl/particles/imposter/sphere/picking/without_depth.vs",
							":/core/glsl/particles/imposter/sphere/picking/flat_shading.fs",
							":/core/glsl/particles/imposter/sphere/picking/without_depth.gs");
				}
				else {
					_shader = renderer->loadShaderProgram("particle_imposter_spherical_flat",
							":/core/glsl/particles/imposter/sphere/without_depth_tri.vs",
							":/core/glsl/particles/imposter/sphere/flat_shading.fs");
					_pickingShader = renderer->loadShaderProgram("particle_imposter_spherical_nodepth_picking",
							":/core/glsl/particles/imposter/sphere/picking/without_depth_tri.vs",
							":/core/glsl/particles/imposter/sphere/picking/flat_shading.fs");
				}
			}
			else if(shape == SquareShape) {
				if(_usingGeometryShader) {
					_shader = renderer->loadShaderProgram("particle_geomshader_imposter_square_flat",
							":/core/glsl/particles/imposter/sphere/without_depth.vs",
							":/core/glsl/particles/pointsprites/square/flat_shading.fs",
							":/core/glsl/particles/imposter/sphere/without_depth.gs");
					_pickingShader = renderer->loadShaderProgram("particle_geomshader_imposter_square_flat_picking",
							":/core/glsl/particles/imposter/sphere/picking/without_depth.vs",
							":/core/glsl/particles/pointsprites/square/picking/flat_shading.fs",
							":/core/glsl/particles/imposter/sphere/picking/without_depth.gs");
				}
				else {
					_shader = renderer->loadShaderProgram("particle_imposter_square_flat",
							":/core/glsl/particles/imposter/sphere/without_depth_tri.vs",
							":/core/glsl/particles/pointsprites/square/flat_shading.fs");
					_pickingShader = renderer->loadShaderProgram("particle_imposter_square_flat_picking",
							":/core/glsl/particles/imposter/sphere/picking/without_depth_tri.vs",
							":/core/glsl/particles/pointsprites/square/picking/flat_shading.fs");
				}
			}
		}
		else if(shadingMode == NormalShading) {
			if(shape == SphericalShape) {
				if(renderingQuality == LowQuality) {
					if(_usingGeometryShader) {
						_shader = renderer->loadShaderProgram("particle_geomshader_imposter_spherical_shaded_nodepth",
								":/core/glsl/particles/imposter/sphere/without_depth.vs",
								":/core/glsl/particles/imposter/sphere/without_depth.fs",
								":/core/glsl/particles/imposter/sphere/without_depth.gs");
						_pickingShader = renderer->loadShaderProgram("particle_geomshader_imposter_spherical_nodepth_picking",
								":/core/glsl/particles/imposter/sphere/picking/without_depth.vs",
								":/core/glsl/particles/imposter/sphere/picking/flat_shading.fs",
								":/core/glsl/particles/imposter/sphere/picking/without_depth.gs");
					}
					else {
						_shader = renderer->loadShaderProgram("particle_imposter_spherical_shaded_nodepth",
								":/core/glsl/particles/imposter/sphere/without_depth_tri.vs",
								":/core/glsl/particles/imposter/sphere/without_depth.fs");
						_pickingShader = renderer->loadShaderProgram("particle_imposter_spherical_nodepth_picking",
								":/core/glsl/particles/imposter/sphere/picking/without_depth_tri.vs",
								":/core/glsl/particles/imposter/sphere/picking/flat_shading.fs");
					}
				}
				else if(renderingQuality == MediumQuality) {
					if(_usingGeometryShader) {
						_shader = renderer->loadShaderProgram("particle_geomshader_imposter_spherical_shaded_depth",
								":/core/glsl/particles/imposter/sphere/with_depth.vs",
								":/core/glsl/particles/imposter/sphere/with_depth.fs",
								":/core/glsl/particles/imposter/sphere/with_depth.gs");
						_pickingShader = renderer->loadShaderProgram("particle_geomshader_imposter_spherical_shaded_depth_picking",
								":/core/glsl/particles/imposter/sphere/picking/with_depth.vs",
								":/core/glsl/particles/imposter/sphere/picking/with_depth.fs",
								":/core/glsl/particles/imposter/sphere/picking/with_depth.gs");
					}
					else {
						_shader = renderer->loadShaderProgram("particle_imposter_spherical_shaded_depth",
								":/core/glsl/particles/imposter/sphere/with_depth_tri.vs",
								":/core/glsl/particles/imposter/sphere/with_depth.fs");
						_pickingShader = renderer->loadShaderProgram("particle_imposter_spherical_shaded_depth_picking",
								":/core/glsl/particles/imposter/sphere/picking/with_depth_tri.vs",
								":/core/glsl/particles/imposter/sphere/picking/with_depth.fs");
					}
				}
			}
		}
	}
	else if(_renderingTechnique == CUBE_GEOMETRY) {
		if(shadingMode == NormalShading) {
			if(_usingGeometryShader) {
				if(shape == SphericalShape && renderingQuality == HighQuality) {
					_shader = renderer->loadShaderProgram("particle_geomshader_sphere",
							":/core/glsl/particles/geometry/sphere/sphere.vs",
							":/core/glsl/particles/geometry/sphere/sphere.fs",
							":/core/glsl/particles/geometry/sphere/sphere.gs");
					_pickingShader = renderer->loadShaderProgram("particle_geomshader_sphere_picking",
							":/core/glsl/particles/geometry/sphere/picking/sphere.vs",
							":/core/glsl/particles/geometry/sphere/picking/sphere.fs",
							":/core/glsl/particles/geometry/sphere/picking/sphere.gs");
				}
				else if(shape == SquareShape) {
					_shader = renderer->loadShaderProgram("particle_geomshader_cube",
							":/core/glsl/particles/geometry/cube/cube.vs",
							":/core/glsl/particles/geometry/cube/cube.fs",
							":/core/glsl/particles/geometry/cube/cube.gs");
					_pickingShader = renderer->loadShaderProgram("particle_geomshader_cube_picking",
							":/core/glsl/particles/geometry/cube/picking/cube.vs",
							":/core/glsl/particles/geometry/cube/picking/cube.fs",
							":/core/glsl/particles/geometry/cube/picking/cube.gs");
				}
				else if(shape == BoxShape) {
					_shader = renderer->loadShaderProgram("particle_geomshader_box",
							":/core/glsl/particles/geometry/cube/box.vs",
							":/core/glsl/particles/geometry/cube/cube.fs",
							":/core/glsl/particles/geometry/cube/box.gs");
					_pickingShader = renderer->loadShaderProgram("particle_geomshader_box_picking",
							":/core/glsl/particles/geometry/cube/picking/box.vs",
							":/core/glsl/particles/geometry/cube/picking/cube.fs",
							":/core/glsl/particles/geometry/cube/picking/box.gs");
				}
			}
			else {
				if(shape == SphericalShape && renderingQuality == HighQuality) {
					_shader = renderer->loadShaderProgram("particle_tristrip_sphere",
							":/core/glsl/particles/geometry/sphere/sphere_tristrip.vs",
							":/core/glsl/particles/geometry/sphere/sphere.fs");
					_pickingShader = renderer->loadShaderProgram("particle_tristrip_sphere_picking",
							":/core/glsl/particles/geometry/sphere/picking/sphere_tristrip.vs",
							":/core/glsl/particles/geometry/sphere/picking/sphere.fs");
				}
				else if(shape == SquareShape) {
					_shader = renderer->loadShaderProgram("particle_tristrip_cube",
							":/core/glsl/particles/geometry/cube/cube_tristrip.vs",
							":/core/glsl/particles/geometry/cube/cube.fs");
					_pickingShader = renderer->loadShaderProgram("particle_tristrip_cube_picking",
							":/core/glsl/particles/geometry/cube/picking/cube_tristrip.vs",
							":/core/glsl/particles/geometry/cube/picking/cube.fs");
				}
				else if(shape == BoxShape) {
					_shader = renderer->loadShaderProgram("particle_tristrip_box",
							":/core/glsl/particles/geometry/cube/box_tristrip.vs",
							":/core/glsl/particles/geometry/cube/cube.fs");
					_pickingShader = renderer->loadShaderProgram("particle_tristrip_box_picking",
							":/core/glsl/particles/geometry/cube/picking/box_tristrip.vs",
							":/core/glsl/particles/geometry/cube/picking/cube.fs");
				}
			}
		}
	}
	OVITO_ASSERT(_shader != nullptr);
	OVITO_ASSERT(_pickingShader != nullptr);

	// Prepare texture that is required for imposter rendering of spherical particles.
	if(shape == SphericalShape && shadingMode == NormalShading && (_renderingTechnique == POINT_SPRITES || _renderingTechnique == IMPOSTER_QUADS))
		initializeBillboardTexture(renderer);
}

/******************************************************************************
* Allocates a particle buffer with the given number of particles.
******************************************************************************/
void OpenGLParticlePrimitive::setSize(int particleCount)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);

	_particleCount = particleCount;

	// Determine the required number of vertices that need to be sent to the graphics card per particle.
	int verticesPerParticle;
	if(_renderingTechnique == POINT_SPRITES) {
		verticesPerParticle = 1;
	}
	else if(_renderingTechnique == IMPOSTER_QUADS) {
		if(_usingGeometryShader)
			verticesPerParticle = 1;
		else
			verticesPerParticle = 6;
	}
	else if(_renderingTechnique == CUBE_GEOMETRY) {
		if(_usingGeometryShader)
			verticesPerParticle = 1;
		else
			verticesPerParticle = 14;
	}
	else OVITO_ASSERT(false);

	// Determine the VBO chunk size.
	int bytesPerVertex = sizeof(ColorA);
	_chunkSize = std::min(_maxVBOSize / verticesPerParticle / bytesPerVertex, particleCount);

	// Cannot use chunked VBOs when rendering semi-transparent particles,
	// because they will be rendered in arbitrary order.
	if(translucentParticles())
		_chunkSize = particleCount;

	// Allocate VBOs.
	int numChunks = particleCount ? ((particleCount + _chunkSize - 1) / _chunkSize) : 0;
	_positionsBuffers.resize(numChunks);
	_radiiBuffers.resize(numChunks);
	_colorsBuffers.resize(numChunks);
	if(particleShape() == BoxShape)
		_shapeBuffers.resize(numChunks);

	for(int i = 0; i < numChunks; i++) {
		int size = std::min(_chunkSize, particleCount - i * _chunkSize);
		_positionsBuffers[i].create(QOpenGLBuffer::StaticDraw, size, verticesPerParticle);
		_radiiBuffers[i].create(QOpenGLBuffer::StaticDraw, size, verticesPerParticle);
		_colorsBuffers[i].create(QOpenGLBuffer::StaticDraw, size, verticesPerParticle);
		if(particleShape() == BoxShape)
			_shapeBuffers[i].create(QOpenGLBuffer::StaticDraw, size, verticesPerParticle);
	}
}

/******************************************************************************
* Sets the coordinates of the particles.
******************************************************************************/
void OpenGLParticlePrimitive::setParticlePositions(const Point3* coordinates)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);

	// Make a copy of the particle coordinates. They will be needed when rendering
	// semi-transparent particles in the correct order from back to front.
	if(translucentParticles()) {
		_particleCoordinates.resize(particleCount());
		std::copy(coordinates, coordinates + particleCount(), _particleCoordinates.begin());
	}

	for(auto& buffer : _positionsBuffers) {
		buffer.fill(coordinates);
		coordinates += buffer.elementCount();
	}
}

/******************************************************************************
* Sets the radii of the particles.
******************************************************************************/
void OpenGLParticlePrimitive::setParticleRadii(const FloatType* radii)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	for(auto& buffer : _radiiBuffers) {
		buffer.fill(radii);
		radii += buffer.elementCount();
	}
}

/******************************************************************************
* Sets the radius of all particles to the given value.
******************************************************************************/
void OpenGLParticlePrimitive::setParticleRadius(FloatType radius)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	for(auto& buffer : _radiiBuffers)
		buffer.fillConstant(radius);
}

/******************************************************************************
* Sets the colors of the particles.
******************************************************************************/
void OpenGLParticlePrimitive::setParticleColors(const ColorA* colors)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	for(auto& buffer : _colorsBuffers) {
		buffer.fill(colors);
		colors += buffer.elementCount();
	}
}

/******************************************************************************
* Sets the colors of the particles.
******************************************************************************/
void OpenGLParticlePrimitive::setParticleColors(const Color* colors)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	// Need to convert array from Color to ColorA.
	for(auto& buffer : _colorsBuffers) {
		ColorA* dest = buffer.map(QOpenGLBuffer::WriteOnly);
		const Color* end_colors = colors + buffer.elementCount();
		for(; colors != end_colors; ++colors) {
			for(int i = 0; i < buffer.verticesPerElement(); i++, ++dest) {
				*dest = *colors;
			}
		}
		buffer.unmap();
	}
}

/******************************************************************************
* Sets the color of all particles to the given value.
******************************************************************************/
void OpenGLParticlePrimitive::setParticleColor(const ColorA color)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	for(auto& buffer : _colorsBuffers)
		buffer.fillConstant(color);
}

/******************************************************************************
* Sets the aspherical shapes of the particles.
******************************************************************************/
void OpenGLParticlePrimitive::setParticleShapes(const Vector3* shapes)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	if(!_shapeBuffers.empty()) {
		for(auto& buffer : _shapeBuffers) {
			buffer.fill(shapes);
			shapes += buffer.elementCount();
		}
	}
	else {
		std::vector<FloatType> radii(particleCount());
		for(FloatType& r : radii) {
			r = (shapes->x() + shapes->y() + shapes->z()) / 3.0;
			shapes++;
		}
		setParticleRadii(radii.data());
	}
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool OpenGLParticlePrimitive::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);
	if(!vpRenderer) return false;
	return (_particleCount >= 0) && (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void OpenGLParticlePrimitive::render(SceneRenderer* renderer)
{
    OVITO_REPORT_OPENGL_ERRORS();
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_STATIC_ASSERT(sizeof(FloatType) == 4);
	OVITO_STATIC_ASSERT(sizeof(Color) == 12);
	OVITO_STATIC_ASSERT(sizeof(ColorA) == 16);

	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(particleCount() <= 0 || !vpRenderer)
		return;

	// If object is translucent, don't render it during the first rendering pass.
	// Queue primitive so that it gets rendered during the second pass.
	if(!renderer->isPicking() && translucentParticles() && vpRenderer->translucentPass() == false) {
		vpRenderer->registerTranslucentPrimitive(shared_from_this());
		return;
	}

	vpRenderer->rebindVAO();

	if(_renderingTechnique == POINT_SPRITES)
		renderPointSprites(vpRenderer);
	else if(_renderingTechnique == IMPOSTER_QUADS)
		renderImposters(vpRenderer);
	else if(_renderingTechnique == CUBE_GEOMETRY)
		renderCubes(vpRenderer);
}

/******************************************************************************
* Renders the particles using OpenGL point sprites.
******************************************************************************/
void OpenGLParticlePrimitive::renderPointSprites(ViewportSceneRenderer* renderer)
{
	OVITO_ASSERT(!_positionsBuffers.empty());
	OVITO_ASSERT(_positionsBuffers.front().verticesPerElement() == 1);

	// Let the vertex shader compute the point size.
	OVITO_CHECK_OPENGL(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));

	// Enable point sprites when using the compatibility OpenGL profile.
	// In the core profile, they are already enabled by default.
	if(renderer->glformat().profile() != QSurfaceFormat::CoreProfile) {
		OVITO_CHECK_OPENGL(glEnable(GL_POINT_SPRITE));

		// Specify point sprite texture coordinate replacement mode.
		glTexEnvf(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	}

	if(particleShape() == SphericalShape && shadingMode() == NormalShading && !renderer->isPicking())
		activateBillboardTexture(renderer);

	// Pick the right OpenGL shader program.
	QOpenGLShaderProgram* shader = renderer->isPicking() ? _pickingShader : _shader;
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

	if(!renderer->isPicking() && translucentParticles()) {
		glEnable(GL_BLEND);
		renderer->glfuncs()->glBlendEquation(GL_FUNC_ADD);
		renderer->glfuncs()->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	}

	GLint pickingBaseID;
	if(renderer->isPicking()) {
		pickingBaseID = renderer->registerSubObjectIDs(particleCount());
		renderer->activateVertexIDs(shader, _chunkSize);
	}

	for(size_t chunkIndex = 0; chunkIndex < _positionsBuffers.size(); chunkIndex++, pickingBaseID += _chunkSize) {
		int chunkSize = _positionsBuffers[chunkIndex].elementCount();
		_positionsBuffers[chunkIndex].bindPositions(renderer, shader);
		_radiiBuffers[chunkIndex].bind(renderer, shader, "particle_radius", GL_FLOAT, 0, 1);
		if(!renderer->isPicking())
			_colorsBuffers[chunkIndex].bindColors(renderer, shader, 4);
		else
			shader->setUniformValue("pickingBaseID", pickingBaseID);

		// Are we rendering translucent particles? If yes, render them in back to front order to avoid visual artifacts at overlapping particles.
		if(!renderer->isPicking() && translucentParticles() && !_particleCoordinates.empty()) {
			// Create temporary OpenGL index buffer which can be used with glDrawElements to draw particles in desired order.
			OpenGLBuffer<GLuint> primitiveIndices(QOpenGLBuffer::IndexBuffer);
			primitiveIndices.create(QOpenGLBuffer::StaticDraw, particleCount());
			primitiveIndices.fill(determineRenderingOrder(renderer).data());
			primitiveIndices.oglBuffer().bind();
			OVITO_CHECK_OPENGL(glDrawElements(GL_POINTS, particleCount(), GL_UNSIGNED_INT, nullptr));
			primitiveIndices.oglBuffer().release();
		}
		else {
			// By default, render particles in arbitrary order.
			OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, chunkSize));
		}

		_positionsBuffers[chunkIndex].detachPositions(renderer, shader);
		_radiiBuffers[chunkIndex].detach(renderer, shader, "particle_radius");
		if(!renderer->isPicking())
			_colorsBuffers[chunkIndex].detachColors(renderer, shader);
	}
	if(renderer->isPicking())
		renderer->deactivateVertexIDs(shader);

	shader->release();

	OVITO_CHECK_OPENGL(glDisable(GL_VERTEX_PROGRAM_POINT_SIZE));
	if(!renderer->isPicking() && translucentParticles()) {
		glDisable(GL_BLEND);
	}

	// Disable point sprites again.
	if(renderer->glformat().profile() != QSurfaceFormat::CoreProfile)
		OVITO_CHECK_OPENGL(glDisable(GL_POINT_SPRITE));

	if(particleShape() == SphericalShape && shadingMode() == NormalShading && !renderer->isPicking())
		deactivateBillboardTexture(renderer);
}

/******************************************************************************
* Renders a cube for each particle using triangle strips.
******************************************************************************/
void OpenGLParticlePrimitive::renderCubes(ViewportSceneRenderer* renderer)
{
	int verticesPerElement = _positionsBuffers.front().verticesPerElement();
	OVITO_ASSERT(!_usingGeometryShader || verticesPerElement == 1);
	OVITO_ASSERT(_usingGeometryShader || verticesPerElement == 14);

	// Pick the right OpenGL shader program.
	QOpenGLShaderProgram* shader = renderer->isPicking() ? _pickingShader : _shader;
	if(!shader->bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL shader program."));

	// Need to render only the front facing sides of the cubes.
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	if(!_usingGeometryShader) {
		// This is to draw the cube with a single triangle strip.
		// The cube vertices:
		static const QVector3D cubeVerts[14] = {
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
		OVITO_CHECK_OPENGL(shader->setUniformValueArray("cubeVerts", cubeVerts, 14));
	}

	if(particleShape() != SphericalShape && !renderer->isPicking()) {
		shader->setUniformValue("normal_matrix", (QMatrix3x3)(renderer->modelViewTM().linear().inverse().transposed()));
		if(!_usingGeometryShader) {
			// The normal vectors for the cube triangle strip.
			static const QVector3D normals[14] = {
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
			OVITO_CHECK_OPENGL(shader->setUniformValueArray("normals", normals, 14));
		}
	}

	shader->setUniformValue("projection_matrix", (QMatrix4x4)renderer->projParams().projectionMatrix);
	shader->setUniformValue("inverse_projection_matrix", (QMatrix4x4)renderer->projParams().inverseProjectionMatrix);
	shader->setUniformValue("modelview_matrix", (QMatrix4x4)renderer->modelViewTM());
	shader->setUniformValue("modelviewprojection_matrix", (QMatrix4x4)(renderer->projParams().projectionMatrix * renderer->modelViewTM()));
	shader->setUniformValue("is_perspective", renderer->projParams().isPerspective);

	GLint viewportCoords[4];
	glGetIntegerv(GL_VIEWPORT, viewportCoords);
	shader->setUniformValue("viewport_origin", (float)viewportCoords[0], (float)viewportCoords[1]);
	shader->setUniformValue("inverse_viewport_size", 2.0f / (float)viewportCoords[2], 2.0f / (float)viewportCoords[3]);

	if(!renderer->isPicking() && translucentParticles()) {
		glEnable(GL_BLEND);
		renderer->glfuncs()->glBlendEquation(GL_FUNC_ADD);
		renderer->glfuncs()->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	}

	GLint pickingBaseID;
	if(renderer->isPicking()) {
		pickingBaseID = renderer->registerSubObjectIDs(particleCount());
	}

	for(size_t chunkIndex = 0; chunkIndex < _positionsBuffers.size(); chunkIndex++, pickingBaseID += _chunkSize) {
		int chunkSize = _positionsBuffers[chunkIndex].elementCount();

		_positionsBuffers[chunkIndex].bindPositions(renderer, shader);
		if(particleShape() == BoxShape) {
			_shapeBuffers[chunkIndex].bind(renderer, shader, "shape", GL_FLOAT, 0, 3);
		}
		else {
			_radiiBuffers[chunkIndex].bind(renderer, shader, "particle_radius", GL_FLOAT, 0, 1);
		}
		if(!renderer->isPicking()) {
			_colorsBuffers[chunkIndex].bindColors(renderer, shader, 4);
		}
		else {
			shader->setUniformValue("pickingBaseID", pickingBaseID);
			renderer->activateVertexIDs(shader, _positionsBuffers[chunkIndex].elementCount() * verticesPerElement);
		}

		if(_usingGeometryShader) {
			// Are we rendering translucent particles? If yes, render them in back to front order to avoid visual artifacts at overlapping particles.
			if(!renderer->isPicking() && translucentParticles() && !_particleCoordinates.empty()) {
				// Create OpenGL index buffer which can be used with glDrawElements.
				OpenGLBuffer<GLuint> primitiveIndices(QOpenGLBuffer::IndexBuffer);
				primitiveIndices.create(QOpenGLBuffer::StaticDraw, particleCount());
				primitiveIndices.fill(determineRenderingOrder(renderer).data());
				primitiveIndices.oglBuffer().bind();
				OVITO_CHECK_OPENGL(glDrawElements(GL_POINTS, particleCount(), GL_UNSIGNED_INT, nullptr));
				primitiveIndices.oglBuffer().release();
			}
			else {
				// By default, render particles in arbitrary order.
				OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, chunkSize));
			}
		}
		else {
			// Prepare arrays required for glMultiDrawArrays().

			// Are we rendering translucent particles? If yes, render them in back to front order to avoid visual artifacts at overlapping particles.
			if(!renderer->isPicking() && translucentParticles() && !_particleCoordinates.empty()) {
				auto indices = determineRenderingOrder(renderer);
				_primitiveStartIndices.resize(particleCount());
				std::transform(indices.begin(), indices.end(), _primitiveStartIndices.begin(), [verticesPerElement](GLuint i) { return i*verticesPerElement; });
				if(_primitiveVertexCounts.size() != particleCount()) {
					_primitiveVertexCounts.resize(particleCount());
					std::fill(_primitiveVertexCounts.begin(), _primitiveVertexCounts.end(), verticesPerElement);
				}
			}
			else if(_primitiveStartIndices.size() < chunkSize) {
				_primitiveStartIndices.resize(chunkSize);
				_primitiveVertexCounts.resize(chunkSize);
				GLint index = 0;
				for(GLint& s : _primitiveStartIndices) {
					s = index;
					index += verticesPerElement;
				}
				std::fill(_primitiveVertexCounts.begin(), _primitiveVertexCounts.end(), verticesPerElement);
			}

			renderer->activateVertexIDs(shader, chunkSize * _positionsBuffers[chunkIndex].verticesPerElement(), renderer->isPicking());

			OVITO_CHECK_OPENGL(renderer->glMultiDrawArrays(GL_TRIANGLE_STRIP,
					_primitiveStartIndices.data(),
					_primitiveVertexCounts.data(),
					chunkSize));

			renderer->deactivateVertexIDs(shader, renderer->isPicking());
		}

		_positionsBuffers[chunkIndex].detachPositions(renderer, shader);
		if(!renderer->isPicking())
			_colorsBuffers[chunkIndex].detachColors(renderer, shader);
		if(particleShape() == BoxShape) {
			_shapeBuffers[chunkIndex].detach(renderer, shader, "shape");
		}
		else {
			_radiiBuffers[chunkIndex].detach(renderer, shader, "particle_radius");
		}
	}

	if(!renderer->isPicking())
		glDisable(GL_BLEND);
	else
		renderer->deactivateVertexIDs(shader);

	shader->release();
}

/******************************************************************************
* Renders particles using quads.
******************************************************************************/
void OpenGLParticlePrimitive::renderImposters(ViewportSceneRenderer* renderer)
{
	int verticesPerElement = _positionsBuffers.front().verticesPerElement();

	// Pick the right OpenGL shader program.
	QOpenGLShaderProgram* shader = renderer->isPicking() ? _pickingShader : _shader;
	if(!shader->bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL shader program."));

	if(particleShape() == SphericalShape && shadingMode() == NormalShading && !renderer->isPicking())
		activateBillboardTexture(renderer);

	// Need to render only the front facing sides.
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	if(!_usingGeometryShader) {
		// The texture coordinates of a quad made of two triangles.
		static const QVector2D texcoords[6] = {{0,1},{1,1},{1,0},{0,1},{1,0},{0,0}};
		OVITO_CHECK_OPENGL(shader->setUniformValueArray("imposter_texcoords", texcoords, 6));

		// The coordinate offsets of the six vertices of a quad made of two triangles.
		static const QVector4D voffsets[6] = {{-1,-1,0,0},{1,-1,0,0},{1,1,0,0},{-1,-1,0,0},{1,1,0,0},{-1,1,0,0}};
		OVITO_CHECK_OPENGL(shader->setUniformValueArray("imposter_voffsets", voffsets, 6));
	}

	shader->setUniformValue("projection_matrix", (QMatrix4x4)renderer->projParams().projectionMatrix);
	shader->setUniformValue("modelview_matrix", (QMatrix4x4)renderer->modelViewTM());
	shader->setUniformValue("modelviewprojection_matrix", (QMatrix4x4)(renderer->projParams().projectionMatrix * renderer->modelViewTM()));

	if(!renderer->isPicking() && translucentParticles()) {
		glEnable(GL_BLEND);
		renderer->glfuncs()->glBlendEquation(GL_FUNC_ADD);
		renderer->glfuncs()->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	}

	GLint pickingBaseID;
	if(renderer->isPicking()) {
		pickingBaseID = renderer->registerSubObjectIDs(particleCount());
		renderer->activateVertexIDs(shader, _chunkSize);
	}

	for(size_t chunkIndex = 0; chunkIndex < _positionsBuffers.size(); chunkIndex++, pickingBaseID += _chunkSize) {
		int chunkSize = _positionsBuffers[chunkIndex].elementCount();

		_positionsBuffers[chunkIndex].bindPositions(renderer, shader);
		_radiiBuffers[chunkIndex].bind(renderer, shader, "particle_radius", GL_FLOAT, 0, 1);
		if(!renderer->isPicking()) {
			_colorsBuffers[chunkIndex].bindColors(renderer, shader, 4);
		}
		else {
			shader->setUniformValue("pickingBaseID", pickingBaseID);
		}

		renderer->activateVertexIDs(shader, _positionsBuffers[chunkIndex].elementCount() * verticesPerElement);

		if(_usingGeometryShader) {
			OVITO_ASSERT(verticesPerElement == 1);
			// Are we rendering translucent particles? If yes, render them in back to front order to avoid visual artifacts at overlapping particles.
			if(!renderer->isPicking() && translucentParticles() && !_particleCoordinates.empty()) {
				// Create OpenGL index buffer which can be used with glDrawElements.
				OpenGLBuffer<GLuint> primitiveIndices(QOpenGLBuffer::IndexBuffer);
				primitiveIndices.create(QOpenGLBuffer::StaticDraw, particleCount());
				primitiveIndices.fill(determineRenderingOrder(renderer).data());
				primitiveIndices.oglBuffer().bind();
				OVITO_CHECK_OPENGL(glDrawElements(GL_POINTS, particleCount(), GL_UNSIGNED_INT, nullptr));
				primitiveIndices.oglBuffer().release();
			}
			else {
				// By default, render particles in arbitrary order.
				OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, chunkSize));
			}
		}
		else {
			OVITO_ASSERT(verticesPerElement == 6);
			// Are we rendering translucent particles? If yes, render them in back to front order to avoid visual artifacts at overlapping particles.
			if(!renderer->isPicking() && translucentParticles() && !_particleCoordinates.empty()) {
				auto indices = determineRenderingOrder(renderer);
				// Create OpenGL index buffer which can be used with glDrawElements.
				OpenGLBuffer<GLuint> primitiveIndices(QOpenGLBuffer::IndexBuffer);
				primitiveIndices.create(QOpenGLBuffer::StaticDraw, verticesPerElement * particleCount());
				GLuint* p = primitiveIndices.map(QOpenGLBuffer::WriteOnly);
				for(size_t i = 0; i < indices.size(); i++, p += verticesPerElement)
					std::iota(p, p + verticesPerElement, indices[i]*verticesPerElement);
				primitiveIndices.unmap();
				primitiveIndices.oglBuffer().bind();
				OVITO_CHECK_OPENGL(glDrawElements(GL_TRIANGLES, particleCount() * verticesPerElement, GL_UNSIGNED_INT, nullptr));
				primitiveIndices.oglBuffer().release();
			}
			else {
				// By default, render particles in arbitrary order.
				OVITO_CHECK_OPENGL(glDrawArrays(GL_TRIANGLES, 0, chunkSize * verticesPerElement));
			}
		}

		_positionsBuffers[chunkIndex].detachPositions(renderer, shader);
		_radiiBuffers[chunkIndex].detach(renderer, shader, "particle_radius");
		if(!renderer->isPicking())
			_colorsBuffers[chunkIndex].detachColors(renderer, shader);
	}

	renderer->deactivateVertexIDs(shader);
	shader->release();

	if(!renderer->isPicking() && translucentParticles())
		glDisable(GL_BLEND);

	if(particleShape() == SphericalShape && shadingMode() == NormalShading && !renderer->isPicking())
		deactivateBillboardTexture(renderer);
}

/******************************************************************************
* Creates the textures used for billboard rendering of particles.
******************************************************************************/
void OpenGLParticlePrimitive::initializeBillboardTexture(ViewportSceneRenderer* renderer)
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
                    Vector2 r((FloatType(x - resolution/2) + 0.5f) / (resolution/2), (FloatType(y - resolution/2) + 0.5f) / (resolution/2));
					FloatType r2 = r.squaredLength();
					FloatType r2_clamped = std::min(r2, FloatType(1));
                    FloatType diffuse_brightness = sqrt(1 - r2_clamped) * 0.6f + 0.4f;

					textureImages[mipmapLevel][pixelOffset][0] =
                            (GLubyte)(std::min(diffuse_brightness, (FloatType)1.0f) * 255.0f);

					textureImages[mipmapLevel][pixelOffset][2] = 255;
					textureImages[mipmapLevel][pixelOffset][3] = 255;

                    if(r2 < 1.0f) {
						// Store specular brightness in alpha channel of texture.
                        Vector2 sr = r + Vector2(0.6883f, 0.982f);
						FloatType specular = std::max(FloatType(1) - sr.squaredLength(), FloatType(0));
						specular *= specular;
						specular *= specular * (1 - r2_clamped*r2_clamped);
						textureImages[mipmapLevel][pixelOffset][1] =
                                (GLubyte)(std::min(specular, FloatType(1)) * 255.0f);
					}
					else {
						// Set transparent pixel.
						textureImages[mipmapLevel][pixelOffset][1] = 0;
					}
				}
			}
		}
	}

	_billboardTexture.create();
	_billboardTexture.bind();

	// Transfer pixel data to OpenGL texture.
	for(int mipmapLevel = 0; mipmapLevel < BILLBOARD_TEXTURE_LEVELS; mipmapLevel++) {
		int resolution = (1 << (BILLBOARD_TEXTURE_LEVELS - mipmapLevel - 1));

		OVITO_CHECK_OPENGL(glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGBA,
				resolution, resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImages[mipmapLevel].data()));
	}
}

/******************************************************************************
* Activates a texture for billboard rendering of spherical particles.
******************************************************************************/
void OpenGLParticlePrimitive::activateBillboardTexture(ViewportSceneRenderer* renderer)
{
	OVITO_ASSERT(_billboardTexture.isCreated());
	OVITO_ASSERT(shadingMode() != FlatShading);
	OVITO_ASSERT(!renderer->isPicking());
	OVITO_ASSERT(particleShape() == SphericalShape);

	// Enable texture mapping when using compatibility OpenGL.
	// In the core profile, this is already enabled by default.
	if(renderer->isCoreProfile() == false)
		OVITO_CHECK_OPENGL(glEnable(GL_TEXTURE_2D));

	_billboardTexture.bind();

	OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
	OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	OVITO_STATIC_ASSERT(BILLBOARD_TEXTURE_LEVELS >= 3);
	OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, BILLBOARD_TEXTURE_LEVELS - 3));
}

/******************************************************************************
* Deactivates the texture used for billboard rendering of spherical particles.
******************************************************************************/
void OpenGLParticlePrimitive::deactivateBillboardTexture(ViewportSceneRenderer* renderer)
{
	// Disable texture mapping again when not using core profile.
	if(renderer->isCoreProfile() == false)
		OVITO_CHECK_OPENGL(glDisable(GL_TEXTURE_2D));
}

/******************************************************************************
* Returns an array of particle indices, sorted back-to-front, which is used
* to render translucent particles.
******************************************************************************/
std::vector<GLuint> OpenGLParticlePrimitive::determineRenderingOrder(ViewportSceneRenderer* renderer)
{
	// Create array of particle indices.
	std::vector<GLuint> indices(particleCount());
	std::iota(indices.begin(), indices.end(), 0);
	if(!_particleCoordinates.empty()) {
		// Viewing direction in object space:
		Vector3 direction = renderer->modelViewTM().inverse().column(2);

		OVITO_ASSERT(_particleCoordinates.size() == particleCount());
		// First compute distance of each particle from the camera along viewing direction (=camera z-axis).
		std::vector<FloatType> distances(particleCount());
		std::transform(_particleCoordinates.begin(), _particleCoordinates.end(), distances.begin(), [direction](const Point3& p) {
			return direction.dot(p - Point3::Origin());
		});
		// Now sort particle indices with respect to distance (back-to-front order).
		std::sort(indices.begin(), indices.end(), [&distances](GLuint a, GLuint b) {
			return distances[a] < distances[b];
		});
	}
	return indices;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
