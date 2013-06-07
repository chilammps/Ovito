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
#include <core/utilities/units/UnitsManager.h>

#include "SimulationCell.h"
#include "SimulationCellDisplay.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(SimulationCell, SceneObject)
DEFINE_PROPERTY_FIELD(SimulationCell, _cellVector1, "CellVector1")
DEFINE_PROPERTY_FIELD(SimulationCell, _cellVector2, "CellVector2")
DEFINE_PROPERTY_FIELD(SimulationCell, _cellVector3, "CellVector3")
DEFINE_PROPERTY_FIELD(SimulationCell, _cellOrigin, "CellTranslation")
DEFINE_PROPERTY_FIELD(SimulationCell, _pbcX, "PeriodicX")
DEFINE_PROPERTY_FIELD(SimulationCell, _pbcY, "PeriodicY")
DEFINE_PROPERTY_FIELD(SimulationCell, _pbcZ, "PeriodicZ")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _cellVector1, "Cell vector 1")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _cellVector2, "Cell vector 2")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _cellVector3, "Cell vector 3")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _cellOrigin, "Cell origin")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _pbcX, "Periodic boundary conditions (X)")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _pbcY, "Periodic boundary conditions (Y)")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _pbcZ, "Periodic boundary conditions (Z)")
SET_PROPERTY_FIELD_UNITS(SimulationCell, _cellVector1, WorldParameterUnit)
SET_PROPERTY_FIELD_UNITS(SimulationCell, _cellVector2, WorldParameterUnit)
SET_PROPERTY_FIELD_UNITS(SimulationCell, _cellVector3, WorldParameterUnit)
SET_PROPERTY_FIELD_UNITS(SimulationCell, _cellOrigin, WorldParameterUnit)

/******************************************************************************
* Creates the storage for the internal parameters.
******************************************************************************/
void SimulationCell::init()
{
	INIT_PROPERTY_FIELD(SimulationCell::_cellVector1);
	INIT_PROPERTY_FIELD(SimulationCell::_cellVector2);
	INIT_PROPERTY_FIELD(SimulationCell::_cellVector3);
	INIT_PROPERTY_FIELD(SimulationCell::_cellOrigin);
	INIT_PROPERTY_FIELD(SimulationCell::_pbcX);
	INIT_PROPERTY_FIELD(SimulationCell::_pbcY);
	INIT_PROPERTY_FIELD(SimulationCell::_pbcZ);

	setDisplayObject(new SimulationCellDisplay());
}

};
