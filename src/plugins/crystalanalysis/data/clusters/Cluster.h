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

#ifndef __OVITO_CA_CLUSTER_H
#define __OVITO_CA_CLUSTER_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/reference/RefTarget.h>
#include "../patterns/StructurePattern.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

class Cluster;

/**
 * \brief Stores a cluster transition matrix.
 */
class OVITO_CRYSTALANALYSIS_EXPORT ClusterTransition
{
public:

	/// Default constructor.
	ClusterTransition() : _tm(Matrix3::Zero()), _cluster2(nullptr) {}

	/// Constructor.
	ClusterTransition(const Matrix3& tm, Cluster* cluster2) : _tm(tm), _cluster2(cluster2) {}

	/// Copy constructor.
	ClusterTransition(const ClusterTransition& other) : _tm(other._tm), _cluster2(other._cluster2) {}

	/// Returns the lattice vector transformation matrix.
	const Matrix3& tm() const { return _tm; }

	/// Returns the destination cluster.
	Cluster* cluster2() const { return _cluster2; }

private:

	/// The lattice vector transformation matrix.
	Matrix3 _tm;

	/// The cluster to whose coordinate system vectors are transformed into by this transition matrix.
	QPointer<Cluster> _cluster2;
};

/**
 * \brief Stores a cluster.
 */
class OVITO_CRYSTALANALYSIS_EXPORT Cluster : public RefTarget
{
public:

	/// \brief Constructs a new cluster.
	Q_INVOKABLE Cluster(DataSet* dataset);

	/// Returns the structure type of the cluster
	StructurePattern* pattern() const { return _pattern; }

	/// Sets the structure type of the cluster
	void setPattern(StructurePattern* pattern) { _pattern = pattern; }

	/// Returns the identifier of the cluster.
	int id() const { return _id; }

	/// Sets the identifier of the cluster.
	void setId(int id) { _id = id; }

	/// Returns the number of atoms that are part of the cluster.
	int atomCount() const { return _atomCount; }

	/// Sets the number of atoms that are part of the cluster.
	void setAtomCount(int count) { _atomCount = count; }

	/// Returns the matrix that transforms local lattice vectors to the simulation coordinate system.
	const Matrix3& orientation() const { return _orientation; }

	/// Sets the matrix that transforms local lattice vectors to the simulation coordinate system.
	void setOrientation(const Matrix3& tm) { _orientation = tm; }

	/// Returns the  list of transitions from this cluster to adjacent clusters.
	const QVector<ClusterTransition>& transitions() const { return _transitions; }

	/// Adds a transition to this cluster.
	void addTransition(Cluster* cluster2, const Matrix3& tm) {
		_transitions.push_back(ClusterTransition(tm,cluster2));
	}

protected:

	/// The structure type of the cluster.
	ReferenceField<StructurePattern> _pattern;

	/// The internal identifier of the cluster.
	PropertyField<int> _id;

	/// Number of atoms that are part of the cluster.
	PropertyField<int> _atomCount;

	/// Matrix that transforms local lattice vectors to the simulation coordinate system.
	PropertyField<Matrix3> _orientation;

	/// The list of transitions from this cluster to adjacent clusters.
	QVector<ClusterTransition> _transitions;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_pattern);
	DECLARE_PROPERTY_FIELD(_id);
	DECLARE_PROPERTY_FIELD(_atomCount);
	DECLARE_PROPERTY_FIELD(_orientation);
};

}	// End of namespace
}	// End of namespace
}	// End of namespace

#endif // __OVITO_CA_CLUSTER_H
