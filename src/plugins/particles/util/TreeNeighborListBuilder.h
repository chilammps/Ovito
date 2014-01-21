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
 * \file TreeNeighborListBuilder.h
 * \brief Contains the definition of the Particles::TreeNeighborListBuilder class.
 */

#ifndef __OVITO_TREE_NEIGHBOR_LIST_BUILDER_H
#define __OVITO_TREE_NEIGHBOR_LIST_BUILDER_H

#include <core/Core.h>
#include <base/utilities/BoundedPriorityQueue.h>
#include <base/utilities/MemoryPool.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/SimulationCellData.h>

namespace Particles {

using namespace Ovito;

/**
 * \brief Finds the N nearest neighbors of particles.
 */
class OVITO_PARTICLES_EXPORT TreeNeighborListBuilder
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

	struct TreeNode {
		/// Constructor for a leaf node.
		TreeNode(TreeNode* _parent, const Box3& _bounds) :
			parent(_parent), bounds(_bounds), atoms(nullptr), numAtoms(0), children{nullptr,nullptr} {}

		/// Returns true this is a leaf node.
		bool isLeaf() const { return children[0] == nullptr; }

		/// The parent node of this tree node.
		TreeNode* parent;
		/// The bounding box of the node.
		Box3 bounds;
		/// The dimension of the splitting if this is not a leaf node.
		int splitDim;
		/// The position of the split plane.
		FloatType splitPos;
		/// The two child nodes if this is not a leaf node.
		TreeNode* children[2];
		/// The linked list of atoms if this is a leaf node.
		NeighborListAtom* atoms;
		/// Number of atoms in this leaf node.
		int numAtoms;
	};

public:

	//// Constructor that builds the binary search tree.
	TreeNeighborListBuilder(int _numNeighbors) : numNeighbors(_numNeighbors), numLeafNodes(0) {
		bucketSize = std::max(numNeighbors * 2, 16);
		maxTreeDepth = 17;
	}

	/// \brief Prepares the tree data structure.
	/// \param posProperty The positions of the particles.
	/// \param simCell The simulation cell data.
	/// \return \c false when the operation has been canceled by the user;
	///         \c true on success.
	/// \throw Exception on error.
	bool prepare(ParticleProperty* posProperty, const SimulationCellData& cellData);

	/// Returns the position of the i-th particle.
	const Point3& particlePos(size_t index) const {
		OVITO_ASSERT(index >= 0 && index < atoms.size());
		return atoms[index].pos;
	}

	/// Returns the index of the particle closest to the given point.
	int findClosestParticle(const Point3& query_point, FloatType& closestDistanceSq) const {
		int closestIndex = -1;
		closestDistanceSq = std::numeric_limits<FloatType>::max();
		Point3 qr = simCellInverse * query_point;
		for(auto pbcImage = pbcImages.begin(); pbcImage != pbcImages.end(); ++pbcImage) {
			findClosestParticleRecursive(root, pbcImage->first, pbcImage->second, query_point, qr, closestIndex, closestDistanceSq);
		}
		return closestIndex;
	}

	struct Neighbor
	{
		NeighborListAtom* atom;
		FloatType distanceSq;
		Vector3 delta;

		/// Used for ordering.
		bool operator<(const Neighbor& other) const { return distanceSq < other.distanceSq; }
	};

	template<int MAX_NEIGHBORS_LIMIT>
	class Locator
	{
	public:

		/// Constructor.
		Locator(const TreeNeighborListBuilder& tree) : t(tree), queue(tree.numNeighbors) {}

		/// Builds the sorted list of neighbors around the given point.
		void findNeighbors(const Point3& query_point) {
			q = query_point;
			qr = t.simCellInverse * query_point;
			queue.clear();
			for(auto pbcImage = t.pbcImages.begin(); pbcImage != t.pbcImages.end(); ++pbcImage)
				visitNode(t.root, pbcImage->first, pbcImage->second);
			queue.sort();
		}

		/// Returns the neighbor list.
		const BoundedPriorityQueue<Neighbor, std::less<Neighbor>, MAX_NEIGHBORS_LIMIT>& results() const { return queue; }

	private:

		/// Inserts all atoms of the given leaf node into the priority queue.
		void visitNode(TreeNode* node, const Vector3& shift, const Vector3& rshift) {
			if(node->isLeaf()) {
				Point3 qs = q - shift;
				for(NeighborListAtom* atom = node->atoms; atom != nullptr; atom = atom->nextInBin) {
					Neighbor n;
					n.delta = atom->pos - qs;
					n.distanceSq = n.delta.squaredLength();
					if(n.distanceSq != 0) {
						n.atom = atom;
						queue.insert(n);
					}
				}
			}
			else {
				if(qr[node->splitDim] < node->splitPos + rshift[node->splitDim]) {
					visitNode(node->children[0], shift, rshift);
					if(!queue.full() || queue.top().distanceSq > t.minimumDistance(node->children[1]->bounds, shift, q))
						visitNode(node->children[1], shift, rshift);
				}
				else {
					visitNode(node->children[1], shift, rshift);
					if(!queue.full() || queue.top().distanceSq > t.minimumDistance(node->children[0]->bounds, shift, q))
						visitNode(node->children[0], shift, rshift);
				}
			}
		}

	private:
		const TreeNeighborListBuilder& t;
		Point3 q, qr;
		BoundedPriorityQueue<Neighbor, std::less<Neighbor>, MAX_NEIGHBORS_LIMIT> queue;
	};

private:

	/// Inserts a particle into the binary tree.
	void insertParticle(NeighborListAtom* atom, const Point3& p, TreeNode* node, int depth);

	/// Splits a leaf node into two new leaf nodes and redistributes the atoms to the child nodes.
	void splitLeafNode(TreeNode* node, int splitDim);

	/// Determines in which direction to split the given leaf node.
	int determineSplitDirection(TreeNode* node);

	/// Computes the minimum distance from the query point to the given bounding box.
	FloatType minimumDistance(const Box3& box, const Vector3& shift, const Point3& query_point) const {
		Vector3 p1 = simCell * box.minc - query_point + shift;
		Vector3 p2 = query_point - simCell * box.maxc - shift;
		FloatType minDistance = 0;
		for(size_t dim = 0; dim < 3; dim++) {
			FloatType t_min = planeNormals[dim].dot(p1);
			if(t_min > minDistance) minDistance = t_min;
			FloatType t_max = planeNormals[dim].dot(p2);
			if(t_max > minDistance) minDistance = t_max;
		}
		return minDistance * minDistance;
	}

	/// Recursive closest particle search function.
	void findClosestParticleRecursive(TreeNode* node, const Vector3& shift, const Vector3& rshift, const Point3& q, const Point3& qr, int& closestIndex, FloatType& closestDistanceSq) const;

private:

	/// The internal list of atoms.
	std::vector<NeighborListAtom> atoms;

	// Simulation cell properties.
	AffineTransformation simCell;
	AffineTransformation simCellInverse;
	std::array<bool,3> pbc;

	/// The normal vectors of the three cell planes.
	Vector3 planeNormals[3];

	/// Used to allocate instances of TreeNode.
	MemoryPool<TreeNode> nodePool;

	/// The root node of the binary tree.
	TreeNode* root;

	/// The number of neighbors to finds for each atom.
	int numNeighbors;

	/// The maximum number of atoms per leaf node.
	int bucketSize;

	/// The maximum depth of the binary tree.
	int maxTreeDepth;

	/// List of pbc image shift vectors.
	std::vector<std::pair<Vector3,Vector3>> pbcImages;

public:

	/// The number of leaf nodes in the tree.
	int numLeafNodes;
};

}; // End of namespace

#endif // __OVITO_TREE_NEIGHBOR_LIST_BUILDER_H
