#ifndef _RANDOM_COLOR_MODIFIER_H		// Prevent this header from being included
#define _RANDOM_COLOR_MODIFIER_H		// more than once in the same compilation unit.

// This header defines the base class for modifiers that act on particles.
#include <plugins/particles/modifier/ParticleModifier.h>

using namespace Ovito;
using namespace Ovito::Particles;

/// This modifier assigns a random color to every particle.
class RandomColorModifier : public ParticleModifier
{
public:

	/// Constructor.
	/// Must be marked as Q_INVOKABLE so that the system can instantiate the class at runtime.
	/// The modifier will become part of the given DataSet.
	Q_INVOKABLE RandomColorModifier(DataSet* dataset) : ParticleModifier(dataset) {
		// Nothing to do here for this modifier...
	}

protected:

	/// Modifies the particles.
	/// This method is called by the system every time the modification pipeline is evaluated.
	virtual PipelineStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

private:

	// This is required for every OVITO object class:
	Q_OBJECT
	OVITO_OBJECT

	// Give this modifier class a a user interface name.
	Q_CLASSINFO("DisplayName", "Random particle colors");
	// Insert the modifier into a category in the modifier list.
	Q_CLASSINFO("ModifierCategory", "Coloring");
};

#endif // _RANDOM_COLOR_MODIFIER_H
