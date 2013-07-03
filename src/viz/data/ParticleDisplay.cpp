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

#include "ParticleDisplay.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ParticleDisplay, DisplayObject)
SET_OVITO_OBJECT_EDITOR(ParticleDisplay, ParticleDisplayEditor)
DEFINE_PROPERTY_FIELD(ParticleDisplay, _defaultParticleRadius, "DefaultParticleRadius")
SET_PROPERTY_FIELD_LABEL(ParticleDisplay, _defaultParticleRadius, "Particle radius")
SET_PROPERTY_FIELD_UNITS(ParticleDisplay, _defaultParticleRadius, WorldParameterUnit)

IMPLEMENT_OVITO_OBJECT(Viz, ParticleDisplayEditor, PropertiesEditor)

/******************************************************************************
* Constructor.
******************************************************************************/
ParticleDisplay::ParticleDisplay() : _defaultParticleRadius(1.2)
{
	INIT_PROPERTY_FIELD(ParticleDisplay::_defaultParticleRadius);
}

/******************************************************************************
* Searches for the given standard particle property in the scene objects
* stored in the pipeline flow state.
******************************************************************************/
ParticlePropertyObject* ParticleDisplay::findStandardProperty(ParticleProperty::Type type, const PipelineFlowState& flowState) const
{
	for(const auto& entry : flowState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(entry.first.get());
		if(property && property->type() == type) return property;
	}
	return nullptr;
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 ParticleDisplay::boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	ParticlePropertyObject* positionProperty = dynamic_object_cast<ParticlePropertyObject>(sceneObject);
	ParticlePropertyObject* radiusProperty = findStandardProperty(ParticleProperty::RadiusProperty, flowState);
	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(
			positionProperty, positionProperty ? positionProperty->revisionNumber() : 0,
			radiusProperty, radiusProperty ? radiusProperty->revisionNumber() : 0,
			defaultParticleRadius()) || _cachedBoundingBox.isEmpty()) {
		// Recompute bounding box.
		_cachedBoundingBox.setEmpty();
		if(positionProperty) {
			const Point3* p = positionProperty->constDataPoint3();
			const Point3* p_end = p + positionProperty->size();
			for(; p != p_end; ++p)
				_cachedBoundingBox.addPoint(*p);
		}
		// Take into account radii of particles.
		FloatType maxAtomRadius = defaultParticleRadius();
		if(radiusProperty && radiusProperty->size() > 0) {
			maxAtomRadius = *std::max_element(radiusProperty->constDataFloat(), radiusProperty->constDataFloat() + radiusProperty->size());
		}
		// Enlarge the bounding box by the largest particle radius.
		_cachedBoundingBox = _cachedBoundingBox.padBox(std::max(maxAtomRadius, FloatType(0)));
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Lets the display object render a scene object.
******************************************************************************/
void ParticleDisplay::render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	// Get input data.
	ParticlePropertyObject* positionProperty = dynamic_object_cast<ParticlePropertyObject>(sceneObject);
	ParticlePropertyObject* radiusProperty = findStandardProperty(ParticleProperty::RadiusProperty, flowState);
	ParticlePropertyObject* colorProperty = findStandardProperty(ParticleProperty::ColorProperty, flowState);

	// Get number of particles.
	int particleCount = positionProperty ? positionProperty->size() : 0;

	// Do we have to re-create the geometry buffer from scratch?
	bool recreateBuffer = !_particleBuffer || !_particleBuffer->isValid(renderer);

	// Do we have to resize the geometry buffer?
	bool resizeBuffer = recreateBuffer || (_particleBuffer->particleCount() != particleCount);

	// Do we have to update the particle positions in the geometry buffer?
	bool updatePositions = _positionsCacheHelper.updateState(positionProperty, positionProperty ? positionProperty->revisionNumber() : 0)
			|| resizeBuffer;

	// Do we have to update the particle radii in the geometry buffer?
	bool updateRadii = _radiiCacheHelper.updateState(radiusProperty, radiusProperty ? radiusProperty->revisionNumber() : 0, defaultParticleRadius())
			|| resizeBuffer;

	// Do we have to update the particle colors in the geometry buffer?
	bool updateColors = _colorsCacheHelper.updateState(colorProperty, colorProperty ? colorProperty->revisionNumber() : 0)
			|| resizeBuffer;

	// Re-create the geometry buffer if necessary.
	if(recreateBuffer)
		_particleBuffer = renderer->createParticleGeometryBuffer();

	// Re-size the geometry buffer if necessary.
	if(resizeBuffer)
		_particleBuffer->setSize(particleCount);

	// Update buffers.

	if(updatePositions && positionProperty) {
		OVITO_ASSERT(positionProperty->size() == particleCount);
		_particleBuffer->setParticlePositions(positionProperty->constDataPoint3());
	}

	if(updateRadii) {
		if(radiusProperty) {
			OVITO_ASSERT(radiusProperty->size() == particleCount);
			_particleBuffer->setParticleRadii(radiusProperty->constDataFloat());
		}
		else
			_particleBuffer->setParticleRadius(defaultParticleRadius());
	}

	if(updateColors) {
		if(colorProperty) {
			OVITO_ASSERT(colorProperty->size() == particleCount);
			_particleBuffer->setParticleColors(colorProperty->constDataColor());
		}
		else
			_particleBuffer->setParticleColor(Color(1,1,1));
	}

	_particleBuffer->render(renderer);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ParticleDisplayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Particle display"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
#ifndef Q_WS_MAC
	layout->setSpacing(0);
#endif
	layout->setColumnStretch(1, 1);

#if 0
	BooleanPropertyUI* highQualityDisplayUI = new BooleanPropertyUI(this, PROPERTY_FIELD_DESCRIPTOR(PositionDataChannel, _useHighQualityRenderingInViewports));
	layout->addWidget(highQualityDisplayUI->checkBox(), 1, 0, 1, 3);

	BooleanPropertyUI* flatAtomDisplayUI = new BooleanPropertyUI(this, PROPERTY_FIELD_DESCRIPTOR(PositionDataChannel, _flatAtomRendering));
	layout->addWidget(flatAtomDisplayUI->checkBox(), 2, 0, 1, 3);
#endif

	// Default radius parameter.
	FloatParameterUI* radiusUI = new FloatParameterUI(this, PROPERTY_FIELD(ParticleDisplay::_defaultParticleRadius));
	layout->addWidget(radiusUI->label(), 2, 0);
	layout->addLayout(radiusUI->createFieldLayout(), 2, 1);
	radiusUI->setMinValue(0);
}

};
