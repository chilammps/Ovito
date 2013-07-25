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
 * \file AmbientOcclusionModifier.h
 * \brief Contains the definition of the Viz::AmbientOcclusionModifier class.
 */

#ifndef __OVITO_AMBIENT_OCCLUSION_MODIFIER_H
#define __OVITO_AMBIENT_OCCLUSION_MODIFIER_H

#include <core/Core.h>
#include <core/gui/properties/RefTargetListParameterUI.h>
#include <viz/modifier/AsynchronousParticleModifier.h>

namespace Viz {

using namespace Ovito;

/**
 * \brief Calculates ambient occlusion lighting for particles.
 */
class AmbientOcclusionModifier : public AsynchronousParticleModifier
{
public:

	/// Computes the modifier's results.
	class AmbientOcclusionEngine : public AsynchronousParticleModifier::Engine
	{
	public:

		/// Constructor.
		AmbientOcclusionEngine(int resolution, int samplingCount, ParticleProperty* positions, const Box3& boundingBox, std::vector<FloatType>&& particleRadii) :
			_resolution(resolution),
			_samplingCount(samplingCount),
			_positions(positions),
			_boundingBox(boundingBox),
			_brightness(new ParticleProperty(positions->size(), qMetaTypeId<FloatType>(), sizeof(FloatType), 1, tr("Brightness"))),
			_particleRadii(particleRadii) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void compute(FutureInterfaceBase& futureInterface) override;

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
	};

public:

	/// Default constructor.
	Q_INVOKABLE AmbientOcclusionModifier();

	/// Returns the computed per-particle brightness values.
	const ParticleProperty& brightnessValues() const { OVITO_CHECK_POINTER(_brightnessValues.constData()); return *_brightnessValues; }

	/// Returns the intensity of the shading.
	FloatType intensity() const { return _intensity; }

	/// Sets the intensity of the shading.
	void setIntensity(FloatType newIntensity) { _intensity = newIntensity; }

	/// Returns the amount of spherical sampling points used in the shading computation.
	int samplingCount() const { return _samplingCount; }

	/// Sets the amount of spherical sampling points used in the shading computation.
	void setSamplingCount(int count) { _samplingCount = count; }

	/// Returns the buffer resolution level.
	/// This must be an integer value between 0 and 5.
	int bufferResolution() const { return _bufferResolution; }

	/// Sets the buffer resolution level.
	/// This must be an integer value between 0 and 5.
	void setBufferResolution(int res) { _bufferResolution = res; }

public:

	Q_PROPERTY(FloatType intensity READ intensity WRITE setIntensity)
	Q_PROPERTY(int samplingCount READ samplingCount WRITE setSamplingCount)
	Q_PROPERTY(int bufferResolution READ bufferResolution WRITE setBufferResolution)

protected:

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Creates and initializes a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<Engine> createEngine(TimePoint time) override;

	/// Unpacks the computation results stored in the given engine object.
	virtual void retrieveModifierResults(Engine* engine) override;

	/// This lets the modifier insert the previously computed results into the pipeline.
	virtual ObjectStatus applyModifierResults(TimePoint time, TimeInterval& validityInterval) override;

private:

	/// This stores the cached results of the modifier, i.e. the brightness assigned to the particles.
	QExplicitlySharedDataPointer<ParticleProperty> _brightnessValues;

	/// This controls the intensity of the ambient lighting effect.
	PropertyField<FloatType> _intensity;

	/// Controls the quality of the lighting computation.
	PropertyField<int> _samplingCount;

	/// Controls the resolution of the offscreen rendering buffer.
	PropertyField<int> _bufferResolution;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Ambient Occlusion");
	Q_CLASSINFO("ModifierCategory", "Coloring");

	DECLARE_PROPERTY_FIELD(_intensity);
	DECLARE_PROPERTY_FIELD(_samplingCount);
	DECLARE_PROPERTY_FIELD(_bufferResolution);
};

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


};	// End of namespace

#endif // __OVITO_AMBIENT_OCCLUSION_MODIFIER_H
