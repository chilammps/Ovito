#########################################################################################
# This script tests and demonstrates the use of the 
# Atomic Strain modifier from Python.
#########################################################################################

from ovito import *
from ovito.particles import *

# Load a simulation file. This is a binary dump file without metdata. That's why we have to
# explicitly specify the meaning of the five data fields in the file.
node = load("../files/shear.void.dump.bin", { 
	'columnMapping': ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"],
	'multiTimestepFile': True
})

# Check if the animation sequence has been loaded.
assert dataset.animationSettings.lastFrame == 100

# Jump to a frame in the simulation sequence. 
# This frame will serve as the deformed configuration for the strain calculation.
dataset.animationSettings.currentFrame = 40

# Create and apply atomic strain modifier.
mod = AtomicStrainModifier({ 'cutoff' : 3.1 })
node.applyModifier(mod)

# Now load reference configuration containing the initial atomic positions.
# We'll use frame 0 of the same simulation sequence as reference.

mod.referenceConfiguration.load("../files/shear.void.dump.bin", { 
	'columnMapping': ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"] 
})

# Verify that call to load() was successful.
assert mod.referenceConfiguration.sourceUrl == "../files/shear.void.dump.bin"

# Wait until loading of the current and reference configurations is completed and the modifier
# has computed the atomic strain values. This only is necessary because we want the new color coding
# modifier to initialize its range to the min/max strain values.
wait()

# Color atoms according to the local shear strain.
node.applyModifier(ColorCodingModifier({ 'sourceProperty' : "Shear Strain" }))
