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

#include <core/Core.h>
#include "OnTheFlyNeighborListBuilder.h"

namespace Viz {

/******************************************************************************
* Constructor
******************************************************************************/
OnTheFlyNeighborListBuilder::OnTheFlyNeighborListBuilder(FloatType cutoffRadius) :
		_cutoffRadius(cutoffRadius), _cutoffRadiusSquared(cutoffRadius * cutoffRadius)
{
}

/******************************************************************************
* Preparation
******************************************************************************/
bool OnTheFlyNeighborListBuilder::prepare(ParticleProperty* posProperty, const SimulationCellData& cellData)
{
	OVITO_CHECK_POINTER(posProperty);

	if(_cutoffRadius <= 0.0)
		throw Exception("Invalid parameter: Neighbor cutoff radius must be positive.");

	simCell = cellData.matrix();
	if(fabs(simCell.determinant()) <= FLOATTYPE_EPSILON)
		throw Exception("Simulation cell is degenerate.");

	simCellInverse = simCell.inverse();
	pbc = cellData.pbcFlags();

	// Compute normal vectors of simulation cell faces.
	Vector3 planeNormals[3];
	planeNormals[0] = simCell.column(1).cross(simCell.column(2)).normalized();
	planeNormals[1] = simCell.column(2).cross(simCell.column(0)).normalized();
	planeNormals[2] = simCell.column(0).cross(simCell.column(1)).normalized();

	// Calculate the number of bins required in each spatial direction.
	binDim[0] = binDim[1] = binDim[2] = 1;
	for(size_t i = 0; i < 3; i++) {
		binDim[i] = (int)floor(fabs(simCell.column(i).dot(planeNormals[i])) / _cutoffRadius);
		binDim[i] = std::min(binDim[i], 60);
		binDim[i] = std::max(binDim[i], 1);
		if(binDim[i] < 2) {
			if(pbc[i]) {
				qDebug() << "Periodic simulation cell too small: axis:" << i << "  cutoff radius:" << _cutoffRadius << "   cell size:" << simCell.column(i).dot(planeNormals[i]);
				throw Exception("Periodic simulation cell is smaller than twice the neighbor cutoff radius. Minimum image convention cannot be used with such a small simulation box.");
			}
			binDim[i] = 1;
		}
	}
	bins.resize(binDim[0] * binDim[1] * binDim[2]);

	// Put atoms into bins.
	atoms.resize(posProperty->size());

	const Point3* p = posProperty->constDataPoint3();
	const Point3* p_end = p + posProperty->size();
	auto a = atoms.begin();
	size_t atomIndex = 0;
	for(; p != p_end; ++a, ++p, ++atomIndex) {
		a->index = atomIndex;

		// Transform atom position from absolute coordinates to reduced coordinates.
		a->pos = *p;
		Point3 reducedp = simCellInverse * (*p);

		int indices[3];
		for(size_t k = 0; k < 3; k++) {
			// Shift atom position to be inside simulation cell.
			if(pbc[k]) {
				while(reducedp[k] < 0) {
					reducedp[k] += 1;
					a->pos += simCell.column(k);
				}
				while(reducedp[k] > 1) {
					reducedp[k] -= 1;
					a->pos -= simCell.column(k);
				}
			}
			else {
				reducedp[k] = std::max(reducedp[k], (FloatType)0);
				reducedp[k] = std::min(reducedp[k], (FloatType)1);
			}

			// Determine the atom's bin from its position.
			indices[k] = std::max(std::min((int)(reducedp[k] * binDim[k]), binDim[k]-1), 0);
			OVITO_ASSERT(indices[k] >= 0 && indices[k] < binDim[k]);
		}

		// Put atom into its bin.
		NeighborListAtom*& binList = bins[indices[0] + indices[1]*binDim[0] + indices[2]*binDim[0]*binDim[1]];
		a->nextInBin = binList;
		binList = &*a;
	}

	return true;
}

/******************************************************************************
* Tests whether two atoms are closer to each other than the
* nearest-neighbor cutoff radius.
******************************************************************************/
bool OnTheFlyNeighborListBuilder::areNeighbors(size_t atom1, size_t atom2) const
{
	OVITO_ASSERT(atom1 < atoms.size());
	OVITO_ASSERT(atom2 < atoms.size());
	OVITO_ASSERT(atom1 != atom2);
	for(iterator neighborIter(*this, atom1); !neighborIter.atEnd(); neighborIter.next()) {
		if(neighborIter.current() == atom2) return true;
	}
	return false;
}

/******************************************************************************
* Iterator constructor
******************************************************************************/
OnTheFlyNeighborListBuilder::iterator::iterator(const OnTheFlyNeighborListBuilder& builder, size_t atomIndex)
	: _builder(builder), centerindex(atomIndex)
{
	dir[0] = -2;
	dir[1] = 1;
	dir[2] = 1;
	binatom = nullptr;
	center = _builder.atoms[atomIndex].pos;
	neighborindex = std::numeric_limits<size_t>::max();

	// Determine the bin the central atom is located in.
	// Transform atom position from absolute coordinates to reduced coordinates.
	OVITO_ASSERT(atomIndex < _builder.atoms.size());
	Point3 reducedp = _builder.simCellInverse * center;

	for(size_t k = 0; k < 3; k++) {
		// Determine the atom's bin from its position.
		centerbin[k] = std::max(std::min((int)(reducedp[k] * _builder.binDim[k]), _builder.binDim[k]-1), 0);
		OVITO_ASSERT(centerbin[k] >= 0 && centerbin[k] < _builder.binDim[k]);
	}

	next();
}

/******************************************************************************
* Iterator function.
******************************************************************************/
size_t OnTheFlyNeighborListBuilder::iterator::next()
{
	while(dir[0] != 2) {
		while(binatom) {
			_delta = binatom->pos - center - pbcOffset;
			neighborindex = binatom->index;
			OVITO_ASSERT(neighborindex < _builder.atoms.size());
			binatom = binatom->nextInBin;
			distsq = _delta.squaredLength();
			if(distsq <= _builder._cutoffRadiusSquared && neighborindex != centerindex) {
				return neighborindex;
			}
		};
		if(dir[2] == 1) {
			dir[2] = -1;
			if(dir[1] == 1) {
				dir[1] = -1;
				if(dir[0] == 1) {
					dir[0]++;
					neighborindex = std::numeric_limits<size_t>::max();
					return std::numeric_limits<size_t>::max();
				}
				else dir[0]++;
			}
			else dir[1]++;
		}
		else dir[2]++;

		currentbin[0] = centerbin[0] + dir[0];
		if(currentbin[0] == -1 && !_builder.pbc[0]) continue;
		if(currentbin[0] == _builder.binDim[0] && !_builder.pbc[0]) continue;

		currentbin[1] = centerbin[1] + dir[1];
		if(currentbin[1] == -1 && !_builder.pbc[1]) continue;
		if(currentbin[1] == _builder.binDim[1] && !_builder.pbc[1]) continue;

		currentbin[2] = centerbin[2] + dir[2];
		if(currentbin[2] == -1 && !_builder.pbc[2]) continue;
		if(currentbin[2] == _builder.binDim[2] && !_builder.pbc[2]) continue;

		pbcOffset.setZero();
		_pbcShift.setZero();
		for(size_t k = 0; k < 3; k++) {
			if(currentbin[k] == -1) {
				currentbin[k] = _builder.binDim[k]-1;
				pbcOffset += _builder.simCell.column(k);
				_pbcShift[k]--;
			}
			else if(currentbin[k] == _builder.binDim[k]) {
				currentbin[k] = 0;
				pbcOffset -= _builder.simCell.column(k);
				_pbcShift[k]++;
			}
		}

		binatom = _builder.bins[currentbin[0] + currentbin[1]*_builder.binDim[0] + currentbin[2] * _builder.binDim[0]*_builder.binDim[1]];
	}
	neighborindex = std::numeric_limits<size_t>::max();
	return std::numeric_limits<size_t>::max();
}

};	// End of namespace

