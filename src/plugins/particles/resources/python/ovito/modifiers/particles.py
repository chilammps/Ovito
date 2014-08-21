# Load dependencies first.
import ovito

# Load the native code modules
import Particles
import ParticlesModify

# Inject modifier classes into parent module.
ovito.modifiers.ColorCodingModifier = ParticlesModify.ColorCodingModifier
ovito.modifiers.AssignColorModifier = ParticlesModify.AssignColorModifier