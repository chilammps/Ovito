# Load dependencies
import ovito.io.particles

# Load the native code module
import NetCDFImporter

# Inject selected classes into parent module.
ovito.io.particles.NetCDFImporter = NetCDFImporter.NetCDFImporter
