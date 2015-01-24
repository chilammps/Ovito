///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_CUTOFF_NEIGHBOR_FINDER_H
#define __OVITO_CUTOFF_NEIGHBOR_FINDER_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/SimulationCell.h>
#include <core/utilities/concurrent/FutureInterface.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util)

/**
 * \brief This utility class finds all neighbor particles within a cutoff radius of a central particle.
 *
 * OVITO provides two facilities for finding the neighbors of particles: The CutoffNeighborFinder class, which
 * finds all neighbors within a certain cutoff radius, and the NearestNeighborFinder class, which finds
 * the *k* nearest neighbor of a particle, where *k* is some positive integer. Note that the cutoff-based neighbor finder
 * can return an unknown number of neighbor particles, while the nearest neighbor finder will return exactly
 * the requested number of nearest neighbors (ordered by increasing distance from the central particle).
 * Whether CutoffNeighborFinder or NearestNeighborFinder is the right choice depends on the application.
 *
 * The CutoffNeighborFinder class must be initialized by a call to prepare(). This function generates a grid of bin
 * cells whose size is on the order of the specified cutoff radius. It sorts all input particles into these bin cells
 * for fast neighbor queries.
 *
 * After the CutoffNeighborFinder has been initialized, one can find the neighbors of some central
 * particle by constructing an instance of the CutoffNeighborFinder::Query class. This is a light-weight class which
 * iterates over all neighbors within the cutoff range of the selected particle.
 *
 * The CutoffNeighborFinder class takes into account periodic boundary conditions. With periodic boundary conditions,
 * a particle can be appear multiple times in the neighbor list of another particle. Note, however, that a different neighbor *vector* is
 * reported for each periodic image of a neighbor.
 */
class OVITO_PARTICLES_EXPORT CutoffNeighborFinder
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

	/// Default constructor.
	/// You need to call prepare() first before the neighbor finder can be used.
	CutoffNeighborFinder() : _cutoffRadius(0), _cutoffRadiusSquared(0) {}

	/// \brief Prepares the neighbor finder by sorting particles into a grid of bin cells.
	/// \param cutoffRadius The cutoff radius for neighbor lists.
	/// \param positions The ParticleProperty containing the particle coordinates.
	/// \param simCell The input simulation cell geometry and boundary conditions.
	/// \param progress An optional callback object that will be used to the report progress.
	/// \return \c false when the operation has been canceled by the user;s
	///         \c true on success.
	/// \throw Exception on error.
	bool prepare(FloatType cutoffRadius, ParticleProperty* positions, const SimulationCell& simCell, FutureInterfaceBase* progress = nullptr);

	/// Returns the cutoff radius set via prepare().
	FloatType cutoffRadius() const { return _cutoffRadius; }

	/// Returns the square of the cutoff radius set via prepare().
	FloatType cutoffRadiusSquared() const { return _cutoffRadiusSquared; }

	/// \brief An iterator class that returns all neighbors of a central particle.
	class Query
	{
	public:

		/// Constructs a new neighbor query object that can be used to iterate over the neighbors of a particle.
		/// \param finder The object that stores the particles, and which is used to compute the results of the query.
		/// \param particleIndex The index of the particle for which to find the neighbors.
		Query(const CutoffNeighborFinder& finder, size_t particleIndex);

		/// Indicates whether the end of the list of neighbors has been reached.
		bool atEnd() const { return _atEnd; }

		/// Finds the next neighbor particle within the cutoff radius.
		/// Use atEnd() to test whether another neighbor has been found.
		void next();

		/// Returns the index of the current neighbor particle.
		size_t current() { return _neighborIndex; }

		/// Returns the vector connecting the central particle with the current neighbor.
		const Vector3& delta() const { return _delta; }

		/// Returns the distance squared between the central particle and the current neighbor.
		FloatType distanceSquared() const { return _distsq; }

		/// Returns the PBC shift vector between the central particle and the current neighbor.
		/// The vector is non-zero if the current neighbor vector crosses a periodic boundary.
		const Vector_3<int8_t>& pbcShift() const { return _pbcShift; }

		/// Returns the PBC shift vector between the central particle and the current neighbor as if the two particles
		/// were not wrapped at the periodic boundaries of the simulation cell.
		Vector_3<int8_t> unwrappedPbcShift() const {
			const auto& s1 = _builder.particles[_centerIndex].pbcShift;
			const auto& s2 = _builder.particles[_neighborIndex].pbcShift;
			return Vector_3<int8_t>(
					_pbcShift.x() - s1.x() + s2.x(),
					_pbcShift.y() - s1.y() + s2.y(),
					_pbcShift.z() - s1.z() + s2.z());
		}

	private:

		const CutoffNeighborFinder& _builder;
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

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_CUTOFF_NEIGHBOR_FINDER_H
