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
 * \brief Contains the definition of the Viz::OnTheFlyNeighborList class.
 */

#ifndef __OVITO_ONTHEFLY_NEIGHBOR_LIST_H
#define __OVITO_ONTHEFLY_NEIGHBOR_LIST_H

#include <core/Core.h>
#include <viz/data/ParticleProperty.h>
#include <viz/data/SimulationCellData.h>

namespace Viz {

using namespace Ovito;

/**
 * \brief This class returns the list of neighbors within a given cutoff radius for a particle.
 *        The neighbor lists are not stored in memory but are calculated on demand for individual particles.
 */
class OnTheFlyNeighborListBuilder
{
private:

	// An internal atom structure.
	struct NeighborListAtom {
		/// The next atom in the linked list used for binning.
		NeighborListAtom* nextInBin;
		/// The index of the atom in the original AtomsObject.
		size_t index;
		/// The wrapped position of the atom.
		Point3 pos;
	};

public:

	/// \brief Constructor.
	OnTheFlyNeighborListBuilder(FloatType cutoffRadius);

	/// \brief Prepares the bin cells.
	/// \param posProperty The positions of the particles.
	/// \param simCell The simulation cell data.
	/// \return \c false when the operation has been canceled by the user;
	///         \c true on success.
	/// \throw Exception on error.
	bool prepare(ParticleProperty* posProperty, const SimulationCellData& cellData);

	/// Returns the neighbor cutoff radius.
	FloatType cutoffRadius() const { return _cutoffRadius; }

	/// Returns the square of the neighbor cutoff radius.
	FloatType cutoffRadiusSquared() const { return _cutoffRadiusSquared; }

	/// \brief Tests whether two atoms are closer to each other than the
	///        nearest-neighbor cutoff radius.
	bool areNeighbors(size_t atom1, size_t atom2) const;

	/// \brief This iterator class lists all neighbors of a given atom.
	class iterator {
	public:

		/// \brief Constructor
		iterator(const OnTheFlyNeighborListBuilder& builder, size_t atomIndex);

		bool atEnd() const { return dir[0] > 1; }
		size_t next();
		size_t current() { return neighborindex; }
		const Vector3& delta() const { return _delta; }
		FloatType distanceSquared() const { return distsq; }
		const Vector_3<int8_t>& pbcShift() const { return _pbcShift; }

	private:
		const OnTheFlyNeighborListBuilder& _builder;
		Point3 center;
		size_t centerindex;
		int dir[3];
		int centerbin[3];
		int currentbin[3];
		NeighborListAtom* binatom;
		size_t neighborindex;
		Vector3 pbcOffset;
		Vector_3<int8_t> _pbcShift;
		Vector3 _delta;
		FloatType distsq;
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

	/// The internal list of atoms.
	std::vector<NeighborListAtom> atoms;

	/// An 3d array of cubic bins. Each bin is a linked list of atoms.
	std::vector<NeighborListAtom*> bins;
};

};	// End of namespace

#endif // __OVITO_ONTHEFLY_NEIGHBOR_LIST_H
