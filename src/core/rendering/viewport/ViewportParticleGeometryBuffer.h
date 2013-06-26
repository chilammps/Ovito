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

class ViewportSceneRenderer;

/**
 * \brief Buffer object that stores a set of particles to be rendered in the viewports.
 */
class ViewportParticleGeometryBuffer : public ParticleGeometryBuffer, private SharedOpenGLResource
{
protected:

	/// Constructor.
	ViewportParticleGeometryBuffer(ViewportSceneRenderer* renderer);

public:

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

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render() override;

	/// \brief Returns the renderer that created this buffer object.
	ViewportSceneRenderer* renderer() const { return _renderer; }

protected:

    /// This method that takes care of freeing the shared OpenGL resources owned by this class.
    virtual void freeOpenGLResources() override;

	/// Creates the texture used for billboard rendering of particles.
	void initializeBillboardTexture();

	/// Activates a texture for billboard rendering of particles.
	void activateBillboardTexture();

	/// Renders the particles using OpenGL point sprites.
	void renderPointSprites();

	/// Renders the particles using raytracing implemented in an OpenGL fragment shader.
	void renderRaytracedSpheres();

private:

	/// The renderer that created this buffer object.
	ViewportSceneRenderer* _renderer;

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

	/// Resource identifier of the OpenGL texture that is used for billboard rendering of particles.
	GLuint _billboardTexture;

	/// The OpenGL shader programs that are used to render the particles.
	QOpenGLShaderProgram* _flatImposterShader;
	QOpenGLShaderProgram* _shadedImposterShaderWithoutDepth;
	QOpenGLShaderProgram* _shadedImposterShaderWithDepth;
	QOpenGLShaderProgram* _raytracedSphereShader;

	Q_OBJECT
	OVITO_OBJECT

	friend class ViewportSceneRenderer;
};

};

#endif // __OVITO_VIEWPORT_PARTICLE_GEOMETRY_BUFFER_H
