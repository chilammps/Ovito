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
#include <core/rendering/SceneRenderer.h>

#include "SimulationCellDisplay.h"
#include "SimulationCell.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, SimulationCellDisplay, DisplayObject)
DEFINE_PROPERTY_FIELD(SimulationCellDisplay, _renderSimulationCell, "RenderSimulationCell")
DEFINE_PROPERTY_FIELD(SimulationCellDisplay, _simulationCellLineWidth, "SimulationCellLineWidth")
DEFINE_PROPERTY_FIELD(SimulationCellDisplay, _simulationCellColor, "SimulationCellRenderingColor")
SET_PROPERTY_FIELD_LABEL(SimulationCellDisplay, _simulationCellLineWidth, "Line width")
SET_PROPERTY_FIELD_LABEL(SimulationCellDisplay, _renderSimulationCell, "Render cell")
SET_PROPERTY_FIELD_LABEL(SimulationCellDisplay, _simulationCellColor, "Line color")
SET_PROPERTY_FIELD_UNITS(SimulationCellDisplay, _simulationCellLineWidth, WorldParameterUnit)

/******************************************************************************
* Constructor.
******************************************************************************/
SimulationCellDisplay::SimulationCellDisplay() :
	_renderSimulationCell(true),
	_simulationCellLineWidth(0.5),
	_simulationCellColor(0, 0, 0)
{
	INIT_PROPERTY_FIELD(SimulationCellDisplay::_renderSimulationCell);
	INIT_PROPERTY_FIELD(SimulationCellDisplay::_simulationCellLineWidth);
	INIT_PROPERTY_FIELD(SimulationCellDisplay::_simulationCellColor);
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 SimulationCellDisplay::boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	SimulationCell* cellObject = dynamic_object_cast<SimulationCell>(sceneObject);
	OVITO_CHECK_OBJECT_POINTER(cellObject);

	return cellObject->boundingBox().padBox(simulationCellLineWidth());
}

/******************************************************************************
* Lets the display object render a scene object.
******************************************************************************/
void SimulationCellDisplay::render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	if(!renderSimulationCell())
		return;		// Do nothing if rendering has been disabled by the user.

	SimulationCell* cell = dynamic_object_cast<SimulationCell>(sceneObject);
	OVITO_CHECK_OBJECT_POINTER(cell);

	if(_geometryCacheHelper.updateState(cell, cell ? cell->revisionNumber() : 0)
			|| !_lineGeometry
			|| !_lineGeometry->isValid(renderer)) {
		_lineGeometry = renderer->createLineGeometryBuffer();
		_lineGeometry->setSize(24);
		Point3 corners[8];
		corners[0] = cell->origin();
		corners[1] = corners[0] + cell->edgeVector1();
		corners[2] = corners[1] + cell->edgeVector2();
		corners[3] = corners[0] + cell->edgeVector2();
		corners[4] = corners[0] + cell->edgeVector3();
		corners[5] = corners[1] + cell->edgeVector3();
		corners[6] = corners[2] + cell->edgeVector3();
		corners[7] = corners[3] + cell->edgeVector3();
		Point3 vertices[24] = {
			corners[0], corners[1],
			corners[1], corners[2],
			corners[2], corners[3],
			corners[3], corners[0],
			corners[4], corners[5],
			corners[5], corners[6],
			corners[6], corners[7],
			corners[7], corners[4],
			corners[0], corners[4],
			corners[1], corners[5],
			corners[2], corners[6],
			corners[3], corners[7]};
		_lineGeometry->setVertexPositions(vertices);
		_lineGeometry->setVertexColor(ColorA(1,1,1));
	}
	_lineGeometry->render();
}

};
