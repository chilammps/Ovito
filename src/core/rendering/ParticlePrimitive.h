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

#ifndef __OVITO_PARTICLE_PRIMITIVE_H
#define __OVITO_PARTICLE_PRIMITIVE_H

#include <core/Core.h>
#include "PrimitiveBase.h"

namespace Ovito { inline namespace Rendering {

/**
 * \brief Abstract base class for particle drawing primitives.
 */
class OVITO_CORE_EXPORT ParticlePrimitive : public PrimitiveBase
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
	ParticlePrimitive(ShadingMode shadingMode, RenderingQuality renderingQuality, ParticleShape shape = SphericalShape)
		: _shadingMode(shadingMode), _renderingQuality(renderingQuality), _particleShape(shape) {}

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

	/// \brief Sets the colors and alpha values of the particles.
	virtual void setParticleColorsWithAlpha(const ColorA* colors, const Point3* positions) = 0;

	/// \brief Sets the color and alpha value of all particles to the given value.
	virtual void setParticleColorWithAlpha(const ColorA color, const Point3* positions) = 0;

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

}}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Rendering::ParticlePrimitive::ShadingMode);
Q_DECLARE_METATYPE(Ovito::Rendering::ParticlePrimitive::RenderingQuality);
Q_DECLARE_METATYPE(Ovito::Rendering::ParticlePrimitive::ParticleShape);
Q_DECLARE_TYPEINFO(Ovito::Rendering::ParticlePrimitive::ShadingMode, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Rendering::ParticlePrimitive::RenderingQuality, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Rendering::ParticlePrimitive::ParticleShape, Q_PRIMITIVE_TYPE);

#endif // __OVITO_PARTICLE_PRIMITIVE_H
