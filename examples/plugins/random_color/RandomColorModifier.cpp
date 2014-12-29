#include "RandomColorModifier.h"

// This macro call is needed for each defined OVITO object class.
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(RandomColorPlugin, RandomColorModifier, ParticleModifier);

/******************************************************************************
* Modifies the particles.
******************************************************************************/
PipelineStatus RandomColorModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Create the 'Color' particle property.
	ParticlePropertyObject* colorProperty = outputStandardProperty(ParticleProperty::ColorProperty);

	// Initialize random number generator.
	// Use current simulation time as seed.
	std::default_random_engine rng(time);
	std::uniform_real_distribution<FloatType> distribution;

	// Set the color property of each particle to a random value.
	// The Color::fromHSV() function converts a hue value to an RGB color.
	for(Color& c : colorProperty->colorRange())
		c = Color::fromHSV(distribution(rng), 1, 1);

	// By convention, ParticlePropertyObject::changed() must be called whenever a particle property
	// has been modified. This is to invalidate any data caches down the modification pipeline.
	colorProperty->changed();

	return PipelineStatus::Success;
}
