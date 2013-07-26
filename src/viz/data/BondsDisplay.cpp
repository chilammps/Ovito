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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>

#include "BondsDisplay.h"
#include "ParticleDisplay.h"
#include "SimulationCell.h"
#include "ParticleTypeProperty.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, BondsDisplay, DisplayObject)
IMPLEMENT_OVITO_OBJECT(Viz, BondsDisplayEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(BondsDisplay, BondsDisplayEditor)
DEFINE_PROPERTY_FIELD(BondsDisplay, _bondWidth, "BondWidth")
DEFINE_PROPERTY_FIELD(BondsDisplay, _bondColor, "BondColor")
DEFINE_PROPERTY_FIELD(BondsDisplay, _useParticleColors, "UseParticleColors")
DEFINE_PROPERTY_FIELD(BondsDisplay, _shadingMode, "ShadingMode")
DEFINE_PROPERTY_FIELD(BondsDisplay, _renderingQuality, "RenderingQuality")
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _bondWidth, "Bond width")
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _bondColor, "Bond color")
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _useParticleColors, "Use particle colors")
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _shadingMode, "Shading mode")
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _renderingQuality, "RenderingQuality")
SET_PROPERTY_FIELD_UNITS(BondsDisplay, _bondWidth, WorldParameterUnit)

/******************************************************************************
* Constructor.
******************************************************************************/
BondsDisplay::BondsDisplay() :
	_bondWidth(0.4), _bondColor(0.6, 0.6, 0.6), _useParticleColors(true),
	_shadingMode(ArrowGeometryBuffer::NormalShading),
	_renderingQuality(ArrowGeometryBuffer::HighQuality)
{
	INIT_PROPERTY_FIELD(BondsDisplay::_bondWidth);
	INIT_PROPERTY_FIELD(BondsDisplay::_bondColor);
	INIT_PROPERTY_FIELD(BondsDisplay::_useParticleColors);
	INIT_PROPERTY_FIELD(BondsDisplay::_shadingMode);
	INIT_PROPERTY_FIELD(BondsDisplay::_renderingQuality);

	// Load the default parameters stored in the application settings.
	QSettings settings;
	settings.beginGroup("viz/bonds");
	setBondWidth(settings.value("DefaultBondWidth", qVariantFromValue(bondWidth())).value<FloatType>());
	setBondColor(settings.value("DefaultBondColor", qVariantFromValue(bondColor())).value<Color>());
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
	ParticlePropertyObject* positionProperty = findStandardProperty(ParticleProperty::PositionProperty, flowState);
	SimulationCell* simulationCell = flowState.findObject<SimulationCell>();

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(
			bondsObj, bondsObj ? bondsObj->revisionNumber() : 0,
			positionProperty, positionProperty ? positionProperty->revisionNumber() : 0,
			simulationCell, simulationCell ? simulationCell->revisionNumber() : 0,
			bondWidth())) {

		// Recompute bounding box.
		_cachedBoundingBox.setEmpty();
		if(bondsObj && positionProperty) {

			unsigned int particleCount = (unsigned int)positionProperty->size();
			const Point3* positions = positionProperty->constDataPoint3();
			const AffineTransformation cell = simulationCell ? simulationCell->cellMatrix() : AffineTransformation::Zero();

			for(const BondsStorage::Bond& bond : bondsObj->bonds()) {
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

			_cachedBoundingBox = _cachedBoundingBox.padBox(bondWidth() / 2);
		}
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Lets the display object render a scene object.
******************************************************************************/
void BondsDisplay::render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	BondsObject* bondsObj = dynamic_object_cast<BondsObject>(sceneObject);
	ParticlePropertyObject* positionProperty = findStandardProperty(ParticleProperty::PositionProperty, flowState);
	SimulationCell* simulationCell = flowState.findObject<SimulationCell>();
	ParticlePropertyObject* colorProperty = findStandardProperty(ParticleProperty::ColorProperty, flowState);
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(findStandardProperty(ParticleProperty::ParticleTypeProperty, flowState));
	if(!useParticleColors()) {
		colorProperty = nullptr;
		typeProperty = nullptr;
	}

	if(_geometryCacheHelper.updateState(
			bondsObj, bondsObj ? bondsObj->revisionNumber() : 0,
			positionProperty, positionProperty ? positionProperty->revisionNumber() : 0,
			colorProperty, colorProperty ? colorProperty->revisionNumber() : 0,
			typeProperty, typeProperty ? typeProperty->revisionNumber() : 0,
			simulationCell, simulationCell ? simulationCell->revisionNumber() : 0,
			bondWidth(), bondColor(), useParticleColors())
			|| !_buffer	|| !_buffer->isValid(renderer)
			|| !_buffer->setShadingMode(shadingMode())
			|| !_buffer->setRenderingQuality(renderingQuality())) {

		FloatType bondRadius = bondWidth() / 2;
		if(bondsObj && positionProperty && bondRadius > 0) {

			// Create bond geometry buffer.
			_buffer = renderer->createArrowGeometryBuffer(ArrowGeometryBuffer::CylinderShape, shadingMode(), renderingQuality());
			_buffer->startSetElements(bondsObj->bonds().size());

			// Obtain particle colors since they determine the bond colors.
			std::vector<Color> particleColors(positionProperty->size());
			ParticleDisplay* particleDisplay = dynamic_object_cast<ParticleDisplay>(positionProperty->displayObject());
			if(particleDisplay && useParticleColors())
				particleDisplay->particleColors(particleColors, colorProperty, typeProperty, nullptr);
			else
				std::fill(particleColors.begin(), particleColors.end(), bondColor());

			// Cache some variables.
			unsigned int particleCount = (unsigned int)positionProperty->size();
			const Point3* positions = positionProperty->constDataPoint3();
			const AffineTransformation cell = simulationCell ? simulationCell->cellMatrix() : AffineTransformation::Zero();

			int elementIndex = 0;
			for(const BondsStorage::Bond& bond : bondsObj->bonds()) {
				if(bond.index1 < particleCount && bond.index2 < particleCount) {
					if(bond.pbcShift == Vector_3<int8_t>::Zero()) {
						_buffer->setElement(elementIndex, positions[bond.index1],
								(positions[bond.index2] - positions[bond.index1]) * FloatType(0.5), (ColorA)particleColors[bond.index1], bondRadius);
					}
					else {
						Vector3 vec = positions[bond.index2] - positions[bond.index1];
						for(size_t k = 0; k < 3; k++)
							if(bond.pbcShift[k] != 0) vec += cell.column(k) * (FloatType)bond.pbcShift[k];
						_buffer->setElement(elementIndex, positions[bond.index1], vec * FloatType(0.5), (ColorA)particleColors[bond.index1], bondRadius);
					}
				}
				else _buffer->setElement(elementIndex, Point3::Origin(), Vector3::Zero(), ColorA(1,1,1), 0);
				elementIndex++;
			}

			_buffer->endSetElements();
		}
		else _buffer.reset();
	}

	if(_buffer)
		_buffer->render(renderer);
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
	connect(bondWidthUI, SIGNAL(valueEntered()), this, SLOT(memorizeParameters()));

	// Bond color.
	ColorParameterUI* bondColorUI = new ColorParameterUI(this, PROPERTY_FIELD(BondsDisplay::_bondColor));
	layout->addWidget(bondColorUI->label(), 3, 0);
	layout->addWidget(bondColorUI->colorPicker(), 3, 1);
	connect(bondColorUI, SIGNAL(valueEntered()), this, SLOT(memorizeParameters()));

	// Use particle colors.
	BooleanParameterUI* useParticleColorsUI = new BooleanParameterUI(this, PROPERTY_FIELD(BondsDisplay::_useParticleColors));
	layout->addWidget(useParticleColorsUI->checkBox(), 4, 0, 1, 2);
}

/******************************************************************************
* Stores the current parameters in the application settings
* so they can be used as default value in the future.
******************************************************************************/
void BondsDisplayEditor::memorizeParameters()
{
	if(!editObject()) return;
	BondsDisplay* displayObj = static_object_cast<BondsDisplay>(editObject());

	QSettings settings;
	settings.beginGroup("viz/bonds");
	settings.setValue("DefaultBondWidth", qVariantFromValue(displayObj->bondWidth()));
	settings.setValue("DefaultBondColor", qVariantFromValue(displayObj->bondColor()));
	settings.endGroup();
}

};
