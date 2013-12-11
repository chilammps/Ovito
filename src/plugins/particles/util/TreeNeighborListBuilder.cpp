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
#include "TreeNeighborListBuilder.h"

namespace Particles {

/// Used to sort PBC images by distance from the master image.
static bool pbcShiftCompare(const std::pair<Vector3, Vector3>& a, const std::pair<Vector3, Vector3>& b) {
	return a.first.squaredLength() < b.first.squaredLength();
}

/******************************************************************************
* Prepares the neighbor list builder.
******************************************************************************/
bool TreeNeighborListBuilder::prepare(ParticleProperty* posProperty, const SimulationCellData& cellData)
{
	OVITO_CHECK_POINTER(posProperty);

	simCell = cellData.matrix();
	if(std::fabs(simCell.determinant()) <= FLOATTYPE_EPSILON)
		throw Exception("Simulation cell is degenerate.");

	simCellInverse = simCell.inverse();
	pbc = cellData.pbcFlags();

	// Compute normal vectors of simulation cell faces.
	planeNormals[0] = cellData.cellNormalVector(0);
	planeNormals[1] = cellData.cellNormalVector(1);
	planeNormals[2] = cellData.cellNormalVector(2);

	// Create list of periodic image shift vectors.
	int nx = pbc[0] ? 1 : 0;
	int ny = pbc[1] ? 1 : 0;
	int nz = pbc[2] ? 1 : 0;
	for(int iz = -nz; iz <= nz; iz++) {
		for(int iy = -ny; iy <= ny; iy++) {
			for(int ix = -nx; ix <= nx; ix++) {
				Vector3 rshift(ix,iy,iz);
				Vector3 shift = simCell * rshift;
				pbcImages.push_back(std::make_pair(shift, rshift));
			}
		}
	}
	std::sort(pbcImages.begin(), pbcImages.end(), pbcShiftCompare);

	// Compute bounding box of all particles (only for non-periodic directions).
	Box3 boundingBox(Point3(0,0,0), Point3(1,1,1));
	if(pbc[0] == false || pbc[1] == false || pbc[2] == false) {
		const Point3* p = posProperty->constDataPoint3();
		const Point3* pend = p + posProperty->size();
		for(; p != pend; ++p) {
			Point3 reducedp = simCellInverse * (*p);
			if(pbc[0] == false) {
				if(reducedp.x() < boundingBox.minc.x()) boundingBox.minc.x() = reducedp.x();
				else if(reducedp.x() > boundingBox.maxc.x()) boundingBox.maxc.x() = reducedp.x();
			}
			if(pbc[1] == false) {
				if(reducedp.y() < boundingBox.minc.y()) boundingBox.minc.y() = reducedp.y();
				else if(reducedp.y() > boundingBox.maxc.y()) boundingBox.maxc.y() = reducedp.y();
			}
			if(pbc[2] == false) {
				if(reducedp.z() < boundingBox.minc.z()) boundingBox.minc.z() = reducedp.z();
				else if(reducedp.z() > boundingBox.maxc.z()) boundingBox.maxc.z() = reducedp.z();
			}
		}
	}

	// Create root node.
	root = nodePool.construct(nullptr, boundingBox);
	numLeafNodes++;

	// Create first level of child nodes by splitting in X direction.
	splitLeafNode(root, 0);

	// Create second level of child nodes by splitting in Y direction.
	splitLeafNode(root->children[0], 1);
	splitLeafNode(root->children[1], 1);

	// Create third level of child nodes by splitting in Z direction.
	splitLeafNode(root->children[0]->children[0], 2);
	splitLeafNode(root->children[0]->children[1], 2);
	splitLeafNode(root->children[1]->children[0], 2);
	splitLeafNode(root->children[1]->children[1], 2);

	// Put particles into bins.
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

		for(int k = 0; k < 3; k++) {
			if(pbc[k]) {
				while(reducedp[k] < 0) { reducedp[k] += 1; a->pos += simCell.column(k); }
				while(reducedp[k] > 1) { reducedp[k] -= 1; a->pos -= simCell.column(k); }
			}
		}

		insertParticle(&*a, reducedp, root, 0);
	}

	return true;
}

/******************************************************************************
* Inserts an atom into the binary tree.
******************************************************************************/
void TreeNeighborListBuilder::insertParticle(NeighborListAtom* atom, const Point3& p, TreeNode* node, int depth)
{
	if(node->isLeaf()) {
		OVITO_ASSERT(node->bounds.classifyPoint(p) != -1);
		// Insert atom into leaf node.
		atom->nextInBin = node->atoms;
		node->atoms = atom;
		node->numAtoms++;
		// If leaf node becomes too large, split it in the largest dimension.
		if(node->numAtoms > bucketSize && depth < maxTreeDepth) {
			OVITO_ASSERT(node->parent != NULL);
			splitLeafNode(node, determineSplitDirection(node));
		}
	}
	else {
		// Decide on which side of the splitting plane the atom is located.
		if(p[node->splitDim] < node->splitPos)
			insertParticle(atom, p, node->children[0], depth+1);
		else
			insertParticle(atom, p, node->children[1], depth+1);
	}
}

/******************************************************************************
* Determines in which direction to split the given leaf node.
******************************************************************************/
int TreeNeighborListBuilder::determineSplitDirection(TreeNode* node)
{
	FloatType dmax = 0.0;
	int dmax_dim = -1;
	for(int dim = 0; dim < 3; dim++) {
		FloatType d = simCell.column(dim).squaredLength() * node->bounds.size(dim) * node->bounds.size(dim);
		if(d > dmax) {
			dmax = d;
			dmax_dim = dim;
		}
	}
	OVITO_ASSERT(dmax_dim >= 0);
	return dmax_dim;
}

/******************************************************************************
* Splits a leaf node into two new leaf nodes and redistributes the atoms to the child nodes.
******************************************************************************/
void TreeNeighborListBuilder::splitLeafNode(TreeNode* node, int splitDim)
{
	node->splitDim = splitDim;
	node->splitPos = (node->bounds.minc[splitDim] + node->bounds.maxc[splitDim]) * 0.5;

	// Create child nodes and define their bounding boxes.
	Box3 lowerBounds(node->bounds);
	Box3 upperBounds(node->bounds);
	lowerBounds.maxc[splitDim] = upperBounds.minc[splitDim] = node->splitPos;
	node->children[0] = nodePool.construct(node, lowerBounds);
	node->children[1] = nodePool.construct(node, upperBounds);

	// Redistribute atoms to child nodes.
	NeighborListAtom* atom = node->atoms;
	while(atom != NULL) {
		NeighborListAtom* next = atom->nextInBin;
		FloatType p = simCellInverse.prodrow(atom->pos, splitDim);
		if(p < node->splitPos) {
			atom->nextInBin = node->children[0]->atoms;
			node->children[0]->atoms = atom;
		}
		else {
			atom->nextInBin = node->children[1]->atoms;
			node->children[1]->atoms = atom;
		}
		atom = next;
	}

	numLeafNodes++;
}

}; // End of namespace
