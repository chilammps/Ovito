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

#ifndef __OVITO_AMBIENT_OCCLUSION_MODIFIER_H
#define __OVITO_AMBIENT_OCCLUSION_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <core/gui/properties/RefTargetListParameterUI.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>
#include <plugins/particles/modifier/AsynchronousParticleModifier.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Coloring)

/**
 * \brief Calculates ambient occlusion lighting for particles.
 */
class OVITO_PARTICLES_EXPORT AmbientOcclusionModifier : public AsynchronousParticleModifier
{
public:

	/// Computes the modifier's results.
	class AmbientOcclusionEngine : public ComputeEngine
	{
	public:

		/// Constructor.
		AmbientOcclusionEngine(const TimeInterval& validityInterval, int resolution, int samplingCount, ParticleProperty* positions, const Box3& boundingBox, std::vector<FloatType>&& particleRadii) :
			ComputeEngine(validityInterval),
			_resolution(resolution),
			_samplingCount(samplingCount),
			_positions(positions),
			_boundingBox(boundingBox),
			_brightness(new ParticleProperty(positions->size(), qMetaTypeId<FloatType>(), sizeof(FloatType), 1, sizeof(FloatType), tr("Brightness"), true)),
			_particleRadii(particleRadii) {
			_offscreenSurface.setFormat(ViewportSceneRenderer::getDefaultSurfaceFormat());
			_offscreenSurface.create();
		}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void perform() override;

		/// Returns the property storage that contains the input particle positions.
		ParticleProperty* positions() const { return _positions.data(); }

		/// Returns the property storage that contains the computed per-particle brightness values.
		ParticleProperty* brightness() const { return _brightness.data(); }

	private:

		int _resolution;
		int _samplingCount;
		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<ParticleProperty> _brightness;
		Box3 _boundingBox;
		std::vector<FloatType> _particleRadii;
		QOffscreenSurface _offscreenSurface;
	};

public:

	/// Constructor.
	Q_INVOKABLE AmbientOcclusionModifier(DataSet* dataset);

	/// Returns the intensity of the shading.
	FloatType intensity() const { return _intensity; }

	/// Sets the intensity of the shading.
	void setIntensity(FloatType newIntensity) { _intensity = newIntensity; }

	/// Returns the amount of spherical sampling points used in the shading computation.
	int samplingCount() const { return _samplingCount; }

	/// Sets the amount of spherical sampling points used in the shading computation.
	void setSamplingCount(int count) { _samplingCount = count; }

	/// Returns the buffer resolution level.
	int bufferResolution() const { return _bufferResolution; }

	/// Sets the buffer resolution level.
	void setBufferResolution(int res) { _bufferResolution = res; }

protected:

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Creates and initializes a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<ComputeEngine> createEngine(TimePoint time, TimeInterval validityInterval) override;

	/// Unpacks the results of the computation engine and stores them in the modifier.
	virtual void transferComputationResults(ComputeEngine* engine) override;

	/// Lets the modifier insert the cached computation results into the modification pipeline.
	virtual PipelineStatus applyComputationResults(TimePoint time, TimeInterval& validityInterval) override;

private:

	/// This stores the cached results of the modifier, i.e. the brightness value of each particle.
	QExplicitlySharedDataPointer<ParticleProperty> _brightnessValues;

	/// This controls the intensity of the shading effect.
	PropertyField<FloatType> _intensity;

	/// Controls the quality of the lighting computation.
	PropertyField<int> _samplingCount;

	/// Controls the resolution of the offscreen rendering buffer.
	PropertyField<int> _bufferResolution;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Ambient occlusion");
	Q_CLASSINFO("ModifierCategory", "Coloring");

	DECLARE_PROPERTY_FIELD(_intensity);
	DECLARE_PROPERTY_FIELD(_samplingCount);
	DECLARE_PROPERTY_FIELD(_bufferResolution);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A properties editor for the AmbientOcclusionModifier class.
 */
class AmbientOcclusionModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE AmbientOcclusionModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_AMBIENT_OCCLUSION_MODIFIER_H
