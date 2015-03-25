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

#ifndef __OVITO_OPENGL_PARTICLE_PRIMITIVE_H
#define __OVITO_OPENGL_PARTICLE_PRIMITIVE_H

#include <core/Core.h>
#include <core/rendering/ParticlePrimitive.h>
#include "OpenGLBuffer.h"
#include "OpenGLTexture.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief This class is responsible for rendering particle primitives using OpenGL.
 */
class OVITO_CORE_EXPORT OpenGLParticlePrimitive : public ParticlePrimitive, public std::enable_shared_from_this<OpenGLParticlePrimitive>
{
public:

	/// Constructor.
	OpenGLParticlePrimitive(ViewportSceneRenderer* renderer,
			ShadingMode shadingMode, RenderingQuality renderingQuality, ParticleShape shape, bool translucentParticles);

	/// \brief Allocates a geometry buffer with the given number of particles.
	virtual void setSize(int particleCount) override;

	/// \brief Returns the number of particles stored in the buffer.
	virtual int particleCount() const override { return _particleCount; }

	/// \brief Sets the coordinates of the particles.
	virtual void setParticlePositions(const Point3* positions) override;

	/// \brief Sets the radii of the particles.
	virtual void setParticleRadii(const FloatType* radii) override;

	/// \brief Sets the radius of all particles to the given value.
	virtual void setParticleRadius(FloatType radius) override;

	/// \brief Sets the colors of the particles.
	virtual void setParticleColors(const ColorA* colors) override;

	/// \brief Sets the colors of the particles.
	virtual void setParticleColors(const Color* colors) override;

	/// \brief Sets the color of all particles to the given value.
	virtual void setParticleColor(const ColorA color) override;

	/// \brief Sets the aspherical shapes of the particles.
	virtual void setParticleShapes(const Vector3* shapes) override;

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer) override;

	/// \brief Changes the shading mode for particles.
	virtual bool setShadingMode(ShadingMode mode) override { return (mode == shadingMode()); }

	/// \brief Changes the rendering quality of particles.
	virtual bool setRenderingQuality(RenderingQuality level) override { return (level == renderingQuality()); }

	/// \brief Changes the display shape of particles.
	virtual bool setParticleShape(ParticleShape shape) override { return (shape == particleShape()); }

protected:

	/// Creates the texture used for billboard rendering of particles.
	void initializeBillboardTexture(ViewportSceneRenderer* renderer);

	/// Activates a texture for billboard rendering of particles.
	void activateBillboardTexture(ViewportSceneRenderer* renderer);

	/// Deactivates the texture used for billboard rendering of spherical particles.
	void deactivateBillboardTexture(ViewportSceneRenderer* renderer);

	/// Renders the particles using OpenGL point sprites.
	void renderPointSprites(ViewportSceneRenderer* renderer);

	/// Renders a cube for each particle using triangle strips.
	void renderCubes(ViewportSceneRenderer* renderer);

	/// Renders the particles using quads.
	void renderImposters(ViewportSceneRenderer* renderer);

	/// Returns an array of particle indices, sorted back-to-front, which is used to render translucent particles.
	std::vector<GLuint> determineRenderingOrder(ViewportSceneRenderer* renderer);

private:

	/// The available techniques for rendering particles.
	enum RenderingTechnique {
		POINT_SPRITES,	///< Use OpenGL point sprites to render imposter quads with a texture map.
		IMPOSTER_QUADS,	///< Render explicit quad geometry made of two triangles.
		CUBE_GEOMETRY	///< Render a cube for each particle (possibly using a raytracing fragment shader to make it look spherical).
	};

	/// The maximum size (in bytes) of a single VBO buffer.
	int _maxVBOSize;

	/// The maximum number of render elements per VBO buffer.
	int _chunkSize;

	/// The number of particles stored in the class.
	int _particleCount;

	/// The internal OpenGL vertex buffer that stores the particle positions.
	std::vector<OpenGLBuffer<Point3>> _positionsBuffers;

	/// The internal OpenGL vertex buffer that stores the particle radii.
	std::vector<OpenGLBuffer<FloatType>> _radiiBuffers;

	/// The internal OpenGL vertex buffer that stores the particle colors.
	std::vector<OpenGLBuffer<ColorA>> _colorsBuffers;

	/// The internal OpenGL vertex buffer that stores the shape of aspherical particles.
	std::vector<OpenGLBuffer<Vector3>> _shapeBuffers;

	/// The GL context group under which the GL vertex buffers have been created.
	QPointer<QOpenGLContextGroup> _contextGroup;

	/// The OpenGL texture that is used for billboard rendering of particles.
	OpenGLTexture _billboardTexture;

	/// This array contains the start indices of primitives and is passed to glMultiDrawArrays().
	std::vector<GLint> _primitiveStartIndices;

	/// This array contains the vertex counts of primitives and is passed to glMultiDrawArrays().
	std::vector<GLsizei> _primitiveVertexCounts;

	/// The OpenGL shader program that is used to render the particles.
	QOpenGLShaderProgram* _shader;

	/// The OpenGL shader program that is used to render the particles in picking mode.
	QOpenGLShaderProgram* _pickingShader;

	/// The technique used to render particles. This depends on settings such as rendering quality, shading etc.
	RenderingTechnique _renderingTechnique;

	/// Indicates that an OpenGL geometry shader is being used.
	bool _usingGeometryShader;

	/// A copy of the particle coordinates. This is only required to render translucent
	/// particles in the correct order from back to front.
	std::vector<Point3> _particleCoordinates;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_OPENGL_PARTICLE_PRIMITIVE_H
