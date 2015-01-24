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

#ifndef __OVITO_CA_CLUSTER_GRAPH_H
#define __OVITO_CA_CLUSTER_GRAPH_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/scene/objects/DataObject.h>
#include "Cluster.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/**
 * \brief Wraps the ClusterGraph class of the CALib.
 */
class OVITO_CRYSTALANALYSIS_EXPORT ClusterGraph : public DataObject
{
public:

	/// \brief Constructor that creates an empty ClusterGraph object.
	Q_INVOKABLE ClusterGraph(DataSet* dataset);

	/// Returns the title of this object.
	virtual QString objectTitle() override { return tr("Clusters"); }

	/// Returns the list of clusters.
	const QVector<Cluster*>& clusters() const { return _clusters; }

	/// Discards all existing clusters and transitions.
	void clear() { _clusters.clear(); }

	/// Adds a cluster to this graph.
	void addCluster(Cluster* cluster) { _clusters.push_back(cluster); }

protected:

	/// Stores the list of clusters.
	VectorReferenceField<Cluster> _clusters;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_clusters);
};

/**
 * \brief A properties editor for the ClusterGraph class.
 */
class OVITO_CRYSTALANALYSIS_EXPORT ClusterGraphEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE ClusterGraphEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

}	// End of namespace
}	// End of namespace
}	// End of namespace

#endif // __OVITO_CA_CLUSTER_GRAPH_H
