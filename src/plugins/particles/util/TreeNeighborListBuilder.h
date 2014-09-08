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
	TreeNeighborListBuilder(int _numNeighbors = 16) : numNeighbors(_numNeighbors), numLeafNodes(0), maxTreeDepth(1) {
		bucketSize = std::max(numNeighbors * 2, 16);
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
		closestDistanceSq = FLOATTYPE_MAX;
		auto visitor = [&closestIndex, &closestDistanceSq](const Neighbor& n, FloatType& mrs) {
			if(n.distanceSq < closestDistanceSq) {
				mrs = closestDistanceSq = n.distanceSq;
				closestIndex = n.index();
			}
		};
		visitNeighbors(query_point, visitor);
		return closestIndex;
	}

	struct Neighbor
	{
		NeighborListAtom* atom;
		FloatType distanceSq;
		Vector3 delta;

		/// Returns the index of the neighbor atom.
		size_t index() const { return atom->index; }

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
			queue.clear();
			for(const Vector3& pbcShift : t.pbcImages) {
				q = query_point - pbcShift;
				qr = t.simCellInverse * q;
				visitNode(t.root);
			}
			queue.sort();
		}

		/// Returns the neighbor list.
		const BoundedPriorityQueue<Neighbor, std::less<Neighbor>, MAX_NEIGHBORS_LIMIT>& results() const { return queue; }

	private:

		/// Inserts all atoms of the given leaf node into the priority queue.
		void visitNode(TreeNode* node) {
			if(node->isLeaf()) {
				for(NeighborListAtom* atom = node->atoms; atom != nullptr; atom = atom->nextInBin) {
					Neighbor n;
					n.delta = atom->pos - q;
					n.distanceSq = n.delta.squaredLength();
					if(n.distanceSq != 0) {
						n.atom = atom;
						queue.insert(n);
					}
				}
			}
			else {
				if(qr[node->splitDim] < node->splitPos) {
					visitNode(node->children[0]);
					if(!queue.full() || queue.top().distanceSq > t.minimumDistance(node->children[1]->bounds, q))
						visitNode(node->children[1]);
				}
				else {
					visitNode(node->children[1]);
					if(!queue.full() || queue.top().distanceSq > t.minimumDistance(node->children[0]->bounds, q))
						visitNode(node->children[0]);
				}
			}
		}

	private:
		const TreeNeighborListBuilder& t;
		Point3 q, qr;
		BoundedPriorityQueue<Neighbor, std::less<Neighbor>, MAX_NEIGHBORS_LIMIT> queue;
	};

	template<class Visitor>
	void visitNeighbors(const Point3& query_point, Visitor& v) const {
		FloatType mrs = FLOATTYPE_MAX;
		for(const Vector3& pbcShift : pbcImages) {
			Point3 q = query_point - pbcShift;
			Point3 qr = simCellInverse * q;
			if(mrs > minimumDistance(root->bounds, q))
				visitNode(root, q, qr, v, mrs);
		}
	}

private:

	/// Inserts a particle into the binary tree.
	void insertParticle(NeighborListAtom* atom, const Point3& p, TreeNode* node, int depth);

	/// Splits a leaf node into two new leaf nodes and redistributes the atoms to the child nodes.
	void splitLeafNode(TreeNode* node, int splitDim);

	/// Determines in which direction to split the given leaf node.
	int determineSplitDirection(TreeNode* node);

	/// Computes the minimum distance from the query point to the given bounding box.
	FloatType minimumDistance(const Box3& box, const Point3& query_point) const {
		Vector3 p1 = simCell * box.minc - query_point;
		Vector3 p2 = query_point - simCell * box.maxc;
		FloatType minDistance = 0;
		for(size_t dim = 0; dim < 3; dim++) {
			FloatType t_min = planeNormals[dim].dot(p1);
			if(t_min > minDistance) minDistance = t_min;
			FloatType t_max = planeNormals[dim].dot(p2);
			if(t_max > minDistance) minDistance = t_max;
		}
		return minDistance * minDistance;
	}

	template<class Visitor>
	void visitNode(TreeNode* node, const Point3& q, const Point3& qr, Visitor& v, FloatType& mrs) const {
		if(node->isLeaf()) {
			for(NeighborListAtom* atom = node->atoms; atom != nullptr; atom = atom->nextInBin) {
				Neighbor n;
				n.delta = atom->pos - q;
				n.distanceSq = n.delta.squaredLength();
				if(n.distanceSq != 0) {
					n.atom = atom;
					v(n, mrs);
				}
			}
		}
		else {
			if(qr[node->splitDim] < node->splitPos) {
				visitNode(node->children[0], q, qr, v, mrs);
				if(mrs > minimumDistance(node->children[1]->bounds, q))
					visitNode(node->children[1], q, qr, v, mrs);
			}
			else {
				visitNode(node->children[1], q, qr, v, mrs);
				if(mrs > minimumDistance(node->children[0]->bounds, q))
					visitNode(node->children[0], q, qr, v, mrs);
			}
		}
	}

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

	/// The maximum number of particles per leaf node.
	int bucketSize;

	/// List of pbc image shift vectors.
	std::vector<Vector3> pbcImages;

public:

	/// The number of leaf nodes in the tree.
	int numLeafNodes;

	/// The maximum depth of this binary tree.
	int maxTreeDepth;
};

}; // End of namespace

#endif // __OVITO_TREE_NEIGHBOR_LIST_BUILDER_H
