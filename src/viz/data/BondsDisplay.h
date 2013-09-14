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
 * \file BondsDisplay.h
 * \brief Contains the definition of the Ovito::BondsDisplay class.
 */

#ifndef __OVITO_BONDS_DISPLAY_H
#define __OVITO_BONDS_DISPLAY_H

#include <core/Core.h>
#include <core/scene/display/DisplayObject.h>
#include <core/rendering/ArrowGeometryBuffer.h>
#include <core/gui/properties/PropertiesEditor.h>
#include "BondsObject.h"
#include "ParticlePropertyObject.h"
#include "SimulationCell.h"

namespace Viz {

using namespace Ovito;

/**
 * \brief A scene display object for bonds.
 */
class BondsDisplay : public DisplayObject
{
public:

	/// \brief Default constructor.
	Q_INVOKABLE BondsDisplay();

	/// \brief Renders the associated scene object.
	virtual void render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the display bounding box of the scene object.
	virtual Box3 boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Bonds"); }

	/// \brief Returns the display width of bonds.
	FloatType bondWidth() const { return _bondWidth; }

	/// \brief Sets the display width of bonds.
	void setBondWidth(FloatType newWidth) { _bondWidth = newWidth; }

	/// \brief Returns the selected shading mode for bonds.
	ArrowGeometryBuffer::ShadingMode shadingMode() const { return _shadingMode; }

	/// \brief Sets the shading mode for bonds.
	void setShadingMode(ArrowGeometryBuffer::ShadingMode mode) { _shadingMode = mode; }

	/// \brief Returns the selected rendering quality mode for bonds.
	ArrowGeometryBuffer::RenderingQuality renderingQuality() const { return _renderingQuality; }

	/// \brief Sets the rendering quality mode for bonds.
	void setRenderingQuality(ArrowGeometryBuffer::RenderingQuality quality) { _renderingQuality = quality; }

	/// Returns the display color for bonds.
	const Color& bondColor() const { return _bondColor; }

	/// Sets the display color for bonds.
	void setBondColor(const Color& color) { _bondColor = color; }

	/// Returns whether bonds colors are derived from particle colors.
	bool useParticleColors() const { return _useParticleColors; }

	/// Controls whether bonds colors are derived from particle colors.
	void setUseParticleColors(bool enable) { _useParticleColors = enable; }

public:

	Q_PROPERTY(FloatType bondWidth READ bondWidth WRITE setBondWidth)
	Q_PROPERTY(Ovito::Color bondColor READ bondColor WRITE setBondColor)
	Q_PROPERTY(bool useParticleColors READ useParticleColors WRITE setUseParticleColors)
	Q_PROPERTY(Ovito::ArrowGeometryBuffer::ShadingMode shadingMode READ shadingMode WRITE setShadingMode)
	Q_PROPERTY(Ovito::ArrowGeometryBuffer::RenderingQuality renderingQuality READ renderingQuality WRITE setRenderingQuality)

protected:

	/// Searches for the given standard particle property in the scene objects stored in the pipeline flow state.
	ParticlePropertyObject* findStandardProperty(ParticleProperty::Type type, const PipelineFlowState& flowState) const;

protected:

	/// Controls the display width of bonds.
	PropertyField<FloatType> _bondWidth;

	/// Controls the color of the bonds.
	PropertyField<Color, QColor> _bondColor;

	/// Controls whether bonds colors are derived from particle colors.
	PropertyField<bool> _useParticleColors;

	/// Controls the shading mode for bonds.
	PropertyField<ArrowGeometryBuffer::ShadingMode, int> _shadingMode;

	/// Controls the rendering quality mode for bonds.
	PropertyField<ArrowGeometryBuffer::RenderingQuality, int> _renderingQuality;

	/// The buffered geometry used to render the bonds.
	OORef<ArrowGeometryBuffer> _buffer;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffer.
	SceneObjectCacheHelper<
		QPointer<BondsObject>, unsigned int,				// The bonds scene object + revision number
		QPointer<ParticlePropertyObject>, unsigned int,		// Particle position property + revision number
		QPointer<ParticlePropertyObject>, unsigned int,		// Particle color property + revision number
		QPointer<ParticlePropertyObject>, unsigned int,		// Particle type property + revision number
		QPointer<SimulationCell>, unsigned int,				// Simulation cell + revision number
		FloatType,											// Bond width
		Color,												// Bond color
		bool												// Use particle colors
	> _geometryCacheHelper;

	/// The bounding box that includes all bonds.
	Box3 _cachedBoundingBox;

	/// This helper structure is used to detect changes in the input data
	/// that require recomputing the bounding box.
	SceneObjectCacheHelper<
		QPointer<BondsObject>, unsigned int,				// The bonds scene object + revision number
		QPointer<ParticlePropertyObject>, unsigned int,		// Particle position property + revision number
		QPointer<SimulationCell>, unsigned int,				// Simulation cell + revision number
		FloatType											// Bond width
	> _boundingBoxCacheHelper;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_bondWidth);
	DECLARE_PROPERTY_FIELD(_bondColor);
	DECLARE_PROPERTY_FIELD(_useParticleColors);
	DECLARE_PROPERTY_FIELD(_shadingMode);
	DECLARE_PROPERTY_FIELD(_renderingQuality);
};

/**
 * \brief A properties editor for the BondsDisplay class.
 */
class BondsDisplayEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE BondsDisplayEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_BONDS_DISPLAY_H
