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

#include <plugins/particles/Particles.h>
#include <core/utilities/units/UnitsManager.h>
#include <core/rendering/SceneRenderer.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>

#include "SimulationCellDisplay.h"
#include "SimulationCellObject.h"

namespace Ovito { namespace Particles {

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, SimulationCellDisplayEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, SimulationCellDisplay, DisplayObject);
SET_OVITO_OBJECT_EDITOR(SimulationCellDisplay, SimulationCellDisplayEditor);
DEFINE_PROPERTY_FIELD(SimulationCellDisplay, _renderSimulationCell, "RenderSimulationCell");
DEFINE_PROPERTY_FIELD(SimulationCellDisplay, _simulationCellLineWidth, "SimulationCellLineWidth");
DEFINE_FLAGS_PROPERTY_FIELD(SimulationCellDisplay, _simulationCellColor, "SimulationCellRenderingColor", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(SimulationCellDisplay, _simulationCellLineWidth, "Line width");
SET_PROPERTY_FIELD_LABEL(SimulationCellDisplay, _renderSimulationCell, "Render cell");
SET_PROPERTY_FIELD_LABEL(SimulationCellDisplay, _simulationCellColor, "Line color");
SET_PROPERTY_FIELD_UNITS(SimulationCellDisplay, _simulationCellLineWidth, WorldParameterUnit);

/******************************************************************************
* Constructor.
******************************************************************************/
SimulationCellDisplay::SimulationCellDisplay(DataSet* dataset) : DisplayObject(dataset),
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
Box3 SimulationCellDisplay::boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	SimulationCellObject* cellObject = dynamic_object_cast<SimulationCellObject>(dataObject);
	OVITO_CHECK_OBJECT_POINTER(cellObject);

	return cellObject->boundingBox().padBox(simulationCellLineWidth());
}

/******************************************************************************
* Lets the display object render the data object.
******************************************************************************/
void SimulationCellDisplay::render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	SimulationCellObject* cell = dynamic_object_cast<SimulationCellObject>(dataObject);
	OVITO_CHECK_OBJECT_POINTER(cell);

	if(renderer->isInteractive() && !renderer->viewport()->renderPreviewMode()) {
		renderWireframe(cell, renderer, contextNode);
	}
	else {
		if(!renderSimulationCell())
			return;		// Do nothing if rendering has been disabled by the user.

		renderSolid(cell, renderer, contextNode);
	}
}

/******************************************************************************
* Renders the given simulation cell using lines.
******************************************************************************/
void SimulationCellDisplay::renderWireframe(SimulationCellObject* cell, SceneRenderer* renderer, ObjectNode* contextNode)
{
	ColorA color = ViewportSettings::getSettings().viewportColor(contextNode->isSelected() ? ViewportSettings::COLOR_SELECTION : ViewportSettings::COLOR_UNSELECTED);

	if(_wireframeGeometryCacheHelper.updateState(cell, color)
			|| !_wireframeGeometry
			|| !_wireframeGeometry->isValid(renderer)
			|| !_wireframePickingGeometry
			|| !_wireframePickingGeometry->isValid(renderer)) {
		_wireframeGeometry = renderer->createLinePrimitive();
		_wireframePickingGeometry = renderer->createLinePrimitive();
		_wireframeGeometry->setVertexCount(24);
		_wireframePickingGeometry->setVertexCount(24, renderer->defaultLinePickingWidth());
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
		_wireframeGeometry->setLineColor(color);
		_wireframePickingGeometry->setVertexPositions(vertices);
		_wireframePickingGeometry->setLineColor(color);
	}

	renderer->beginPickObject(contextNode);
	if(!renderer->isPicking())
		_wireframeGeometry->render(renderer);
	else
		_wireframePickingGeometry->render(renderer);
	renderer->endPickObject();
}

/******************************************************************************
* Renders the given simulation cell using solid shading mode.
******************************************************************************/
void SimulationCellDisplay::renderSolid(SimulationCellObject* cell, SceneRenderer* renderer, ObjectNode* contextNode)
{
	if(_solidGeometryCacheHelper.updateState(cell, simulationCellLineWidth(), simulationCellRenderingColor())
			|| !_edgeGeometry || !_cornerGeometry
			|| !_edgeGeometry->isValid(renderer)
			|| !_cornerGeometry->isValid(renderer)) {
		_edgeGeometry = renderer->createArrowPrimitive(ArrowPrimitive::CylinderShape, ArrowPrimitive::NormalShading, ArrowPrimitive::HighQuality);
		_cornerGeometry = renderer->createParticlePrimitive(ParticlePrimitive::NormalShading, ParticlePrimitive::HighQuality);
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
	renderer->beginPickObject(contextNode);
	_edgeGeometry->render(renderer);
	_cornerGeometry->render(renderer);
	renderer->endPickObject();
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

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

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace
