# Load dependencies
import ovito.vis

# Load the native code module
import Particles

# Inject selected classes into parent module.
ovito.vis.SimulationCellDisplay = Particles.SimulationCellDisplay
ovito.vis.ParticleDisplay = Particles.ParticleDisplay
ovito.vis.VectorDisplay = Particles.VectorDisplay
ovito.vis.BondsDisplay = Particles.BondsDisplay
ovito.vis.SurfaceMeshDisplay = Particles.SurfaceMeshDisplay

# Inject enum types.
ovito.vis.ParticleDisplay.Shape = ovito.vis.ParticleShape
ovito.vis.ParticleDisplay.Shading = ovito.vis.ParticleShadingMode
ovito.vis.VectorDisplay.Shading = ovito.vis.ArrowShadingMode