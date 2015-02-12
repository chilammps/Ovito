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

#ifndef __OVITO_SIMULATION_CELL_H
#define __OVITO_SIMULATION_CELL_H

#include <plugins/particles/Particles.h>

namespace Ovito { namespace Particles {

/**
* \brief Stores the geometry and boundary conditions of a simulation box.
 *
 * The simulation box geometry is a parallelepiped defined by three edge vectors.
 * A fourth vector specifies the origin of the simulation box in space.
 */
class OVITO_PARTICLES_EXPORT SimulationCell
{
public:

	/// Default constructor.
	SimulationCell() {
		_simulationCell = AffineTransformation::Zero();
		_reciprocalSimulationCell = AffineTransformation::Zero();
		_pbcFlags.fill(true);
	}

	/// Returns the current simulation cell matrix.
	const AffineTransformation& matrix() const { return _simulationCell; }

	/// Returns the current reciprocal simulation cell matrix.
	const AffineTransformation& inverseMatrix() const { return _reciprocalSimulationCell; }

	/// Sets the simulation cell matrix.
	void setMatrix(const AffineTransformation& cellMatrix) {
		_simulationCell = cellMatrix;
		if(!cellMatrix.inverse(_reciprocalSimulationCell))
			_reciprocalSimulationCell.setIdentity();
	}

	/// Returns the PBC flags.
	const std::array<bool,3>& pbcFlags() const { return _pbcFlags; }

	/// Sets the PBC flags.
	void setPbcFlags(const std::array<bool,3>& flags) { _pbcFlags = flags; }

	/// Sets the PBC flags.
	void setPbcFlags(bool pbcX, bool pbcY, bool pbcZ) { _pbcFlags[0] = pbcX; _pbcFlags[1] = pbcY; _pbcFlags[2] = pbcZ; }

	/// Computes the (positive) volume of the cell.
	FloatType volume() const {
		return std::abs(_simulationCell.determinant());
	}

	/// Returns true if the three edges of the cell are parallel to the three
	/// coordinates axes.
	bool isAxisAligned() const {
		if(matrix()(1,0) != 0 || matrix()(2,0) != 0) return false;
		if(matrix()(0,1) != 0 || matrix()(2,1) != 0) return false;
		if(matrix()(0,2) != 0 || matrix()(1,2) != 0) return false;
		return true;
	}

	/// Checks if two simulation cells are identical.
	bool operator==(const SimulationCell& other) const {
		return (_simulationCell == other._simulationCell && _pbcFlags == other._pbcFlags);
	}

	/// Converts a point given in reduced cell coordinates to a point in absolute coordinates.
	Point3 reducedToAbsolute(const Point3& reducedPoint) const { return _simulationCell * reducedPoint; }

	/// Converts a point given in absolute coordinates to a point in reduced cell coordinates.
	Point3 absoluteToReduced(const Point3& absPoint) const { return _reciprocalSimulationCell * absPoint; }

	/// Converts a vector given in reduced cell coordinates to a vector in absolute coordinates.
	Vector3 reducedToAbsolute(const Vector3& reducedVec) const { return _simulationCell * reducedVec; }

	/// Converts a vector given in absolute coordinates to a point in vector cell coordinates.
	Vector3 absoluteToReduced(const Vector3& absVec) const { return _reciprocalSimulationCell * absVec; }

	/// Wraps a point at the periodic boundaries of the cell.
	Point3 wrapPoint(const Point3& p) const {
		Point3 pout = p;
		for(size_t dim = 0; dim < 3; dim++) {
			if(_pbcFlags[dim]) {
				if(FloatType s = floor(_reciprocalSimulationCell.prodrow(p, dim)))
					pout -= s * _simulationCell.column(dim);
			}
		}
		return pout;
	}

	/// Wraps a vector at the periodic boundaries of the cell using minimum image convention.
	Vector3 wrapVector(const Vector3& v) const {
		Vector3 vout = v;
		for(size_t dim = 0; dim < 3; dim++) {
			if(_pbcFlags[dim]) {
				if(FloatType s = floor(_reciprocalSimulationCell.prodrow(v, dim) + FloatType(0.5)))
					vout -= s * _simulationCell.column(dim);
			}
		}
		return vout;
	}

	/// Calculates the normal vector of the given simulation cell side.
	Vector3 cellNormalVector(size_t dim) const {
		Vector3 normal = _simulationCell.column((dim+1)%3).cross(_simulationCell.column((dim+2)%3));
		// Flip normal if necessary.
		if(normal.dot(_simulationCell.column(dim)) < 0.0f)
			return normal / (-normal.length());
		else
			return normal.normalized();
	}

	/// Tests if a vector so long that it would be wrapped at a periodic boundary when using the minimum image convention.
	bool isWrappedVector(const Vector3& v) const {
		for(size_t dim = 0; dim < 3; dim++) {
			if(_pbcFlags[dim]) {
				if(std::abs(_reciprocalSimulationCell.prodrow(v, dim)) >= 0.5f)
					return true;
			}
		}
		return false;
	}

	/// \brief Helper function that computes the modulo operation for two integer numbers k and n.
	///
	/// This function can handle negative numbers k. This allows mapping any number k that is
	/// outside the interval [0,n) back into the interval. Use this to implement periodic boundary conditions.
	static inline int modulo(int k, int n) {
		return ((k %= n) < 0) ? k+n : k;
	}

	/// \brief Helper function that computes the modulo operation for two floating-point numbers k and n.
	///
	/// This function can handle negative numbers k. This allows mapping any number k that is
	/// outside the interval [0,n) back into the interval. Use this to implement periodic boundary conditions.
	static inline FloatType modulo(FloatType k, FloatType n) {
		k = fmod(k, n);
		return (k < 0) ? k+n : k;
	}

private:

	/// The geometry of the cell.
	AffineTransformation _simulationCell;

	/// The reciprocal cell matrix.
	AffineTransformation _reciprocalSimulationCell;

	/// PBC flags.
	std::array<bool,3> _pbcFlags;
};

}	// End of namespace
}	// End of namespace

#endif // __OVITO_SIMULATION_CELL_H
