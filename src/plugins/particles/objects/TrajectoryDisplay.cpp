///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2015) Alexander Stukowski
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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>
#include "TrajectoryDisplay.h"

namespace Ovito { namespace Particles {

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, TrajectoryDisplayEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, TrajectoryDisplay, DisplayObject);
SET_OVITO_OBJECT_EDITOR(TrajectoryDisplay, TrajectoryDisplayEditor);
DEFINE_FLAGS_PROPERTY_FIELD(TrajectoryDisplay, _lineWidth, "LineWidth", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(TrajectoryDisplay, _lineColor, "LineColor", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(TrajectoryDisplay, _shadingMode, "ShadingMode", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(TrajectoryDisplay, _showUpToCurrentTime, "ShowUpToCurrentTime");
SET_PROPERTY_FIELD_LABEL(TrajectoryDisplay, _lineWidth, "Line width");
SET_PROPERTY_FIELD_LABEL(TrajectoryDisplay, _lineColor, "Line color");
SET_PROPERTY_FIELD_LABEL(TrajectoryDisplay, _shadingMode, "Shading mode");
SET_PROPERTY_FIELD_LABEL(TrajectoryDisplay, _showUpToCurrentTime, "Show up to current time only");
SET_PROPERTY_FIELD_UNITS(TrajectoryDisplay, _lineWidth, WorldParameterUnit);

/******************************************************************************
* Constructor.
******************************************************************************/
TrajectoryDisplay::TrajectoryDisplay(DataSet* dataset) : DisplayObject(dataset),
	_lineWidth(0.2), _lineColor(0.6, 0.6, 0.6),
	_shadingMode(ArrowPrimitive::FlatShading), _showUpToCurrentTime(false)
{
	INIT_PROPERTY_FIELD(TrajectoryDisplay::_lineWidth);
	INIT_PROPERTY_FIELD(TrajectoryDisplay::_lineColor);
	INIT_PROPERTY_FIELD(TrajectoryDisplay::_shadingMode);
	INIT_PROPERTY_FIELD(TrajectoryDisplay::_showUpToCurrentTime);
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 TrajectoryDisplay::boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	TrajectoryObject* trajObj = dynamic_object_cast<TrajectoryObject>(dataObject);

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(trajObj, lineWidth())) {
		// Compute bounding box.
		_cachedBoundingBox.setEmpty();
		if(trajObj) {
			_cachedBoundingBox.addPoints(trajObj->points().constData(), trajObj->points().size());
			_cachedBoundingBox = _cachedBoundingBox.padBox(lineWidth() / 2);
		}
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Lets the display object render the data object.
******************************************************************************/
void TrajectoryDisplay::render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	TrajectoryObject* trajObj = dynamic_object_cast<TrajectoryObject>(dataObject);

	// Do we have to re-create the geometry buffers from scratch?
	bool recreateBuffers = !_segmentBuffer || !_segmentBuffer->isValid(renderer)
						|| !_cornerBuffer || !_cornerBuffer->isValid(renderer);

	// Set up shading mode.
	ParticlePrimitive::ShadingMode cornerShadingMode = (shadingMode() == ArrowPrimitive::NormalShading)
			? ParticlePrimitive::NormalShading : ParticlePrimitive::FlatShading;
	if(!recreateBuffers) {
		recreateBuffers |= !_segmentBuffer->setShadingMode(shadingMode());
		recreateBuffers |= !_cornerBuffer->setShadingMode(cornerShadingMode);
	}

	TimePoint endTime = showUpToCurrentTime() ? time : TimePositiveInfinity();

	// Do we have to update contents of the geometry buffers?
	bool updateContents = _geometryCacheHelper.updateState(trajObj, lineWidth(), lineColor(), endTime) || recreateBuffers;

	// Re-create the geometry buffers if necessary.
	if(recreateBuffers) {
		_segmentBuffer = renderer->createArrowPrimitive(ArrowPrimitive::CylinderShape, shadingMode(), ArrowPrimitive::HighQuality);
		_cornerBuffer = renderer->createParticlePrimitive(cornerShadingMode, ParticlePrimitive::HighQuality);
	}

	if(updateContents) {
		FloatType lineRadius = lineWidth() / 2;
		if(trajObj && lineRadius > 0) {
			int timeSamples = std::upper_bound(trajObj->sampleTimes().cbegin(), trajObj->sampleTimes().cend(), endTime) - trajObj->sampleTimes().cbegin();

			int lineSegmentCount = std::max(0, timeSamples - 1) * trajObj->trajectoryCount();

			_segmentBuffer->startSetElements(lineSegmentCount);
			int lineSegmentIndex = 0;
			for(int pindex = 0; pindex < trajObj->trajectoryCount(); pindex++) {
				for(int tindex = 0; tindex < timeSamples - 1; tindex++) {
					const Point3& p1 = trajObj->points()[tindex * trajObj->trajectoryCount() + pindex];
					const Point3& p2 = trajObj->points()[(tindex+1) * trajObj->trajectoryCount() + pindex];
					_segmentBuffer->setElement(lineSegmentIndex++, p1, p2 - p1, ColorA(lineColor()), lineRadius);
				}
			}
			_segmentBuffer->endSetElements();

			int pointCount = std::max(0, timeSamples - 2) * trajObj->trajectoryCount();
			_cornerBuffer->setSize(pointCount);
			if(pointCount)
				_cornerBuffer->setParticlePositions(trajObj->points().constData() + trajObj->trajectoryCount());
			_cornerBuffer->setParticleColor(ColorA(lineColor()));
			_cornerBuffer->setParticleRadius(lineRadius);
		}
		else {
			_segmentBuffer.reset();
			_cornerBuffer.reset();
		}
	}

	if(!_segmentBuffer)
		return;

	renderer->beginPickObject(contextNode);
	_segmentBuffer->render(renderer);
	_cornerBuffer->render(renderer);
	renderer->endPickObject();
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void TrajectoryDisplayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Trajectory display"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	// Shading mode.
	VariantComboBoxParameterUI* shadingModeUI = new VariantComboBoxParameterUI(this, "shadingMode");
	shadingModeUI->comboBox()->addItem(tr("Normal"), qVariantFromValue(ArrowPrimitive::NormalShading));
	shadingModeUI->comboBox()->addItem(tr("Flat"), qVariantFromValue(ArrowPrimitive::FlatShading));
	layout->addWidget(new QLabel(tr("Shading:")), 0, 0);
	layout->addWidget(shadingModeUI->comboBox(), 0, 1);

	// Line width.
	FloatParameterUI* lineWidthUI = new FloatParameterUI(this, PROPERTY_FIELD(TrajectoryDisplay::_lineWidth));
	layout->addWidget(lineWidthUI->label(), 1, 0);
	layout->addLayout(lineWidthUI->createFieldLayout(), 1, 1);
	lineWidthUI->setMinValue(0);

	// Line color.
	ColorParameterUI* lineColorUI = new ColorParameterUI(this, PROPERTY_FIELD(TrajectoryDisplay::_lineColor));
	layout->addWidget(lineColorUI->label(), 2, 0);
	layout->addWidget(lineColorUI->colorPicker(), 2, 1);

	// Up to current time.
	BooleanParameterUI* showUpToCurrentTimeUI = new BooleanParameterUI(this, PROPERTY_FIELD(TrajectoryDisplay::_showUpToCurrentTime));
	layout->addWidget(showUpToCurrentTimeUI->checkBox(), 3, 0, 1, 2);
}

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace
