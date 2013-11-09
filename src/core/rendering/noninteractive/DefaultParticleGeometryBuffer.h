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
 * \file DefaultParticleGeometryBuffer.h
 * \brief Contains the definition of the Ovito::DefaultParticleGeometryBuffer class.
 */

#ifndef __OVITO_DEFAULT_PARTICLE_GEOMETRY_BUFFER_H
#define __OVITO_DEFAULT_PARTICLE_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/ParticleGeometryBuffer.h>

namespace Ovito {

/**
 * \brief Buffer object that stores a set of particles to be rendered by a non-interactive renderer.
 */
class OVITO_CORE_EXPORT DefaultParticleGeometryBuffer : public ParticleGeometryBuffer
{
public:

	/// Constructor.
	DefaultParticleGeometryBuffer(ShadingMode shadingMode, RenderingQuality renderingQuality, ParticleShape shape) :
		ParticleGeometryBuffer(shadingMode, renderingQuality, shape) {}

	/// \brief Allocates a geometry buffer with the given number of particles.
	virtual void setSize(int particleCount) override {
		OVITO_ASSERT(particleCount >= 0);
		_positionsBuffer.resize(particleCount);
		_radiiBuffer.resize(particleCount);
		_colorsBuffer.resize(particleCount);
		_transparenciesBuffer.clear();
	}

	/// \brief Returns the number of particles stored in the buffer.
	virtual int particleCount() const override { return _positionsBuffer.size(); }

	/// \brief Sets the coordinates of the particles.
	virtual void setParticlePositions(const Point3* coordinates) override {
		std::copy(coordinates, coordinates + _positionsBuffer.size(), _positionsBuffer.begin());
	}

	/// \brief Sets the radii of the particles.
	virtual void setParticleRadii(const FloatType* radii) override {
		std::copy(radii, radii + _radiiBuffer.size(), _radiiBuffer.begin());
	}

	/// \brief Sets the radius of all particles to the given value.
	virtual void setParticleRadius(FloatType radius) override {
		std::fill(_radiiBuffer.begin(), _radiiBuffer.end(), radius);
	}

	/// \brief Sets the colors of the particles.
	virtual void setParticleColors(const Color* colors) override {
		std::copy(colors, colors + _colorsBuffer.size(), _colorsBuffer.begin());
	}

	/// \brief Sets the color of all particles to the given value.
	virtual void setParticleColor(const Color color) override {
		std::fill(_colorsBuffer.begin(), _colorsBuffer.end(), color);
	}

	/// \brief Sets the transparency of the particles.
	virtual void setParticleTransparencies(const FloatType* transparencies) override {
		_transparenciesBuffer.assign(transparencies, transparencies + _positionsBuffer.size());
	}

	/// \brief Sets the transparency of all particles to the given value.
	virtual void setParticleTransparency(FloatType transparency) override {
		if(transparency != FloatType(0))
			_transparenciesBuffer.assign(_positionsBuffer.size(), transparency);
		else
			_transparenciesBuffer.clear();
	}

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer, quint32 pickingBaseID = 0) override;

	/// Returns a reference to the internal buffer that stores the particle positions.
	const std::vector<Point3>& positions() const { return _positionsBuffer; }

	/// Returns a reference to the internal buffer that stores the particle radii.
	const std::vector<FloatType>& radii() const { return _radiiBuffer; }

	/// Returns a reference to the internal buffer that stores the particle colors.
	const std::vector<Color>& colors() const { return _colorsBuffer; }

	/// Returns a reference to the internal buffer that stores the particle transparencies.
	const std::vector<FloatType>& transparencies() const { return _transparenciesBuffer; }

private:

	/// The internal buffer that stores the particle positions.
	std::vector<Point3> _positionsBuffer;

	/// The internal buffer that stores the particle radii.
	std::vector<FloatType> _radiiBuffer;

	/// The internal buffer that stores the particle colors.
	std::vector<Color> _colorsBuffer;

	/// The internal buffer that stores the particle transparencies.
	std::vector<FloatType> _transparenciesBuffer;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_DEFAULT_PARTICLE_GEOMETRY_BUFFER_H
