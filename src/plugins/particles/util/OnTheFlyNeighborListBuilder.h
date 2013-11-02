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
 * \file OnTheFlyNeighborList.h
 * \brief Contains the definition of the Particles::OnTheFlyNeighborList class.
 */

#ifndef __OVITO_ONTHEFLY_NEIGHBOR_LIST_H
#define __OVITO_ONTHEFLY_NEIGHBOR_LIST_H

#include <core/Core.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/SimulationCellData.h>

namespace Particles {

using namespace Ovito;

/**
 * \brief This class returns the list of neighbors within a given cutoff radius for a particle.
 *        The neighbor lists are not stored in memory but are calculated on demand for individual particles.
 */
class OnTheFlyNeighborListBuilder
{
private:

	// An internal per-particle data structure.
	struct NeighborListParticle {

		/// The position of the particle, wrapped at periodic boundaries.
		Point3 pos;

		/// The next item in the linked list used for binning.
		NeighborListParticle* nextInBin;

		/// The index of the particle.
		size_t index;
	};

public:

	/// \brief Constructor.
	OnTheFlyNeighborListBuilder(FloatType cutoffRadius);

	/// \brief Prepares the bin cells.
	/// \param posProperty The positions of the particles.
	/// \param simCell The simulation cell data.
	/// \param hasWrappedParticles If non-null, this output parameter will indicate whether one or more particles
	///                            have been wrapped at periodic boundaries.
	/// \return \c false when the operation has been canceled by the user;
	///         \c true on success.
	/// \throw Exception on error.
	bool prepare(ParticleProperty* posProperty, const SimulationCellData& cellData, bool* hasWrappedParticles = nullptr);

	/// Returns the neighbor cutoff radius.
	FloatType cutoffRadius() const { return _cutoffRadius; }

	/// Returns the square of the neighbor cutoff radius.
	FloatType cutoffRadiusSquared() const { return _cutoffRadiusSquared; }

	/// \brief Tests whether two particles are closer to each other than the
	///        nearest-neighbor cutoff radius.
	bool areNeighbors(size_t particle1, size_t particle2) const;

	/// \brief This iterator class lists all neighbors of a given atom.
	class iterator {
	public:

		iterator(const OnTheFlyNeighborListBuilder& builder, size_t particleIndex);

		bool atEnd() const { return _atEnd; }
		size_t next();
		size_t current() { return _neighborIndex; }
		const Vector3& delta() const { return _delta; }
		FloatType distanceSquared() const { return _distsq; }
		const Vector_3<int8_t>& pbcShift() const { return _pbcShift; }

	private:

		const OnTheFlyNeighborListBuilder& _builder;
		bool _atEnd;
		Point3 _center;
		size_t _centerIndex;
		std::vector<Vector3I>::const_iterator _stencilIter;
		Point3I _centerBin;
		Point3I _currentBin;
		NeighborListParticle* _neighbor;
		size_t _neighborIndex;
		Vector3 _pbcOffset;
		Vector_3<int8_t> _pbcShift;
		Vector3 _delta;
		FloatType _distsq;
	};

private:

	/// The neighbor criterion.
	FloatType _cutoffRadius;

	/// The neighbor criterion.
	FloatType _cutoffRadiusSquared;

	// Simulation cell properties.
	AffineTransformation simCell;
	AffineTransformation simCellInverse;
	std::array<bool,3> pbc;

	/// Number of bins in each spatial direction.
	int binDim[3];

	/// Used to determine the bin from a particle position.
	AffineTransformation reciprocalBinCell;

	/// The internal list of particles.
	std::vector<NeighborListParticle> particles;

	/// An 3d array of cubic bins. Each bin is a linked list of particles.
	std::vector<NeighborListParticle*> bins;

	/// The list of adjacent cells to visit while finding the neighbors of a
	/// central particle.
	std::vector<Vector3I> stencil;
};

};	// End of namespace

#endif // __OVITO_ONTHEFLY_NEIGHBOR_LIST_H
