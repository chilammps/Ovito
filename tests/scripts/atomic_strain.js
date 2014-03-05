
// Load test data. This is a binary dump file, so we have to explicitly specify the meaning of the 
// five data columns.
node = load("../files/shear.void.dump.bin", { 
	columnMapping: ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"],
	isMultiTimestepFile: true
})

// Make sure the animation has been loaded.
assert(ovito.animationSettings.lastFrame == 100)

// Jump to a later frame of the simulation sequence. 
// This frame will serve as the deformed configuration for the strain calculation.
ovito.animationSettings.currentFrame = 40

// Create and apply atomic strain modifier.
node.applyModifier(mod = new AtomicStrainModifier({ cutoff : 3.1 }))

// Now load reference configuration containing the initial atomic positions.
// We'll use frame 0 of the same simulation sequence as reference.

// In principle, we could simply set the 'sourceUrl' property of the modifier's reference configuration to the
// file containing the initial atomic positions, i.e.

//mod.referenceConfiguration.sourceUrl = "../files/shear.void.dump.bin"

// The above statement, however, gives an error, because loading a binary dump file requires additional parameters 
// that need to be passed to the importer in addition to the file name (see beginning of script).
// This is why we have to use the load() method of the reference configuration object here to load the file:

mod.referenceConfiguration.load("../files/shear.void.dump.bin", { 
	columnMapping: ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"] 
})

// Verify that call to load() was successful.
assert(mod.referenceConfiguration.sourceUrl == "file:../files/shear.void.dump.bin")

// Wait until loading of the current and reference configurations is completed and the modifier
// has computed the atomic strain values. This only is necessary because we want the new color coding
// modifier to initialize its range to the min/max strain values.
wait()

// Color atoms according to the local shear strain.
node.applyModifier(new ColorCodingModifier({ sourceProperty : "Shear Strain" }))
