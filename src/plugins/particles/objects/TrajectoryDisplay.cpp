///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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
SET_PROPERTY_FIELD_LABEL(TrajectoryDisplay, _lineWidth, "Line width");
SET_PROPERTY_FIELD_LABEL(TrajectoryDisplay, _lineColor, "Line color");
SET_PROPERTY_FIELD_LABEL(TrajectoryDisplay, _shadingMode, "Shading mode");
SET_PROPERTY_FIELD_UNITS(TrajectoryDisplay, _lineWidth, WorldParameterUnit);

/******************************************************************************
* Constructor.
******************************************************************************/
TrajectoryDisplay::TrajectoryDisplay(DataSet* dataset) : DisplayObject(dataset),
	_lineWidth(0.3), _lineColor(0.6, 0.6, 0.6),
	_shadingMode(ArrowPrimitive::NormalShading)
{
	INIT_PROPERTY_FIELD(TrajectoryDisplay::_lineWidth);
	INIT_PROPERTY_FIELD(TrajectoryDisplay::_lineColor);
	INIT_PROPERTY_FIELD(TrajectoryDisplay::_shadingMode);
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 TrajectoryDisplay::boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	TrajectoryObject* trajObj = dynamic_object_cast<TrajectoryObject>(dataObject);

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(trajObj, lineWidth())) {

		// Recompute bounding box.
		_cachedBoundingBox.setEmpty();
		if(trajObj) {

#if 0
			unsigned int particleCount = (unsigned int)positionProperty->size();
			const Point3* positions = positionProperty->constDataPoint3();
			const AffineTransformation cell = simulationCell ? simulationCell->cellMatrix() : AffineTransformation::Zero();

			for(const Bond& bond : *bondsObj->storage()) {
				if(bond.index1 >= particleCount || bond.index2 >= particleCount)
					continue;

				_cachedBoundingBox.addPoint(positions[bond.index1]);
				if(bond.pbcShift != Vector_3<int8_t>::Zero()) {
					Vector3 vec = positions[bond.index2] - positions[bond.index1];
					for(size_t k = 0; k < 3; k++)
						if(bond.pbcShift[k] != 0) vec += cell.column(k) * (FloatType)bond.pbcShift[k];
					_cachedBoundingBox.addPoint(positions[bond.index1] + (vec * FloatType(0.5)));
				}
			}
#endif

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

	if(_geometryCacheHelper.updateState(trajObj, lineWidth(), lineColor())
			|| !_buffer	|| !_buffer->isValid(renderer)
			|| !_buffer->setShadingMode(shadingMode())) {

		FloatType lineRadius = lineWidth() / 2;
		if(trajObj && lineRadius > 0) {
		}
		else _buffer.reset();
	}

	if(!_buffer)
		return;

	renderer->beginPickObject(contextNode);
	_buffer->render(renderer);
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
}

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace
