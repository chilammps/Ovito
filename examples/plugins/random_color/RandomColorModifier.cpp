#include "RandomColorModifier.h"

// This macro call is needed for each defined OVITO object class.
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(ExamplePlugin, RandomColorModifier, ParticleModifier);

/******************************************************************************
* Modifies the particles.
******************************************************************************/
PipelineStatus RandomColorModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	return PipelineStatus::Success;
}
