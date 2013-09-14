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
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>

#include "SimulationCellDisplay.h"
#include "SimulationCell.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, SimulationCellDisplay, DisplayObject)
IMPLEMENT_OVITO_OBJECT(Viz, SimulationCellDisplayEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(SimulationCellDisplay, SimulationCellDisplayEditor)
DEFINE_PROPERTY_FIELD(SimulationCellDisplay, _renderSimulationCell, "RenderSimulationCell")
DEFINE_PROPERTY_FIELD(SimulationCellDisplay, _simulationCellLineWidth, "SimulationCellLineWidth")
DEFINE_FLAGS_PROPERTY_FIELD(SimulationCellDisplay, _simulationCellColor, "SimulationCellRenderingColor", PROPERTY_FIELD_MEMORIZE)
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
	SimulationCell* cell = dynamic_object_cast<SimulationCell>(sceneObject);
	OVITO_CHECK_OBJECT_POINTER(cell);

	if(renderer->isInteractive()) {
		renderWireframe(cell, renderer);
	}
	else {
		if(!renderSimulationCell())
			return;		// Do nothing if rendering has been disabled by the user.

		renderSolid(cell, renderer);
	}
}

/******************************************************************************
* Renders the given simulation using wireframe mode.
******************************************************************************/
void SimulationCellDisplay::renderWireframe(SimulationCell* cell, SceneRenderer* renderer)
{
	if(_wireframeGeometryCacheHelper.updateState(cell, cell->revisionNumber())
			|| !_wireframeGeometry
			|| !_wireframeGeometry->isValid(renderer)) {
		_wireframeGeometry = renderer->createLineGeometryBuffer();
		_wireframeGeometry->setSize(24);
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
		_wireframeGeometry->setVertexPositions(vertices);
		_wireframeGeometry->setVertexColor(ColorA(1,1,1));
	}
	_wireframeGeometry->render(renderer);
}

/******************************************************************************
* Renders the given simulation using solid shading mode.
******************************************************************************/
void SimulationCellDisplay::renderSolid(SimulationCell* cell, SceneRenderer* renderer)
{
	if(_solidGeometryCacheHelper.updateState(cell, cell->revisionNumber(), simulationCellLineWidth(), simulationCellRenderingColor())
			|| !_edgeGeometry || !_cornerGeometry
			|| !_edgeGeometry->isValid(renderer)
			|| !_cornerGeometry->isValid(renderer)) {
		_edgeGeometry = renderer->createArrowGeometryBuffer(ArrowGeometryBuffer::CylinderShape, ArrowGeometryBuffer::NormalShading, ArrowGeometryBuffer::HighQuality);
		_cornerGeometry = renderer->createParticleGeometryBuffer(ParticleGeometryBuffer::NormalShading, ParticleGeometryBuffer::HighQuality);
		_edgeGeometry->startSetElements(12);
		ColorA color = (ColorA)simulationCellRenderingColor();
		Point3 corners[8];
		corners[0] = cell->origin();
		corners[1] = corners[0] + cell->edgeVector1();
		corners[2] = corners[1] + cell->edgeVector2();
		corners[3] = corners[0] + cell->edgeVector2();
		corners[4] = corners[0] + cell->edgeVector3();
		corners[5] = corners[1] + cell->edgeVector3();
		corners[6] = corners[2] + cell->edgeVector3();
		corners[7] = corners[3] + cell->edgeVector3();
		_edgeGeometry->setElement(0, corners[0], corners[1] - corners[0], color, simulationCellLineWidth());
		_edgeGeometry->setElement(1, corners[1], corners[2] - corners[1], color, simulationCellLineWidth());
		_edgeGeometry->setElement(2, corners[2], corners[3] - corners[2], color, simulationCellLineWidth());
		_edgeGeometry->setElement(3, corners[3], corners[0] - corners[3], color, simulationCellLineWidth());
		_edgeGeometry->setElement(4, corners[4], corners[5] - corners[4], color, simulationCellLineWidth());
		_edgeGeometry->setElement(5, corners[5], corners[6] - corners[5], color, simulationCellLineWidth());
		_edgeGeometry->setElement(6, corners[6], corners[7] - corners[6], color, simulationCellLineWidth());
		_edgeGeometry->setElement(7, corners[7], corners[4] - corners[7], color, simulationCellLineWidth());
		_edgeGeometry->setElement(8, corners[0], corners[4] - corners[0], color, simulationCellLineWidth());
		_edgeGeometry->setElement(9, corners[1], corners[5] - corners[1], color, simulationCellLineWidth());
		_edgeGeometry->setElement(10, corners[2], corners[6] - corners[2], color, simulationCellLineWidth());
		_edgeGeometry->setElement(11, corners[3], corners[7] - corners[3], color, simulationCellLineWidth());
		_edgeGeometry->endSetElements();
		_cornerGeometry->setSize(8);
		_cornerGeometry->setParticlePositions(corners);
		_cornerGeometry->setParticleRadius(simulationCellLineWidth());
		_cornerGeometry->setParticleColor(simulationCellRenderingColor());
	}
	_edgeGeometry->render(renderer);
	_cornerGeometry->render(renderer);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void SimulationCellDisplayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Simulation cell"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	// Render cell
	BooleanParameterUI* renderCellUI = new BooleanParameterUI(this, PROPERTY_FIELD(SimulationCellDisplay::_renderSimulationCell));
	layout->addWidget(renderCellUI->checkBox(), 0, 0, 1, 2);

	// Line width
	FloatParameterUI* scalingFactorUI = new FloatParameterUI(this, PROPERTY_FIELD(SimulationCellDisplay::_simulationCellLineWidth));
	layout->addWidget(scalingFactorUI->label(), 1, 0);
	layout->addLayout(scalingFactorUI->createFieldLayout(), 1, 1);
	scalingFactorUI->setMinValue(0);

	// Line color
	ColorParameterUI* lineColorUI = new ColorParameterUI(this, PROPERTY_FIELD(SimulationCellDisplay::_simulationCellColor));
	layout->addWidget(lineColorUI->label(), 2, 0);
	layout->addWidget(lineColorUI->colorPicker(), 2, 1);
}

};
