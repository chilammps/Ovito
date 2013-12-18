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
 * \file ParticleGeometryBuffer.h
 * \brief Contains the definition of the Ovito::ParticleGeometryBuffer class.
 */

#ifndef __OVITO_PARTICLE_GEOMETRY_BUFFER_H
#define __OVITO_PARTICLE_GEOMETRY_BUFFER_H

#include <core/Core.h>

namespace Ovito {

/**
 * \brief Abstract base class for buffer objects that store point-like particles.
 */
class OVITO_CORE_EXPORT ParticleGeometryBuffer
{
public:

	enum ShadingMode {
		NormalShading,
		FlatShading,
	};
	Q_ENUMS(ShadingMode);

	enum RenderingQuality {
		LowQuality,
		MediumQuality,
		HighQuality,
		AutoQuality
	};
	Q_ENUMS(RenderingQuality);

	enum ParticleShape {
		SphericalShape,
		SquareShape
	};
	Q_ENUMS(ParticleShape);

public:

	/// Constructor.
	ParticleGeometryBuffer(ShadingMode shadingMode, RenderingQuality renderingQuality, ParticleShape shape = SphericalShape)
		: _shadingMode(shadingMode), _renderingQuality(renderingQuality), _particleShape(shape) {}

	/// \brief Virtual base constructor.
	virtual ~ParticleGeometryBuffer() {}

	/// \brief Allocates a geometry buffer with the given number of particles.
	virtual void setSize(int particleCount) = 0;

	/// \brief Returns the number of particles stored in the buffer.
	virtual int particleCount() const = 0;

	/// \brief Sets the coordinates of the particles.
	virtual void setParticlePositions(const Point3* coordinates) = 0;

	/// \brief Sets the radii of the particles.
	virtual void setParticleRadii(const FloatType* radii) = 0;

	/// \brief Sets the radius of all particles to the given value.
	virtual void setParticleRadius(FloatType radius) = 0;

	/// \brief Sets the colors of the particles.
	virtual void setParticleColors(const Color* colors) = 0;

	/// \brief Sets the color of all particles to the given value.
	virtual void setParticleColor(const Color color) = 0;

	/// \brief Sets the transparency of the particles.
	virtual void setParticleTransparencies(const FloatType* transparencies) = 0;

	/// \brief Sets the transparency of all particles to the given value.
	virtual void setParticleTransparency(FloatType transparency) = 0;

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) = 0;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer) = 0;

	/// \brief Returns the shading mode for particles.
	ShadingMode shadingMode() const { return _shadingMode; }

	/// \brief Changes the shading mode for particles.
	/// \return false if the shading mode cannot be changed after the buffer has been created; true otherwise.
	virtual bool setShadingMode(ShadingMode mode) { _shadingMode = mode; return true; }

	/// \brief Returns the rendering quality of particles.
	RenderingQuality renderingQuality() const { return _renderingQuality; }

	/// \brief Changes the rendering quality of particles.
	/// \return false if the quality level cannot be changed after the buffer has been created; true otherwise.
	virtual bool setRenderingQuality(RenderingQuality level) { _renderingQuality = level; return true; }

	/// \brief Returns the display shape of particles.
	ParticleShape particleShape() const { return _particleShape; }

	/// \brief Changes the display shape of particles.
	/// \return false if the shape cannot be changed after the buffer has been created; true otherwise.
	virtual bool setParticleShape(ParticleShape shape) { _particleShape = shape; return true; }

private:

	/// Controls the shading of particles.
	ShadingMode _shadingMode;

	/// Controls the rendering quality of particles.
	RenderingQuality _renderingQuality;

	/// Controls the shape of particles.
	ParticleShape _particleShape;
};

};

Q_DECLARE_METATYPE(Ovito::ParticleGeometryBuffer::ShadingMode);
Q_DECLARE_METATYPE(Ovito::ParticleGeometryBuffer::RenderingQuality);
Q_DECLARE_METATYPE(Ovito::ParticleGeometryBuffer::ParticleShape);
Q_DECLARE_TYPEINFO(Ovito::ParticleGeometryBuffer::ShadingMode, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ParticleGeometryBuffer::RenderingQuality, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ParticleGeometryBuffer::ParticleShape, Q_PRIMITIVE_TYPE);

#endif // __OVITO_PARTICLE_GEOMETRY_BUFFER_H
