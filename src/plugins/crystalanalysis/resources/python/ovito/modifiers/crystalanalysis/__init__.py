# Load dependencies
import ovito.modifiers
import ovito.modifiers.particles

# Load the native code modules.
import CrystalAnalysis

# Inject modifier classes into parent module.
ovito.modifiers.ConstructSurfaceModifier = CrystalAnalysis.ConstructSurfaceModifier
ovito.modifiers.ShiftModifier = CrystalAnalysis.ShiftModifier
ovito.modifiers.SmoothDislocationsModifier = CrystalAnalysis.SmoothDislocationsModifier
ovito.modifiers.SmoothSurfaceModifier = CrystalAnalysis.SmoothSurfaceModifier
