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
#include <core/gui/properties/VariantComboBoxParameterUI.h>

#include "BondsDisplay.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, BondsDisplay, DisplayObject)
IMPLEMENT_OVITO_OBJECT(Viz, BondsDisplayEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(BondsDisplay, BondsDisplayEditor)
DEFINE_PROPERTY_FIELD(BondsDisplay, _bondWidth, "BondWidth")
DEFINE_PROPERTY_FIELD(BondsDisplay, _shadingMode, "ShadingMode")
DEFINE_PROPERTY_FIELD(BondsDisplay, _renderingQuality, "RenderingQuality")
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _bondWidth, "Bond width")
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _shadingMode, "Shading mode")
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _renderingQuality, "RenderingQuality")
SET_PROPERTY_FIELD_UNITS(BondsDisplay, _bondWidth, WorldParameterUnit)

/******************************************************************************
* Constructor.
******************************************************************************/
BondsDisplay::BondsDisplay() :
	_bondWidth(0.3),
	_shadingMode(ArrowGeometryBuffer::NormalShading),
	_renderingQuality(ArrowGeometryBuffer::LowQuality)
{
	INIT_PROPERTY_FIELD(BondsDisplay::_bondWidth);
	INIT_PROPERTY_FIELD(BondsDisplay::_shadingMode);
	INIT_PROPERTY_FIELD(BondsDisplay::_renderingQuality);

	// Load the default bond width stored in the application settings.
	QSettings settings;
	settings.beginGroup("viz/bonds");
	setBondWidth(settings.value("DefaultBondWidth", bondWidth()).value<FloatType>());
	settings.endGroup();
}

/******************************************************************************
* Searches for the given standard particle property in the scene objects
* stored in the pipeline flow state.
******************************************************************************/
ParticlePropertyObject* BondsDisplay::findStandardProperty(ParticleProperty::Type type, const PipelineFlowState& flowState) const
{
	for(const auto& sceneObj : flowState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(sceneObj.get());
		if(property && property->type() == type) return property;
	}
	return nullptr;
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 BondsDisplay::boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	BondsObject* bondsObj = dynamic_object_cast<BondsObject>(sceneObject);
	ParticlePropertyObject* positionProperty = dynamic_object_cast<ParticlePropertyObject>(sceneObject);

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(
			bondsObj, bondsObj ? bondsObj->revisionNumber() : 0,
			positionProperty, positionProperty ? positionProperty->revisionNumber() : 0,
			bondWidth())) {
		// Recompute bounding box.
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Lets the display object render a scene object.
******************************************************************************/
void BondsDisplay::render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void BondsDisplayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Bonds display"), rolloutParams);

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

	// Rendering quality.
	VariantComboBoxParameterUI* renderingQualityUI = new VariantComboBoxParameterUI(this, "renderingQuality");
	renderingQualityUI->comboBox()->addItem(tr("Low"), qVariantFromValue(ArrowGeometryBuffer::LowQuality));
	renderingQualityUI->comboBox()->addItem(tr("Medium"), qVariantFromValue(ArrowGeometryBuffer::MediumQuality));
	renderingQualityUI->comboBox()->addItem(tr("High"), qVariantFromValue(ArrowGeometryBuffer::HighQuality));
	layout->addWidget(new QLabel(tr("Rendering quality:")), 1, 0);
	layout->addWidget(renderingQualityUI->comboBox(), 1, 1);

	// Bond width.
	FloatParameterUI* bondWidthUI = new FloatParameterUI(this, PROPERTY_FIELD(BondsDisplay::_bondWidth));
	layout->addWidget(bondWidthUI->label(), 2, 0);
	layout->addLayout(bondWidthUI->createFieldLayout(), 2, 1);
	bondWidthUI->setMinValue(0);
	connect(bondWidthUI->spinner(), SIGNAL(spinnerValueChanged()), this, SLOT(memorizeBondWidth()));
}

/******************************************************************************
* Stores the current bond display width in the application settings
* so it can be used as default value in the future.
******************************************************************************/
void BondsDisplayEditor::memorizeBondWidth()
{
	if(!editObject()) return;
	BondsDisplay* displayObj = static_object_cast<BondsDisplay>(editObject());

	QSettings settings;
	settings.beginGroup("viz/bonds");
	settings.setValue("DefaultBondWidth", displayObj->bondWidth());
	settings.endGroup();
}

};
