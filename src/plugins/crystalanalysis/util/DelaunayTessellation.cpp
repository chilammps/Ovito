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

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include "DelaunayTessellation.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/******************************************************************************
* Generates the tessellation.
******************************************************************************/
void DelaunayTessellation::generateTessellation(const SimulationCell& simCell, const Point3* positions, size_t numPoints, FloatType ghostLayerSize)
{
	std::vector<Point3WithIndex> cgalPoints;

	// Set up random number generator to generate random perturbations.
	std::mt19937 rng;
	rng.seed(1);
	std::uniform_real_distribution<double> displacement(-1e-8, +1e-8);

	// Insert the original points first.
	cgalPoints.reserve(numPoints);
	for(size_t i = 0; i < numPoints; i++, ++positions) {

		// Add a small random perturbation to the particle positions to make the Delaunay triangulation more robust
		// against singular input data, e.g. particles forming an ideal crystal lattice.
		Point3 wp = simCell.wrapPoint(*positions);
		double coords[3];
		coords[0] = (double)wp.x() + displacement(rng);
		coords[1] = (double)wp.y() + displacement(rng);
		coords[2] = (double)wp.z() + displacement(rng);

		cgalPoints.emplace_back(coords[0], coords[1], coords[2], i, false);
	}

	int vertexCount = numPoints;

	Vector3I stencilCount;
	FloatType cuts[3][2];
	Vector3 cellNormals[3];
	for(size_t dim = 0; dim < 3; dim++) {
		cellNormals[dim] = simCell.cellNormalVector(dim);
		cuts[dim][0] = cellNormals[dim].dot(simCell.reducedToAbsolute(Point3(0,0,0)) - Point3::Origin());
		cuts[dim][1] = cellNormals[dim].dot(simCell.reducedToAbsolute(Point3(1,1,1)) - Point3::Origin());

		if(simCell.pbcFlags()[dim]) {
			stencilCount[dim] = (int)ceil(ghostLayerSize / simCell.matrix().column(dim).dot(cellNormals[dim]));
			cuts[dim][0] -= ghostLayerSize;
			cuts[dim][1] += ghostLayerSize;
		}
		else {
			stencilCount[dim] = 0;
			cuts[dim][0] -= ghostLayerSize;
			cuts[dim][1] += ghostLayerSize;
		}
	}

	// Create periodic images of input vertices.
	for(int ix = -stencilCount[0]; ix <= +stencilCount[0]; ix++) {
		for(int iy = -stencilCount[1]; iy <= +stencilCount[1]; iy++) {
			for(int iz = -stencilCount[2]; iz <= +stencilCount[2]; iz++) {
				if(ix == 0 && iy == 0 && iz == 0) continue;
				Vector3 shift = simCell.reducedToAbsolute(Vector3(ix,iy,iz));
				Vector_3<double> shiftd = (Vector_3<double>)shift;
				for(int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++) {
					Point3 pimage = (Point3)cgalPoints[vertexIndex] + shift;
					bool isClipped = false;
					for(size_t dim = 0; dim < 3; dim++) {
						FloatType d = cellNormals[dim].dot(pimage - Point3::Origin());
						if(d < cuts[dim][0] || d > cuts[dim][1]) {
							isClipped = true;
							break;
						}
					}
					if(!isClipped)
						cgalPoints.emplace_back(cgalPoints[vertexIndex].x() + shift.x(),
								cgalPoints[vertexIndex].y() + shift.y(), cgalPoints[vertexIndex].z() + shift.z(), vertexIndex, true);
				}
			}
		}
	}

	_dt.insert(cgalPoints.begin(), cgalPoints.end());

	// Classify tessellation cells as ghost or local cells.
	for(CellIterator cell = begin_cells(); cell != end_cells(); ++cell) {
		cell->info().isGhost = isGhostCell(cell);
	}
}

/******************************************************************************
* /// Determines whether the given tetrahedral cell is a ghost cell (or an invalid cell).
******************************************************************************/
bool DelaunayTessellation::isGhostCell(CellHandle cell) const
{
	// Find head vertex with the lowest index.
	const auto& p0 = cell->vertex(0)->point();
	int headVertex = p0.index();
	if(headVertex == -1) {
		OVITO_ASSERT(!isValidCell(cell));
		return true;
	}
	bool isGhost = p0.isGhost();
	for(int v = 1; v < 4; v++) {
		const auto& p = cell->vertex(v)->point();
		if(p.index() == -1) {
			OVITO_ASSERT(!isValidCell(cell));
			return true;
		}
		if(p.index() < headVertex) {
			headVertex = p.index();
			isGhost = p.isGhost();
		}
	}

	OVITO_ASSERT(isValidCell(cell));
	return isGhost;
}

}	// End of namespace
}	// End of namespace
}	// End of namespace
