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
#include "DislocationNetwork.h"

namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, DislocationNetwork, SceneObject)
DEFINE_VECTOR_REFERENCE_FIELD(DislocationNetwork, _segments, "DislocationSegments", DislocationSegment)
SET_PROPERTY_FIELD_LABEL(DislocationNetwork, _segments, "Dislocation segments")

/******************************************************************************
* Constructor.
******************************************************************************/
DislocationNetwork::DislocationNetwork()
{
	INIT_PROPERTY_FIELD(DislocationNetwork::_segments);
}

};	// End of namespace
