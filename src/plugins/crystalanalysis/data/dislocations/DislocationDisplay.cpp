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
#include <core/rendering/SceneRenderer.h>
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <plugins/particles/data/SimulationCell.h>
#include "DislocationDisplay.h"
#include "DislocationNetwork.h"

namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, DislocationDisplay, DisplayObject)
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, DislocationDisplayEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(DislocationDisplay, DislocationDisplayEditor)
DEFINE_FLAGS_PROPERTY_FIELD(DislocationDisplay, _lineWidth, "LineWidth", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(DislocationDisplay, _shadingMode, "ShadingMode", PROPERTY_FIELD_MEMORIZE)
SET_PROPERTY_FIELD_LABEL(DislocationDisplay, _lineWidth, "Dislocation line width")
SET_PROPERTY_FIELD_LABEL(DislocationDisplay, _shadingMode, "Shading mode")
SET_PROPERTY_FIELD_UNITS(DislocationDisplay, _lineWidth, WorldParameterUnit)

/******************************************************************************
* Constructor.
******************************************************************************/
DislocationDisplay::DislocationDisplay() :
	_lineWidth(1.0f), _shadingMode(ArrowGeometryBuffer::NormalShading)
{
	INIT_PROPERTY_FIELD(DislocationDisplay::_lineWidth);
	INIT_PROPERTY_FIELD(DislocationDisplay::_shadingMode);
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 DislocationDisplay::boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	SimulationCell* cellObject = flowState.findObject<SimulationCell>();
	if(!cellObject)
		return Box3();

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(sceneObject, sceneObject ? sceneObject->revisionNumber() : 0,
			cellObject->data(), lineWidth()) || _cachedBoundingBox.isEmpty()) {
		// Recompute bounding box.
		_cachedBoundingBox = Box3(Point3(0,0,0), Point3(1,1,1)).transformed(cellObject->cellMatrix());
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Lets the display object render a scene object.
******************************************************************************/
void DislocationDisplay::render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	// Get the simulation cell.
	SimulationCell* cellObject = flowState.findObject<SimulationCell>();
	if(!cellObject)
		return;

	// Do we have to re-create the geometry buffers from scratch?
	bool recreateBuffers = !_segmentBuffer || !_segmentBuffer->isValid(renderer)
						|| !_cornerBuffer || !_cornerBuffer->isValid(renderer);

	// Set up shading mode.
	ParticleGeometryBuffer::ShadingMode cornerShadingMode = (shadingMode() == ArrowGeometryBuffer::NormalShading)
			? ParticleGeometryBuffer::NormalShading : ParticleGeometryBuffer::FlatShading;
	if(!recreateBuffers) {
		recreateBuffers |= !_segmentBuffer->setShadingMode(shadingMode());
		recreateBuffers |= !_cornerBuffer->setShadingMode(cornerShadingMode);
	}

	// Do we have to update contents of the geometry buffers?
	bool updateContents = _geometryCacheHelper.updateState(
			sceneObject, sceneObject ? sceneObject->revisionNumber() : 0,
			cellObject->data(), lineWidth()) || recreateBuffers;

	// Re-create the geometry buffers if necessary.
	if(recreateBuffers) {
		_segmentBuffer = renderer->createArrowGeometryBuffer(ArrowGeometryBuffer::CylinderShape, shadingMode(), ArrowGeometryBuffer::HighQuality);
		_cornerBuffer = renderer->createParticleGeometryBuffer(cornerShadingMode, ParticleGeometryBuffer::HighQuality);
	}

	// Update buffer contents.
	if(updateContents) {
		SimulationCellData cellData = cellObject->data();
		if(OORef<DislocationNetwork> dislocationObj = sceneObject->convertTo<DislocationNetwork>(time)) {
			int lineSegmentCount = 0, cornerCount = 0;
			for(DislocationSegment* segment : dislocationObj->segments()) {
				clipDislocationLine(segment->line(), cellData, [&lineSegmentCount, &cornerCount](const Point3&, const Point3&, bool isInitialSegment) {
					lineSegmentCount++;
					if(!isInitialSegment) cornerCount++;
				});
			}
			_segmentBuffer->startSetElements(lineSegmentCount);
			int lineSegmentIndex = 0;
			FloatType lineRadius = std::max(lineWidth() / 2, FloatType(0));
			QVector<Point3> cornerPoints;
			QVector<Color> cornerColors;
			cornerPoints.reserve(cornerCount);
			cornerColors.reserve(cornerCount);
			for(DislocationSegment* segment : dislocationObj->segments()) {
				Color lineColor = segment->burgersVectorFamily()->color();
				clipDislocationLine(segment->line(), cellData, [this, &lineSegmentIndex, &cornerPoints, &cornerColors, lineColor, lineRadius](const Point3& v1, const Point3& v2, bool isInitialSegment) {
					_segmentBuffer->setElement(lineSegmentIndex++, v1, v2 - v1, ColorA(lineColor), lineRadius);
					if(!isInitialSegment) {
						cornerPoints.push_back(v1);
						cornerColors.push_back(lineColor);
					}
				});
			}
			_segmentBuffer->endSetElements();
			_cornerBuffer->setSize(cornerPoints.size());
			_cornerBuffer->setParticlePositions(cornerPoints.empty() ? nullptr : cornerPoints.data());
			_cornerBuffer->setParticleColors(cornerColors.empty() ? nullptr : cornerColors.data());
			_cornerBuffer->setParticleRadius(lineRadius);
		}
		else {
			_cornerBuffer = nullptr;
			_segmentBuffer = nullptr;
		}
	}

	// Render segments.
	quint32 pickingBaseID = 0;
	if(_cornerBuffer) {
		if(renderer->isPicking())
			pickingBaseID = renderer->registerPickObject(contextNode, sceneObject, _cornerBuffer->particleCount());
		_cornerBuffer->render(renderer, pickingBaseID);
	}
	if(_segmentBuffer) {
		if(renderer->isPicking())
			pickingBaseID = renderer->registerPickObject(contextNode, sceneObject, _segmentBuffer->elementCount());
		_segmentBuffer->render(renderer, pickingBaseID);
	}
}

/******************************************************************************
* Clips a dislocation line at the periodic box boundaries.
******************************************************************************/
void DislocationDisplay::clipDislocationLine(const QVector<Point3>& line, const SimulationCellData& simulationCell, const std::function<void(const Point3&, const Point3&, bool)>& segmentCallback)
{
	auto v1 = line.cbegin();
	Point3 rp1 = simulationCell.absoluteToReduced(*v1);
	Vector3 shiftVector = Vector3::Zero();
	for(size_t dim = 0; dim < 3; dim++) {
		if(simulationCell.pbcFlags()[dim]) {
			while(rp1[dim] > 0) { rp1[dim] -= 1; shiftVector[dim] -= 1; }
			while(rp1[dim] < 0) { rp1[dim] += 1; shiftVector[dim] += 1; }
		}
	}
	bool isInitialSegment = true;
	for(auto v2 = v1 + 1; v2 != line.cend(); v1 = v2, ++v2) {
		Point3 rp2 = simulationCell.absoluteToReduced(*v2) + shiftVector;
		FloatType smallestT;
		do {
			size_t crossDim;
			FloatType crossDir;
			smallestT = FLOATTYPE_MAX;
			for(size_t dim = 0; dim < 3; dim++) {
				if(simulationCell.pbcFlags()[dim]) {
					int d = (int)floor(rp2[dim]) - (int)floor(rp1[dim]);
					if(d == 0) continue;
					FloatType t;
					if(d > 0)
						t = (ceil(rp1[dim]) - rp1[dim]) / (rp2[dim] - rp1[dim]);
					else
						t = (floor(rp1[dim]) - rp1[dim]) / (rp2[dim] - rp1[dim]);
					if(t > 0 && t < smallestT) {
						smallestT = t;
						crossDim = dim;
						crossDir = (d > 0) ? 1 : -1;
					}
				}
			}
			if(smallestT != FLOATTYPE_MAX) {
				Point3 intersection = rp1 + smallestT * (rp2 - rp1);
				intersection[crossDim] = floor(intersection[crossDim] + FloatType(0.5));
				segmentCallback(simulationCell.reducedToAbsolute(rp1), simulationCell.reducedToAbsolute(intersection), isInitialSegment);
				shiftVector[crossDim] -= crossDir;
				rp1 = intersection;
				rp1[crossDim] -= crossDir;
				rp2[crossDim] -= crossDir;
				isInitialSegment = true;
			}
		}
		while(smallestT != FLOATTYPE_MAX);

		segmentCallback(simulationCell.reducedToAbsolute(rp1), simulationCell.reducedToAbsolute(rp2), isInitialSegment);
		isInitialSegment = false;
		rp1 = rp2;
	}
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void DislocationDisplayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Dislocation display"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	// Shading mode.
	VariantComboBoxParameterUI* shadingModeUI = new VariantComboBoxParameterUI(this, "shadingMode");
	shadingModeUI->comboBox()->addItem(tr("Normal"), qVariantFromValue(ArrowGeometryBuffer::NormalShading));
	shadingModeUI->comboBox()->addItem(tr("Flat"), qVariantFromValue(ArrowGeometryBuffer::FlatShading));
	layout->addWidget(new QLabel(tr("Shading mode:")), 0, 0);
	layout->addWidget(shadingModeUI->comboBox(), 0, 1);

	// Line width parameter.
	FloatParameterUI* lineWidthUI = new FloatParameterUI(this, PROPERTY_FIELD(DislocationDisplay::_lineWidth));
	layout->addWidget(lineWidthUI->label(), 1, 0);
	layout->addLayout(lineWidthUI->createFieldLayout(), 1, 1);
	lineWidthUI->setMinValue(0);
}

};
