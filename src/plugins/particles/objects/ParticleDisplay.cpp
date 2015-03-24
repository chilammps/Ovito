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
#include <core/gui/properties/VariantComboBoxParameterUI.h>

#include "ParticleDisplay.h"
#include "ParticleTypeProperty.h"

namespace Ovito { namespace Particles {

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, ParticleDisplayEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ParticleDisplay, DisplayObject);
IMPLEMENT_OVITO_OBJECT(Particles, ParticlePickInfo, ObjectPickInfo);
SET_OVITO_OBJECT_EDITOR(ParticleDisplay, ParticleDisplayEditor);
DEFINE_FLAGS_PROPERTY_FIELD(ParticleDisplay, _defaultParticleRadius, "DefaultParticleRadius", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(ParticleDisplay, _shadingMode, "ShadingMode", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(ParticleDisplay, _renderingQuality, "RenderingQuality");
DEFINE_FLAGS_PROPERTY_FIELD(ParticleDisplay, _particleShape, "ParticleShape", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(ParticleDisplay, _defaultParticleRadius, "Default particle radius");
SET_PROPERTY_FIELD_LABEL(ParticleDisplay, _shadingMode, "Shading mode");
SET_PROPERTY_FIELD_LABEL(ParticleDisplay, _renderingQuality, "Rendering quality");
SET_PROPERTY_FIELD_LABEL(ParticleDisplay, _particleShape, "Shape");
SET_PROPERTY_FIELD_UNITS(ParticleDisplay, _defaultParticleRadius, WorldParameterUnit);

/******************************************************************************
* Constructor.
******************************************************************************/
ParticleDisplay::ParticleDisplay(DataSet* dataset) : DisplayObject(dataset),
	_defaultParticleRadius(1.2),
	_shadingMode(ParticlePrimitive::NormalShading),
	_renderingQuality(ParticlePrimitive::AutoQuality),
	_particleShape(ParticlePrimitive::SphericalShape)
{
	INIT_PROPERTY_FIELD(ParticleDisplay::_defaultParticleRadius);
	INIT_PROPERTY_FIELD(ParticleDisplay::_shadingMode);
	INIT_PROPERTY_FIELD(ParticleDisplay::_renderingQuality);
	INIT_PROPERTY_FIELD(ParticleDisplay::_particleShape);
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 ParticleDisplay::boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	ParticlePropertyObject* positionProperty = dynamic_object_cast<ParticlePropertyObject>(dataObject);
	ParticlePropertyObject* radiusProperty = ParticlePropertyObject::findInState(flowState, ParticleProperty::RadiusProperty);
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(ParticlePropertyObject::findInState(flowState, ParticleProperty::ParticleTypeProperty));
	ParticlePropertyObject* shapeProperty = ParticlePropertyObject::findInState(flowState, ParticleProperty::AsphericalShapeProperty);

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(
			positionProperty,
			radiusProperty,
			typeProperty,
			shapeProperty,
			defaultParticleRadius()) || _cachedBoundingBox.isEmpty()) {
		// Recompute bounding box.
		_cachedBoundingBox = particleBoundingBox(positionProperty, typeProperty, radiusProperty, shapeProperty);
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Computes the bounding box of the particles.
******************************************************************************/
Box3 ParticleDisplay::particleBoundingBox(ParticlePropertyObject* positionProperty, ParticleTypeProperty* typeProperty, ParticlePropertyObject* radiusProperty, ParticlePropertyObject* shapeProperty, bool includeParticleRadius)
{
	OVITO_ASSERT(positionProperty == nullptr || positionProperty->type() == ParticleProperty::PositionProperty);
	OVITO_ASSERT(typeProperty == nullptr || typeProperty->type() == ParticleProperty::ParticleTypeProperty);
	OVITO_ASSERT(radiusProperty == nullptr || radiusProperty->type() == ParticleProperty::RadiusProperty);
	OVITO_ASSERT(shapeProperty == nullptr || shapeProperty->type() == ParticleProperty::AsphericalShapeProperty);
	if(shadingMode() != ParticlePrimitive::NormalShading || particleShape() != ParticlePrimitive::SquareShape)
		shapeProperty = nullptr;

	Box3 bbox;
	if(positionProperty) {
		bbox.addPoints(positionProperty->constDataPoint3(), positionProperty->size());
	}
	if(!includeParticleRadius)
		return bbox;

	// Extend box to account for radii/shape of particles.
	FloatType maxAtomRadius = 0;
	if(!shapeProperty) {
		maxAtomRadius = defaultParticleRadius();
		if(radiusProperty && radiusProperty->size() > 0) {
			maxAtomRadius = *std::max_element(radiusProperty->constDataFloat(), radiusProperty->constDataFloat() + radiusProperty->size());
		}
		else if(typeProperty) {
			for(const auto& it : typeProperty->radiusMap())
				maxAtomRadius = std::max(maxAtomRadius, it.second);
		}
	}
	else {
		for(const Vector3& s : shapeProperty->constVector3Range())
			maxAtomRadius = std::max(maxAtomRadius, std::max(s.x(), std::max(s.y(), s.z())));
	}

	// Extend the bounding box by the largest particle radius.
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

	Color defaultColor = defaultParticleColor();
	if(colorProperty) {
		// Take particle colors directly from the color property.
		OVITO_ASSERT(colorProperty->size() == output.size());
		std::copy(colorProperty->constDataColor(), colorProperty->constDataColor() + output.size(), output.begin());
	}
	else if(typeProperty) {
		// Assign colors based on particle types.
		OVITO_ASSERT(typeProperty->size() == output.size());
		// Generate a lookup map for particle type colors.
		const std::map<int,Color> colorMap = typeProperty->colorMap();
		std::array<Color,16> colorArray;
		// Check if all type IDs are within a small, non-negative range.
		// If yes, we can use an array lookup strategy. Otherwise we have to use a dictionary lookup strategy, which is slower.
		if(std::all_of(colorMap.begin(), colorMap.end(),
				[&colorArray](const std::map<int,Color>::value_type& i) { return i.first >= 0 && i.first < (int)colorArray.size(); })) {
			colorArray.fill(defaultColor);
			for(const auto& entry : colorMap)
				colorArray[entry.first] = entry.second;
			// Fill color array.
			const int* t = typeProperty->constDataInt();
			for(auto c = output.begin(); c != output.end(); ++c, ++t) {
				if(*t >= 0 && *t < (int)colorArray.size())
					*c = colorArray[*t];
				else
					*c = defaultColor;
			}
		}
		else {
			// Fill color array.
			const int* t = typeProperty->constDataInt();
			for(auto c = output.begin(); c != output.end(); ++c, ++t) {
				auto it = colorMap.find(*t);
				if(it != colorMap.end())
					*c = it->second;
				else
					*c = defaultColor;
			}
		}
	}
	else {
		// Assign a constant color to all particles.
		std::fill(output.begin(), output.end(), defaultColor);
	}

	// Highlight selected particles.
	if(selectionProperty) {
		OVITO_ASSERT(selectionProperty->size() == output.size());
		const Color selColor = selectionParticleColor();
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
* Determines the display color of a single particle.
******************************************************************************/
ColorA ParticleDisplay::particleColor(size_t particleIndex, ParticlePropertyObject* colorProperty, ParticleTypeProperty* typeProperty, ParticlePropertyObject* selectionProperty, ParticlePropertyObject* transparencyProperty)
{
	OVITO_ASSERT(colorProperty == nullptr || colorProperty->type() == ParticleProperty::ColorProperty);
	OVITO_ASSERT(typeProperty == nullptr || typeProperty->type() == ParticleProperty::ParticleTypeProperty);
	OVITO_ASSERT(selectionProperty == nullptr || selectionProperty->type() == ParticleProperty::SelectionProperty);
	OVITO_ASSERT(transparencyProperty == nullptr || transparencyProperty->type() == ParticleProperty::TransparencyProperty);

	// Check if particle is selected.
	if(selectionProperty) {
		OVITO_ASSERT(particleIndex < selectionProperty->size());
		if(selectionProperty->getInt(particleIndex))
			return selectionParticleColor();
	}

	ColorA c = defaultParticleColor();
	if(colorProperty) {
		// Take particle color directly from the color property.
		OVITO_ASSERT(particleIndex < colorProperty->size());
		c = colorProperty->getColor(particleIndex);
	}
	else if(typeProperty) {
		// Return color based on particle types.
		OVITO_ASSERT(particleIndex < typeProperty->size());
		ParticleType* ptype = typeProperty->particleType(typeProperty->getInt(particleIndex));
		if(ptype)
			c = ptype->color();
	}

	// Apply alpha component.
	if(transparencyProperty) {
		OVITO_ASSERT(particleIndex < transparencyProperty->size());
		c.a() = FloatType(1) - transparencyProperty->getFloat(particleIndex);
	}

	return c;
}

/******************************************************************************
* Returns the actual rendering quality used to render the given particles.
******************************************************************************/
ParticlePrimitive::RenderingQuality ParticleDisplay::effectiveRenderingQuality(SceneRenderer* renderer, ParticlePropertyObject* positionProperty) const
{
	ParticlePrimitive::RenderingQuality renderQuality = renderingQuality();
	if(renderQuality == ParticlePrimitive::AutoQuality) {
		if(!positionProperty) return ParticlePrimitive::HighQuality;
		size_t particleCount = positionProperty->size();
		if(particleCount < 2000 || renderer->isInteractive() == false)
			renderQuality = ParticlePrimitive::HighQuality;
		else if(particleCount < 100000)
			renderQuality = ParticlePrimitive::MediumQuality;
		else
			renderQuality = ParticlePrimitive::LowQuality;
	}
	return renderQuality;
}

/******************************************************************************
* Lets the display object render the data object.
******************************************************************************/
void ParticleDisplay::render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	// Get input data.
	ParticlePropertyObject* positionProperty = dynamic_object_cast<ParticlePropertyObject>(dataObject);
	ParticlePropertyObject* radiusProperty = ParticlePropertyObject::findInState(flowState, ParticleProperty::RadiusProperty);
	ParticlePropertyObject* colorProperty = ParticlePropertyObject::findInState(flowState, ParticleProperty::ColorProperty);
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(ParticlePropertyObject::findInState(flowState, ParticleProperty::ParticleTypeProperty));
	ParticlePropertyObject* selectionProperty = renderer->isInteractive() ? ParticlePropertyObject::findInState(flowState, ParticleProperty::SelectionProperty) : nullptr;
	ParticlePropertyObject* transparencyProperty = ParticlePropertyObject::findInState(flowState, ParticleProperty::TransparencyProperty);
	ParticlePropertyObject* shapeProperty = ParticlePropertyObject::findInState(flowState, ParticleProperty::AsphericalShapeProperty);
	if(shadingMode() != ParticlePrimitive::NormalShading)
		shapeProperty = nullptr;

	// Get number of particles.
	int particleCount = positionProperty ? (int)positionProperty->size() : 0;

	// Do we have to re-create the geometry buffer from scratch?
	bool recreateBuffer = !_particleBuffer || !_particleBuffer->isValid(renderer);

	// If rendering quality is set to automatic, pick quality level based on number of particles.
	ParticlePrimitive::RenderingQuality renderQuality = effectiveRenderingQuality(renderer, positionProperty);

	// Determine effective particle shape.
	ParticlePrimitive::ParticleShape effectiveParticleShape = particleShape();
	if(effectiveParticleShape == ParticlePrimitive::SquareShape && shapeProperty != nullptr)
		effectiveParticleShape = ParticlePrimitive::BoxShape;
	else
		shapeProperty = nullptr;

	// Set shading mode and rendering quality.
	if(!recreateBuffer) {
		recreateBuffer |= !(_particleBuffer->setShadingMode(shadingMode()));
		recreateBuffer |= !(_particleBuffer->setRenderingQuality(renderQuality));
		recreateBuffer |= !(_particleBuffer->setParticleShape(effectiveParticleShape));
		recreateBuffer |= ((transparencyProperty != nullptr) != _particleBuffer->translucentParticles());
	}

	// Do we have to resize the render buffer?
	bool resizeBuffer = recreateBuffer || (_particleBuffer->particleCount() != particleCount);

	// Do we have to update the particle positions in the render buffer?
	bool updatePositions = _positionsCacheHelper.updateState(positionProperty)
			|| resizeBuffer;

	// Do we have to update the particle radii in the geometry buffer?
	bool updateRadii = _radiiCacheHelper.updateState(
			radiusProperty,
			typeProperty,
			defaultParticleRadius())
			|| resizeBuffer;

	// Do we have to update the particle colors in the geometry buffer?
	bool updateColors = _colorsCacheHelper.updateState(
			colorProperty,
			typeProperty,
			selectionProperty,
			transparencyProperty,
			positionProperty)
			|| resizeBuffer;

	// Do we have to update the particle shapes in the geometry buffer?
	bool updateShapes = _shapesCacheHelper.updateState(
			shapeProperty) || resizeBuffer;

	// Re-create the geometry buffer if necessary.
	if(recreateBuffer)
		_particleBuffer = renderer->createParticlePrimitive(shadingMode(), renderQuality, effectiveParticleShape, transparencyProperty != nullptr);

	// Re-size the geometry buffer if necessary.
	if(resizeBuffer)
		_particleBuffer->setSize(particleCount);

	// Update position buffer.
	if(updatePositions && positionProperty) {
		OVITO_ASSERT(positionProperty->size() == particleCount);
		_particleBuffer->setParticlePositions(positionProperty->constDataPoint3());
	}

	// Update radius buffer.
	if(updateRadii && particleCount) {
		if(radiusProperty) {
			// Take particle radii directly from the radius property.
			OVITO_ASSERT(radiusProperty->size() == particleCount);
			_particleBuffer->setParticleRadii(radiusProperty->constDataFloat());
		}
		else if(typeProperty) {
			// Assign radii based on particle types.
			OVITO_ASSERT(typeProperty->size() == particleCount);
			// Build a lookup map for particle type raii.
			const std::map<int,FloatType> radiusMap = typeProperty->radiusMap();
			// Skip the following loop if all per-type radii are zero. In this case, simply use the default radius for all particles.
			if(std::any_of(radiusMap.cbegin(), radiusMap.cend(), [](const std::pair<int,FloatType>& it) { return it.second != 0; })) {
				// Allocate memory buffer.
				std::vector<FloatType> particleRadii(particleCount, defaultParticleRadius());
				// Fill radius array.
				const int* t = typeProperty->constDataInt();
				for(auto c = particleRadii.begin(); c != particleRadii.end(); ++c, ++t) {
					auto it = radiusMap.find(*t);
					// Set particle radius only if the type's radius is non-zero.
					if(it != radiusMap.end() && it->second != 0)
						*c = it->second;
				}
				_particleBuffer->setParticleRadii(particleRadii.data());
			}
			else {
				// Assign a constant radius to all particles.
				_particleBuffer->setParticleRadius(defaultParticleRadius());
			}
		}
		else {
			// Assign a constant radius to all particles.
			_particleBuffer->setParticleRadius(defaultParticleRadius());
		}
	}

	// Update color buffer.
	if(updateColors && particleCount) {
		if(colorProperty && !selectionProperty && !transparencyProperty) {
			// Direct particle colors.
			OVITO_ASSERT(colorProperty->size() == particleCount);
			_particleBuffer->setParticleColors(colorProperty->constDataColor());
		}
		else {
			std::vector<Color> colors(particleCount);
			particleColors(colors, colorProperty, typeProperty, selectionProperty);
			if(!transparencyProperty) {
				_particleBuffer->setParticleColors(colors.data());
			}
			else {
				// Add alpha channel based on transparency particle property.
				std::vector<ColorA> colorsWithAlpha(particleCount);
				const FloatType* t = transparencyProperty->constDataFloat();
				auto c_in = colors.cbegin();
				for(auto c_out = colorsWithAlpha.begin(); c_out != colorsWithAlpha.end(); ++c_out, ++c_in, ++t) {
					c_out->r() = c_in->r();
					c_out->g() = c_in->g();
					c_out->b() = c_in->b();
					c_out->a() = FloatType(1) - (*t);
				}
				_particleBuffer->setParticleColors(colorsWithAlpha.data());
			}
		}
	}

	// Update shapes buffer.
	if(updateShapes && particleCount) {
		if(shapeProperty) {
			OVITO_ASSERT(shapeProperty->size() == particleCount);
			_particleBuffer->setParticleShapes(shapeProperty->constDataVector3());
		}
	}

	if(renderer->isPicking()) {
		OORef<ParticlePickInfo> pickInfo(new ParticlePickInfo(flowState));
		renderer->beginPickObject(contextNode, pickInfo);
	}

	_particleBuffer->render(renderer);

	if(renderer->isPicking()) {
		renderer->endPickObject();
	}
}

/******************************************************************************
* Returns a human-readable string describing the picked object,
* which will be displayed in the status bar by OVITO.
******************************************************************************/
QString ParticlePickInfo::infoString(ObjectNode* objectNode, quint32 subobjectId)
{
	QString str;
	for(DataObject* dataObj : pipelineState().objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(dataObj);
		if(!property || property->size() <= subobjectId) continue;
		if(property->type() == ParticleProperty::SelectionProperty) continue;
		if(property->type() == ParticleProperty::ColorProperty) continue;
		if(property->dataType() != qMetaTypeId<int>() && property->dataType() != qMetaTypeId<FloatType>()) continue;
		if(!str.isEmpty()) str += QStringLiteral(" | ");
		str += property->name();
		str += QStringLiteral(" ");
		for(size_t component = 0; component < property->componentCount(); component++) {
			if(component != 0) str += QStringLiteral(", ");
			QString valueString;
			if(property->dataType() == qMetaTypeId<int>()) {
				str += QString::number(property->getIntComponent(subobjectId, component));
				ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(property);
				if(typeProperty && typeProperty->particleTypes().empty() == false) {
					if(ParticleType* ptype = typeProperty->particleType(property->getIntComponent(subobjectId, component)))
						str += QString(" (%1)").arg(ptype->name());
				}
			}
			else if(property->dataType() == qMetaTypeId<FloatType>())
				str += QString::number(property->getFloatComponent(subobjectId, component));
		}
	}
	return str;
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ParticleDisplayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Particle display"), rolloutParams, "display_objects.particles.html");

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	// Shading mode.
	VariantComboBoxParameterUI* shadingModeUI = new VariantComboBoxParameterUI(this, "shadingMode");
	shadingModeUI->comboBox()->addItem(tr("Normal"), qVariantFromValue(ParticlePrimitive::NormalShading));
	shadingModeUI->comboBox()->addItem(tr("Flat"), qVariantFromValue(ParticlePrimitive::FlatShading));
	layout->addWidget(new QLabel(tr("Shading mode:")), 0, 0);
	layout->addWidget(shadingModeUI->comboBox(), 0, 1);

	// Rendering quality.
	VariantComboBoxParameterUI* renderingQualityUI = new VariantComboBoxParameterUI(this, "renderingQuality");
	renderingQualityUI->comboBox()->addItem(tr("Low"), qVariantFromValue(ParticlePrimitive::LowQuality));
	renderingQualityUI->comboBox()->addItem(tr("Medium"), qVariantFromValue(ParticlePrimitive::MediumQuality));
	renderingQualityUI->comboBox()->addItem(tr("High"), qVariantFromValue(ParticlePrimitive::HighQuality));
	renderingQualityUI->comboBox()->addItem(tr("Automatic"), qVariantFromValue(ParticlePrimitive::AutoQuality));
	layout->addWidget(new QLabel(tr("Rendering quality:")), 1, 0);
	layout->addWidget(renderingQualityUI->comboBox(), 1, 1);

	// Shape.
	VariantComboBoxParameterUI* particleShapeUI = new VariantComboBoxParameterUI(this, "particleShape");
	particleShapeUI->comboBox()->addItem(tr("Spherical"), qVariantFromValue(ParticlePrimitive::SphericalShape));
	particleShapeUI->comboBox()->addItem(tr("Square"), qVariantFromValue(ParticlePrimitive::SquareShape));
	layout->addWidget(new QLabel(tr("Shape:")), 2, 0);
	layout->addWidget(particleShapeUI->comboBox(), 2, 1);

	// Default radius.
	FloatParameterUI* radiusUI = new FloatParameterUI(this, PROPERTY_FIELD(ParticleDisplay::_defaultParticleRadius));
	layout->addWidget(radiusUI->label(), 3, 0);
	layout->addLayout(radiusUI->createFieldLayout(), 3, 1);
	radiusUI->setMinValue(0);
}

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace
