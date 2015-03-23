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
#include <core/utilities/concurrent/ParallelFor.h>
#include <core/rendering/SceneRenderer.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>

#include "VectorDisplay.h"
#include "ParticleTypeProperty.h"

namespace Ovito { namespace Particles {

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, VectorDisplayEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, VectorDisplay, DisplayObject);
SET_OVITO_OBJECT_EDITOR(VectorDisplay, VectorDisplayEditor);
DEFINE_FLAGS_PROPERTY_FIELD(VectorDisplay, _reverseArrowDirection, "ReverseArrowDirection", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(VectorDisplay, _flipVectors, "FlipVectors", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(VectorDisplay, _arrowColor, "ArrowColor", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(VectorDisplay, _arrowWidth, "ArrowWidth", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(VectorDisplay, _scalingFactor, "ScalingFactor", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(VectorDisplay, _shadingMode, "ShadingMode", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(VectorDisplay, _renderingQuality, "RenderingQuality");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, _arrowColor, "Arrow color");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, _arrowWidth, "Arrow width");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, _scalingFactor, "Scaling factor");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, _reverseArrowDirection, "Reverse arrow direction");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, _flipVectors, "Flip vectors");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, _shadingMode, "Shading mode");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, _renderingQuality, "RenderingQuality");
SET_PROPERTY_FIELD_UNITS(VectorDisplay, _arrowWidth, WorldParameterUnit);

/******************************************************************************
* Constructor.
******************************************************************************/
VectorDisplay::VectorDisplay(DataSet* dataset) : DisplayObject(dataset),
	_reverseArrowDirection(false), _flipVectors(false), _arrowColor(1, 1, 0), _arrowWidth(0.5), _scalingFactor(1),
	_shadingMode(ArrowPrimitive::FlatShading),
	_renderingQuality(ArrowPrimitive::LowQuality)
{
	INIT_PROPERTY_FIELD(VectorDisplay::_arrowColor);
	INIT_PROPERTY_FIELD(VectorDisplay::_arrowWidth);
	INIT_PROPERTY_FIELD(VectorDisplay::_scalingFactor);
	INIT_PROPERTY_FIELD(VectorDisplay::_reverseArrowDirection);
	INIT_PROPERTY_FIELD(VectorDisplay::_flipVectors);
	INIT_PROPERTY_FIELD(VectorDisplay::_shadingMode);
	INIT_PROPERTY_FIELD(VectorDisplay::_renderingQuality);
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 VectorDisplay::boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	ParticlePropertyObject* vectorProperty = dynamic_object_cast<ParticlePropertyObject>(dataObject);
	ParticlePropertyObject* positionProperty = ParticlePropertyObject::findInState(flowState, ParticleProperty::PositionProperty);
	if(vectorProperty && (vectorProperty->dataType() != qMetaTypeId<FloatType>() || vectorProperty->componentCount() != 3))
		vectorProperty = nullptr;

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(
			vectorProperty,
			positionProperty,
			scalingFactor(), arrowWidth()) || _cachedBoundingBox.isEmpty()) {
		// Recompute bounding box.
		_cachedBoundingBox = arrowBoundingBox(vectorProperty, positionProperty);
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Computes the bounding box of the arrows.
******************************************************************************/
Box3 VectorDisplay::arrowBoundingBox(ParticlePropertyObject* vectorProperty, ParticlePropertyObject* positionProperty)
{
	if(!positionProperty || !vectorProperty)
		return Box3();

	OVITO_ASSERT(positionProperty->type() == ParticleProperty::PositionProperty);
	OVITO_ASSERT(vectorProperty->dataType() == qMetaTypeId<FloatType>());
	OVITO_ASSERT(vectorProperty->componentCount() == 3);

	// Compute bounding box of particle positions.
	Box3 bbox;
	const Point3* p = positionProperty->constDataPoint3();
	const Point3* p_end = p + positionProperty->size();
	for(; p != p_end; ++p)
		bbox.addPoint(*p);

	// Find largest vector magnitude.
	FloatType maxMagnitude = 0;
	const Vector3* v = vectorProperty->constDataVector3();
	const Vector3* v_end = v + vectorProperty->size();
	for(; v != v_end; ++v) {
		FloatType m = v->squaredLength();
		if(m > maxMagnitude) maxMagnitude = m;
	}

	// Enlarge the bounding box by the largest vector magnitude + padding.
	return bbox.padBox((sqrt(maxMagnitude) * std::abs(scalingFactor())) + arrowWidth());
}

/******************************************************************************
* Lets the display object render the data object.
******************************************************************************/
void VectorDisplay::render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	// Get input data.
	ParticlePropertyObject* vectorProperty = dynamic_object_cast<ParticlePropertyObject>(dataObject);
	ParticlePropertyObject* positionProperty = ParticlePropertyObject::findInState(flowState, ParticleProperty::PositionProperty);
	if(vectorProperty && (vectorProperty->dataType() != qMetaTypeId<FloatType>() || vectorProperty->componentCount() != 3))
		vectorProperty = nullptr;

	// Get number of vectors.
	int vectorCount = (vectorProperty && positionProperty) ? (int)vectorProperty->size() : 0;

	// Do we have to re-create the geometry buffer from scratch?
	bool recreateBuffer = !_buffer || !_buffer->isValid(renderer);

	// Set shading mode and rendering quality.
	if(!recreateBuffer) {
		recreateBuffer |= !(_buffer->setShadingMode(shadingMode()));
		recreateBuffer |= !(_buffer->setRenderingQuality(renderingQuality()));
	}

	// Do we have to update contents of the geometry buffer?
	bool updateContents = _geometryCacheHelper.updateState(
			vectorProperty,
			positionProperty,
			scalingFactor(), arrowWidth(), arrowColor(), reverseArrowDirection(), flipVectors())
			|| recreateBuffer || (_buffer->elementCount() != vectorCount);

	// Re-create the geometry buffer if necessary.
	if(recreateBuffer)
		_buffer = renderer->createArrowPrimitive(ArrowPrimitive::ArrowShape, shadingMode(), renderingQuality());

	// Update buffer contents.
	if(updateContents) {
		_buffer->startSetElements(vectorCount);
		if(vectorProperty && positionProperty) {
			FloatType scalingFac = scalingFactor();
			if(flipVectors() ^ reverseArrowDirection())
				scalingFac = -scalingFac;
			const Point3* p_begin = positionProperty->constDataPoint3();
			const Vector3* v_begin = vectorProperty->constDataVector3();
			ColorA color(arrowColor());
			FloatType width = arrowWidth();
			ArrowPrimitive* buffer = _buffer.get();
			if(!reverseArrowDirection()) {
				for(int index = 0; index < vectorCount; index++) {
					buffer->setElement(index, p_begin[index], v_begin[index] * scalingFac, color, width);
				}
			}
			else {
				for(int index = 0; index < vectorCount; index++) {
					Vector3 v = v_begin[index] * scalingFac;
					buffer->setElement(index, p_begin[index] - v, v, color, width);
				}
			}
		}
		_buffer->endSetElements();
	}

	renderer->beginPickObject(contextNode);
	_buffer->render(renderer);
	renderer->endPickObject();
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void VectorDisplayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Vector display"), rolloutParams, "display_objects.vectors.html");

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	// Shading mode.
	VariantComboBoxParameterUI* shadingModeUI = new VariantComboBoxParameterUI(this, "shadingMode");
	shadingModeUI->comboBox()->addItem(tr("Normal"), qVariantFromValue(ArrowPrimitive::NormalShading));
	shadingModeUI->comboBox()->addItem(tr("Flat"), qVariantFromValue(ArrowPrimitive::FlatShading));
	layout->addWidget(new QLabel(tr("Shading mode:")), 0, 0);
	layout->addWidget(shadingModeUI->comboBox(), 0, 1);

	// Rendering quality.
	VariantComboBoxParameterUI* renderingQualityUI = new VariantComboBoxParameterUI(this, "renderingQuality");
	renderingQualityUI->comboBox()->addItem(tr("Low"), qVariantFromValue(ArrowPrimitive::LowQuality));
	renderingQualityUI->comboBox()->addItem(tr("Medium"), qVariantFromValue(ArrowPrimitive::MediumQuality));
	renderingQualityUI->comboBox()->addItem(tr("High"), qVariantFromValue(ArrowPrimitive::HighQuality));
	layout->addWidget(new QLabel(tr("Rendering quality:")), 1, 0);
	layout->addWidget(renderingQualityUI->comboBox(), 1, 1);

	// Scaling factor.
	FloatParameterUI* scalingFactorUI = new FloatParameterUI(this, PROPERTY_FIELD(VectorDisplay::_scalingFactor));
	layout->addWidget(scalingFactorUI->label(), 2, 0);
	layout->addLayout(scalingFactorUI->createFieldLayout(), 2, 1);
	scalingFactorUI->setMinValue(0);

	// Arrow width factor.
	FloatParameterUI* arrowWidthUI = new FloatParameterUI(this, PROPERTY_FIELD(VectorDisplay::_arrowWidth));
	layout->addWidget(arrowWidthUI->label(), 3, 0);
	layout->addLayout(arrowWidthUI->createFieldLayout(), 3, 1);
	arrowWidthUI->setMinValue(0);

	BooleanParameterUI* reverseArrowDirectionUI = new BooleanParameterUI(this, PROPERTY_FIELD(VectorDisplay::_reverseArrowDirection));
	layout->addWidget(reverseArrowDirectionUI->checkBox(), 4, 0, 1, 2);

	BooleanParameterUI* flipVectorsUI = new BooleanParameterUI(this, PROPERTY_FIELD(VectorDisplay::_flipVectors));
	layout->addWidget(flipVectorsUI->checkBox(), 5, 0, 1, 2);

	ColorParameterUI* arrowColorUI = new ColorParameterUI(this, PROPERTY_FIELD(VectorDisplay::_arrowColor));
	layout->addWidget(arrowColorUI->label(), 6, 0);
	layout->addWidget(arrowColorUI->colorPicker(), 6, 1);
}

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace
