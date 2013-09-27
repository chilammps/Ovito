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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <viz/data/ParticleDisplay.h>
#include "AmbientOcclusionModifier.h"
#include "AmbientOcclusionRenderer.h"

namespace Viz {

enum { MAX_AO_RENDER_BUFFER_RESOLUTION = 4 };

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, AmbientOcclusionModifier, AsynchronousParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, AmbientOcclusionModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(AmbientOcclusionModifier, AmbientOcclusionModifierEditor)
DEFINE_PROPERTY_FIELD(AmbientOcclusionModifier, _intensity, "Intensity")
DEFINE_PROPERTY_FIELD(AmbientOcclusionModifier, _samplingCount, "SamplingCount")
DEFINE_PROPERTY_FIELD(AmbientOcclusionModifier, _bufferResolution, "BufferResolution")
SET_PROPERTY_FIELD_LABEL(AmbientOcclusionModifier, _intensity, "Shading intensity")
SET_PROPERTY_FIELD_LABEL(AmbientOcclusionModifier, _samplingCount, "Number of exposure samples")
SET_PROPERTY_FIELD_LABEL(AmbientOcclusionModifier, _bufferResolution, "Render buffer resolution")
SET_PROPERTY_FIELD_UNITS(AmbientOcclusionModifier, _intensity, PercentParameterUnit)

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AmbientOcclusionModifier::AmbientOcclusionModifier() :
	_brightnessValues(new ParticleProperty(0, qMetaTypeId<FloatType>(), sizeof(FloatType), 1, tr("Brightness"))),
	_intensity(0.7f), _samplingCount(20), _bufferResolution(3)
{
	INIT_PROPERTY_FIELD(AmbientOcclusionModifier::_intensity);
	INIT_PROPERTY_FIELD(AmbientOcclusionModifier::_samplingCount);
	INIT_PROPERTY_FIELD(AmbientOcclusionModifier::_bufferResolution);
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::Engine> AmbientOcclusionModifier::createEngine(TimePoint time)
{
	if(inputParticleCount() == 0)
		throw Exception(tr("There are no input particles"));

	// Get modifier input.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(inputStandardProperty(ParticleProperty::PositionProperty));
	ParticlePropertyObject* radiusProperty = inputStandardProperty(ParticleProperty::RadiusProperty);
	ParticleDisplay* displayObj = dynamic_object_cast<ParticleDisplay>(posProperty->displayObject());
	if(!displayObj)
		throw Exception(tr("Particles have no display object."));

	// Compute bounding box of input particles.
	Box3 boundingBox = displayObj->particleBoundingBox(posProperty, typeProperty, radiusProperty);

	// The render buffer resolution.
	int res = std::min(std::max(bufferResolution(), 0), (int)MAX_AO_RENDER_BUFFER_RESOLUTION);
	int resolution = (128 << res);

	TimeInterval interval;
	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<AmbientOcclusionEngine>(resolution, samplingCount(), posProperty->storage(), boundingBox, inputParticleRadii(time, interval));
}

/******************************************************************************
* Performs the actual computation. This method is executed in a worker thread.
******************************************************************************/
void AmbientOcclusionModifier::AmbientOcclusionEngine::compute(FutureInterfaceBase& futureInterface)
{
	futureInterface.setProgressText(tr("Computing ambient occlusion lighting"));

	AmbientOcclusionRenderer renderer(QSize(_resolution, _resolution));
	renderer.startRender(nullptr, nullptr);
	try {
		OVITO_ASSERT(!_boundingBox.isEmpty());

		// The buffered particle geometry used to render the particles.
		OORef<ParticleGeometryBuffer> particleBuffer;

		futureInterface.setProgressRange(_samplingCount);
		for(int sample = 0; sample < _samplingCount && !futureInterface.isCanceled(); sample++) {
			futureInterface.setProgressValue(sample);

			// Generate lighting direction on unit sphere.
			FloatType y = (FloatType)sample * 2 / _samplingCount - FloatType(1) + FloatType(1) / _samplingCount;
			FloatType r = sqrt(FloatType(1) - y * y);
			FloatType phi = (FloatType)sample * FLOATTYPE_PI * (3.0 - sqrt(5.0));
			Vector3 dir(cos(phi), y, sin(phi));

			// Set up view projection.
			ViewProjectionParameters projParams;
			projParams.viewMatrix = AffineTransformation::lookAlong(_boundingBox.center(), dir, Vector3(0,0,1));

			// Transform bounding box to camera space.
			Box3 bb = _boundingBox.transformed(projParams.viewMatrix).centerScale(1.01);

			// Complete projection parameters.
			projParams.aspectRatio = 1;
			projParams.isPerspective = false;
			projParams.inverseViewMatrix = projParams.viewMatrix.inverse();
			projParams.fieldOfView = 0.5 * _boundingBox.size().length();
			projParams.znear = -bb.maxc.z();
			projParams.zfar  = std::max(-bb.minc.z(), projParams.znear + 1.0f);
			projParams.projectionMatrix = Matrix4::ortho(-projParams.fieldOfView, projParams.fieldOfView,
								-projParams.fieldOfView, projParams.fieldOfView,
								projParams.znear, projParams.zfar);
			projParams.inverseProjectionMatrix = projParams.projectionMatrix.inverse();
			projParams.validityInterval = TimeInterval::forever();

			renderer.beginFrame(0, projParams, nullptr);
			renderer.setWorldTransform(AffineTransformation::Identity());
			try {
				// Create particle buffer.
				if(!particleBuffer || !particleBuffer->isValid(&renderer)) {
					particleBuffer = renderer.createParticleGeometryBuffer(ParticleGeometryBuffer::FlatShading, ParticleGeometryBuffer::LowQuality);
					particleBuffer->setSize(positions()->size());
					particleBuffer->setParticlePositions(positions()->constDataPoint3());
					particleBuffer->setParticleRadii(_particleRadii.data());
				}
				particleBuffer->render(&renderer, 1);
			}
			catch(...) {
				renderer.endFrame();
				throw;
			}
			renderer.endFrame();

			// Extract brightness values from rendered image.
			const QImage image = renderer.image();
			FloatType* brightnessValues = brightness()->dataFloat();
			for(int y = 0; y < _resolution; y++) {
				const QRgb* pixel = reinterpret_cast<const QRgb*>(image.scanLine(y));
				for(int x = 0; x < _resolution; x++, ++pixel) {
					quint32 red = qRed(*pixel);
					quint32 green = qGreen(*pixel);
					quint32 blue = qBlue(*pixel);
					quint32 alpha = qAlpha(*pixel);
					quint32 id = red + (green << 8) + (blue << 16) + (alpha << 24);
					if(id == 0)
						continue;
					quint32 particleIndex = id - 1;
					OVITO_ASSERT(particleIndex < positions()->size());
					brightnessValues[particleIndex] += 1;
				}
			}
		}
	}
	catch(...) {
		renderer.endRender();
		throw;
	}
	renderer.endRender();

	if(!futureInterface.isCanceled()) {
		futureInterface.setProgressValue(_samplingCount);
		// Normalize brightness values.
		FloatType* b = brightness()->dataFloat();
		FloatType* b_end = b + brightness()->size();
		FloatType maxBrightness = *std::max_element(b, b_end);
		if(maxBrightness != 0) {
			for(; b != b_end; ++b) {
				*b /= maxBrightness;
			}
		}
	}
}

/******************************************************************************
* Unpacks the computation results stored in the given engine object.
******************************************************************************/
void AmbientOcclusionModifier::retrieveModifierResults(Engine* engine)
{
	AmbientOcclusionEngine* eng = static_cast<AmbientOcclusionEngine*>(engine);
	if(eng->brightness())
		_brightnessValues = eng->brightness();
}

/******************************************************************************
* This lets the modifier insert the previously computed results into the pipeline.
******************************************************************************/
ObjectStatus AmbientOcclusionModifier::applyModifierResults(TimePoint time, TimeInterval& validityInterval)
{
	if(inputParticleCount() != brightnessValues().size())
		throw Exception(tr("The number of input particles has changed. The stored results have become invalid."));

	// Get effect intensity.
	FloatType intens = std::min(std::max(intensity(), FloatType(0)), FloatType(1));

	// Get output property object.
	ParticlePropertyObject* colorProperty = outputStandardProperty(ParticleProperty::ColorProperty);
	OVITO_ASSERT(colorProperty->size() == brightnessValues().size());

	std::vector<Color> existingColors = inputParticleColors(time, validityInterval);
	OVITO_ASSERT(colorProperty->size() == existingColors.size());
	const FloatType* b = brightnessValues().constDataFloat();
	Color* c = colorProperty->dataColor();
	Color* c_end = c + colorProperty->size();
	auto c_in = existingColors.cbegin();
	for(; c != c_end; ++b, ++c, ++c_in) {
		FloatType factor = std::min(FloatType(1), FloatType(1) - intens + (*b));
		*c = factor * (*c_in);
	}
	colorProperty->changed();

	return ObjectStatus::Success;
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void AmbientOcclusionModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	// Recompute brightness values when the AO parameters have been changed.
	if(autoUpdateEnabled()) {
		if(field == PROPERTY_FIELD(AmbientOcclusionModifier::_samplingCount) ||
			field == PROPERTY_FIELD(AmbientOcclusionModifier::_bufferResolution))
			invalidateCachedResults();
	}

	AsynchronousParticleModifier::propertyChanged(field);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void AmbientOcclusionModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Ambient occlusion"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(4);

#if 0
	BooleanParameterUI* autoUpdateUI = new BooleanParameterUI(this, PROPERTY_FIELD(AsynchronousParticleModifier::_autoUpdate));
	layout1->addWidget(autoUpdateUI->checkBox());
#endif

	QGridLayout* layout2 = new QGridLayout();
	layout2->setContentsMargins(0,0,0,0);
	layout2->setSpacing(4);
	layout2->setColumnStretch(1, 1);
	layout1->addLayout(layout2);

	// Intensity parameter.
	FloatParameterUI* intensityPUI = new FloatParameterUI(this, PROPERTY_FIELD(AmbientOcclusionModifier::_intensity));
	layout2->addWidget(intensityPUI->label(), 0, 0);
	layout2->addLayout(intensityPUI->createFieldLayout(), 0, 1);
	intensityPUI->setMinValue(0);
	intensityPUI->setMaxValue(1);

	// Sampling level parameter.
	IntegerParameterUI* samplingCountPUI = new IntegerParameterUI(this, PROPERTY_FIELD(AmbientOcclusionModifier::_samplingCount));
	layout2->addWidget(samplingCountPUI->label(), 1, 0);
	layout2->addLayout(samplingCountPUI->createFieldLayout(), 1, 1);
	samplingCountPUI->setMinValue(3);
	samplingCountPUI->setMaxValue(2000);

	// Buffer resolution parameter.
	IntegerParameterUI* bufferResPUI = new IntegerParameterUI(this, PROPERTY_FIELD(AmbientOcclusionModifier::_bufferResolution));
	layout2->addWidget(bufferResPUI->label(), 2, 0);
	layout2->addLayout(bufferResPUI->createFieldLayout(), 2, 1);
	bufferResPUI->setMinValue(1);
	bufferResPUI->setMaxValue(MAX_AO_RENDER_BUFFER_RESOLUTION);

	// Status label.
	layout1->addSpacing(10);
	layout1->addWidget(statusLabel());
}


};	// End of namespace
