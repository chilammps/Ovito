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

/**
 * \file ViewportParticleGeometryBuffer.h
 * \brief Contains the definition of the Ovito::ViewportParticleGeometryBuffer class.
 */

#ifndef __OVITO_VIEWPORT_PARTICLE_GEOMETRY_BUFFER_H
#define __OVITO_VIEWPORT_PARTICLE_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/ParticleGeometryBuffer.h>
#include <core/utilities/opengl/SharedOpenGLResource.h>

namespace Ovito {

/**
 * \brief Buffer object that stores a set of particles to be rendered in the viewports.
 */
class OVITO_CORE_EXPORT ViewportParticleGeometryBuffer : public ParticleGeometryBuffer, private SharedOpenGLResource
{
public:

	/// Constructor.
	ViewportParticleGeometryBuffer(ViewportSceneRenderer* renderer,
			ShadingMode shadingMode, RenderingQuality renderingQuality, ParticleShape shape);

	/// Destructor.
	virtual ~ViewportParticleGeometryBuffer();

	/// \brief Allocates a geometry buffer with the given number of particles.
	virtual void setSize(int particleCount) override;

	/// \brief Returns the number of particles stored in the buffer.
	virtual int particleCount() const override { return _particleCount; }

	/// \brief Sets the coordinates of the particles.
	virtual void setParticlePositions(const Point3* coordinates) override;

	/// \brief Sets the radii of the particles.
	virtual void setParticleRadii(const FloatType* radii) override;

	/// \brief Sets the radius of all particles to the given value.
	virtual void setParticleRadius(FloatType radius) override;

	/// \brief Sets the colors of the particles.
	virtual void setParticleColors(const Color* colors) override;

	/// \brief Sets the color of all particles to the given value.
	virtual void setParticleColor(const Color color) override;

	/// \brief Sets the transparency of the particles.
	virtual void setParticleTransparencies(const FloatType* transparencies) override { /* Not supported by OpenGL renderer. */ }

	/// \brief Sets the transparency of all particles to the given value.
	virtual void setParticleTransparency(FloatType transparency) override { /* Not supported by OpenGL renderer. */ }

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

    /// This method that takes care of freeing the shared OpenGL resources owned by this class.
    virtual void freeOpenGLResources() override;

	/// Creates the texture used for billboard rendering of particles.
	void initializeBillboardTexture(ViewportSceneRenderer* renderer);

	/// Activates a texture for billboard rendering of particles.
	void activateBillboardTexture(ViewportSceneRenderer* renderer);

	/// Deactivates the texture used for billboard rendering of spherical particles.
	void deactivateBillboardTexture(ViewportSceneRenderer* renderer);

	/// Binds the vertex buffer containing the particle positions to the corresponding
	/// shader input attribute.
	void bindParticlePositionBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader);

	/// Binds the vertex buffer containing the particle colors to the corresponding
	/// shader input attribute.
	void bindParticleColorBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader);

	/// Binds the vertex buffer containing the particle radii to the corresponding
	/// shader input attribute.
	void bindParticleRadiusBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader);

	/// Detaches the vertex buffer containing the particle positions from the corresponding
	/// shader input attribute.
	void detachParticlePositionBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader);

	/// Detaches the vertex buffer containing the particle colors from the corresponding
	/// shader input attribute.
	void detachParticleColorBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader);

	/// Detaches the vertex buffer containing the particle radii from the corresponding
	/// shader input attribute.
	void detachParticleRadiusBuffer(ViewportSceneRenderer* renderer, QOpenGLShaderProgram* shader);

	/// Renders the particles using OpenGL point sprites.
	void renderPointSprites(ViewportSceneRenderer* renderer);

	/// Renders a cube for each particle using triangle strips.
	void renderCubes(ViewportSceneRenderer* renderer);

	/// Returns true if the OpenGL implementation supports geometry shaders.
	bool hasGeometryShaders() const { return _raytracedSphereShader != nullptr; }

private:

	/// The internal OpenGL vertex buffer that stores the particle positions.
	QOpenGLBuffer _glPositionsBuffer;

	/// The internal OpenGL vertex buffer that stores the particle radii.
	QOpenGLBuffer _glRadiiBuffer;

	/// The internal OpenGL vertex buffer that stores the particle colors.
	QOpenGLBuffer _glColorsBuffer;

	/// The GL context group under which the GL vertex buffers have been created.
	QPointer<QOpenGLContextGroup> _contextGroup;

	/// The number of particles stored in the buffers.
	int _particleCount;

	/// The number of vertices per particles stored in the buffers.
	int _verticesPerParticle;

	/// Resource identifier of the OpenGL texture that is used for billboard rendering of particles.
	GLuint _billboardTexture;

	/// This array contains the start indices of primitives and is passed to glMultiDrawArrays().
	std::vector<GLint> _primitiveStartIndices;

	/// This array contains the vertex counts of primitives and is passed to glMultiDrawArrays().
	std::vector<GLsizei> _primitiveVertexCounts;

	/// The OpenGL shader programs that are used to render the particles.
	QPointer<QOpenGLShaderProgram> _flatImposterShader;
	QPointer<QOpenGLShaderProgram> _shadedImposterShaderWithoutDepth;
	QPointer<QOpenGLShaderProgram> _shadedImposterShaderWithDepth;
	QPointer<QOpenGLShaderProgram> _raytracedSphereShader;
	QPointer<QOpenGLShaderProgram> _raytracedSphereTristripShader;
	QPointer<QOpenGLShaderProgram> _raytracedSphereTristripPickingShader;
	QPointer<QOpenGLShaderProgram> _cubeShader;
	QPointer<QOpenGLShaderProgram> _cubePickingShader;
	QPointer<QOpenGLShaderProgram> _cubeTristripShader;
	QPointer<QOpenGLShaderProgram> _cubeTristripPickingShader;
	QPointer<QOpenGLShaderProgram> _flatSquareImposterShader;

	QPointer<QOpenGLShaderProgram> _imposterPickingShaderWithoutDepth;
	QPointer<QOpenGLShaderProgram> _imposterPickingShaderWithDepth;
	QPointer<QOpenGLShaderProgram> _raytracedPickingSphereShader;
	QPointer<QOpenGLShaderProgram> _imposterSquarePickingShaderWithoutDepth;
};

};

#endif // __OVITO_VIEWPORT_PARTICLE_GEOMETRY_BUFFER_H
