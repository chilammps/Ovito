# Load dependencies
import ovito.io.particles

# Load the native code module
import NetCDFPlugin

# Inject selected classes into parent module.
ovito.io.particles.NetCDFImporter = NetCDFPlugin.NetCDFImporter
