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
 * \file ParticleDisplay.h
 * \brief Contains the definition of the Particles::ParticleDisplay class.
 */

#ifndef __OVITO_PARTICLE_DISPLAY_H
#define __OVITO_PARTICLE_DISPLAY_H

#include <core/Core.h>
#include <core/scene/display/DisplayObject.h>
#include <core/rendering/ParticleGeometryBuffer.h>
#include <core/gui/properties/PropertiesEditor.h>
#include "ParticlePropertyObject.h"

namespace Particles {

using namespace Ovito;

class ParticleTypeProperty;

/**
 * \brief A scene display object for particles.
 */
class ParticleDisplay : public DisplayObject
{
public:

	/// \brief Default constructor.
	Q_INVOKABLE ParticleDisplay();

	/// \brief Lets the display object render a scene object.
	virtual void render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Particles"); }

	/// \brief Returns the default display radius of atomic particles.
	FloatType defaultParticleRadius() const { return _defaultParticleRadius; }

	/// \brief Sets the default display radius of atomic particles.
	void setDefaultParticleRadius(FloatType newRadius) { _defaultParticleRadius = newRadius; }

	/// \brief Returns the selected shading mode for particles.
	ParticleGeometryBuffer::ShadingMode shadingMode() const { return _shadingMode; }

	/// \brief Sets the shading mode for particles.
	void setShadingMode(ParticleGeometryBuffer::ShadingMode mode) { _shadingMode = mode; }

	/// \brief Returns the selected rendering quality mode for particles.
	ParticleGeometryBuffer::RenderingQuality renderingQuality() const { return _renderingQuality; }

	/// \brief Sets the rendering quality mode for particles.
	void setRenderingQuality(ParticleGeometryBuffer::RenderingQuality quality) { _renderingQuality = quality; }

	/// \brief Determines the display particle colors.
	void particleColors(std::vector<Color>& output, ParticlePropertyObject* colorProperty, ParticleTypeProperty* typeProperty, ParticlePropertyObject* selectionProperty = nullptr);

	/// \brief Determines the display particle radii.
	void particleRadii(std::vector<FloatType>& output, ParticlePropertyObject* radiusProperty, ParticleTypeProperty* typeProperty);

	/// \brief Determines the display radius of a single particle.
	FloatType particleRadius(size_t particleIndex, ParticlePropertyObject* radiusProperty, ParticleTypeProperty* typeProperty);

	/// \brief Computes the bounding box of the particles.
	Box3 particleBoundingBox(ParticlePropertyObject* positionProperty, ParticleTypeProperty* typeProperty, ParticlePropertyObject* radiusProperty, bool includeParticleRadius = true);

public:

	Q_PROPERTY(FloatType defaultParticleRadius READ defaultParticleRadius WRITE setDefaultParticleRadius)
	Q_PROPERTY(Ovito::ParticleGeometryBuffer::ShadingMode shadingMode READ shadingMode WRITE setShadingMode)
	Q_PROPERTY(Ovito::ParticleGeometryBuffer::RenderingQuality renderingQuality READ renderingQuality WRITE setRenderingQuality)

protected:

	/// Searches for the given standard particle property in the scene objects stored in the pipeline flow state.
	ParticlePropertyObject* findStandardProperty(ParticleProperty::Type type, const PipelineFlowState& flowState) const;

protected:

	/// Controls the default display radius of atomic particles.
	PropertyField<FloatType> _defaultParticleRadius;

	/// Controls the shading mode for particles.
	PropertyField<ParticleGeometryBuffer::ShadingMode, int> _shadingMode;

	/// Controls the rendering quality mode for particles.
	PropertyField<ParticleGeometryBuffer::RenderingQuality, int> _renderingQuality;

	/// The buffered particle geometry used to render the particles.
	OORef<ParticleGeometryBuffer> _particleBuffer;

	/// This helper structure is used to detect any changes in the particle positions
	/// that require updating the particle position buffer.
	SceneObjectCacheHelper<QPointer<ParticlePropertyObject>, unsigned int> _positionsCacheHelper;

	/// This helper structure is used to detect any changes in the particle radii
	/// that require updating the particle radius buffer.
	SceneObjectCacheHelper<
		QPointer<ParticlePropertyObject>, unsigned int,		// Radius property + revision number
		QPointer<ParticlePropertyObject>, unsigned int,		// Type property + revision number
		FloatType											// Default radius
		> _radiiCacheHelper;

	/// This helper structure is used to detect any changes in the particle colors
	/// that require updating the particle color buffer.
	SceneObjectCacheHelper<
		QPointer<ParticlePropertyObject>, unsigned int,		// Color property + revision number
		QPointer<ParticlePropertyObject>, unsigned int,		// Type property + revision number
		QPointer<ParticlePropertyObject>, unsigned int		// Selection property + revision number
		> _colorsCacheHelper;

	/// The bounding box that includes all particles.
	Box3 _cachedBoundingBox;

	/// This helper structure is used to detect changes in the input objects
	/// that require rebuilding the bounding box.
	SceneObjectCacheHelper<
		QPointer<ParticlePropertyObject>, unsigned int,	// Position property + revision number
		QPointer<ParticlePropertyObject>, unsigned int,	// Radius property + revision number
		QPointer<ParticlePropertyObject>, unsigned int,	// Type property + revision number
		FloatType> 										// Default particle radius
		_boundingBoxCacheHelper;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_defaultParticleRadius);
	DECLARE_PROPERTY_FIELD(_shadingMode);
	DECLARE_PROPERTY_FIELD(_renderingQuality);
};

/**
 * \brief A properties editor for the ParticleDisplay class.
 */
class ParticleDisplayEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE ParticleDisplayEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_PARTICLE_DISPLAY_H
