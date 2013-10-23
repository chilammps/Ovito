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

/**
 * \file VectorDisplay.h
 * \brief Contains the definition of the Viz::VectorDisplay class.
 */

#ifndef __OVITO_VECTOR_DISPLAY_H
#define __OVITO_VECTOR_DISPLAY_H

#include <core/Core.h>
#include <core/scene/display/DisplayObject.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/rendering/ArrowGeometryBuffer.h>
#include "ParticlePropertyObject.h"

namespace Viz {

using namespace Ovito;

/**
 * \brief A scene display object for per-particle vectors.
 */
class VectorDisplay : public DisplayObject
{
public:

	/// \brief Default constructor.
	Q_INVOKABLE VectorDisplay();

	/// \brief Lets the display object render a scene object.
	virtual void render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Vectors"); }

	/// \brief Returns the selected shading mode for arrows.
	ArrowGeometryBuffer::ShadingMode shadingMode() const { return _shadingMode; }

	/// \brief Sets the shading mode for arrows.
	void setShadingMode(ArrowGeometryBuffer::ShadingMode mode) { _shadingMode = mode; }

	/// \brief Returns the selected rendering quality mode for arrows.
	ArrowGeometryBuffer::RenderingQuality renderingQuality() const { return _renderingQuality; }

	/// \brief Sets the rendering quality mode for arrows.
	void setRenderingQuality(ArrowGeometryBuffer::RenderingQuality quality) { _renderingQuality = quality; }

	/// Returns whether the arrow pointing direction is reversed.
	bool reverseArrowDirection() const { return _reverseArrowDirection; }

	/// Returns whether vectors are flipped.
	bool flipVectors() const { return _flipVectors; }

	/// Returns the display color of the arrows.
	const Color& arrowColor() const { return _arrowColor; }

	/// Sets the display color of the arrows.
	void setArrowColor(const Color& color) { _arrowColor = color; }

	/// Returns the display width of the arrows.
	FloatType arrowWidth() const { return _arrowWidth; }

	/// Sets the display width of the arrows.
	void setArrowWidth(FloatType width) { _arrowWidth = width; }

	/// Returns the scaling factor that is applied to the vectors.
	FloatType scalingFactor() const { return _scalingFactor; }

	/// Sets the scaling factor that is applied to the vectors.
	void setScalingFactor(FloatType factor) { _scalingFactor = factor; }

public:

	Q_PROPERTY(Ovito::ArrowGeometryBuffer::ShadingMode shadingMode READ shadingMode WRITE setShadingMode)
	Q_PROPERTY(Ovito::ArrowGeometryBuffer::RenderingQuality renderingQuality READ renderingQuality WRITE setRenderingQuality)
	Q_PROPERTY(Ovito::Color arrowColor READ arrowColor WRITE setArrowColor)
	Q_PROPERTY(FloatType arrowWidth READ arrowWidth WRITE setArrowWidth)
	Q_PROPERTY(FloatType scalingFactor READ scalingFactor WRITE setScalingFactor)

protected:

	/// Searches for the given standard particle property in the scene objects stored in the pipeline flow state.
	ParticlePropertyObject* findStandardProperty(ParticleProperty::Type type, const PipelineFlowState& flowState) const;

	/// Computes the bounding box of the arrows.
	Box3 arrowBoundingBox(ParticlePropertyObject* vectorProperty, ParticlePropertyObject* positionProperty);

protected:

	/// Enables the reversal of the arrow pointing direction.
	PropertyField<bool> _reverseArrowDirection;

	/// Controls the flipping of the vectors.
	PropertyField<bool> _flipVectors;

	/// Controls the color of the arrows.
	PropertyField<Color, QColor> _arrowColor;

	/// Controls the width of the arrows in world units.
	PropertyField<FloatType> _arrowWidth;

	/// Controls the scaling factor applied to the vectors.
	PropertyField<FloatType> _scalingFactor;

	/// Controls the shading mode for arrows.
	PropertyField<ArrowGeometryBuffer::ShadingMode, int> _shadingMode;

	/// Controls the rendering quality mode for arrows.
	PropertyField<ArrowGeometryBuffer::RenderingQuality, int> _renderingQuality;

	/// The buffered geometry used to render the arrows.
	OORef<ArrowGeometryBuffer> _buffer;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffer.
	SceneObjectCacheHelper<
		QPointer<ParticlePropertyObject>, unsigned int,		// Vector property + revision number
		QPointer<ParticlePropertyObject>, unsigned int,		// Particle position property + revision number
		FloatType,											// Scaling factor
		FloatType,											// Arrow width
		Color,												// Arrow color
		bool,												// Reverse arrow direction
		bool												// Flip vectors
		> _geometryCacheHelper;

	/// The bounding box that includes all arrows.
	Box3 _cachedBoundingBox;

	/// This helper structure is used to detect changes in the input
	/// that require recalculating the bounding box.
	SceneObjectCacheHelper<
		QPointer<ParticlePropertyObject>, unsigned int,	// Vector property + revision number
		QPointer<ParticlePropertyObject>, unsigned int,	// Particle position property + revision number
		FloatType,										// Scaling factor
		FloatType										// Arrow width
		> _boundingBoxCacheHelper;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_reverseArrowDirection);
	DECLARE_PROPERTY_FIELD(_flipVectors);
	DECLARE_PROPERTY_FIELD(_arrowColor);
	DECLARE_PROPERTY_FIELD(_arrowWidth);
	DECLARE_PROPERTY_FIELD(_scalingFactor);
	DECLARE_PROPERTY_FIELD(_shadingMode);
	DECLARE_PROPERTY_FIELD(_renderingQuality);
};

/**
 * \brief A properties editor for the VectorDisplay class.
 */
class VectorDisplayEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE VectorDisplayEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_VECTOR_DISPLAY_H
