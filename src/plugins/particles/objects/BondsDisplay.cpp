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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>

#include "BondsDisplay.h"
#include "ParticleDisplay.h"
#include "SimulationCellObject.h"
#include "ParticleTypeProperty.h"

namespace Ovito { namespace Particles {

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, BondsDisplayEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, BondsDisplay, DisplayObject);
SET_OVITO_OBJECT_EDITOR(BondsDisplay, BondsDisplayEditor);
DEFINE_FLAGS_PROPERTY_FIELD(BondsDisplay, _bondWidth, "BondWidth", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(BondsDisplay, _bondColor, "BondColor", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(BondsDisplay, _useParticleColors, "UseParticleColors", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(BondsDisplay, _shadingMode, "ShadingMode", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(BondsDisplay, _renderingQuality, "RenderingQuality");
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _bondWidth, "Bond width");
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _bondColor, "Bond color");
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _useParticleColors, "Use particle colors");
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _shadingMode, "Shading mode");
SET_PROPERTY_FIELD_LABEL(BondsDisplay, _renderingQuality, "RenderingQuality");
SET_PROPERTY_FIELD_UNITS(BondsDisplay, _bondWidth, WorldParameterUnit);

/******************************************************************************
* Constructor.
******************************************************************************/
BondsDisplay::BondsDisplay(DataSet* dataset) : DisplayObject(dataset),
	_bondWidth(0.4), _bondColor(0.6, 0.6, 0.6), _useParticleColors(true),
	_shadingMode(ArrowPrimitive::NormalShading),
	_renderingQuality(ArrowPrimitive::HighQuality)
{
	INIT_PROPERTY_FIELD(BondsDisplay::_bondWidth);
	INIT_PROPERTY_FIELD(BondsDisplay::_bondColor);
	INIT_PROPERTY_FIELD(BondsDisplay::_useParticleColors);
	INIT_PROPERTY_FIELD(BondsDisplay::_shadingMode);
	INIT_PROPERTY_FIELD(BondsDisplay::_renderingQuality);
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 BondsDisplay::boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	BondsObject* bondsObj = dynamic_object_cast<BondsObject>(dataObject);
	ParticlePropertyObject* positionProperty = ParticlePropertyObject::findInState(flowState, ParticleProperty::PositionProperty);
	SimulationCellObject* simulationCell = flowState.findObject<SimulationCellObject>();

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(
			bondsObj,
			positionProperty,
			simulationCell,
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
* Lets the display object render the data object.
******************************************************************************/
void BondsDisplay::render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	BondsObject* bondsObj = dynamic_object_cast<BondsObject>(dataObject);
	ParticlePropertyObject* positionProperty = ParticlePropertyObject::findInState(flowState, ParticleProperty::PositionProperty);
	SimulationCellObject* simulationCell = flowState.findObject<SimulationCellObject>();
	ParticlePropertyObject* colorProperty = ParticlePropertyObject::findInState(flowState, ParticleProperty::ColorProperty);
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(ParticlePropertyObject::findInState(flowState, ParticleProperty::ParticleTypeProperty));
	if(!useParticleColors()) {
		colorProperty = nullptr;
		typeProperty = nullptr;
	}

	if(_geometryCacheHelper.updateState(
			bondsObj,
			positionProperty,
			colorProperty,
			typeProperty,
			simulationCell,
			bondWidth(), bondColor(), useParticleColors())
			|| !_buffer	|| !_buffer->isValid(renderer)
			|| !_buffer->setShadingMode(shadingMode())
			|| !_buffer->setRenderingQuality(renderingQuality())) {

		FloatType bondRadius = bondWidth() / 2;
		if(bondsObj && positionProperty && bondRadius > 0) {

			// Create bond geometry buffer.
			_buffer = renderer->createArrowPrimitive(ArrowPrimitive::CylinderShape, shadingMode(), renderingQuality());
			_buffer->startSetElements((int)bondsObj->bonds().size());

			// Obtain particle colors since they determine the bond colors.
			std::vector<Color> particleColors(positionProperty->size());
			ParticleDisplay* particleDisplay = nullptr;
			if(useParticleColors()) {
				for(DisplayObject* displayObj : positionProperty->displayObjects())
					if((particleDisplay = dynamic_object_cast<ParticleDisplay>(displayObj)) != nullptr)
						break;
			}
			if(particleDisplay)
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
					Vector3 vec = positions[bond.index2] - positions[bond.index1];
					for(size_t k = 0; k < 3; k++)
						if(bond.pbcShift[k] != 0) vec += cell.column(k) * (FloatType)bond.pbcShift[k];
					_buffer->setElement(elementIndex, positions[bond.index1], vec * FloatType(0.5), (ColorA)particleColors[bond.index1], bondRadius);
				}
				else _buffer->setElement(elementIndex, Point3::Origin(), Vector3::Zero(), ColorA(1,1,1), 0);
				elementIndex++;
			}

			_buffer->endSetElements();
		}
		else _buffer.reset();
	}

	if(_buffer) {
		renderer->beginPickObject(contextNode);
		_buffer->render(renderer);
		renderer->endPickObject();
	}
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void BondsDisplayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Bonds display"), rolloutParams, "display_objects.bonds.html");

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

	// Bond width.
	FloatParameterUI* bondWidthUI = new FloatParameterUI(this, PROPERTY_FIELD(BondsDisplay::_bondWidth));
	layout->addWidget(bondWidthUI->label(), 2, 0);
	layout->addLayout(bondWidthUI->createFieldLayout(), 2, 1);
	bondWidthUI->setMinValue(0);

	// Bond color.
	ColorParameterUI* bondColorUI = new ColorParameterUI(this, PROPERTY_FIELD(BondsDisplay::_bondColor));
	layout->addWidget(bondColorUI->label(), 3, 0);
	layout->addWidget(bondColorUI->colorPicker(), 3, 1);

	// Use particle colors.
	BooleanParameterUI* useParticleColorsUI = new BooleanParameterUI(this, PROPERTY_FIELD(BondsDisplay::_useParticleColors));
	layout->addWidget(useParticleColorsUI->checkBox(), 4, 0, 1, 2);
}

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace
