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
#include "Cluster.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, Cluster, RefTarget);
DEFINE_REFERENCE_FIELD(Cluster, _pattern, "Structure", StructurePattern);
DEFINE_PROPERTY_FIELD(Cluster, _id, "ID");
DEFINE_PROPERTY_FIELD(Cluster, _atomCount, "AtomCount");
DEFINE_PROPERTY_FIELD(Cluster, _orientation, "Orientation");

/******************************************************************************
* Constructs a new Cluster.
******************************************************************************/
Cluster::Cluster(DataSet* dataset) : RefTarget(dataset), _id(-1), _atomCount(0), _orientation(Matrix3::Zero())
{
	INIT_PROPERTY_FIELD(Cluster::_pattern);
	INIT_PROPERTY_FIELD(Cluster::_id);
	INIT_PROPERTY_FIELD(Cluster::_atomCount);
	INIT_PROPERTY_FIELD(Cluster::_orientation);
}

}	// End of namespace
}	// End of namespace
}	// End of namespace

