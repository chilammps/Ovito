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

#ifndef __OVITO_PARTICLE_DISPLAY_H
#define __OVITO_PARTICLE_DISPLAY_H

#include <plugins/particles/Particles.h>
#include <core/scene/objects/DisplayObject.h>
#include <core/scene/objects/WeakVersionedObjectReference.h>
#include <core/rendering/ParticlePrimitive.h>
#include <core/rendering/SceneRenderer.h>
#include <core/gui/properties/PropertiesEditor.h>
#include "ParticlePropertyObject.h"
#include "ParticleTypeProperty.h"

namespace Ovito { namespace Particles {

/**
 * \brief A scene display object for particles.
 */
class OVITO_PARTICLES_EXPORT ParticleDisplay : public DisplayObject
{
public:

	/// \brief Constructor.
	Q_INVOKABLE ParticleDisplay(DataSet* dataset);

	/// \brief Lets the display object render the data object.
	virtual void render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Particles"); }

	/// \brief Returns the default display radius of particles.
	FloatType defaultParticleRadius() const { return _defaultParticleRadius; }

	/// \brief Returns the default display color for particles.
	Color defaultParticleColor() const { return Color(1,1,1); }

	/// \brief Returns the display color used for selected particles.
	Color selectionParticleColor() const { return Color(1,0,0); }

	/// \brief Sets the default display radius of atomic particles.
	void setDefaultParticleRadius(FloatType newRadius) { _defaultParticleRadius = newRadius; }

	/// \brief Returns the selected shading mode for particles.
	ParticlePrimitive::ShadingMode shadingMode() const { return _shadingMode; }

	/// \brief Sets the shading mode for particles.
	void setShadingMode(ParticlePrimitive::ShadingMode mode) { _shadingMode = mode; }

	/// \brief Returns the selected rendering quality mode for particles.
	ParticlePrimitive::RenderingQuality renderingQuality() const { return _renderingQuality; }

	/// \brief Returns the actual rendering quality used to render the given particles.
	ParticlePrimitive::RenderingQuality effectiveRenderingQuality(SceneRenderer* renderer, ParticlePropertyObject* positionProperty) const;

	/// \brief Sets the rendering quality mode for particles.
	void setRenderingQuality(ParticlePrimitive::RenderingQuality quality) { _renderingQuality = quality; }

	/// \brief Returns the display shape of particles.
	ParticlePrimitive::ParticleShape particleShape() const { return _particleShape; }

	/// \brief Sets the display shape of particles.
	void setParticleShape(ParticlePrimitive::ParticleShape shape) { _particleShape = shape; }

	/// \brief Determines the display particle colors.
	void particleColors(std::vector<Color>& output, ParticlePropertyObject* colorProperty, ParticleTypeProperty* typeProperty, ParticlePropertyObject* selectionProperty = nullptr);

	/// \brief Determines the display particle radii.
	void particleRadii(std::vector<FloatType>& output, ParticlePropertyObject* radiusProperty, ParticleTypeProperty* typeProperty);

	/// \brief Determines the display radius of a single particle.
	FloatType particleRadius(size_t particleIndex, ParticlePropertyObject* radiusProperty, ParticleTypeProperty* typeProperty);

	/// \brief Determines the display color of a single particle.
	ColorA particleColor(size_t particleIndex, ParticlePropertyObject* colorProperty, ParticleTypeProperty* typeProperty, ParticlePropertyObject* selectionProperty, ParticlePropertyObject* transparencyProperty);

	/// \brief Computes the bounding box of the particles.
	Box3 particleBoundingBox(ParticlePropertyObject* positionProperty, ParticleTypeProperty* typeProperty, ParticlePropertyObject* radiusProperty, ParticlePropertyObject* shapeProperty, bool includeParticleRadius = true);

public:

    Q_PROPERTY(Ovito::ParticlePrimitive::ShadingMode shadingMode READ shadingMode WRITE setShadingMode);
    Q_PROPERTY(Ovito::ParticlePrimitive::RenderingQuality renderingQuality READ renderingQuality WRITE setRenderingQuality);
    Q_PROPERTY(Ovito::ParticlePrimitive::ParticleShape particleShape READ particleShape WRITE setParticleShape);

protected:

	/// Controls the default display radius of atomic particles.
	PropertyField<FloatType> _defaultParticleRadius;

	/// Controls the shading mode for particles.
	PropertyField<ParticlePrimitive::ShadingMode, int> _shadingMode;

	/// Controls the rendering quality mode for particles.
	PropertyField<ParticlePrimitive::RenderingQuality, int> _renderingQuality;

	/// Controls the display shape of particles.
	PropertyField<ParticlePrimitive::ParticleShape, int> _particleShape;

	/// The buffered particle geometry used to render the particles.
	std::shared_ptr<ParticlePrimitive> _particleBuffer;

	/// This helper structure is used to detect any changes in the particle positions
	/// that require updating the particle position buffer.
	SceneObjectCacheHelper<
		WeakVersionedOORef<ParticlePropertyObject>
		> _positionsCacheHelper;

	/// This helper structure is used to detect any changes in the particle radii
	/// that require updating the particle radius buffer.
	SceneObjectCacheHelper<
		WeakVersionedOORef<ParticlePropertyObject>,		// Radius property + revision number
		WeakVersionedOORef<ParticlePropertyObject>,		// Type property + revision number
		FloatType										// Default radius
		> _radiiCacheHelper;

	/// This helper structure is used to detect any changes in the particle shapes
	/// that require updating the particle shape buffer.
	SceneObjectCacheHelper<
		WeakVersionedOORef<ParticlePropertyObject>		// Shape property + revision number
		> _shapesCacheHelper;

	/// This helper structure is used to detect any changes in the particle colors
	/// that require updating the particle color buffer.
	SceneObjectCacheHelper<
		WeakVersionedOORef<ParticlePropertyObject>,		// Color property + revision number
		WeakVersionedOORef<ParticlePropertyObject>,		// Type property + revision number
		WeakVersionedOORef<ParticlePropertyObject>,		// Selection property + revision number
		WeakVersionedOORef<ParticlePropertyObject>,		// Transparency property + revision number
		WeakVersionedOORef<ParticlePropertyObject>		// Position property + revision number
		> _colorsCacheHelper;

	/// The bounding box that includes all particles.
	Box3 _cachedBoundingBox;

	/// This helper structure is used to detect changes in the input objects
	/// that require rebuilding the bounding box.
	SceneObjectCacheHelper<
		WeakVersionedOORef<ParticlePropertyObject>,	// Position property + revision number
		WeakVersionedOORef<ParticlePropertyObject>,	// Radius property + revision number
		WeakVersionedOORef<ParticlePropertyObject>,	// Type property + revision number
		WeakVersionedOORef<ParticlePropertyObject>,	// Asperical shape property + revision number
		FloatType> 									// Default particle radius
		_boundingBoxCacheHelper;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_defaultParticleRadius);
	DECLARE_PROPERTY_FIELD(_shadingMode);
	DECLARE_PROPERTY_FIELD(_renderingQuality);
	DECLARE_PROPERTY_FIELD(_particleShape);
};

/**
 * An information record used for particle picking in the viewports.
 */
class OVITO_PARTICLES_EXPORT ParticlePickInfo : public ObjectPickInfo
{
public:

	/// Constructor.
	ParticlePickInfo(const PipelineFlowState& pipelineState) : _pipelineState(pipelineState) {}

	/// The pipeline flow state containing the particle properties.
	const PipelineFlowState& pipelineState() const { return _pipelineState; }

	/// Returns a human-readable string describing the picked object, which will be displayed in the status bar by OVITO.
	virtual QString infoString(ObjectNode* objectNode, quint32 subobjectId) override;

private:

	/// The pipeline flow state containing the particle properties.
	PipelineFlowState _pipelineState;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

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

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace

#endif // __OVITO_PARTICLE_DISPLAY_H
