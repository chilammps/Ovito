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
#include <core/object/OvitoObject.h>

namespace Ovito {

class SceneRenderer;			// defined in SceneRenderer.h

/**
 * \brief Abstract base class for buffer objects that store point-like particles.
 */
class ParticleGeometryBuffer : public OvitoObject
{
public:

	/// \brief Virtual destructor.
	virtual ~ParticleGeometryBuffer

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

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) = 0;

	/// \brief Renders the geometry.
	virtual void render() = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_PARTICLE_GEOMETRY_BUFFER_H
