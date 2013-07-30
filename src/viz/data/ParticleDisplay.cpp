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

#include "ParticleDisplay.h"
#include "ParticleTypeProperty.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ParticleDisplay, DisplayObject)
IMPLEMENT_OVITO_OBJECT(Viz, ParticleDisplayEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(ParticleDisplay, ParticleDisplayEditor)
DEFINE_PROPERTY_FIELD(ParticleDisplay, _defaultParticleRadius, "DefaultParticleRadius")
DEFINE_PROPERTY_FIELD(ParticleDisplay, _shadingMode, "ShadingMode")
DEFINE_PROPERTY_FIELD(ParticleDisplay, _renderingQuality, "RenderingQuality")
SET_PROPERTY_FIELD_LABEL(ParticleDisplay, _defaultParticleRadius, "Default particle radius")
SET_PROPERTY_FIELD_LABEL(ParticleDisplay, _shadingMode, "Shading mode")
SET_PROPERTY_FIELD_LABEL(ParticleDisplay, _renderingQuality, "RenderingQuality")
SET_PROPERTY_FIELD_UNITS(ParticleDisplay, _defaultParticleRadius, WorldParameterUnit)

/******************************************************************************
* Constructor.
******************************************************************************/
ParticleDisplay::ParticleDisplay() :
	_defaultParticleRadius(1.2),
	_shadingMode(ParticleGeometryBuffer::NormalShading),
	_renderingQuality(ParticleGeometryBuffer::LowQuality)
{
	INIT_PROPERTY_FIELD(ParticleDisplay::_defaultParticleRadius);
	INIT_PROPERTY_FIELD(ParticleDisplay::_shadingMode);
	INIT_PROPERTY_FIELD(ParticleDisplay::_renderingQuality);
}

/******************************************************************************
* Searches for the given standard particle property in the scene objects
* stored in the pipeline flow state.
******************************************************************************/
ParticlePropertyObject* ParticleDisplay::findStandardProperty(ParticleProperty::Type type, const PipelineFlowState& flowState) const
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
Box3 ParticleDisplay::boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	ParticlePropertyObject* positionProperty = dynamic_object_cast<ParticlePropertyObject>(sceneObject);
	ParticlePropertyObject* radiusProperty = findStandardProperty(ParticleProperty::RadiusProperty, flowState);
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(findStandardProperty(ParticleProperty::ParticleTypeProperty, flowState));

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(
			positionProperty, positionProperty ? positionProperty->revisionNumber() : 0,
			radiusProperty, radiusProperty ? radiusProperty->revisionNumber() : 0,
			typeProperty, typeProperty ? typeProperty->revisionNumber() : 0,
			defaultParticleRadius()) || _cachedBoundingBox.isEmpty()) {
		// Recompute bounding box.
		_cachedBoundingBox = particleBoundingBox(positionProperty, typeProperty, radiusProperty);
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Computes the bounding box of the particles.
******************************************************************************/
Box3 ParticleDisplay::particleBoundingBox(ParticlePropertyObject* positionProperty, ParticleTypeProperty* typeProperty, ParticlePropertyObject* radiusProperty, bool includeParticleRadius)
{
	OVITO_ASSERT(positionProperty == nullptr || positionProperty->type() == ParticleProperty::PositionProperty);
	OVITO_ASSERT(typeProperty == nullptr || typeProperty->type() == ParticleProperty::ParticleTypeProperty);
	OVITO_ASSERT(radiusProperty == nullptr || radiusProperty->type() == ParticleProperty::RadiusProperty);

	Box3 bbox;
	if(positionProperty) {
		const Point3* p = positionProperty->constDataPoint3();
		const Point3* p_end = p + positionProperty->size();
		for(; p != p_end; ++p)
			bbox.addPoint(*p);
	}
	if(!includeParticleRadius)
		return bbox;

	// Take into account radii of particles.
	FloatType maxAtomRadius = defaultParticleRadius();
	if(radiusProperty && radiusProperty->size() > 0) {
		maxAtomRadius = *std::max_element(radiusProperty->constDataFloat(), radiusProperty->constDataFloat() + radiusProperty->size());
	}
	else if(typeProperty) {
		for(const auto& it : typeProperty->radiusMap())
			maxAtomRadius = std::max(maxAtomRadius, it.second);
	}
	// Enlarge the bounding box by the largest particle radius.
	return bbox.padBox(std::max(maxAtomRadius, FloatType(0)));
}

/******************************************************************************
* Determines the the display particle colors.
******************************************************************************/
void ParticleDisplay::particleColors(std::vector<Color>& output, ParticlePropertyObject* colorProperty, ParticleTypeProperty* typeProperty, ParticlePropertyObject* selectionProperty)
{
	OVITO_ASSERT(colorProperty == nullptr || colorProperty->type() == ParticleProperty::ColorProperty);
	OVITO_ASSERT(typeProperty == nullptr || typeProperty->type() == ParticleProperty::ParticleTypeProperty);
	OVITO_ASSERT(selectionProperty == nullptr || selectionProperty->type() == ParticleProperty::SelectionProperty);

	if(colorProperty) {
		// Take particle colors directly from the color property.
		OVITO_ASSERT(colorProperty->size() == output.size());
		std::copy(colorProperty->constDataColor(), colorProperty->constDataColor() + output.size(), output.begin());
	}
	else if(typeProperty) {
		// Assign colors based on particle types.
		OVITO_ASSERT(typeProperty->size() == output.size());
		// Build a lookup map for particle type colors.
		const std::map<int,Color> colorMap = typeProperty->colorMap();
		// Fill color array.
		const int* t = typeProperty->constDataInt();
		for(auto c = output.begin(); c != output.end(); ++c, ++t) {
			auto it = colorMap.find(*t);
			if(it != colorMap.end())
				*c = it->second;
			else
				c->setWhite();
		}
	}
	else {
		// Assign a constant color to all particles.
		std::fill(output.begin(), output.end(), Color(1,1,1));
	}

	// Highlight selected particles.
	if(selectionProperty) {
		OVITO_ASSERT(selectionProperty->size() == output.size());
		const Color selColor(1,0,0);
		const int* t = selectionProperty->constDataInt();
		for(auto c = output.begin(); c != output.end(); ++c, ++t) {
			if(*t)
				*c = selColor;
		}
	}
}

/******************************************************************************
* Determines the the display particle radii.
******************************************************************************/
void ParticleDisplay::particleRadii(std::vector<FloatType>& output, ParticlePropertyObject* radiusProperty, ParticleTypeProperty* typeProperty)
{
	OVITO_ASSERT(radiusProperty == nullptr || radiusProperty->type() == ParticleProperty::RadiusProperty);
	OVITO_ASSERT(typeProperty == nullptr || typeProperty->type() == ParticleProperty::ParticleTypeProperty);

	if(radiusProperty) {
		// Take particle radii directly from the radius property.
		OVITO_ASSERT(radiusProperty->size() == output.size());
		std::copy(radiusProperty->constDataFloat(), radiusProperty->constDataFloat() + output.size(), output.begin());
	}
	else if(typeProperty) {
		// Assign radii based on particle types.
		OVITO_ASSERT(typeProperty->size() == output.size());
		// Build a lookup map for particle type radii.
		const std::map<int,FloatType> radiusMap = typeProperty->radiusMap();
		// Skip the following loop if all per-type radii are zero. In this case, simply use the default radius for all particles.
		if(std::any_of(radiusMap.cbegin(), radiusMap.cend(), [](const std::pair<int,FloatType>& it) { return it.second != 0; })) {
			// Fill radius array.
			const int* t = typeProperty->constDataInt();
			for(auto c = output.begin(); c != output.end(); ++c, ++t) {
				auto it = radiusMap.find(*t);
				// Set particle radius only if the type's radius is non-zero.
				if(it != radiusMap.end() && it->second != 0)
					*c = it->second;
			}
		}
		else {
			// Assign a constant radius to all particles.
			std::fill(output.begin(), output.end(), defaultParticleRadius());
		}
	}
	else {
		// Assign a constant radius to all particles.
		std::fill(output.begin(), output.end(), defaultParticleRadius());
	}
}

/******************************************************************************
* Determines the display radius of a single particle.
******************************************************************************/
FloatType ParticleDisplay::particleRadius(size_t particleIndex, ParticlePropertyObject* radiusProperty, ParticleTypeProperty* typeProperty)
{
	OVITO_ASSERT(radiusProperty == nullptr || radiusProperty->type() == ParticleProperty::RadiusProperty);
	OVITO_ASSERT(typeProperty == nullptr || typeProperty->type() == ParticleProperty::ParticleTypeProperty);

	if(radiusProperty) {
		// Take particle radius directly from the radius property.
		OVITO_ASSERT(particleIndex < radiusProperty->size());
		return radiusProperty->getFloat(particleIndex);
	}
	else if(typeProperty) {
		// Assign radius based on particle types.
		OVITO_ASSERT(particleIndex < typeProperty->size());
		ParticleType* ptype = typeProperty->particleType(typeProperty->getInt(particleIndex));
		if(ptype && ptype->radius() > 0)
			return ptype->radius();
	}

	return defaultParticleRadius();
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
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(findStandardProperty(ParticleProperty::ParticleTypeProperty, flowState));
	ParticlePropertyObject* selectionProperty = renderer->isInteractive() ? findStandardProperty(ParticleProperty::SelectionProperty, flowState) : nullptr;

	// Get number of particles.
	int particleCount = positionProperty ? positionProperty->size() : 0;

	// Do we have to re-create the geometry buffer from scratch?
	bool recreateBuffer = !_particleBuffer || !_particleBuffer->isValid(renderer);

	// Set shading mode and rendering quality.
	if(!recreateBuffer) {
		recreateBuffer |= !(_particleBuffer->setShadingMode(shadingMode()));
		recreateBuffer |= !(_particleBuffer->setRenderingQuality(renderingQuality()));
	}

	// Do we have to resize the geometry buffer?
	bool resizeBuffer = recreateBuffer || (_particleBuffer->particleCount() != particleCount);

	// Do we have to update the particle positions in the geometry buffer?
	bool updatePositions = _positionsCacheHelper.updateState(positionProperty, positionProperty ? positionProperty->revisionNumber() : 0)
			|| resizeBuffer;

	// Do we have to update the particle radii in the geometry buffer?
	bool updateRadii = _radiiCacheHelper.updateState(
			radiusProperty, radiusProperty ? radiusProperty->revisionNumber() : 0,
			typeProperty, typeProperty ? typeProperty->revisionNumber() : 0,
			defaultParticleRadius())
			|| resizeBuffer;

	// Do we have to update the particle colors in the geometry buffer?
	bool updateColors = _colorsCacheHelper.updateState(
			colorProperty, colorProperty ? colorProperty->revisionNumber() : 0,
			typeProperty, typeProperty ? typeProperty->revisionNumber() : 0,
			selectionProperty, selectionProperty ? selectionProperty->revisionNumber() : 0)
			|| resizeBuffer;

	// Re-create the geometry buffer if necessary.
	if(recreateBuffer)
		_particleBuffer = renderer->createParticleGeometryBuffer(shadingMode(), renderingQuality());

	// Re-size the geometry buffer if necessary.
	if(resizeBuffer)
		_particleBuffer->setSize(particleCount);

	// Update position buffer.
	if(updatePositions && positionProperty) {
		OVITO_ASSERT(positionProperty->size() == particleCount);
		_particleBuffer->setParticlePositions(positionProperty->constDataPoint3());
	}

	// Update radius buffer.
	if(updateRadii) {
		if(radiusProperty) {
			// Take particle radii directly from the radius property.
			OVITO_ASSERT(radiusProperty->size() == particleCount);
			_particleBuffer->setParticleRadii(radiusProperty->constDataFloat());
		}
		else if(typeProperty) {
			// Assign radii based on particle types.
			OVITO_ASSERT(typeProperty->size() == particleCount);
			// Allocate memory buffer.
			std::vector<FloatType> particleRadii(particleCount, defaultParticleRadius());
			// Build a lookup map for particle type raii.
			const std::map<int,FloatType> radiusMap = typeProperty->radiusMap();
			// Skip the following loop if all per-type radii are zero. In this case, simply use the default radius for all particles.
			if(std::any_of(radiusMap.cbegin(), radiusMap.cend(), [](const std::pair<int,FloatType>& it) { return it.second != 0; })) {
				// Fill radius array.
				const int* t = typeProperty->constDataInt();
				for(auto c = particleRadii.begin(); c != particleRadii.end(); ++c, ++t) {
					auto it = radiusMap.find(*t);
					// Set particle radius only if the type's radius is non-zero.
					if(it != radiusMap.end() && it->second != 0)
						*c = it->second;
				}
			}
			_particleBuffer->setParticleRadii(particleRadii.data());
		}
		else {
			// Assign a constant radius to all particles.
			_particleBuffer->setParticleRadius(defaultParticleRadius());
		}
	}

	// Update color buffer.
	if(updateColors) {
		// Allocate memory buffer.
		std::vector<Color> colors(particleCount);
		particleColors(colors, colorProperty, typeProperty, selectionProperty);
		_particleBuffer->setParticleColors(colors.data());
	}

	// Support picking of particles.
	quint32 pickingBaseID = 0;
	if(renderer->isPicking())
		pickingBaseID = renderer->registerPickObject(contextNode, sceneObject, particleCount);

	_particleBuffer->render(renderer, pickingBaseID);
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
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	// Shading mode.
	VariantComboBoxParameterUI* shadingModeUI = new VariantComboBoxParameterUI(this, "shadingMode");
	shadingModeUI->comboBox()->addItem(tr("Normal"), qVariantFromValue(ParticleGeometryBuffer::NormalShading));
	shadingModeUI->comboBox()->addItem(tr("Flat"), qVariantFromValue(ParticleGeometryBuffer::FlatShading));
	layout->addWidget(new QLabel(tr("Shading mode:")), 0, 0);
	layout->addWidget(shadingModeUI->comboBox(), 0, 1);

	// Rendering quality.
	VariantComboBoxParameterUI* renderingQualityUI = new VariantComboBoxParameterUI(this, "renderingQuality");
	renderingQualityUI->comboBox()->addItem(tr("Low"), qVariantFromValue(ParticleGeometryBuffer::LowQuality));
	renderingQualityUI->comboBox()->addItem(tr("Medium"), qVariantFromValue(ParticleGeometryBuffer::MediumQuality));
	renderingQualityUI->comboBox()->addItem(tr("High"), qVariantFromValue(ParticleGeometryBuffer::HighQuality));
	layout->addWidget(new QLabel(tr("Rendering quality:")), 1, 0);
	layout->addWidget(renderingQualityUI->comboBox(), 1, 1);

	// Default radius.
	FloatParameterUI* radiusUI = new FloatParameterUI(this, PROPERTY_FIELD(ParticleDisplay::_defaultParticleRadius));
	layout->addWidget(radiusUI->label(), 2, 0);
	layout->addLayout(radiusUI->createFieldLayout(), 2, 1);
	radiusUI->setMinValue(0);
}

};
