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

#ifndef __OVITO_ONTHEFLY_NEIGHBOR_LIST_H
#define __OVITO_ONTHEFLY_NEIGHBOR_LIST_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/SimulationCell.h>

namespace Ovito { namespace Plugins { namespace Particles { namespace Util {

/**
 * \brief This class returns the list of neighbors within a given cutoff radius for a particle.
 *        The neighbor lists are not stored in memory but are calculated on demand for individual particles.
 */
class OVITO_PARTICLES_EXPORT OnTheFlyNeighborListBuilder
{
private:

	// An internal per-particle data structure.
	struct NeighborListParticle {
		/// The position of the particle, wrapped at periodic boundaries.
		Point3 pos;
		/// The offset applied to the particle when wrapping it at periodic boundaries.
		Vector_3<int8_t> pbcShift;
		/// Pointer to next particle in linked list.
		const NeighborListParticle* nextInBin;
	};

public:

	/// \brief Constructor.
	OnTheFlyNeighborListBuilder(FloatType cutoffRadius);

	/// \brief Prepares the bin cells.
	/// \param posProperty The positions of the particles.
	/// \param cellData The simulation cell data.
	/// \param hasWrappedParticles If non-null, this output parameter will indicate whether one or more particles
	///                            have been wrapped at periodic boundaries.
	/// \return \c false when the operation has been canceled by the user;
	///         \c true on success.
	/// \throw Exception on error.
	bool prepare(ParticleProperty* posProperty, const SimulationCell& cellData, bool* hasWrappedParticles = nullptr);

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
		void next();
		size_t current() { return _neighborIndex; }
		const Vector3& delta() const { return _delta; }
		FloatType distanceSquared() const { return _distsq; }

		// Returns the PBC shift vector between the two particles.
		// This vector is non-zero if the current neighbor bond crosses a periodic boundary.
		const Vector_3<int8_t>& pbcShift() const { return _pbcShift; }

		// Returns the PBC shift vector between the two particles as if the two particles
		// were not wrapped at the periodic boundaries of the simulation cell.
		Vector_3<int8_t> unwrappedPbcShift() const {
			const auto& s1 = _builder.particles[_centerIndex].pbcShift;
			const auto& s2 = _builder.particles[_neighborIndex].pbcShift;
			return Vector_3<int8_t>(
					_pbcShift.x() - s1.x() + s2.x(),
					_pbcShift.y() - s1.y() + s2.y(),
					_pbcShift.z() - s1.z() + s2.z());
		}

	private:

		const OnTheFlyNeighborListBuilder& _builder;
		bool _atEnd;
		Point3 _center, _shiftedCenter;
		size_t _centerIndex;
		std::vector<Vector3I>::const_iterator _stencilIter;
		Point3I _centerBin;
		Point3I _currentBin;
		const NeighborListParticle* _neighbor;
		size_t _neighborIndex;
		Vector_3<int8_t> _pbcShift;
		Vector3 _delta;
		FloatType _distsq;
	};

private:

	/// The neighbor criterion.
	FloatType _cutoffRadius;

	/// The neighbor criterion.
	FloatType _cutoffRadiusSquared;

	// Simulation cell.
	SimulationCell simCell;

	/// Number of bins in each spatial direction.
	int binDim[3];

	/// Used to determine the bin from a particle position.
	AffineTransformation reciprocalBinCell;

	/// The internal list of particles.
	std::vector<NeighborListParticle> particles;

	/// An 3d array of cubic bins. Each bin is a linked list of particles.
	std::vector<const NeighborListParticle*> bins;

	/// The list of adjacent cells to visit while finding the neighbors of a
	/// central particle.
	std::vector<Vector3I> stencil;
};

}}}}	// End of namespace

#endif // __OVITO_ONTHEFLY_NEIGHBOR_LIST_H
