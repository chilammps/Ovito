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

#include <plugins/particles/Particles.h>
#include "OnTheFlyNeighborListBuilder.h"

namespace Particles {

/******************************************************************************
* Constructor
******************************************************************************/
OnTheFlyNeighborListBuilder::OnTheFlyNeighborListBuilder(FloatType cutoffRadius) :
		_cutoffRadius(cutoffRadius), _cutoffRadiusSquared(cutoffRadius * cutoffRadius)
{
}

/******************************************************************************
* Initialization function.
******************************************************************************/
bool OnTheFlyNeighborListBuilder::prepare(ParticleProperty* posProperty, const SimulationCellData& cellData, bool* hasWrappedParticles)
{
	OVITO_CHECK_POINTER(posProperty);

	if(_cutoffRadius <= 0.0)
		throw Exception("Invalid parameter: Neighbor cutoff radius must be positive.");

	simCell = cellData.matrix();
	if(std::fabs(simCell.determinant()) <= FLOATTYPE_EPSILON)
		throw Exception("Simulation cell is degenerate.");

	simCellInverse = simCell.inverse();
	pbc = cellData.pbcFlags();

	// Calculate the number of bins required in each spatial direction.
	AffineTransformation binCell;
	binCell.translation() = simCell.translation();
	Vector3 planeNormals[3];
	for(size_t i = 0; i < 3; i++) {
		planeNormals[i] = cellData.cellNormalVector(i);
		binDim[i] = (int)floor(simCell.column(i).dot(planeNormals[i]) / _cutoffRadius);
		binDim[i] = std::min(binDim[i], 60);
		binDim[i] = std::max(binDim[i], 1);
		binCell.column(i) = simCell.column(i) / binDim[i];
	}
	bins.resize(binDim[0] * binDim[1] * binDim[2]);

	// Compute the reciprocal bin cell for fast lookup.
	reciprocalBinCell = binCell.inverse();

	// Calculate size of stencil.
	Vector3I stencilCount;
	for(size_t dim = 0; dim < 3; dim++) {
		stencilCount[dim] = (int)floor(binCell.column(dim).dot(planeNormals[dim]) / _cutoffRadius);
		stencilCount[dim] = std::min(stencilCount[dim], 50);
		stencilCount[dim] = std::max(stencilCount[dim], 1);
	}

	// Generate stencil.
	for(int ix = -stencilCount[0]; ix <= +stencilCount[0]; ix++) {
		for(int iy = -stencilCount[1]; iy <= +stencilCount[1]; iy++) {
			for(int iz = -stencilCount[2]; iz <= +stencilCount[2]; iz++) {
				stencil.push_back(Vector3I(ix,iy,iz));
			}
		}
	}

	// Reset flag.
	if(hasWrappedParticles)
		*hasWrappedParticles = false;

	particles.resize(posProperty->size());

	// Sort particles into bins.
	const Point3* p = posProperty->constDataPoint3();
	const Point3* p_end = p + posProperty->size();
	auto a = particles.begin();
	size_t particleIndex = 0;
	for(; p != p_end; ++a, ++p, ++particleIndex) {
		a->pos = *p;
		a->pbcShift.setZero();
		a->index = particleIndex;

		// Determine the bin the atom is located in.
		Point3 rp = reciprocalBinCell * (*p);

		Point3I binLocation;
		for(size_t k = 0; k < 3; k++) {

			binLocation[k] = (int)floor(rp[k]);

			if(pbc[k]) {
				if(binLocation[k] < 0 || binLocation[k] >= binDim[k]) {
					int shift;
					if(binLocation[k] < 0)
						shift = -(binLocation[k]+1)/binDim[k]+1;
					else
						shift = -binLocation[k]/binDim[k];
					a->pbcShift[k] = (int8_t)shift;
					a->pos += (FloatType)shift * simCell.column(k);
					binLocation[k] = SimulationCellData::modulo(binLocation[k], binDim[k]);
					if(hasWrappedParticles)
						*hasWrappedParticles = true;
				}
			}
			else if(binLocation[k] < 0) {
				binLocation[k] = 0;
			}
			else if(binLocation[k] >= binDim[k]) {
				binLocation[k] = binDim[k] - 1;
			}
			OVITO_ASSERT(binLocation[k] >= 0 && binLocation[k] < binDim[k]);
		}

		// Put particle into its bin.
		NeighborListParticle*& binList = bins[binLocation[0] + binLocation[1]*binDim[0] + binLocation[2]*binDim[0]*binDim[1]];
		a->nextInBin = binList;
		binList = &*a;
	}

	return true;
}

/******************************************************************************
* Tests whether two particles are closer to each other than the
* nearest-neighbor cutoff radius.
******************************************************************************/
bool OnTheFlyNeighborListBuilder::areNeighbors(size_t particle1, size_t particle2) const
{
	OVITO_ASSERT(particle1 < particles.size());
	OVITO_ASSERT(particle2 < particles.size());
	OVITO_ASSERT(particle1 != particle2);
	// Check if particle 2 is in the neighbor list of particle 1.
	for(iterator neighborIter(*this, particle1); !neighborIter.atEnd(); neighborIter.next()) {
		if(neighborIter.current() == particle2) return true;
	}
	return false;
}

/******************************************************************************
* Iterator constructor
******************************************************************************/
OnTheFlyNeighborListBuilder::iterator::iterator(const OnTheFlyNeighborListBuilder& builder, size_t particleIndex)
	: _builder(builder), _centerIndex(particleIndex)
{
	OVITO_ASSERT(particleIndex < _builder.particles.size());

	_stencilIter = _builder.stencil.begin();
	_neighbor = nullptr;
	_atEnd = false;
	_center = _builder.particles[particleIndex].pos;
	_neighborIndex = std::numeric_limits<size_t>::max();

	// Determine the bin the central particle is located in.
	for(size_t k = 0; k < 3; k++) {
		_centerBin[k] = (int)floor(_builder.reciprocalBinCell.prodrow(_center, k));
		if(_centerBin[k] < 0) _centerBin[k] = 0;
		else if(_centerBin[k] >= _builder.binDim[k]) _centerBin[k] = _builder.binDim[k];
	}

	next();
}

/******************************************************************************
* Iterator function.
******************************************************************************/
size_t OnTheFlyNeighborListBuilder::iterator::next()
{
	OVITO_ASSERT(!_atEnd);

	for(;;) {
		while(_neighbor) {
			_delta = _neighbor->pos - _center + _pbcOffset;
			_neighborIndex = _neighbor->index;
			OVITO_ASSERT(_neighborIndex < _builder.particles.size());
			_neighbor = _neighbor->nextInBin;
			_distsq = _delta.squaredLength();
			if(_distsq <= _builder._cutoffRadiusSquared && (_neighborIndex != _centerIndex || _pbcShift != Vector_3<int8_t>::Zero())) {
				return _neighborIndex;
			}
		};

		for(;;) {
			if(_stencilIter == _builder.stencil.end()) {
				_atEnd = true;
				return (_neighborIndex = std::numeric_limits<size_t>::max());
			}

			_pbcOffset.setZero();
			_pbcShift.setZero();
			bool skipBin = false;
			for(size_t k = 0; k < 3; k++) {
				_currentBin[k] = _centerBin[k] + (*_stencilIter)[k];
				if(!_builder.pbc[k]) {
					if(_currentBin[k] < 0 || _currentBin[k] >= _builder.binDim[k]) {
						skipBin = true;
						break;
					}
				}
				else {
					if(_currentBin[k] >= _builder.binDim[k]) {
						int s = _currentBin[k] / _builder.binDim[k];
						if(s > std::numeric_limits<int8_t>::max())
							throw Exception(QString("Periodic simulation cell is too small or cutoff radius is too large to generate neighbor lists."));
						_pbcShift[k] = (int8_t)s;
						_currentBin[k] -= s * _builder.binDim[k];
						_pbcOffset += _builder.simCell.column(k) * (FloatType)s;
					}
					else if(_currentBin[k] < 0) {
						int s = (_currentBin[k] - _builder.binDim[k] + 1) / _builder.binDim[k];
						if(s < std::numeric_limits<int8_t>::min())
							throw Exception(QString("Periodic simulation cell is too small or cutoff radius is too large to generate neighbor lists."));
						_pbcShift[k] = (int8_t)s;
						_currentBin[k] -= s * _builder.binDim[k];
						_pbcOffset += _builder.simCell.column(k) * (FloatType)s;
					}
				}
				OVITO_ASSERT(_currentBin[k] >= 0 && _currentBin[k] < _builder.binDim[k]);
			}
			++_stencilIter;
			if(!skipBin) {
				_neighbor = _builder.bins[_currentBin[0] + _currentBin[1] * _builder.binDim[0] + _currentBin[2] * _builder.binDim[0] * _builder.binDim[1]];
				break;
			}
		}
	}
}

};	// End of namespace

